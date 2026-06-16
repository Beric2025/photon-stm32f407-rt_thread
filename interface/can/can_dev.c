/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * can dev
 */

#include <string.h>
#include <stdio.h>
#include "main.h"
#include "can_dev.h"
#include "bsp_can.h"
#include "log_print.h"

#ifdef USE_OS
#include <rtthread.h>
#endif

#define TAG "can dev:"


/* Buffer definitions */
#define CAN1_RX_BUF_SIZE		    16
static uint8_t s_can1_rx_buffer[CAN1_RX_BUF_SIZE];

/* Private data structure */
typedef struct {
	CAN_HandleTypeDef *can;
    CAN_TxHeaderTypeDef *can_tx;
    CAN_RxHeaderTypeDef  *can_rx;
	unsigned char (*init)(unsigned short presc, unsigned char tsjw,
	                        unsigned short ntsg1, unsigned char ntsg2,
	                        uint32_t mode);
	uint8_t tx_flag;
	uint8_t *rx_buf;
	uint16_t rx_size;
}Can_Data_T;

/* Function declarations */
static int can_init(void *privatedata);
static int can_send(void *privatedata, unsigned char *data, unsigned short lenght);
static unsigned short can_receive(void *privatedata, unsigned char *data, unsigned short size);

/* CAN1 private data */
static Can_Data_T s_can1_data = {
	.can = &g_can1,
    .can_tx = &g_can1_tx,
    .can_rx = &g_can1_rx,
	.init = bsp_can1_init,
	.rx_buf = s_can1_rx_buffer,
	.rx_size = CAN1_RX_BUF_SIZE
};

/* CAN1 device instance */
static Can_Device_T s_can1_dev = {
	.name = "can1",
	.init = can_init,
	.send = can_send,
	.receive = can_receive,
	.private_data = &s_can1_data,
};

static int can_init(void *privatedata)
{
    if(privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s Invalid private data is null!\n", TAG);
        return -1;
    }

    Can_Device_T *cav_dev = (Can_Device_T *)privatedata;
    Can_Data_T *cav_data = (Can_Data_T *)cav_dev->private_data;

    cav_data->init(10, 8, 31, 8, CAN_MODE_NORMAL);
    LOG_PRINT(LOG_OUT_DEBUG, "%s Invalid successful!\n", TAG);

    return 0;
}
static int can_send(void *privatedata, unsigned char *data, unsigned short lenght)
{
    if(privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s Send private data is null!\n", TAG);
        return -1;
    }

    Can_Device_T *cav_dev = (Can_Device_T *)privatedata;
    Can_Data_T *cav_data = (Can_Data_T *)cav_dev->private_data;
    uint32_t TxMailbox = CAN_TX_MAILBOX0;
    uint32_t id = 0x12; /* Example standard ID */

    cav_data->can_tx->StdId = id;         
    //  g_canx_txheader.ExtId = id;         
    cav_data->can_tx->IDE = CAN_ID_STD;   
    cav_data->can_tx->RTR = CAN_RTR_DATA; 
    cav_data->can_tx->DLC = lenght;

    if (HAL_CAN_AddTxMessage(cav_data->can, cav_data->can_tx, data, &TxMailbox) != HAL_OK) 
    {
        return 1;
    }
    while (HAL_CAN_GetTxMailboxesFreeLevel(cav_data->can) != 3); 


    LOG_PRINT(LOG_OUT_DEBUG, "%s Send successful!\n", TAG);

    return 0;
}

#if CAN1_RX0_INT_ENABLE
/**
 * Interrupt mode: data is buffered in ISR and copied out here.
 * Hardware ISR (HAL_CAN_RxFifo0Callback) stores frames into rx_buf,
 * can_receive reads from the buffer. Non-blocking, suitable for
 * high-throughput scenarios where message loss is unacceptable.
 *
 * Return: number of bytes received, 0 if no data or buffer too small
 */
static unsigned short can_receive(void *privatedata, unsigned char *data, unsigned short size)
{
    if(privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s Receive private data is null!\n", TAG);
        return 0;
    }

    unsigned short ret_size = 0;
    Can_Device_T *cav_dev = (Can_Device_T *)privatedata;
    Can_Data_T *cav_data = (Can_Data_T *)cav_dev->private_data;

    if(cav_data->rx_size){
        /* Return if receive buffer is smaller than the received data */
        if(cav_data->rx_size > size){
            LOG_PRINT(LOG_OUT_WARN, "%s Receive buffer too small! Data size: %d, Buffer size: %d\n", TAG, cav_data->rx_size, size);
            return 0;
        }
#ifdef USE_OS
	    rt_enter_critical();
#endif        
        memcpy(data, cav_data->rx_buf, cav_data->rx_size);
        ret_size = cav_data->rx_size;
        cav_data->rx_size = 0;
#ifdef USE_OS
	    rt_exit_critical();
#endif
	}
    LOG_PRINT(LOG_OUT_DEBUG, "%s Receive successful!\n", TAG);

    return ret_size;
}
#else
/**
 * Polling mode: read directly from hardware RX FIFO each call.
 * Simpler, no ISR or intermediate buffer needed. Suitable for
 * low-traffic use where callers can poll at their own pace.
 *
 * Return: number of bytes received (from DataLength), 0 if no data
 */
static unsigned short can_receive(void *privatedata, unsigned char *data, unsigned short size)
{
    if(privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s Receive private data is null!\n", TAG);
        return 0;
    }

    Can_Device_T *cav_dev = (Can_Device_T *)privatedata;
    Can_Data_T *cav_data = (Can_Data_T *)cav_dev->private_data;

    if (HAL_CAN_GetRxFifoFillLevel(cav_data->can, CAN_RX_FIFO0) == 0) {
        /* No messages pending in FIFO0 */
        return 0;
    }

    if (HAL_CAN_GetRxMessage(cav_data->can, CAN_RX_FIFO0, cav_data->can_rx, data) != HAL_OK) {
        /* No new message or error */
        LOG_PRINT(LOG_OUT_ERROR, "%s Receive message error!\n", TAG);
        return 0;
    }
    LOG_PRINT(LOG_OUT_DEBUG, "%s Receive message success!\n", TAG);

    return cav_data->can_rx->DLC; /* Return number of bytes received */
}
#endif




#if CAN1_RX0_INT_ENABLE

/* ====================================================================
 * can_dev_on_event — single entry point from BSP HAL callbacks
 *
 * Handles RX FIFO0 messages in interrupt mode.
 * Runs in ISR context — keep it short.
 * ==================================================================== */
void can_dev_on_event(void *hcan_void, Can_Event_T event)
{
    CAN_HandleTypeDef *hcan = (CAN_HandleTypeDef *)hcan_void;
    Can_Data_T *cav_data = NULL;

    if (hcan == s_can1_data.can) {
        cav_data = &s_can1_data;
    }

    if (cav_data == NULL)
        return;

    switch (event) {
    case CAN_EVENT_RX_FIFO0:
    {
        uint8_t rxdata[8];
        /* Read the pending message and buffer it */
        if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, cav_data->can_rx, rxdata) == HAL_OK) {
            uint16_t dlc = cav_data->can_rx->DLC;
            if (dlc <= cav_data->rx_size) {
                memcpy(cav_data->rx_buf, rxdata, dlc);
                cav_data->rx_size = dlc;
            }
        }
        break;
    }
    default:
        break;
    }
}

#endif

/**
 * @brief:  get CAN device structure by name
 * @name:   device name (e.g. "can1")
 *
 * Return: pointer to Can_Device_T structure, NULL if not found
 */
Can_Device_T *get_can_device(char *name)
{
    Can_Device_T *cav_dev = NULL;

    if(0 == strcmp(name , "can1"))
      cav_dev = &s_can1_dev;

    return cav_dev;
}

