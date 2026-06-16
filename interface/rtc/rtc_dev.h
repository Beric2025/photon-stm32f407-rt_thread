/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef _RTC_DEV_H_
#define _RTC_DEV_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>


typedef struct {
    /* device identifier (e.g. "rtc") */
    char *name;

    /**
     * @brief:  initialize RTC hardware
     * @privatedata:  pointer to this Rtc_Device_T
     *
     * Return: 0 on success, -1 on failure
     */
    int (*init)(void *privatedata);

    /**
     * @brief:  get current timestamp (seconds since epoch)
     * @privatedata:  pointer to this Rtc_Device_T
     * @timestamp:    output buffer for timestamp in seconds
     *
     * Return: 0 on success, -1 on failure
     */
    int (*get_time)(void *privatedata, uint32_t *timestamp);

    /**
     * @brief:  set current timestamp
     * @privatedata:  pointer to this Rtc_Device_T
     * @timestamp:    timestamp in seconds
     *
     * Return: 0 on success, -1 on failure
     */
    int (*set_time)(void *privatedata, uint32_t timestamp);

    void *private_data;
} Rtc_Device_T;

/**
 * @brief:  get RTC device structure by name
 * @name:   device name (e.g. "rtc")
 *
 * Return: pointer to Rtc_Device_T structure, NULL if not found
 */
Rtc_Device_T *get_rtc_device(char *name);

#ifdef __cplusplus
}
#endif

#endif /* _RTC_DEV_H_ */
