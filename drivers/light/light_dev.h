/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef _LIGHT_DEV_H_
#define _LIGHT_DEV_H_

#ifdef __cplusplus
 extern "C" {
#endif

typedef struct {
    /* device name (e.g. "light1") */
    char *name;

    /**
     * @brief:  initialize light device
     * @privatedata:  light handle (Light_Device_T *)
     * Return: 0 on success, -1 on failure
     */
    int (*init)(void *privatedata);

    /**
     * @brief:  set light strip color
     * @privatedata:  light handle (Light_Device_T *)
     * @color:        color data buffer
     * Return: 0 on success, -1 on failure
     */
    int (*set_light_color)(void *privatedata, unsigned char *color);

    /**
     * @brief:  set multiple LED colors starting from index
     * @privatedata:  light handle (Light_Device_T *)
     * @start:        start LED index
     * @num:          number of LEDs to set
     * @color:        color data buffer
     * Return: 0 on success, -1 on failure
     */
    int (*set_light_num_color)(void *privatedata, unsigned short start,
         unsigned short num, unsigned char *color);

    /**
     * @brief:  read light strip color
     * @privatedata:  light handle (Light_Device_T *)
     * Return: color value
     */
    unsigned char (*read_light_color)(void *privatedata);

    /**
     * @brief:  turn off light
     * @privatedata:  light handle (Light_Device_T *)
     * Return: 0 on success, -1 on failure
     */
    int (*light_off)(void *privatedata);

    void *private_data;
} Light_Device_T;

/**
 * @brief:  get light device structure by name
 * @name:   device name (e.g. "light1")
 * Return: pointer to Light_Device_T structure, NULL if not found
 */
Light_Device_T *get_light_device(char *name);

#ifdef __cplusplus
}
#endif

#endif /* _LIGHT_DEV_H_ */
