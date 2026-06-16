/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * Independent watchdog device abstraction layer
 */

#include <string.h>
#include <stdio.h>
#include "main.h"
#include "wdg_dev.h"
#include "bsp_wdg.h"
#include "log_print.h"

#define TAG "wdg dev:"

typedef struct {
    IWDG_HandleTypeDef *hiwdg;
    void (*init)(uint32_t timeout_ms);
} Wdg_Data_T;

/* function declarations */
static int wdg_init(void *privatedata, uint32_t timeout_ms);
static int wdg_refresh(void *privatedata);

/* IWDG hardware data */
static Wdg_Data_T s_iwdg_data = {
    .hiwdg = &g_iwdg,
    .init = bsp_iwdg_init,
};

/* IWDG device instance */
static Wdg_Device_T s_iwdg_dev = {
    .name = "iwdg",
    .init = wdg_init,
    .refresh = wdg_refresh,
    .private_data = &s_iwdg_data,
};


static int wdg_init(void *privatedata, uint32_t timeout_ms)
{
    if (privatedata == NULL || timeout_ms == 0) {
        LOG_PRINT(LOG_OUT_ERROR, "%s IWDG init invalid parameter!\n", TAG);
        return -1;
    }

    Wdg_Device_T *wdg_dev = (Wdg_Device_T *)privatedata;
    Wdg_Data_T *wdg_data = (Wdg_Data_T *)wdg_dev->private_data;

    wdg_data->init(timeout_ms);

    LOG_PRINT(LOG_OUT_DEBUG, "%s IWDG init %u ms\n", TAG, timeout_ms);

    return 0;
}

static int wdg_refresh(void *privatedata)
{
    if (privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s IWDG refresh private data is null!\n", TAG);
        return -1;
    }

    Wdg_Device_T *wdg_dev = (Wdg_Device_T *)privatedata;
    Wdg_Data_T *wdg_data = (Wdg_Data_T *)wdg_dev->private_data;

    HAL_IWDG_Refresh(wdg_data->hiwdg);

    return 0;
}

Wdg_Device_T *get_wdg_device(char *name)
{
    Wdg_Device_T *wdg_dev = NULL;

    if (0 == strcmp(name, "iwdg"))
        wdg_dev = &s_iwdg_dev;

    return wdg_dev;
}
