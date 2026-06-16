/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef __BATTERY_DEV_H_
#define __BATTERY_DEV_H_

#ifdef __cplusplus
	extern "C" {
#endif

typedef struct
{
    unsigned char  charge;           /* charge status */
    unsigned short voltage;          /* battery voltage (mV) */
    unsigned char  electricity;      /* battery level percentage (0-100) */
    unsigned char  FET;              /* FET status */
    unsigned char  protection;       /* protection status */
    unsigned char  error_code;       /* error code */
    unsigned char  factory;          /* factory mode flag */
    unsigned char  timeout;          /* timeout flag */
    short          total_current;    /* total current (mA) */
    unsigned char  manual_charge;    /* manual charge flag */
    unsigned short capacity;         /* remaining capacity (mAh) */
    unsigned short capacity_max;     /* maximum capacity (mAh) */
} Battery_Info_T;

/**
 * @brief:  battery device structure
 */
typedef struct {
    /* battery device name (e.g. "bat0", "bat1") */
    char *name;

    /**
     * @brief:  initialize battery device
     * @privatedata:  battery device handle (Battery_Device_T *)
     *
     * Return: 0 on success, -1 on failure
     */
    int (*init)(void *privatedata);

    /**
     * @brief:  read battery voltage
     * @privatedata:  battery device handle (Battery_Device_T *)
     * @voltage:      buffer for voltage value (mV)
     *
     * Return: 0 on success, -1 on failure
     */
    int (*read_voltage)(void *privatedata, unsigned short *voltage);

    /**
     * @brief:  read battery level percentage
     * @privatedata:  battery device handle (Battery_Device_T *)
     * @level:        buffer for battery level (0-100)
     *
     * Return: 0 on success, -1 on failure
     */
    int (*read_level)(void *privatedata, unsigned char *level);

    /**
     * @brief:  read battery remaining capacity
     * @privatedata:  battery device handle (Battery_Device_T *)
     * @capacity:     buffer for capacity value (mAh)
     *
     * Return: 0 on success, -1 on failure
     */
    int (*read_capacity)(void *privatedata, unsigned short *capacity);

    /**
     * @brief:  send data to battery device
     * @privatedata:  battery device handle (Battery_Device_T *)
     * @buff:         data buffer to send
     * @length:       number of bytes to send
     *
     * Return: 0 on success, -1 on failure
     */
    int (*send)(void *privatedata, unsigned char *buff, unsigned short length);

    /**
     * @brief:  read data from battery device
     * @privatedata:  battery device handle (Battery_Device_T *)
     * @info:         buffer for battery info data
     *
     * Return: 0 on success, -1 on failure
     */
    int (*read)(void *privatedata, Battery_Info_T *info);

    /* battery private data */
    void *private_data;
} Battery_Device_T;

/**
 * @brief:  get battery device structure by name
 * @name:   device name (e.g. "bat0", "bat1")
 *
 * Return: pointer to Battery_Device_T structure, NULL if not found
 */
Battery_Device_T *get_battery_device(char *name);


#ifdef __cplusplus
}
#endif

#endif  /* __BATTERY_DEV_H_ */
