/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef __BSP_DAC_H__
#define __BSP_DAC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

extern DAC_HandleTypeDef g_dac1;

/**
 * @brief:  initialize DAC1 with 2 channels (PA4=OUT1, PA5=OUT2)
 */
void bsp_dac1_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_DAC_H__ */
