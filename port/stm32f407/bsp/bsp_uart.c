/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * UART initialization
 */

#include <string.h>
#include <stdio.h>
#include "bsp_uart.h"
#include "uart_dev.h"

/* UART handle definition */
UART_HandleTypeDef g_uart1;
UART_HandleTypeDef g_uart2;
UART_HandleTypeDef g_uart4;

static DMA_HandleTypeDef s_hdma_uart2_rx;
static DMA_HandleTypeDef s_hdma_uart2_tx;
static DMA_HandleTypeDef s_hdma_uart4_rx;
static DMA_HandleTypeDef s_hdma_uart4_tx;

void bsp_uart1_init(uint32_t baudrate)
{
    g_uart1.Instance = USART1;
    g_uart1.Init.BaudRate = baudrate;
    g_uart1.Init.WordLength = UART_WORDLENGTH_8B;
    g_uart1.Init.StopBits = UART_STOPBITS_1;
    g_uart1.Init.Parity = UART_PARITY_NONE;
    g_uart1.Init.Mode = UART_MODE_TX_RX;
    g_uart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    g_uart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&g_uart1) != HAL_OK)
    {
      Error_Handler();
    }
}

void bsp_uart2_init(uint32_t baudRate)
{
    /* DMA controller clock enable */
    __HAL_RCC_DMA1_CLK_ENABLE();
    HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
    /* DMA1_Stream6_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);

    g_uart2.Instance = USART2;
    g_uart2.Init.BaudRate = baudRate;
    g_uart2.Init.WordLength = UART_WORDLENGTH_8B;
    g_uart2.Init.StopBits = UART_STOPBITS_1;
    g_uart2.Init.Parity = UART_PARITY_NONE;
    g_uart2.Init.Mode = UART_MODE_TX_RX;
    g_uart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    g_uart2.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&g_uart2) != HAL_OK)
    {
      Error_Handler();
    }
}
void bsp_uart4_init(uint32_t baudRate)
{
    __HAL_RCC_DMA1_CLK_ENABLE();
    /* DMA1_Stream2_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Stream2_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream2_IRQn);
    /* DMA1_Stream4_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);


    g_uart4.Instance = UART4;
    g_uart4.Init.BaudRate = baudRate;
    g_uart4.Init.WordLength = UART_WORDLENGTH_8B;
    g_uart4.Init.StopBits = UART_STOPBITS_1;
    g_uart4.Init.Parity = UART_PARITY_NONE;
    g_uart4.Init.Mode = UART_MODE_TX_RX;
    g_uart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    g_uart4.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&g_uart4) != HAL_OK)
    {
      Error_Handler();
    }
}


void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==UART4)
  {
    __HAL_RCC_UART4_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**UART4 GPIO Configuration
    PA0-WKUP     ------> UART4_TX
    PA1     ------> UART4_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* UART4 DMA Init */
    /* UART4_RX Init */
    s_hdma_uart4_rx.Instance = DMA1_Stream2;
    s_hdma_uart4_rx.Init.Channel = DMA_CHANNEL_4;
    s_hdma_uart4_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    s_hdma_uart4_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    s_hdma_uart4_rx.Init.MemInc = DMA_MINC_ENABLE;
    s_hdma_uart4_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    s_hdma_uart4_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    s_hdma_uart4_rx.Init.Mode = DMA_NORMAL;
    s_hdma_uart4_rx.Init.Priority = DMA_PRIORITY_LOW;
    s_hdma_uart4_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&s_hdma_uart4_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmarx,s_hdma_uart4_rx);

    /* UART4_TX Init */
    s_hdma_uart4_tx.Instance = DMA1_Stream4;
    s_hdma_uart4_tx.Init.Channel = DMA_CHANNEL_4;
    s_hdma_uart4_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    s_hdma_uart4_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    s_hdma_uart4_tx.Init.MemInc = DMA_MINC_ENABLE;
    s_hdma_uart4_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    s_hdma_uart4_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    s_hdma_uart4_tx.Init.Mode = DMA_NORMAL;
    s_hdma_uart4_tx.Init.Priority = DMA_PRIORITY_LOW;
    s_hdma_uart4_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&s_hdma_uart4_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmatx,s_hdma_uart4_tx);

    /* UART4 interrupt Init */
    HAL_NVIC_SetPriority(UART4_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(UART4_IRQn);
  }
  else if(uartHandle->Instance==USART1)
  {
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  }
  else if(uartHandle->Instance==USART2)
  {
    __HAL_RCC_USART2_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART2 DMA Init */
    /* USART2_RX Init */
    s_hdma_uart2_rx.Instance = DMA1_Stream5;
    s_hdma_uart2_rx.Init.Channel = DMA_CHANNEL_4;
    s_hdma_uart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    s_hdma_uart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    s_hdma_uart2_rx.Init.MemInc = DMA_MINC_ENABLE;
    s_hdma_uart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    s_hdma_uart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    s_hdma_uart2_rx.Init.Mode = DMA_NORMAL;
    s_hdma_uart2_rx.Init.Priority = DMA_PRIORITY_LOW;
    s_hdma_uart2_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&s_hdma_uart2_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmarx,s_hdma_uart2_rx);

    /* USART2_TX Init */
    s_hdma_uart2_tx.Instance = DMA1_Stream6;
    s_hdma_uart2_tx.Init.Channel = DMA_CHANNEL_4;
    s_hdma_uart2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    s_hdma_uart2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    s_hdma_uart2_tx.Init.MemInc = DMA_MINC_ENABLE;
    s_hdma_uart2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    s_hdma_uart2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    s_hdma_uart2_tx.Init.Mode = DMA_NORMAL;
    s_hdma_uart2_tx.Init.Priority = DMA_PRIORITY_LOW;
    s_hdma_uart2_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&s_hdma_uart2_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmatx,s_hdma_uart2_tx);

    /* USART2 interrupt Init */
    HAL_NVIC_SetPriority(USART2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==UART4)
  {

    __HAL_RCC_UART4_CLK_DISABLE();

    /**UART4 GPIO Configuration
    PA0-WKUP     ------> UART4_TX
    PA1     ------> UART4_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0|GPIO_PIN_1);

    /* UART4 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);
    HAL_DMA_DeInit(uartHandle->hdmatx);

    /* UART4 interrupt Deinit */
    HAL_NVIC_DisableIRQ(UART4_IRQn);

  }
  else if(uartHandle->Instance==USART1)
  {

    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

  }
  else if(uartHandle->Instance==USART2)
  {

    __HAL_RCC_USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);

    /* USART2 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);
    HAL_DMA_DeInit(uartHandle->hdmatx);

    /* USART2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  }
}


void DMA1_Stream2_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&s_hdma_uart4_rx);
}

/**
  * @brief This function handles DMA1 stream4 global interrupt.
  */
void DMA1_Stream4_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&s_hdma_uart4_tx);
}

/**
  * @brief This function handles DMA1 stream5 global interrupt.
  */
void DMA1_Stream5_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&s_hdma_uart2_rx);
}

/**
  * @brief This function handles DMA1 stream6 global interrupt.
  */
void DMA1_Stream6_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&s_hdma_uart2_tx);
}

/**
  * @brief This function handles USART2 global interrupt.
  */
void USART2_IRQHandler(void)
{
    HAL_UART_IRQHandler(&g_uart2);
}

/**
  * @brief This function handles UART4 global interrupt.
  */
void UART4_IRQHandler(void)
{
    HAL_UART_IRQHandler(&g_uart4);
}

/* ====================================================================
 * HAL callback implementations — the *only* place in the project that
 * touches STM32 HAL weak callbacks.  Each one simply translates the
 * HAL-specific event into a generic Uart_Event_T and forwards it to
 * the MCU-independent interface layer via uart_dev_on_event().
 * ==================================================================== */

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    uart_dev_on_event((void *)huart, UART_EVENT_ERROR, 0);
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    uart_dev_on_event((void *)huart, UART_EVENT_RX_IDLE, Size);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    uart_dev_on_event((void *)huart, UART_EVENT_TX_COMPLETE, 0);
}

void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart)
{
    uart_dev_on_event((void *)huart, UART_EVENT_TX_HALF, 0);
}


