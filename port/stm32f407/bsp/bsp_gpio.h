/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef __BSP_GPIO_H__
#define __BSP_GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

void bsp_gpioa_pin0_init(void);
void bsp_gpiob_pin0_init(void);
void bsp_gpiob_pin1_init(void);
void bsp_gpiof_pin8_init(void);
void bsp_gpioh_pin15_init(void);
void bsp_gpioi_pin0_init(void);
void bsp_gpioi_pin1_init(void);

#ifdef __cplusplus
}
#endif

#endif /*__ __BSP_GPIO_H__ */
