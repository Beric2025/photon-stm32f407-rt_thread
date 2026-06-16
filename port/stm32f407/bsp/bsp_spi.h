/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef _BSP_SPI_H_
#define _BSP_SPI_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "main.h"

extern SPI_HandleTypeDef g_spi1;


void bsp_spi1_init(void);


#ifdef __cplusplus
}
#endif

#endif /* _BSP_SPI_H_ */
