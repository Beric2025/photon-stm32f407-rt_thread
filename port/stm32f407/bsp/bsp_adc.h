/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef __BSP_ADC_H__
#define __BSP_ADC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"


extern ADC_HandleTypeDef g_adc1;

/**
 * @brief:  initialize ADC1 with DMA (channels 10, 11, 17)
 */
void bsp_adc1_init(void);


#ifdef __cplusplus
}
#endif

#endif /* __BSP_ADC_H__ */
