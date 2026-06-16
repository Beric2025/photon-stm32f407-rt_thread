/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef _WDG_DEV_H_
#define _WDG_DEV_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>


typedef struct {
    /* device identifier (e.g. "iwdg") */
    char *name;                    

    /**
     * @brief:  initialize watchdog with timeout
     * @privatedata:  pointer to this Wdg_Device_T
     * @timeout_ms:   watchdog timeout in milliseconds
     *
     * Return: 0 on success, -1 on failure
     */
    int (*init)(void *privatedata, uint32_t timeout_ms);

    /**
     * @brief:  feed (refresh) the watchdog
     * @privatedata:  pointer to this Wdg_Device_T
     *
     * Return: 0 on success, -1 on failure
     */
    int (*refresh)(void *privatedata);

    void *private_data;
} Wdg_Device_T;

/**
 * @brief:  get watchdog device structure by name
 * @name:   device name (e.g. "iwdg")
 *
 * Return: pointer to Wdg_Device_T structure, NULL if not found
 */
Wdg_Device_T *get_wdg_device(char *name);

#ifdef __cplusplus
}
#endif

#endif /* _WDG_DEV_H_ */
