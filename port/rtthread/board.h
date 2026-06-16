/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * Board header for STM32F407 RT-Thread port
 */

#ifndef __BOARD_H__
#define __BOARD_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* MCU core clock frequency */
#define STM32_SYSCLK_FREQ          168000000U

/* SRAM size — STM32F407ZGTx has 192 KB */
#define STM32_SRAM_SIZE            (192 * 1024)
#define STM32_SRAM_END             (0x20000000 + STM32_SRAM_SIZE)

/* RT-Thread heap region (top 20KB of SRAM) */
#define HEAP_BEGIN                 ((void *)(0x20000000 + 128 * 1024))
#define HEAP_END                   ((void *)STM32_SRAM_END)

void rt_hw_board_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __BOARD_H__ */
