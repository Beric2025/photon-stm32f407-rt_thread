/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef __BSP_RNG_H__
#define __BSP_RNG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

extern RNG_HandleTypeDef g_rng;

/**
 * @brief:  initialize RNG hardware (48 MHz clock from PLL/HSI48)
 */
void bsp_rng_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_RNG_H__ */
