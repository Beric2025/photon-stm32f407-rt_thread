/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * I2S device abstraction layer
 */

#include <string.h>
#include <stdio.h>
#include "main.h"
#include "i2s_dev.h"
#include "bsp_i2s.h"
#include "log_print.h"

#define TAG "i2s dev:"

typedef struct {
    I2S_HandleTypeDef *hi2s;
    void (*init)(uint32_t sample_rate, unsigned char bits);
} I2s_Data_T;

/* function declarations */
static int i2s_init(void *privatedata, uint32_t sample_rate, unsigned char bits);
static int i2s_write(void *privatedata, unsigned short *buf, uint32_t len);
static int i2s_read(void *privatedata, unsigned short *buf, uint32_t len);
static int i2s_start(void *privatedata);
static int i2s_stop(void *privatedata);

/* I2S hardware data */
static I2s_Data_T s_i2s2_data = {
    .hi2s = &g_i2s2,
    .init = bsp_i2s2_init,
};

/* I2S device instance */
static I2s_Device_T s_i2s2_dev = {
    .name = "i2s2",
    .init = i2s_init,
    .write = i2s_write,
    .read = i2s_read,
    .start = i2s_start,
    .stop = i2s_stop,
    .private_data = &s_i2s2_data,
};


static int i2s_init(void *privatedata, uint32_t sample_rate, unsigned char bits)
{
    if (privatedata == NULL || sample_rate == 0 || bits == 0) {
        LOG_PRINT(LOG_OUT_ERROR, "%s I2S init invalid parameter!\n", TAG);
        return -1;
    }

    I2s_Device_T *i2s_dev = (I2s_Device_T *)privatedata;
    I2s_Data_T *i2s_data = (I2s_Data_T *)i2s_dev->private_data;

    i2s_data->init(sample_rate, bits);

    LOG_PRINT(LOG_OUT_DEBUG, "%s I2S init %u Hz %u-bit\n", TAG, sample_rate, bits);

    return 0;
}

static int i2s_write(void *privatedata, unsigned short *buf, uint32_t len)
{
    if (privatedata == NULL || buf == NULL || len == 0) {
        LOG_PRINT(LOG_OUT_ERROR, "%s I2S write invalid parameter!\n", TAG);
        return -1;
    }

    I2s_Device_T *i2s_dev = (I2s_Device_T *)privatedata;
    I2s_Data_T *i2s_data = (I2s_Data_T *)i2s_dev->private_data;

    if (HAL_I2S_Transmit(i2s_data->hi2s, buf, len, 1000) != HAL_OK)
        return -1;

    return 0;
}

static int i2s_read(void *privatedata, unsigned short *buf, uint32_t len)
{
    if (privatedata == NULL || buf == NULL || len == 0) {
        LOG_PRINT(LOG_OUT_ERROR, "%s I2S read invalid parameter!\n", TAG);
        return -1;
    }

    I2s_Device_T *i2s_dev = (I2s_Device_T *)privatedata;
    I2s_Data_T *i2s_data = (I2s_Data_T *)i2s_dev->private_data;

    if (HAL_I2S_Receive(i2s_data->hi2s, buf, len, 1000) != HAL_OK)
        return -1;

    return 0;
}

static int i2s_start(void *privatedata)
{
    (void)privatedata;
    /* I2S starts automatically on first transmit/receive via HAL */
    return 0;
}

static int i2s_stop(void *privatedata)
{
    (void)privatedata;
    return 0;
}

I2s_Device_T *get_i2s_device(char *name)
{
    I2s_Device_T *i2s_dev = NULL;

    if (0 == strcmp(name, "i2s2"))
        i2s_dev = &s_i2s2_dev;

    return i2s_dev;
}
