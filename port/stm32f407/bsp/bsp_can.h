/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef _BSP_CAN_H_
#define _BSP_CAN_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "main.h"
#include <stdint.h>

#define CAN1_RX0_INT_ENABLE 1

extern CAN_HandleTypeDef       g_can1;
extern CAN_TxHeaderTypeDef     g_can1_tx;
extern CAN_RxHeaderTypeDef     g_can1_rx;

/**
 * @brief:  initialize CAN1 peripheral
 * @presc:  prescaler value, range 1~512
 * @tsjw:   sync jump width, range 1~128
 * @ntsg1:  time segment 1, range 2~256
 * @ntsg2:  time segment 2, range 2~128
 * @mode:   operating mode -- CAN_MODE_NORMAL or CAN_MODE_EXTERNAL_LOOPBACK
 *
 * PLL1Q (200 MHz) is the CAN clock source.
 *
 * Return: 0 on success, non-zero on failure
 */
unsigned char bsp_can1_init(unsigned short presc, unsigned char tsjw,
    unsigned short ntsg1, unsigned char ntsg2, uint32_t mode);

#ifdef __cplusplus
}
#endif

#endif /* _BSP_CAN_H_ */
