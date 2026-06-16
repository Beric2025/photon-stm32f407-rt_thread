/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * ADC1 initialization with DMA (channels 10, 11, 17)
 */

#include "bsp_adc.h"
#include "adc_dev.h"


ADC_HandleTypeDef g_adc1;
DMA_HandleTypeDef g_hdma_adc1;

void bsp_adc1_init(void)
{
    /* -------------------- ADC1 handle configuration -------------------- */
    g_adc1.Instance                   = ADC1;
    g_adc1.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV4;   /* 84/4 = 21 MHz (≤36 MHz max) */
    g_adc1.Init.Resolution            = ADC_RESOLUTION_12B;
    g_adc1.Init.ScanConvMode          = ENABLE;                      /* multi-channel scan */
    g_adc1.Init.ContinuousConvMode    = ENABLE;                      /* continuous for DMA circular */
    g_adc1.Init.DiscontinuousConvMode = DISABLE;
    g_adc1.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
    g_adc1.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
    g_adc1.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    g_adc1.Init.NbrOfConversion       = 3;                           /* ch10, ch11, ch17 */
    g_adc1.Init.DMAContinuousRequests = ENABLE;
    g_adc1.Init.EOCSelection          = ADC_EOC_SEQ_CONV;

    if (HAL_ADC_Init(&g_adc1) != HAL_OK) {
        Error_Handler();
    }

    /* -------------------- Channel configuration -------------------- */
    ADC_ChannelConfTypeDef sConfig = {0};

    /* Rank 1: Channel 10 — PC0 (ADC123_IN10), external analog input */
    sConfig.Channel      = ADC_CHANNEL_10;
    sConfig.Rank         = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_56CYCLES;
    if (HAL_ADC_ConfigChannel(&g_adc1, &sConfig) != HAL_OK) {
        Error_Handler();
    }

    /* Rank 2: Channel 11 — PC1 (ADC123_IN11), external analog input */
    sConfig.Channel      = ADC_CHANNEL_11;
    sConfig.Rank         = 2;
    sConfig.SamplingTime = ADC_SAMPLETIME_56CYCLES;
    if (HAL_ADC_ConfigChannel(&g_adc1, &sConfig) != HAL_OK) {
        Error_Handler();
    }

    /* Rank 3: Channel 17 — VREFINT (internal reference), needs ≥4 µs sampling */
    sConfig.Channel      = ADC_CHANNEL_17;
    sConfig.Rank         = 3;
    sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
    if (HAL_ADC_ConfigChannel(&g_adc1, &sConfig) != HAL_OK) {
        Error_Handler();
    }
}

void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (hadc->Instance == ADC1) {
        /* Enable peripheral clocks */
        __HAL_RCC_ADC1_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_DMA2_CLK_ENABLE();

        /**ADC1 GPIO Configuration
        PC0     ------> ADC123_IN10
        PC1     ------> ADC123_IN11
        (Channel 17 VREFINT is internal — no GPIO needed)
        */
        GPIO_InitStruct.Pin  = GPIO_PIN_0 | GPIO_PIN_1;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        /* ADC1 DMA Init — DMA2 Stream0, Channel 0 */
        g_hdma_adc1.Instance                 = DMA2_Stream0;
        g_hdma_adc1.Init.Channel             = DMA_CHANNEL_0;
        g_hdma_adc1.Init.Direction           = DMA_PERIPH_TO_MEMORY;
        g_hdma_adc1.Init.PeriphInc           = DMA_PINC_DISABLE;
        g_hdma_adc1.Init.MemInc              = DMA_MINC_ENABLE;
        g_hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
        g_hdma_adc1.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
        g_hdma_adc1.Init.Mode                = DMA_CIRCULAR;
        g_hdma_adc1.Init.Priority            = DMA_PRIORITY_HIGH;
        g_hdma_adc1.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;

        if (HAL_DMA_Init(&g_hdma_adc1) != HAL_OK) {
            Error_Handler();
        }

        __HAL_LINKDMA(hadc, DMA_Handle, g_hdma_adc1);

        /* ADC1 DMA interrupt */
        HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 6, 0);
        HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
    }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1) {
        /* Disable ADC1 clock */
        __HAL_RCC_ADC1_CLK_DISABLE();

        /* DeInit GPIO */
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_0 | GPIO_PIN_1);

        /* DeInit DMA */
        HAL_DMA_DeInit(hadc->DMA_Handle);

        /* Disable DMA interrupt */
        HAL_NVIC_DisableIRQ(DMA2_Stream0_IRQn);
    }
}

/* ====================================================================
 * DMA IRQ handler — must match the stream configured in MspInit
 * ==================================================================== */
void DMA2_Stream0_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&g_hdma_adc1);
}

/* ====================================================================
 * HAL callback — translates to generic event for interface layer
 * ==================================================================== */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    adc_dev_on_event((void *)hadc, ADC_EVENT_CONV_COMPLETE);
}
