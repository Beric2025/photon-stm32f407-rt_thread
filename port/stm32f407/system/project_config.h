/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * Centralized project configuration — all tunable switches in one place.
 * Include this header to use any of the macros below.
 */

#ifndef __PROJECT_CONFIG_H__
#define __PROJECT_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * 1. Debug & Logging
 * ============================================================ */

#define LOG_PRINT_ENABLE  1
#define LOG_PRINT_LEVEL   0   /* 0=DEBUG, 1=INFO, 2=WARN, 3=ERROR */

/* ============================================================
 * 2. RTOS
 * ============================================================
 *
 * When USE_OS is 1, UART/CAN/ADC drivers use RT-Thread critical
 * sections for thread-safe buffer access. Set to 0 for bare-metal.
 */

#define USE_OS  1


#ifdef __cplusplus
}
#endif

#endif /* __PROJECT_CONFIG_H__ */
