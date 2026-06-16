/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * RNG device abstraction layer
 */

#include <string.h>
#include <stdio.h>
#include "main.h"
#include "rng_dev.h"
#include "bsp_rng.h"
#include "log_print.h"

#define TAG "rng dev:"

typedef struct {
    RNG_HandleTypeDef *hrng;
    void (*init)(void);
} Rng_Data_T;

/* function declarations */
static int rng_init(void *privatedata);
static int rng_read(void *privatedata, uint32_t *value);

/* RNG hardware data */
static Rng_Data_T s_rng_data = {
    .hrng = &g_rng,
    .init = bsp_rng_init,
};

/* RNG device instance */
static Rng_Device_T s_rng_dev = {
    .name = "rng",
    .init = rng_init,
    .read = rng_read,
    .private_data = &s_rng_data,
};


static int rng_init(void *privatedata)
{
    if (privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s RNG init private data is null!\n", TAG);
        return -1;
    }

    Rng_Device_T *rng_dev = (Rng_Device_T *)privatedata;
    Rng_Data_T *rng_data = (Rng_Data_T *)rng_dev->private_data;

    rng_data->init();

    LOG_PRINT(LOG_OUT_DEBUG, "%s RNG init successful!\n", TAG);

    return 0;
}

static int rng_read(void *privatedata, uint32_t *value)
{
    uint32_t tem = 0;

    if (privatedata == NULL || value == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s RNG read private data is null!\n", TAG);
        return -1;
    }

    Rng_Device_T *rng_dev = (Rng_Device_T *)privatedata;
    Rng_Data_T *rng_data = (Rng_Data_T *)rng_dev->private_data;

    if (HAL_RNG_GenerateRandomNumber(rng_data->hrng, &tem) != HAL_OK)
        return -1;

    *value = (uint32_t)tem;

    return 0;
}

Rng_Device_T *get_rng_device(char *name)
{
    Rng_Device_T *rng_dev = NULL;

    if (0 == strcmp(name, "rng"))
        rng_dev = &s_rng_dev;

    return rng_dev;
}
