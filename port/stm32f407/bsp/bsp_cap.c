/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * TIM4 input capture initialization (2-channel)
 * CH1=PD12, CH2=PD13 (AF2)
 */

#include "bsp_cap.h"
#include "cap_dev.h"

/* APB1 timer clock = 84 MHz (HCLK=168, APB1=/4 → 42 MHz, timer x2 → 84 MHz) */
#define CAP_TIMER_CLK_HZ     84000000UL
#define CAP_TIMER_PRESCALER  83U       /* 84MHz / (83+1) = 1MHz (1 µs resolution) */
#define CAP_TIMER_PERIOD     0xFFFFU   /* max 16-bit range */

TIM_HandleTypeDef g_tim4;

void bsp_cap4_init(void)
{
    /* Enable TIM4 clock and GPIO clock for capture inputs */
    __HAL_RCC_TIM4_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    /**TIM4 GPIO Configuration
    PD12     ------> TIM4_CH1 (AF2)
    PD13     ------> TIM4_CH2 (AF2)
    */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin       = GPIO_PIN_12 | GPIO_PIN_13;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* Configure TIM4 as a 1 MHz free-running counter with input capture */
    g_tim4.Instance               = TIM4;
    g_tim4.Init.Prescaler         = CAP_TIMER_PRESCALER;
    g_tim4.Init.CounterMode       = TIM_COUNTERMODE_UP;
    g_tim4.Init.Period            = CAP_TIMER_PERIOD;
    g_tim4.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    g_tim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_IC_Init(&g_tim4) != HAL_OK) {
        Error_Handler();
    }

    /* Channel 1: PD12 (AF2), rising edge */
    TIM_IC_InitTypeDef sICConfig = {0};
    sICConfig.ICPolarity  = TIM_ICPOLARITY_RISING;
    sICConfig.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sICConfig.ICPrescaler = TIM_ICPSC_DIV1;
    sICConfig.ICFilter    = 0;
    if (HAL_TIM_IC_ConfigChannel(&g_tim4, &sICConfig, TIM_CHANNEL_1) != HAL_OK) {
        Error_Handler();
    }

    /* Channel 2: PD13 (AF2), rising edge */
    sICConfig.ICPolarity  = TIM_ICPOLARITY_RISING;
    sICConfig.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sICConfig.ICPrescaler = TIM_ICPSC_DIV1;
    sICConfig.ICFilter    = 0;
    if (HAL_TIM_IC_ConfigChannel(&g_tim4, &sICConfig, TIM_CHANNEL_2) != HAL_OK) {
        Error_Handler();
    }
}

/* ====================================================================
 * HAL callback — translates to generic event for interface layer
 * ==================================================================== */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    cap_dev_on_event((void *)htim, htim->Channel);
}
