/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef _I2S_DEV_H_
#define _I2S_DEV_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>


typedef struct {
    /* device identifier (e.g. "i2s2") */
    char *name;

    /**
     * @brief:  initialize I2S hardware
     * @privatedata:  pointer to this I2s_Device_T
     * @sample_rate:  sample rate in Hz (e.g. 44100, 48000)
     * @bits:         bits per sample (16, 24, 32)
     *
     * Return: 0 on success, -1 on failure
     */
    int (*init)(void *privatedata, uint32_t sample_rate, unsigned char bits);

    /**
     * @brief:  write stereo sample data
     * @privatedata:  pointer to this I2s_Device_T
     * @buf:          sample buffer (L/R interleaved)
     * @len:          number of 16-bit words to send
     *
     * Return: 0 on success, -1 on failure
     */
    int (*write)(void *privatedata, unsigned short *buf, uint32_t len);

    /**
     * @brief:  read stereo sample data
     * @privatedata:  pointer to this I2s_Device_T
     * @buf:          receive buffer
     * @len:          number of 16-bit words to receive
     *
     * Return: 0 on success, -1 on failure
     */
    int (*read)(void *privatedata, unsigned short *buf, uint32_t len);

    /**
     * @brief:  start I2S transfer
     * @privatedata:  pointer to this I2s_Device_T
     *
     * Return: 0 on success, -1 on failure
     */
    int (*start)(void *privatedata);

    /**
     * @brief:  stop I2S transfer
     * @privatedata:  pointer to this I2s_Device_T
     *
     * Return: 0 on success, -1 on failure
     */
    int (*stop)(void *privatedata);

    void *private_data;
} I2s_Device_T;

/**
 * @brief:  get I2S device structure by name
 * @name:   device name (e.g. "i2s2")
 *
 * Return: pointer to I2s_Device_T structure, NULL if not found
 */
I2s_Device_T *get_i2s_device(char *name);

#ifdef __cplusplus
}
#endif

#endif /* _I2S_DEV_H_ */
