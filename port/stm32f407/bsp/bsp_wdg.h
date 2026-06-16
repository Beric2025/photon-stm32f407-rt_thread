/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef __BSP_WDG_H__
#define __BSP_WDG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

extern IWDG_HandleTypeDef g_iwdg;

/**
 * @brief:  initialize independent watchdog (IWDG) with LSI clock (~32 kHz)
 * @timeout_ms:  watchdog timeout in milliseconds
 */
void bsp_iwdg_init(uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_WDG_H__ */
