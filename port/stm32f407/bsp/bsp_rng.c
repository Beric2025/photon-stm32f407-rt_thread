/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * STM32 RNG initialization (48 MHz clock from HSI48)
 */

#include "bsp_rng.h"

RNG_HandleTypeDef g_rng;

void bsp_rng_init(void)
{
    g_rng.Instance = RNG;

    if (HAL_RNG_Init(&g_rng) != HAL_OK) {
        Error_Handler();
    }
}
