/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef _MOTOR_DEV_H_
#define _MOTOR_DEV_H_

#ifdef __cplusplus
 extern "C" {
#endif
#include <stdint.h>

typedef struct {
    float power;            /* motor power percentage */
    int speed;              /* motor speed (RPM) */
    uint32_t position;  /* motor position (encoder ticks) */
    int error_code;         /* motor error code */
    float voltage;          /* motor voltage (V) */
    float current;          /* motor current (A) */
    float temperature;      /* motor temperature (C) */
    float distance;         /* motor travel distance */
} Motor_Status_T;

typedef void (*motor_step_cb)(void *user_data, unsigned short step_count);
typedef void (*motor_hall_cb)(void *user_data, unsigned char hall_state);
typedef void (*motor_fault_cb)(void *user_data, unsigned char fault_code);

typedef struct {
    motor_step_cb   on_step;
    motor_hall_cb   on_hall;
    motor_fault_cb  on_fault;
    void            *user_data;
} Motor_Callbacks_T;

typedef struct {
    /* device name (e.g. "motor1") */
    char *name;

    /**
     * @brief:  initialize motor device
     * @privatedata:  motor handle (Motor_Device_T *)
     *
     * Return: 0 on success, -1 on failure
     */
    int (*init)(void *privatedata);

    /**
     * @brief:  set motor speed
     * @privatedata:  motor handle (Motor_Device_T *)
     * @speed:        target speed (RPM)
     *
     * Return: 0 on success, -1 on failure
     */
    int (*set_speed)(void *privatedata, int speed);

    /**
     * @brief:  set motor position
     * @privatedata:  motor handle (Motor_Device_T *)
     * @position:     target position (encoder ticks)
     *
     * Return: 0 on success, -1 on failure
     */
    int (*set_position)(void *privatedata, uint32_t position);

    /**
     * @brief:  start motor
     * @privatedata:  motor handle (Motor_Device_T *)
     * @direction:    0-forward, 1-reverse
     *
     * Return: 0 on success, -1 on failure
     */
    int (*start)(void *privatedata, unsigned char direction);

    /**
     * @brief:  stop motor
     * @privatedata:  motor handle (Motor_Device_T *)
     *
     * Return: 0 on success, -1 on failure
     */
    int (*stop)(void *privatedata);

    /**
     * @brief:  parking motor
     * @privatedata:  motor handle (Motor_Device_T *)
     *
     * Return: 0 on success, -1 on failure
     */
    int (*parking)(void *privatedata);

    /**
     * @brief:  read motor speed
     * @privatedata:  motor handle (Motor_Device_T *)
     * @speed:        buffer to store speed value
     *
     * Return: 0 on success, -1 on failure
     */
    int (*read_speed)(void *privatedata, int *speed);

    /**
     * @brief:  read motor position
     * @privatedata:  motor handle (Motor_Device_T *)
     * @position:     buffer to store position value
     *
     * Return: 0 on success, -1 on failure
     */
    int (*read_position)(void *privatedata, uint32_t *position);

    /**
     * @brief:  read motor error code
     * @privatedata:  motor handle (Motor_Device_T *)
     * @error_code:   buffer to store error code
     *
     * Return: 0 on success, -1 on failure
     */
    int (*read_error_code)(void *privatedata, int *error_code);

    /**
     * @brief:  read raw data from motor device
     * @privatedata:  motor handle (Motor_Device_T *)
     * @status:       buffer to store motor status
     *
     * Return: number of bytes read, 0 on failure
     */
    int (*read)(void *privatedata, Motor_Status_T *status);

    /**
     * @brief:  register motor callbacks
     * @privatedata:  motor handle (Motor_Device_T *)
     * @cbs:          callback function table
     *
     * Return: 0 on success, -1 on failure
     */
    int (*register_callbacks)(void *privatedata, Motor_Callbacks_T *cbs);

    /* motor private data */
    void *private_data;
} Motor_Device_T;

/**
 * @brief:  get motor device structure by name
 * @name:   device name (e.g. "motor1")
 *
 * Return: pointer to Motor_Device_T structure, NULL if not found
 */
Motor_Device_T *get_motor_device(char *name);

#ifdef __cplusplus
}
#endif

#endif /* _MOTOR_DEV_H_ */
