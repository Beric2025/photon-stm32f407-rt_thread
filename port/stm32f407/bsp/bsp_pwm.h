/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef __BSP_PWM_H__
#define __BSP_PWM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

extern TIM_HandleTypeDef g_tim3;
extern TIM_HandleTypeDef g_tim5;

/**
 * @brief:  initialize TIM3 as 4-channel PWM output
 * @freq_hz:  PWM frequency in Hz
 *
 * Channels: CH1=PB4, CH2=PB5, CH3=PB0, CH4=PB1 (AF2)
 */
void bsp_pwm3_init(uint32_t freq_hz);

/**
 * @brief:  initialize TIM5 as free-running counter (1 MHz, 32-bit)
 *          Used as the delay timer base — no output channels.
 */
void bsp_tim5_init(uint32_t freq_hz);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_PWM_H__ */
