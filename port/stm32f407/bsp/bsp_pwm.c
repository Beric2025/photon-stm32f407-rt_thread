/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * TIM3 PWM initialization (4-channel)
 * CH1=PB4, CH2=PB5, CH3=PB0, CH4=PB1 (AF2)
 */

#include "bsp_pwm.h"

/* APB1 timer clock = 84 MHz (HCLK=168, APB1=/4 → 42 MHz, timer x2 → 84 MHz) */
#define TIM3_BASE_CLK_HZ  84000000UL

TIM_HandleTypeDef g_tim3;

void bsp_pwm3_init(uint32_t freq_hz)
{
    uint32_t psc = 0;
    uint32_t arr;

    /* Compute prescaler so that ARR fits within 16-bit range */
    while (1) {
        arr = (TIM3_BASE_CLK_HZ / (psc + 1)) / freq_hz;
        if (arr <= 0xFFFF)
            break;
        psc++;
    }

    /* Enable TIM3 clock and GPIO clock for PWM outputs */
    __HAL_RCC_TIM3_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /**TIM3 GPIO Configuration
    PB4     ------> TIM3_CH1 (AF2)
    PB5     ------> TIM3_CH2 (AF2)
    PB0     ------> TIM3_CH3 (AF2)
    PB1     ------> TIM3_CH4 (AF2)
    */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin       = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* TIM3 base configuration */
    g_tim3.Instance               = TIM3;
    g_tim3.Init.Prescaler         = psc;
    g_tim3.Init.CounterMode       = TIM_COUNTERMODE_UP;
    g_tim3.Init.Period            = arr - 1;
    g_tim3.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    g_tim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

    if (HAL_TIM_PWM_Init(&g_tim3) != HAL_OK) {
        Error_Handler();
    }

    /* Configure 4 PWM channels, initial duty = 0% */
    TIM_OC_InitTypeDef sConfigOC = {0};
    sConfigOC.OCMode     = TIM_OCMODE_PWM1;
    sConfigOC.Pulse      = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

    if (HAL_TIM_PWM_ConfigChannel(&g_tim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
        Error_Handler();
    if (HAL_TIM_PWM_ConfigChannel(&g_tim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
        Error_Handler();
    if (HAL_TIM_PWM_ConfigChannel(&g_tim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
        Error_Handler();
    if (HAL_TIM_PWM_ConfigChannel(&g_tim3, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
        Error_Handler();
}


TIM_HandleTypeDef g_tim5;

void bsp_tim5_init(uint32_t freq_hz)
{
    /* TIM5 is on APB1, timer clock = 84 MHz.
     * freq_hz: desired free-running counter frequency in Hz (e.g. 1000000 for 1 µs). */
    uint32_t psc = (TIM3_BASE_CLK_HZ / freq_hz) - 1;

    __HAL_RCC_TIM5_CLK_ENABLE();

    g_tim5.Instance               = TIM5;
    g_tim5.Init.Prescaler         = psc;
    g_tim5.Init.CounterMode       = TIM_COUNTERMODE_UP;
    g_tim5.Init.Period            = 0xFFFFFFFF;   /* 32-bit max range */
    g_tim5.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    g_tim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_Base_Init(&g_tim5) != HAL_OK) {
        Error_Handler();
    }
}
