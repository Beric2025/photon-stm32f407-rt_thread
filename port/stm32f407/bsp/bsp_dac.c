/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * STM32 DAC1 initialization (2-channel, 12-bit)
 * CH1 = PA4, CH2 = PA5
 */

#include "bsp_dac.h"

DAC_HandleTypeDef g_dac1;

void bsp_dac1_init(void)
{
    g_dac1.Instance = DAC1;

    if (HAL_DAC_Init(&g_dac1) != HAL_OK) {
        Error_Handler();
    }
}
