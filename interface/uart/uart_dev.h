/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef _UART_DEV_H_
#define _UART_DEV_H_

#ifdef __cplusplus
 extern "C" {
#endif
#include <stdint.h>

typedef struct {
	/* UART controller identifier (e.g. "uart1", "uart2") */
	char *name;

	/**
	 * @brief:  initialize UART peripheral
	 * @privatedata:  UART handle (Uart_Device_T *)
	 * @baudrate: 	  UART baudrate
	 *
	 * Return: 0 on success, -1 on failure
	 */
	int (*init)(void *privatedata, uint32_t baudrate);

	/**
	 * @brief:  send data via UART
	 * @privatedata: UART handle (Uart_Device_T *)
	 * @buff:   	 data buffer to send
	 * @length:      number of bytes to send
	 *
	 * Return: 0 on success, -1 on failure
	 */
	int (*send)(void *privatedata, char *buff, unsigned short length);

	/**
	 * @brief:  receive data from UART
	 * @privatedata:  UART handle (Uart_Device_T *)
	 * @buff:            buffer for received data
	 * @size:            size of the data buffer
	 *
	 * Return: number of bytes received
	 */
	unsigned short (*receive)(void *privatedata, char *buff, unsigned short size);

	/**
	 * @brief:  UART control (ioctl)
	 * @privatedata:  UART handle (Uart_Device_T *)
	 * @flag:            control command
	 *
	 * Return: 0 on success, -1 on failure
	 */
	int (*ioctl)(void *privatedata, uint32_t flag);

	void *private_data;
}Uart_Device_T;

/* ====================================================================
 * Generic UART events — MCU / SDK independent
 *
 * BSP layer translates HAL-specific callbacks into these events and
 * calls uart_dev_on_event().  The interface layer never sees HAL types
 * directly, which keeps it portable across MCU families.
 * ==================================================================== */
typedef enum {
	UART_EVENT_RX_IDLE,       /* DMA receive idle (variable-length frame)  */
	UART_EVENT_TX_COMPLETE,   /* DMA / IT transmit complete                */
	UART_EVENT_TX_HALF,       /* DMA transmit half-complete                */
	UART_EVENT_ERROR,         /* Frame / noise / overflow / parity error   */
} Uart_Event_T;

/**
 * @brief:  Notify the interface layer of a UART event (called from BSP ISR).
 * @huart:   Opaque handle that identifies the UART peripheral (HAL's
 *           UART_HandleTypeDef* cast to void* — interface layer casts
 *           it back internally to locate the device).
 * @event:   Generic event type (see Uart_Event_T).
 * @size:    Number of bytes received (only meaningful for RX_IDLE,
 *           pass 0 for other events).
 *
 * This is the single entry point from BSP HAL callbacks into the
 * MCU-independent UART device layer.  Keep it short — it runs in ISR
 * context.
 */
void uart_dev_on_event(void *huart, Uart_Event_T event, unsigned short size);

/**
 * @brief:  get UART device structure by name
 * @name:   device name (e.g. "uart1", "uart2")
 *
 * Return: pointer to Uart_Device_T structure, NULL if not found
 */
Uart_Device_T *get_uart_device(char *name);

#ifdef __cplusplus
}
#endif

#endif	/* _UART_DEV_H_ */
