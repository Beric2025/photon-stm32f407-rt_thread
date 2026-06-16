/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef __BSP_RTC_H__
#define __BSP_RTC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

extern RTC_HandleTypeDef g_rtc;

/**
 * @brief:  initialize RTC with LSE clock (32.768 kHz)
 */
void bsp_rtc_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_RTC_H__ */
