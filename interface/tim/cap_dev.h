/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef _CAP_DEV_H_
#define _CAP_DEV_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>


typedef struct {
    /* device identifier (e.g. "tim1") */
    char *name;     
    
    /* number of capture channels */
    unsigned char channel_count;   

    /**
     * @brief:  initialize input capture hardware
     * @privatedata:  pointer to this Cap_Device_T
     *
     * Return: 0 on success, -1 on failure
     */
    int (*init)(void *privatedata);

    /**
     * @brief:  start input capture
     * @privatedata:  pointer to this Cap_Device_T
     *
     * Return: 0 on success, -1 on failure
     */
    int (*start)(void *privatedata);

    /**
     * @brief:  stop input capture
     * @privatedata:  pointer to this Cap_Device_T
     *
     * Return: 0 on success, -1 on failure
     */
    int (*stop)(void *privatedata);

    /**
     * @brief:  get captured pulse width on a channel
     * @privatedata:  pointer to this Cap_Device_T
     * @channel:      channel index (0 to channel_count-1)
     * @width_us:     output buffer for pulse width in microseconds
     *
     * Return: 0 on success, -1 if no capture data available
     */
    int (*get_pulse)(void *privatedata, unsigned char channel, uint32_t *width_us);

    void *private_data;
} Cap_Device_T;

/* ====================================================================
 * Generic Capture events — MCU / SDK independent
 * ==================================================================== */
typedef enum {
	CAP_EVENT_CAPTURE,        /* Input capture triggered on a channel      */
} Cap_Event_T;

/**
 * @brief:  Notify the interface layer of a capture event (from BSP ISR).
 * @htim:    Opaque handle (TIM_HandleTypeDef* cast to void*).
 * @channel: HAL channel mask (HAL_TIM_ACTIVE_CHANNEL_1 << n).
 */
void cap_dev_on_event(void *htim, uint32_t channel);

/**
 * @brief:  get capture device structure by name
 * @name:   device name (e.g. "cap4")
 *
 * Return: pointer to Cap_Device_T structure, NULL if not found
 */
Cap_Device_T *get_cap_device(char *name);

#ifdef __cplusplus
}
#endif

#endif /* _CAP_DEV_H_ */
