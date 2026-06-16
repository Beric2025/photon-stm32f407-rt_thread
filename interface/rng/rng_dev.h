/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef _RNG_DEV_H_
#define _RNG_DEV_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>


typedef struct {
    /* device identifier (e.g. "rng") */
    char *name;

    /**
     * @brief:  initialize RNG hardware
     * @privatedata:  pointer to this Rng_Device_T
     *
     * Return: 0 on success, -1 on failure
     */
    int (*init)(void *privatedata);

    /**
     * @brief:  read 32-bit random value
     * @privatedata:  pointer to this Rng_Device_T
     * @value:        output buffer for random value
     *
     * Return: 0 on success, -1 on failure
     */
    int (*read)(void *privatedata, uint32_t *value);

    void *private_data;
} Rng_Device_T;

/**
 * @brief:  get RNG device structure by name
 * @name:   device name (e.g. "rng")
 *
 * Return: pointer to Rng_Device_T structure, NULL if not found
 */
Rng_Device_T *get_rng_device(char *name);

#ifdef __cplusplus
}
#endif

#endif /* _RNG_DEV_H_ */
