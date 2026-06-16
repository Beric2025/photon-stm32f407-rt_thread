/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * PWM device abstraction layer
 */

#include <string.h>
#include <stdio.h>
#include "main.h"
#include "pwm_dev.h"
#include "bsp_pwm.h"
#include "log_print.h"

#define TAG "pwm dev:"

typedef struct {
    TIM_HandleTypeDef *htim;
    void (*init)(uint32_t freq_hz);
    uint32_t freq_hz;
    uint32_t arr;
} Pwm_Data_T;

/* function declarations */
static int pwm_init(void *privatedata, uint32_t freq_hz);
static int pwm_set_duty(void *privatedata, unsigned char channel, uint32_t duty_percent);
static int pwm_start(void *privatedata);
static int pwm_stop(void *privatedata);
static int pwm_get_count(void *privatedata, uint32_t *count);

/* PWM hardware data */
static Pwm_Data_T s_pwm3_data = {
    .htim = &g_tim3,
    .init = bsp_pwm3_init,
};

static Pwm_Data_T s_tim5_data = {
    .htim = &g_tim5,
    .init = bsp_tim5_init,
};

/* PWM device instances */
static Pwm_Device_T s_pwm3_dev = {
    .name = "pwm3",
    .channel_count = 4,
    .init = pwm_init,
    .set_duty = pwm_set_duty,
    .start = pwm_start,
    .stop = pwm_stop,
    .get_count = pwm_get_count,
    .private_data = &s_pwm3_data,
};

static Pwm_Device_T s_tim5_dev = {
    .name = "tim5",
    .channel_count = 0,
    .init = pwm_init,
    .set_duty = pwm_set_duty,
    .start = pwm_start,
    .stop = pwm_stop,
    .get_count = pwm_get_count,
    .private_data = &s_tim5_data,
};


static int pwm_init(void *privatedata, uint32_t freq_hz)
{
    if (privatedata == NULL || freq_hz == 0) {
        LOG_PRINT(LOG_OUT_ERROR, "%s PWM init invalid parameter!\n", TAG);
        return -1;
    }

    Pwm_Device_T *pwm_dev = (Pwm_Device_T *)privatedata;
    Pwm_Data_T *pwm_data = (Pwm_Data_T *)pwm_dev->private_data;

    pwm_data->init(freq_hz);
    pwm_data->freq_hz = freq_hz;
    pwm_data->arr = pwm_data->htim->Init.Period;

    LOG_PRINT(LOG_OUT_DEBUG, "%s PWM init %u Hz, ARR=%u\n", TAG, freq_hz, pwm_data->arr);

    return 0;
}

static int pwm_set_duty(void *privatedata, unsigned char channel, uint32_t duty_percent)
{
    if (privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s PWM set duty private data is null!\n", TAG);
        return -1;
    }

    Pwm_Device_T *pwm_dev = (Pwm_Device_T *)privatedata;
    Pwm_Data_T *pwm_data = (Pwm_Data_T *)pwm_dev->private_data;

    if (channel >= pwm_dev->channel_count) {
        LOG_PRINT(LOG_OUT_ERROR, "%s PWM channel %d out of range!\n", TAG, channel);
        return -1;
    }

    if (duty_percent > 100) {
        duty_percent = 100;
    }

    uint32_t pulse = (duty_percent * (pwm_data->arr + 1)) / 100;
    __HAL_TIM_SET_COMPARE(pwm_data->htim, channel + 1, pulse);

    return 0;
}

static int pwm_start(void *privatedata)
{
    if (privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s PWM start private data is null!\n", TAG);
        return -1;
    }

    Pwm_Device_T *pwm_dev = (Pwm_Device_T *)privatedata;
    Pwm_Data_T *pwm_data = (Pwm_Data_T *)pwm_dev->private_data;

    if (pwm_dev->channel_count == 0) {
        HAL_TIM_Base_Start(pwm_data->htim);
    } else {
        unsigned char ch;
        for (ch = 0; ch < pwm_dev->channel_count; ch++) {
            HAL_TIM_PWM_Start(pwm_data->htim, ch + 1);
        }
    }

    LOG_PRINT(LOG_OUT_DEBUG, "%s PWM started\n", TAG);

    return 0;
}

static int pwm_stop(void *privatedata)
{
    if (privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s PWM stop private data is null!\n", TAG);
        return -1;
    }

    Pwm_Device_T *pwm_dev = (Pwm_Device_T *)privatedata;
    Pwm_Data_T *pwm_data = (Pwm_Data_T *)pwm_dev->private_data;

    if (pwm_dev->channel_count == 0) {
        HAL_TIM_Base_Stop(pwm_data->htim);
    } else {
        unsigned char ch;
        for (ch = 0; ch < pwm_dev->channel_count; ch++) {
            HAL_TIM_PWM_Stop(pwm_data->htim, ch + 1);
        }
    }

    LOG_PRINT(LOG_OUT_DEBUG, "%s PWM stopped\n", TAG);

    return 0;
}

static int pwm_get_count(void *privatedata, uint32_t *count)
{
    if (privatedata == NULL || count == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s PWM get count private data is null!\n", TAG);
        return -1;
    }

    Pwm_Device_T *pwm_dev = (Pwm_Device_T *)privatedata;
    Pwm_Data_T *pwm_data = (Pwm_Data_T *)pwm_dev->private_data;

    *count = __HAL_TIM_GET_COUNTER(pwm_data->htim);

    return 0;
}

Pwm_Device_T *get_pwm_device(char *name)
{
    Pwm_Device_T *pwm_dev = NULL;

    if (0 == strcmp(name, "pwm3"))
        pwm_dev = &s_pwm3_dev;

    if (0 == strcmp(name, "tim5"))
        pwm_dev = &s_tim5_dev;

    return pwm_dev;
}
