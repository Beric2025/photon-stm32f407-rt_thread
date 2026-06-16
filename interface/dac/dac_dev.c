/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * DAC device abstraction layer
 */

#include <string.h>
#include <stdio.h>
#include "main.h"
#include "dac_dev.h"
#include "bsp_dac.h"
#include "log_print.h"

#define TAG "dac dev:"

typedef struct {
    DAC_HandleTypeDef *hdac;
    void (*init)(void);
} Dac_Data_T;

/* function declarations */
static int dac_init(void *privatedata);
static int dac_write(void *privatedata, unsigned char channel, uint32_t value);
static int dac_start(void *privatedata);
static int dac_stop(void *privatedata);

/* DAC hardware data */
static Dac_Data_T s_dac1_data = {
    .hdac = &g_dac1,
    .init = bsp_dac1_init,
};

/* DAC device instance */
static Dac_Device_T s_dac1_dev = {
    .name = "dac1",
    .channel_count = 2,
    .resolution = 12,
    .init = dac_init,
    .write = dac_write,
    .start = dac_start,
    .stop = dac_stop,
    .private_data = &s_dac1_data,
};


static int dac_init(void *privatedata)
{
    if (privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s DAC init private data is null!\n", TAG);
        return -1;
    }

    Dac_Device_T *dac_dev = (Dac_Device_T *)privatedata;
    Dac_Data_T *dac_data = (Dac_Data_T *)dac_dev->private_data;

    dac_data->init();

    LOG_PRINT(LOG_OUT_DEBUG, "%s DAC init successful!\n", TAG);

    return 0;
}

static int dac_write(void *privatedata, unsigned char channel, uint32_t value)
{
    if (privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s DAC write private data is null!\n", TAG);
        return -1;
    }

    Dac_Device_T *dac_dev = (Dac_Device_T *)privatedata;
    Dac_Data_T *dac_data = (Dac_Data_T *)dac_dev->private_data;

    uint32_t dac_channel[] = { DAC_CHANNEL_1, DAC_CHANNEL_2 };

    if (channel >= dac_dev->channel_count) {
        LOG_PRINT(LOG_OUT_ERROR, "%s DAC channel %d out of range!\n", TAG, channel);
        return -1;
    }

    if (HAL_DAC_SetValue(dac_data->hdac, dac_channel[channel], DAC_ALIGN_12B_R, value) != HAL_OK)
        return -1;

    return 0;
}

static int dac_start(void *privatedata)
{
    if (privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s DAC start private data is null!\n", TAG);
        return -1;
    }

    Dac_Device_T *dac_dev = (Dac_Device_T *)privatedata;
    Dac_Data_T *dac_data = (Dac_Data_T *)dac_dev->private_data;

    uint32_t dac_channel[] = { DAC_CHANNEL_1, DAC_CHANNEL_2 };
    unsigned char ch;
    for (ch = 0; ch < dac_dev->channel_count; ch++) {
        HAL_DAC_Start(dac_data->hdac, dac_channel[ch]);
    }

    LOG_PRINT(LOG_OUT_DEBUG, "%s DAC started\n", TAG);

    return 0;
}

static int dac_stop(void *privatedata)
{
    if (privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s DAC stop private data is null!\n", TAG);
        return -1;
    }

    Dac_Device_T *dac_dev = (Dac_Device_T *)privatedata;
    Dac_Data_T *dac_data = (Dac_Data_T *)dac_dev->private_data;

    uint32_t dac_channel[] = { DAC_CHANNEL_1, DAC_CHANNEL_2 };
    unsigned char ch;
    for (ch = 0; ch < dac_dev->channel_count; ch++) {
        HAL_DAC_Stop(dac_data->hdac, dac_channel[ch]);
    }

    LOG_PRINT(LOG_OUT_DEBUG, "%s DAC stopped\n", TAG);

    return 0;
}

Dac_Device_T *get_dac_device(char *name)
{
    Dac_Device_T *dac_dev = NULL;

    if (0 == strcmp(name, "dac1"))
        dac_dev = &s_dac1_dev;

    return dac_dev;
}
