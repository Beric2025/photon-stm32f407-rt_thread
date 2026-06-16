/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef _ULTRASONIC_DEV_H_
#define _ULTRASONIC_DEV_H_

#ifdef __cplusplus
 extern "C" {
#endif

typedef struct {
    /* device name (e.g. "ultrasonic1") */
    char *name;

    /**
     * @brief:  initialize ultrasonic device
     * @privatedata:  ultrasonic handle (Ultrasonic_Device_T *)
     * Return: 0 on success, -1 on failure
     */
    int (*init)(void *privatedata);

    /**
     * @brief:  configure ultrasonic device parameters
     * @privatedata:  ultrasonic handle (Ultrasonic_Device_T *)
     * Return: 0 on success, -1 on failure
     */
    int (*config)(void *privatedata);

    /**
     * @brief:  send communication data to ultrasonic device
     * @privatedata:  ultrasonic handle (Ultrasonic_Device_T *)
     * @data:         data buffer to send
     * @size:         number of bytes to send
     * Return: 0 on success, -1 on failure
     */
    int (*set)(void *privatedata, unsigned char *data, unsigned short size);

    /**
     * @brief:  read distance value from ultrasonic device
     * @privatedata:  ultrasonic handle (Ultrasonic_Device_T *)
     * Return: distance value in mm
     */
    unsigned short (*read_distance)(void *privatedata);

    void *private_data;
} Ultrasonic_Device_T;

/**
 * @brief:  get ultrasonic device structure by name
 * @name:   device name (e.g. "ultrasonic1")
 * Return: pointer to Ultrasonic_Device_T structure, NULL if not found
 */
Ultrasonic_Device_T *get_ultrasonic_device(char *name);

#ifdef __cplusplus
}
#endif

#endif /* _ULTRASONIC_DEV_H_ */
