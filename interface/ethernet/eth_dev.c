/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * ETH device abstraction layer — buffer pool & frame queue.
 *
 * This file is hardware-independent: it does NOT include any BSP header,
 * PHY driver, or MCU-specific header.  All hardware operations are
 * performed by the BSP layer (bsp_eth.c) through HAL callbacks.
 */

#include <string.h>
#include "eth_dev.h"

/* ---- Compiler-neutral container_of (avoids stddef.h / ARMCC offsetof quirks) ---- */
#define CONTAINER_OF(ptr, type, member) \
    ((type *)((char *)(ptr) - (char *)(&((type *)0)->member)))

/* ---- Compiler-neutral interrupt gate (zero-header, works with GCC/ARMCC/IAR) ---- */
__attribute__((always_inline))
static inline uint32_t eth_dev_get_primask(void)
{
    uint32_t r;
#if defined(__CC_ARM)
    __asm { mrs r, primask }
#elif defined(__ICCARM__)
    r = __get_PRIMASK();           /* IAR built-in, no header required */
#else
    __asm volatile("mrs %0, primask" : "=r"(r));  /* GCC / ARMClang */
#endif
    return r;
}

#if defined(__CC_ARM)
#define ETH_DEV_LOCK()     __asm { cpsid i }
#define ETH_DEV_UNLOCK()   __asm { cpsie i }
#elif defined(__ICCARM__)
#define ETH_DEV_LOCK()     __disable_interrupt()   /* IAR built-in */
#define ETH_DEV_UNLOCK()   __enable_interrupt()    /* IAR built-in */
#else
#define ETH_DEV_LOCK()     __asm volatile("cpsid i" : : : "memory")
#define ETH_DEV_UNLOCK()   __asm volatile("cpsie i" : : : "memory")
#endif

/* ---- Configuration (must match BSP DMA configuration) ---- */

#ifndef ETH_RX_DESC_CNT
#define ETH_RX_DESC_CNT         4U
#endif

#define ETH_RX_BUFFER_SIZE      (1536UL)
#define ETH_RX_BUFFER_NUM       (2U * ETH_RX_DESC_CNT)

/* ---- RX segment descriptor ---- */

typedef struct RxSeg {
    uint8_t       *data;
    uint16_t       length;
    struct RxSeg  *next;
} RxSeg;

/* ---- RX buffer: descriptor + data ---- */

typedef struct {
    RxSeg    seg;
    uint8_t  buff[ETH_RX_BUFFER_SIZE];
} RxBuffer;

/* ---- Module state ---- */

/* Buffer pool — placed in a dedicated MPU region for non-cacheable DMA */
__attribute__((section(".eth_rx_buf")))
static RxBuffer   rx_buff_pool[ETH_RX_BUFFER_NUM];

static uint32_t  rx_buff_free_mask;       /* bitmask: 1 = free */

/* Frame assembly (ISR context) */
static RxSeg *g_current_frame = NULL;

/* Completed frame queue (ISR writes, task reads) */
static RxSeg *g_rx_completed_head = NULL;
static RxSeg *g_rx_completed_tail = NULL;

/* ---- Device registry ---- */

#define ETH_DEV_MAX  4

static Eth_Device_T *s_dev_registry[ETH_DEV_MAX];
static uint32_t  s_dev_count = 0;

/* ====================================================================
 * Public API — Device registry
 * ==================================================================== */

void eth_dev_register(Eth_Device_T *dev)
{
    if (dev && s_dev_count < ETH_DEV_MAX) {
        s_dev_registry[s_dev_count++] = dev;
    }
}

Eth_Device_T *get_eth_device(char *name)
{
    uint32_t i;
    for (i = 0; i < s_dev_count; i++) {
        if (s_dev_registry[i] && 0 == strcmp(s_dev_registry[i]->name, name)) {
            return s_dev_registry[i];
        }
    }
    return NULL;
}

/* ====================================================================
 * Public API — Buffer pool (ISR-safe)
 * ==================================================================== */

static int s_alloc_status = 0;   /* 0 = last alloc OK, non-zero = exhausted */

void eth_dev_rx_buf_alloc(uint8_t **buff)
{
    uint32_t primask;
    uint32_t idx;

    primask = eth_dev_get_primask();
    ETH_DEV_LOCK();

    for (idx = 0; idx < ETH_RX_BUFFER_NUM; idx++) {
        if (rx_buff_free_mask & (1U << idx)) {
            rx_buff_free_mask &= ~(1U << idx);
            s_alloc_status = 0;
            if (!primask) { ETH_DEV_UNLOCK(); }
            *buff = rx_buff_pool[idx].buff;
            return;
        }
    }

    s_alloc_status = 1;
    *buff = NULL;
    if (!primask) { ETH_DEV_UNLOCK(); }
}

int eth_dev_rx_buf_alloc_status(void)
{
    return s_alloc_status;
}

void eth_dev_rx_buf_link(void **pStart, void **pEnd, uint8_t *buff, uint16_t length)
{
    RxBuffer *rx_buf = CONTAINER_OF(buff, RxBuffer, buff);
    RxSeg    *seg    = &rx_buf->seg;

    seg->data   = buff;
    seg->length = length;
    seg->next   = NULL;

    /* Chain using HAL's pStart / pEnd mechanism */
    if (*pStart == NULL) {
        *pStart         = seg;
        g_current_frame = seg;
    } else {
        ((RxSeg *)(*pEnd))->next = seg;
    }
    *pEnd = seg;
}

/* ====================================================================
 * Public API — Frame queue (rx_done ISR-safe, dequeue task-safe)
 * ==================================================================== */

int eth_dev_rx_done(void)
{
    if (g_current_frame == NULL) {
        return 0;
    }

    /* Append current frame to completed queue */
    if (g_rx_completed_tail) {
        g_rx_completed_tail->next = g_current_frame;
    } else {
        g_rx_completed_head = g_current_frame;
    }

    /* Advance tail to the end of the chain */
    RxSeg *s = g_current_frame;
    while (s->next) {
        s = s->next;
    }
    g_rx_completed_tail = s;
    g_current_frame     = NULL;

    return 1;
}

void *eth_dev_rx_dequeue(void)
{
    RxSeg *frame;

    ETH_DEV_LOCK();
    frame = g_rx_completed_head;
    if (frame) {
        g_rx_completed_head = frame->next;
        if (g_rx_completed_head == NULL) {
            g_rx_completed_tail = NULL;
        }
        frame->next = NULL;
    }
    ETH_DEV_UNLOCK();

    return frame;
}

void eth_dev_rx_recycle(void *frame)
{
    RxSeg *seg = (RxSeg *)frame;
    while (seg) {
        RxBuffer *rx_buf = CONTAINER_OF(seg, RxBuffer, seg);
        int idx = rx_buf - rx_buff_pool;

        RxSeg *next = seg->next;

        ETH_DEV_LOCK();
        rx_buff_free_mask |= (1U << idx);
        ETH_DEV_UNLOCK();

        seg = next;
    }
}

/* ---- Frame chain accessors ---- */

uint8_t *eth_dev_rx_seg_data(void *seg)
{
    return seg ? ((RxSeg *)seg)->data : NULL;
}

uint16_t eth_dev_rx_seg_length(void *seg)
{
    return seg ? ((RxSeg *)seg)->length : 0;
}

void *eth_dev_rx_seg_next(void *seg)
{
    return seg ? ((RxSeg *)seg)->next : NULL;
}

/* ====================================================================
 * Initialisation — called once by BSP before any DMA activity
 * ==================================================================== */

void eth_dev_rx_pool_init(void)
{
    rx_buff_free_mask    = (1U << ETH_RX_BUFFER_NUM) - 1;
    g_current_frame      = NULL;
    g_rx_completed_head  = NULL;
    g_rx_completed_tail  = NULL;
    s_alloc_status       = 0;
}
