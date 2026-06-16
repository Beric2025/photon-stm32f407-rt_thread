/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef __PCF8574_H
#define __PCF8574_H

#ifdef __cplusplus
 extern "C" {
#endif   
#include "main.h"

/* Pin definitions */

#define PCF8574_GPIO_PORT                  GPIOB
#define PCF8574_GPIO_PIN                   GPIO_PIN_12
#define PCF8574_GPIO_CLK_ENABLE()          do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)

#define PCF8574_INT  HAL_GPIO_ReadPin(PCF8574_GPIO_PORT,PCF8574_GPIO_PIN)

#define PCF8574_ADDR  0X40      /* PCF8574 address (left-shifted by 1) */

/* PCF8574 IO functions */
#define BEEP_IO         0       /* Buzzer control pin        P0 */
#define AP_INT_IO       1       /* AP3216C interrupt pin     P1 */
#define DCMI_PWDN_IO    2       /* DCMI power control pin    P2 */
#define USB_PWR_IO      3       /* USB power control pin     P3 */
#define EX_IO           4       /* Extension IO, custom      P4 */
#define MPU_INT_IO      5       /* SH3001 interrupt pin      P5 */
#define RS485_RE_IO     6       /* RS485_RE pin              P6 */
#define ETH_RESET_IO    7       /* Ethernet reset pin        P7 */

/* function declarations */

/**
 * @brief:  initialize PCF8574 I2C IO expander, default all IOs to high
 * Return:  0 on success, 1 on failure
 */
unsigned char pcf8574_init(void);

/**
 * @brief:  read 8-bit IO value from PCF8574
 * Return:  read data
 */
unsigned char pcf8574_read_byte(void);

/**
 * @brief:  write 8-bit IO value to PCF8574
 * @data:  data to write
 */
void pcf8574_write_byte(unsigned char data);

/**
 * @brief:  set PCF8574 IO pin level
 * @bit:  IO number (0-7)
 * @sta:  IO state (0 or 1)
 */
void pcf8574_write_bit(unsigned char bit, unsigned char sta);

/**
 * @brief:  read PCF8574 IO pin level
 * @bit:  IO number (0-7)
 * Return:  pin value, 0 or 1
 */
unsigned char pcf8574_read_bit(unsigned char bit);

#ifdef __cplusplus
}
#endif

#endif

