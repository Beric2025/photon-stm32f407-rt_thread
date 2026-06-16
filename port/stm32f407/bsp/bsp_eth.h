/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef __BSP_ETH_H
#define __BSP_ETH_H
#include "main.h"


/* Pin definitions */

#define ETH_CLK_GPIO_PORT               GPIOA
#define ETH_CLK_GPIO_PIN                GPIO_PIN_1
#define ETH_CLK_GPIO_CLK_ENABLE()       do{ __HAL_RCC_GPIOA_CLK_ENABLE();}while(0)

#define ETH_MDIO_GPIO_PORT              GPIOA
#define ETH_MDIO_GPIO_PIN               GPIO_PIN_2
#define ETH_MDIO_GPIO_CLK_ENABLE()       do{ __HAL_RCC_GPIOA_CLK_ENABLE();}while(0)

#define ETH_CRS_GPIO_PORT               GPIOA
#define ETH_CRS_GPIO_PIN                GPIO_PIN_7
#define ETH_CRS_GPIO_CLK_ENABLE()       do{ __HAL_RCC_GPIOA_CLK_ENABLE();}while(0)

#define ETH_MDC_GPIO_PORT               GPIOC
#define ETH_MDC_GPIO_PIN                GPIO_PIN_1
#define ETH_MDC_GPIO_CLK_ENABLE()       do{ __HAL_RCC_GPIOC_CLK_ENABLE();}while(0)

#define ETH_RXD0_GPIO_PORT              GPIOC
#define ETH_RXD0_GPIO_PIN               GPIO_PIN_4
#define ETH_RXD0_GPIO_CLK_ENABLE()      do{ __HAL_RCC_GPIOC_CLK_ENABLE();}while(0)

#define ETH_RXD1_GPIO_PORT              GPIOC
#define ETH_RXD1_GPIO_PIN               GPIO_PIN_5
#define ETH_RXD1_GPIO_CLK_ENABLE()      do{ __HAL_RCC_GPIOC_CLK_ENABLE();}while(0)

#define ETH_TX_EN_GPIO_PORT             GPIOG
#define ETH_TX_EN_GPIO_PIN              GPIO_PIN_11
#define ETH_TX_EN_GPIO_CLK_ENABLE()     do{ __HAL_RCC_GPIOG_CLK_ENABLE();}while(0)

#define ETH_TXD0_GPIO_PORT              GPIOG
#define ETH_TXD0_GPIO_PIN               GPIO_PIN_13
#define ETH_TXD0_GPIO_CLK_ENABLE()      do{ __HAL_RCC_GPIOG_CLK_ENABLE();}while(0)

#define ETH_TXD1_GPIO_PORT              GPIOG
#define ETH_TXD1_GPIO_PIN               GPIO_PIN_14
#define ETH_TXD1_GPIO_CLK_ENABLE()      do{ __HAL_RCC_GPIOG_CLK_ENABLE();}while(0)



extern ETH_HandleTypeDef    g_eth_handler;
extern ETH_DMADescTypeDef   g_ethDmaRxDscrTab[];
extern ETH_DMADescTypeDef   g_ethDmaTxDscrTab[];

/**
 * @brief:  initialize ETH hardware (MPU, HAL ETH Init, MSP GPIO/clock)
 * @mac_addr:  MAC address array (6 bytes)
 * Return: 0 on success, 1 on failure
 */
unsigned char bsp_eth_init(uint8_t *mac_addr);

/* ------------------------------------------------------------------
 * PHY reset method — uncomment ETH_PHY_RESET_VIA_GPIO when the PHY
 * nRST pin is wired to a direct MCU GPIO (e.g. PD3) instead of the
 * PCF8574 I2C expander.
 * ------------------------------------------------------------------ */
#define ETH_PHY_RESET_VIA_GPIO

#ifdef ETH_PHY_RESET_VIA_GPIO
  #define ETH_PHY_RST_PORT       GPIOD
  #define ETH_PHY_RST_PIN        GPIO_PIN_3
  #define ETH_PHY_RST_CLK_EN()   __HAL_RCC_GPIOD_CLK_ENABLE()
#endif

#endif
