/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef _DAC_DEV_H_
#define _DAC_DEV_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>


typedef struct {
    /* device identifier (e.g. "dac1") */
    char *name;
    /* number of output channels */
    unsigned char channel_count;
    /* DAC resolution in bits (8/12) */
    unsigned char resolution;

    /**
     * @brief:  initialize DAC hardware
     * @privatedata:  pointer to this Dac_Device_T
     *
     * Return: 0 on success, -1 on failure
     */
    int (*init)(void *privatedata);

    /**
     * @brief:  write DAC output value for a channel
     * @privatedata:  pointer to this Dac_Device_T
     * @channel:      channel index (0 to channel_count-1)
     * @value:        raw output value (0 to 2^resolution - 1)
     *
     * Return: 0 on success, -1 on failure
     */
    int (*write)(void *privatedata, unsigned char channel, uint32_t value);

    /**
     * @brief:  start DAC output
     * @privatedata:  pointer to this Dac_Device_T
     *
     * Return: 0 on success, -1 on failure
     */
    int (*start)(void *privatedata);

    /**
     * @brief:  stop DAC output
     * @privatedata:  pointer to this Dac_Device_T
     *
     * Return: 0 on success, -1 on failure
     */
    int (*stop)(void *privatedata);

    void *private_data;
} Dac_Device_T;

/**
 * @brief:  get DAC device structure by name
 * @name:   device name (e.g. "dac1")
 *
 * Return: pointer to Dac_Device_T structure, NULL if not found
 */
Dac_Device_T *get_dac_device(char *name);

#ifdef __cplusplus
}
#endif

#endif /* _DAC_DEV_H_ */
