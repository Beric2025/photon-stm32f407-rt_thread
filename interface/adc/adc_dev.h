/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef _ADC_DEV_H_
#define _ADC_DEV_H_

#ifdef __cplusplus
 extern "C" {
#endif


typedef struct {
    /* device identifier (e.g. "adc1") */
    char *name;  
    
    /* ADC resolution in bits (8/10/12/16) */
    unsigned char resolution;   

    /* number of channels on this device */
    unsigned char channel_count; 

    /**
     * @brief:  initialize ADC hardware
     * @privatedata:  pointer to this Adc_Device_T
     *
     * Return: 0 on success, -1 on failure
     */
    int (*init)(void *privatedata);

    /**
     * @brief:  start ADC conversion (DMA or polling)
     * @privatedata:  pointer to this Adc_Device_T
     *
     * Return: 0 on success, -1 on failure
     */
    int (*start)(void *privatedata);

    /**
     * @brief:  stop ADC conversion
     * @privatedata:  pointer to this Adc_Device_T
     *
     * Return: 0 on success, -1 on failure
     */
    int (*stop)(void *privatedata);

    /**
     * @brief:  read raw ADC value for a channel
     * @privatedata:  pointer to this Adc_Device_T
     * @channel:      channel index (0 to channel_count-1)
     * @value:        output buffer for raw ADC value
     *
     * Return: 0 on success, -1 on failure
     */
    int (*read_raw)(void *privatedata, unsigned char channel, unsigned short *value);
    
    void *private_data;
} Adc_Device_T;

/* ====================================================================
 * Generic ADC events — MCU / SDK independent
 * ==================================================================== */
typedef enum {
	ADC_EVENT_CONV_COMPLETE,  /* DMA conversion complete — ring buffer ready */
} Adc_Event_T;

/**
 * @brief:  Notify the interface layer of an ADC event (called from BSP ISR).
 * @hadc:    Opaque handle (ADC_HandleTypeDef* cast to void*).
 * @event:   Generic event type.
 */
void adc_dev_on_event(void *hadc, Adc_Event_T event);

/**
 * @brief:  get ADC device structure by name
 * @name:   device name (e.g. "adc1")
 *
 * Return: pointer to Adc_Device_T structure, NULL if not found
 */
Adc_Device_T *get_adc_device(char *name);

#ifdef __cplusplus
}
#endif

#endif  /* _ADC_DEV_H_ */
