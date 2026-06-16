/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * ETH device abstraction layer — pure interface.
 * No dependency on any BSP, PHY driver, or MCU-specific header.
 */

#ifndef _ETH_DEV_H_
#define _ETH_DEV_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>

/* ====================================================================
 * Callback type definitions
 * ==================================================================== */

/** Called by the ETH stack when a frame is received (task context, not ISR). */
typedef void (*eth_rx_callback_t)(void *user_data, uint8_t *data, uint16_t length);

/** Called when the physical link status changes. */
typedef void (*eth_link_callback_t)(void *user_data, int link_up, int speed, int duplex);

/* ====================================================================
 * Eth_Device_T — virtual table for an Ethernet controller + PHY pair.
 * The BSP layer creates one instance per controller and registers it
 * via eth_dev_register().
 * ==================================================================== */

typedef struct {
    /** Human-readable identifier, e.g. "eth1". Looked up by get_eth_device(). */
    char *name;

    /** Initialise MAC + PHY.  mac_addr is 6 bytes.  Return 0 on success. */
    int (*init)(void *privatedata, uint8_t *mac_addr);

    /** De-initialise and release hardware resources. */
    int (*deinit)(void *privatedata);

    /** Transmit an Ethernet frame.  Return 0 on success. */
    int (*send)(void *privatedata, uint8_t *data, uint16_t length);

    /** Device-specific control (ioctl).  cmd / arg are implementation-defined. */
    int (*ioctl)(void *privatedata, uint32_t cmd, void *arg);

    /**
     * Block waiting for RX frames.
     * For each received frame the registered rx_callback is called.
     * @param timeout_ms  max wait time in ms (0 = infinite in many RTOS).
     * @return  number of frames processed, -1 on error.
     */
    int (*wait_rx)(void *privatedata, uint32_t timeout_ms);

    /** Register the per-frame RX callback. */
    int (*register_rx_callback)(void *privatedata, eth_rx_callback_t cb, void *user_data);

    /** Register the link-state-change callback. */
    int (*register_link_callback)(void *privatedata, eth_link_callback_t cb, void *user_data);

    /** Start MAC DMA (typically called after link-up). */
    int (*start)(void *privatedata);

    /** Stop MAC DMA (typically called after link-down). */
    int (*stop)(void *privatedata);

    /** Read PHY link status.  1 = up, 0 = down, -1 = error. */
    int (*get_link_status)(void *privatedata);

    /** Opaque pointer to BSP private data (type is BSP-defined). */
    void *private_data;
} Eth_Device_T;

/* ====================================================================
 * Device registry
 * ==================================================================== */

/**
 * Register an ETH device.  Call once during startup from BSP code.
 * After registration, get_eth_device(dev->name) returns dev.
 */
void eth_dev_register(Eth_Device_T *dev);

/**
 * Look up a registered device by name (e.g. "eth1").
 * @return  pointer to Eth_Device_T, or NULL if not found.
 */
Eth_Device_T *get_eth_device(char *name);

/* ====================================================================
 * RX buffer pool — ISR-safe alloc / link / commit.
 * BSP HAL callbacks call these to feed DMA buffers into the shared pool.
 * ==================================================================== */

/**
 * Initialise the RX buffer pool and frame queue.  Call once at startup
 * from the BSP layer before any DMA activity begins.
 */
void eth_dev_rx_pool_init(void);

/**
 * Allocate a buffer from the shared RX pool.  ISR-safe.
 * Matches the HAL pETH_rxAllocateCallbackTypeDef signature.
 * @param  buff  [out] pointer to ETH_RX_BUFFER_SIZE-byte buffer, or NULL.
 */
void eth_dev_rx_buf_alloc(uint8_t **buff);

/**
 * Whether the last eth_dev_rx_buf_alloc() call succeeded.
 * @return  0 = success, non-zero = alloc failed.
 */
int eth_dev_rx_buf_alloc_status(void);

/**
 * Link a filled RX buffer into the current frame chain.  ISR-safe.
 * Follows the same contract as HAL_ETH_RxLinkCallback().
 */
void eth_dev_rx_buf_link(void **pStart, void **pEnd, uint8_t *buff, uint16_t length);

/**
 * Commit the current frame to the completed queue.  ISR-safe.
 * Called from HAL_ETH_RxCpltCallback after all segments are linked.
 * @return  1 if a frame was committed, 0 if nothing to commit.
 */
int eth_dev_rx_done(void);

/**
 * Dequeue a single completed frame from the queue.  Task context only.
 * The returned pointer is an opaque RxSeg chain; use the iteration API
 * below to walk segments, and eth_dev_rx_recycle() to return buffers.
 *
 * @return  opaque frame pointer, or NULL if queue is empty.
 */
void *eth_dev_rx_dequeue(void);

/**
 * Return all buffers of a dequeued frame back to the pool.
 * @param  frame  opaque pointer previously returned by eth_dev_rx_dequeue().
 */
void eth_dev_rx_recycle(void *frame);

/* ---- Helpers to walk an opaque frame chain ---- */

/** Get the data pointer of an RxSeg. */
uint8_t  *eth_dev_rx_seg_data(void *seg);
/** Get the data length of an RxSeg. */
uint16_t  eth_dev_rx_seg_length(void *seg);
/** Get the next segment in the chain, or NULL. */
void     *eth_dev_rx_seg_next(void *seg);

#ifdef __cplusplus
}
#endif

#endif /* _ETH_DEV_H_ */
