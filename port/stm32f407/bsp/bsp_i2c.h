/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef _BSP_I2C_H_
#define _BSP_I2C_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "main.h"
#include <stdint.h>

extern I2C_HandleTypeDef g_i2c2;


void bsp_i2c2_init(uint32_t baudrate);


#ifdef __cplusplus
}
#endif

#endif /* _BSP_I2C_H_ */
