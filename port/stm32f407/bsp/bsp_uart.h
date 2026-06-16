/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef _BSP_UART_H_
#define _BSP_UART_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "main.h"
#include <stdint.h>

extern UART_HandleTypeDef g_uart1;
extern UART_HandleTypeDef g_uart2;
extern UART_HandleTypeDef g_uart4;

/**
 * @brief:  initialize UART1 peripheral with specified baud rate
 * @baudrate:   baud rate
 *
 * Return: NULL if not found
 */
void bsp_uart1_init(uint32_t baudrate); 

/**
 * @brief:  initialize UART2 peripheral with specified baud rate
 * @baudrate:   baud rate
 *
 * Return: NULL if not found
 */
void bsp_uart2_init(uint32_t baudrate); 

/**
 * @brief:  initialize UART4 peripheral with specified baud rate
 * @baudrate:   baud rate
 *
 * Return: NULL if not found
 */
void bsp_uart4_init(uint32_t baudrate); 


#ifdef __cplusplus
}
#endif

#endif /* _BSP_UART_H_ */
