/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef _PWM_DEV_H_
#define _PWM_DEV_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>


typedef struct {
    /* device identifier (e.g. "tim3") */
    char *name;    
    
    /* number of PWM channels */
    unsigned char channel_count;   

    /**
     * @brief:  initialize PWM with target frequency
     * @privatedata:  pointer to this Pwm_Device_T
     * @freq_hz:      PWM frequency in Hz
     *
     * Return: 0 on success, -1 on failure
     */
    int (*init)(void *privatedata, uint32_t freq_hz);

    /**
     * @brief:  set duty cycle for a channel
     * @privatedata:   pointer to this Pwm_Device_T
     * @channel:       channel index (0 to channel_count-1)
     * @duty_percent:  duty cycle 0-100
     *
     * Return: 0 on success, -1 on failure
     */
    int (*set_duty)(void *privatedata, unsigned char channel, uint32_t duty_percent);

    /**
     * @brief:  start PWM output on all channels
     * @privatedata:  pointer to this Pwm_Device_T
     *
     * Return: 0 on success, -1 on failure
     */
    int (*start)(void *privatedata);

    /**
     * @brief:  stop PWM output on all channels
     * @privatedata:  pointer to this Pwm_Device_T
     *
     * Return: 0 on success, -1 on failure
     */
    int (*stop)(void *privatedata);

    /**
     * @brief:  read current timer counter value
     * @privatedata:  pointer to this Pwm_Device_T
     * @count:        output buffer for CNT value
     *
     * Return: 0 on success, -1 on failure
     */
    int (*get_count)(void *privatedata, uint32_t *count);

    void *private_data;
} Pwm_Device_T;

/**
 * @brief:  get PWM device structure by name
 * @name:   device name (e.g. "pwm3")
 *
 * Return: pointer to Pwm_Device_T structure, NULL if not found
 */
Pwm_Device_T *get_pwm_device(char *name);

#ifdef __cplusplus
}
#endif

#endif /* _PWM_DEV_H_ */
