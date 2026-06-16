/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef __BSP_I2S_H__
#define __BSP_I2S_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

extern I2S_HandleTypeDef g_i2s2;

/**
 * @brief:  initialize I2S2 as master transmitter (Philips standard)
 * @sample_rate:  sample rate in Hz (e.g. 44100, 48000)
 * @bits:         bits per sample (16, 24, 32)
 *
 * Pins: PB12=WS, PB13=CK, PB15=SD (AF5)
 */
void bsp_i2s2_init(uint32_t sample_rate, unsigned char bits);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_I2S_H__ */
