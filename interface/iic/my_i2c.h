/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef __MY_I2C_H
#define __MY_I2C_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "main.h"
#include <stdint.h>



/* I2C GPIO pin definitions */
#define IIC_SCL_Pin                     GPIO_PIN_4
#define IIC_SCL_GPIO_Port               GPIOH
#define IIC_SDA_Pin                     GPIO_PIN_5
#define IIC_SDA_GPIO_Port               GPIOH

/* SCL/SDA pin control macros */
#define	IIC_SDA_L   (HAL_GPIO_WritePin(IIC_SDA_GPIO_Port, IIC_SDA_Pin, GPIO_PIN_RESET))
#define	IIC_SDA_H   (HAL_GPIO_WritePin(IIC_SDA_GPIO_Port, IIC_SDA_Pin, GPIO_PIN_SET))

#define	IIC_SCL_L   (HAL_GPIO_WritePin(IIC_SCL_GPIO_Port, IIC_SCL_Pin, GPIO_PIN_RESET))
#define	IIC_SCL_H   (HAL_GPIO_WritePin(IIC_SCL_GPIO_Port, IIC_SCL_Pin, GPIO_PIN_SET))

/* Read SDA pin state */
#define READ_SDA 	(HAL_GPIO_ReadPin(IIC_SDA_GPIO_Port, IIC_SDA_Pin))

/**
 * @brief:  initialize software I2C pins and timing
 * @baudrate:  I2C bus baudrate
 */
void my_i2c_init(uint32_t baudrate);

/**
 * @brief:  write multiple bytes to I2C device register
 * @devaddr:  target device address
 * @regaddr:  target register address
 * @data:     data buffer to write
 * @length:      number of bytes to write
 */
void my_i2c_write_multi_byte(uint8_t devaddr, uint8_t regaddr, uint8_t *data, uint32_t length);

/**
 * @brief:  read multiple bytes from I2C device register
 * @devaddr:  target device address
 * @regaddr:  target register address
 * @data:     buffer for received data
 * @length:      number of bytes to read
 */
void my_i2c_read_multi_byte(uint8_t devaddr, uint8_t regaddr, uint8_t *data, uint32_t length);

/**
 * @brief:  transmit multiple bytes to I2C device (no register address)
 * @devaddr:  target device address
 * @data:     data buffer to send
 * @length:      number of bytes to send
 */
void my_i2c_transmit_multi_byte(uint8_t devaddr, uint8_t *data, uint32_t length);

/**
 * @brief:  receive multiple bytes from I2C device (no register address)
 * @devaddr:  target device address
 * @data:     buffer for received data
 * @length:      number of bytes to receive
 */
void my_i2c_receive_multi_byte(uint8_t devaddr, uint8_t *data, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif
