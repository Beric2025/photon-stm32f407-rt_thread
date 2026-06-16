/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * CAN board support package
 */

#include <string.h>
#include <stdio.h>
#include "bsp_can.h"
#include "can_dev.h"

CAN_HandleTypeDef   g_can1;
CAN_TxHeaderTypeDef g_can1_tx;
CAN_RxHeaderTypeDef g_can1_rx;

unsigned char bsp_can1_init(unsigned short presc, unsigned char tsjw,
    unsigned short ntsg1, unsigned char ntsg2, uint32_t mode)
{
    g_can1.Instance = CAN1;
    g_can1.Init.Prescaler = presc;
    g_can1.Init.Mode = mode;
    g_can1.Init.SyncJumpWidth = tsjw;
    g_can1.Init.TimeSeg1 = ntsg1;
    g_can1.Init.TimeSeg2 = ntsg2;
    g_can1.Init.TimeTriggeredMode = DISABLE;
    g_can1.Init.AutoBusOff = DISABLE;
    g_can1.Init.AutoWakeUp = DISABLE;
    g_can1.Init.AutoRetransmission = ENABLE;
    g_can1.Init.ReceiveFifoLocked = DISABLE;
    g_can1.Init.TransmitFifoPriority = DISABLE;
    if (HAL_CAN_Init(&g_can1) != HAL_OK)
    {
        Error_Handler();
        return 0xff;
    }

#if CAN_RX0_INT_ENABLE

    __HAL_CAN_ENABLE_IT(&g_can1, CAN_IT_RX_FIFO0_MSG_PENDING); 
    HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);                          
    HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 1, 0);                 
#endif

    CAN_FilterTypeDef sFilterConfig;

    sFilterConfig.FilterBank = 0;                            
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterIdHigh = 0x0000;                     
    sFilterConfig.FilterIdLow = 0x0000;
    sFilterConfig.FilterMaskIdHigh = 0x0000;                
    sFilterConfig.FilterMaskIdLow = 0x0000;
    sFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;    
    sFilterConfig.FilterActivation = CAN_FILTER_ENABLE;       
    sFilterConfig.SlaveStartFilterBank = 14;

    if (HAL_CAN_ConfigFilter(&g_can1, &sFilterConfig) != HAL_OK)
    {
        return 2;
    }

    if (HAL_CAN_Start(&g_can1) != HAL_OK)
    {
        return 3;
    }
    return 0;
}

void HAL_CAN_MspInit(CAN_HandleTypeDef* hcan)
{

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(hcan->Instance==CAN1)
    {
        /* CAN1 clock enable */
        __HAL_RCC_CAN1_CLK_ENABLE();

        __HAL_RCC_GPIOD_CLK_ENABLE();
        /**CAN1 GPIO Configuration
        PD0     ------> CAN1_RX
        PD1     ------> CAN1_TX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

        /* CAN1 interrupt Init */
        HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
        HAL_NVIC_SetPriority(CAN1_RX1_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(CAN1_RX1_IRQn);
    }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* hcan)
{
    if(hcan->Instance==CAN1)
    {
        /* Peripheral clock disable */
        __HAL_RCC_CAN1_CLK_DISABLE();

        /**CAN1 GPIO Configuration
        PD0     ------> CAN1_RX
        PD1     ------> CAN1_TX
        */
        HAL_GPIO_DeInit(GPIOD, GPIO_PIN_0|GPIO_PIN_1);

        /* CAN1 interrupt Deinit */
        HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
        HAL_NVIC_DisableIRQ(CAN1_RX1_IRQn);
    }
}

/* ====================================================================
 * HAL callback — translates to generic event for interface layer
 * ==================================================================== */
void HAL_CAN_RxFifo0Callback(CAN_HandleTypeDef *hcan, uint32_t RxFifo0ITs)
{
    (void)RxFifo0ITs;
    can_dev_on_event((void *)hcan, CAN_EVENT_RX_FIFO0);
}
void HAL_CAN_RxFifo1Callback(CAN_HandleTypeDef *hcan, uint32_t RxFifo1ITs)
{
    (void)RxFifo1ITs;
    can_dev_on_event((void *)hcan, CAN_EVENT_RX_FIFO1);
}
