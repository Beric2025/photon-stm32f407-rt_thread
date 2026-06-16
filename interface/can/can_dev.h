/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef _CAN_DEV_H_
#define _CAN_DEV_H_

#ifdef __cplusplus
	extern "C" {
#endif

typedef struct {
	/* CAN controller identifier (e.g. "can1", "can2") */
	char *name;             

	/**
	 * @brief:  initialize CAN controller
	 * @privatedata:  CAN handle (Can_Device_T *)
	 *
	 * Return: 0 on success, -1 on failure
	 */
	int (*init)(void *privatedata);

	/**
	 * @brief:  send data via CAN
	 * @privatedata:  CAN handle (Can_Device_T *)
	 * @data:          data buffer to send
	 * @length:         number of bytes to send
	 *
	 * Return: 0 on success, -1 on failure
	 */
	int (*send)(void *privatedata, unsigned char *data, unsigned short length);

	/**
	 * @brief:  receive data from CAN
	 * @privatedata:  CAN handle (Can_Device_T *)
	 * @data:         buffer for received data
	 * @size:         size of the data buffer
	 *
	 * Return: number of bytes received, 0 if no data available
	 */
	unsigned short (*receive)(void *privatedata, unsigned char *data, unsigned short size);

	void *private_data;   /* hardware-specific data (e.g. Can_Device_T *) */
}Can_Device_T;

/* ====================================================================
 * Generic CAN events — MCU / SDK independent
 * ==================================================================== */
typedef enum {
	CAN_EVENT_RX_FIFO0,       /* Message pending in RX FIFO0                */
	CAN_EVENT_RX_FIFO1        /* Message pending in RX FIFO1                */
} Can_Event_T;

/**
 * @brief:  Notify the interface layer of a CAN event (from BSP ISR).
 * @hcan:    Opaque handle (CAN_HandleTypeDef* cast to void*).
 * @event:   Generic event type.
 */
void can_dev_on_event(void *hcan, Can_Event_T event);

/**
 * @brief:  get CAN device structure by name
 * @name:   device name (e.g. "can1")
 *
 * Return: pointer to Can_Device_T structure, NULL if not found
 */
Can_Device_T *get_can_device(char *name);

#ifdef __cplusplus
}
#endif

#endif	/* _CAN_DEV_H_*/
