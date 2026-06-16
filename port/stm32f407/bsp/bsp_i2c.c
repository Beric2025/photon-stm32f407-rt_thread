/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * I2C initialization file
 */

#include <string.h>
#include <stdio.h>
#include "bsp_i2c.h"


I2C_HandleTypeDef g_i2c2;

/**
 * bsp_i2c2_init - I2C2 initialization, baud rate 100kbps
 */
void bsp_i2c2_init(uint32_t baudrate)
{
    g_i2c2.Instance             = I2C2;
    g_i2c2.Init.ClockSpeed      = baudrate;
    g_i2c2.Init.DutyCycle       = I2C_DUTYCYCLE_2;
    g_i2c2.Init.OwnAddress1     = 0;
    g_i2c2.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
    g_i2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    g_i2c2.Init.OwnAddress2     = 0;
    g_i2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    g_i2c2.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&g_i2c2) != HAL_OK) {
        Error_Handler();
    }
}

void HAL_I2C_MspInit(I2C_HandleTypeDef* i2cHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (i2cHandle->Instance == I2C2) {
        __HAL_RCC_I2C2_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();

        /**I2C2 GPIO Configuration
        PB10     ------> I2C2_SCL
        PB11     ------> I2C2_SDA
        */
        GPIO_InitStruct.Pin       = GPIO_PIN_10 | GPIO_PIN_11;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull      = GPIO_PULLUP;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* i2cHandle)
{
    if (i2cHandle->Instance == I2C2) {
        __HAL_RCC_I2C2_CLK_DISABLE();

        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10 | GPIO_PIN_11);
    }
}


