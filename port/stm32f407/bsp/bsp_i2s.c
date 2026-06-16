/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * STM32 I2S2 initialization (master, Philips standard)
 * PB12=WS, PB13=CK, PB15=SD (AF5)
 */

#include "bsp_i2s.h"

/* APB1 peripheral clock = 100 MHz (PCLK1) */
#define I2S_BASE_CLK_HZ  100000000UL

I2S_HandleTypeDef g_i2s2;

void bsp_i2s2_init(uint32_t sample_rate, unsigned char bits)
{
    g_i2s2.Instance       = SPI2;
    g_i2s2.Init.Mode      = I2S_MODE_MASTER_TX;
    g_i2s2.Init.Standard  = I2S_STANDARD_PHILIPS;
    g_i2s2.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
    g_i2s2.Init.CPOL      = I2S_CPOL_LOW;
    g_i2s2.Init.ClockSource = I2S_CLOCK_PLL;

    /* Map bits-per-sample to HAL data format */
    if (bits == 32)
        g_i2s2.Init.DataFormat = I2S_DATAFORMAT_32B;
    else if (bits == 24)
        g_i2s2.Init.DataFormat = I2S_DATAFORMAT_24B;
    else
        g_i2s2.Init.DataFormat = I2S_DATAFORMAT_16B;

    /* Map sample rate to HAL preset (requires PLLI2S configured accordingly) */
    if (sample_rate >= 192000)
        g_i2s2.Init.AudioFreq = I2S_AUDIOFREQ_192K;
    else if (sample_rate >= 96000)
        g_i2s2.Init.AudioFreq = I2S_AUDIOFREQ_96K;
    else if (sample_rate >= 48000)
        g_i2s2.Init.AudioFreq = I2S_AUDIOFREQ_48K;
    else if (sample_rate >= 44100)
        g_i2s2.Init.AudioFreq = I2S_AUDIOFREQ_44K;
    else if (sample_rate >= 32000)
        g_i2s2.Init.AudioFreq = I2S_AUDIOFREQ_32K;
    else if (sample_rate >= 22050)
        g_i2s2.Init.AudioFreq = I2S_AUDIOFREQ_22K;
    else if (sample_rate >= 16000)
        g_i2s2.Init.AudioFreq = I2S_AUDIOFREQ_16K;
    else if (sample_rate >= 11025)
        g_i2s2.Init.AudioFreq = I2S_AUDIOFREQ_11K;
    else
        g_i2s2.Init.AudioFreq = I2S_AUDIOFREQ_8K;

    if (HAL_I2S_Init(&g_i2s2) != HAL_OK) {
        Error_Handler();
    }
}
