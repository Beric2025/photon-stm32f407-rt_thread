/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * Input capture device abstraction layer
 */

#include <string.h>
#include <stdio.h>
#include "main.h"
#include "cap_dev.h"
#include "bsp_cap.h"
#include "log_print.h"

#define TAG "cap dev:"

typedef struct {
    TIM_HandleTypeDef *htim;
    void (*init)(void);
    uint32_t captured_value[2];   /* CH1, CH2 last captured values */
    unsigned char capture_done[2];    /* CH1, CH2 capture flags */
    uint32_t prescaler;           /* timer prescaler for µs conversion */
} Cap_Data_T;

/* function declarations */
static int cap_init(void *privatedata);
static int cap_start(void *privatedata);
static int cap_stop(void *privatedata);
static int cap_get_pulse(void *privatedata, unsigned char channel, uint32_t *width_us);

/* capture hardware data */
static Cap_Data_T s_cap4_data = {
    .htim = &g_tim4,
    .init = bsp_cap4_init,
};

/* capture device instance */
static Cap_Device_T s_cap4_dev = {
    .name = "cap4",
    .channel_count = 2,
    .init = cap_init,
    .start = cap_start,
    .stop = cap_stop,
    .get_pulse = cap_get_pulse,
    .private_data = &s_cap4_data,
};


static int cap_init(void *privatedata)
{
    if (privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s cap init private data is null!\n", TAG);
        return -1;
    }

    Cap_Device_T *cap_dev = (Cap_Device_T *)privatedata;
    Cap_Data_T *cap_data = (Cap_Data_T *)cap_dev->private_data;

    cap_data->init();

    /* store prescaler for µs conversion: timer_clock = base_clk / (PSC+1) */
    cap_data->prescaler = cap_data->htim->Init.Prescaler;

    LOG_PRINT(LOG_OUT_DEBUG, "%s cap init successful!\n", TAG);

    return 0;
}

static int cap_start(void *privatedata)
{
    if (privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s cap start private data is null!\n", TAG);
        return -1;
    }

    Cap_Device_T *cap_dev = (Cap_Device_T *)privatedata;
    Cap_Data_T *cap_data = (Cap_Data_T *)cap_dev->private_data;

    unsigned char ch;
    for (ch = 0; ch < cap_dev->channel_count; ch++) {
        cap_data->capture_done[ch] = 0;
        HAL_TIM_IC_Start_IT(cap_data->htim, ch + 1);
    }

    LOG_PRINT(LOG_OUT_DEBUG, "%s cap started\n", TAG);

    return 0;
}

static int cap_stop(void *privatedata)
{
    if (privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s cap stop private data is null!\n", TAG);
        return -1;
    }

    Cap_Device_T *cap_dev = (Cap_Device_T *)privatedata;
    Cap_Data_T *cap_data = (Cap_Data_T *)cap_dev->private_data;

    unsigned char ch;
    for (ch = 0; ch < cap_dev->channel_count; ch++) {
        HAL_TIM_IC_Stop_IT(cap_data->htim, ch + 1);
    }

    LOG_PRINT(LOG_OUT_DEBUG, "%s cap stopped\n", TAG);

    return 0;
}

static int cap_get_pulse(void *privatedata, unsigned char channel, uint32_t *width_us)
{
    if (privatedata == NULL || width_us == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s cap get pulse private data is null!\n", TAG);
        return -1;
    }

    Cap_Device_T *cap_dev = (Cap_Device_T *)privatedata;
    Cap_Data_T *cap_data = (Cap_Data_T *)cap_dev->private_data;

    if (channel >= cap_dev->channel_count) {
        LOG_PRINT(LOG_OUT_ERROR, "%s cap channel %d out of range!\n", TAG, channel);
        return -1;
    }

    if (!cap_data->capture_done[channel]) {
        LOG_PRINT(LOG_OUT_ERROR, "%s cap channel %d no capture data!\n", TAG, channel);
        return -1;
    }

    /* captured ticks * (psc+1) / (TIM_CLK / 1e6) = µs */
    *width_us = (uint32_t)((unsigned long long)cap_data->captured_value[channel]
                * (cap_data->prescaler + 1) * 1000000UL / CAP_TIMER_CLK_HZ);

    /* clear flag for next capture */
    cap_data->capture_done[channel] = 0;

    return 0;
}


/* ====================================================================
 * cap_dev_on_event — single entry point from BSP HAL callbacks
 *
 * Stores captured value and sets done flag.
 * Runs in ISR context — keep it short.
 * ==================================================================== */
void cap_dev_on_event(void *htim_void, uint32_t channel)
{
    (void)channel;  /* use htim->Channel instead — HAL sets it before callback */
    TIM_HandleTypeDef *htim = (TIM_HandleTypeDef *)htim_void;

    if (htim == s_cap4_data.htim) {
        unsigned char ch;
        for (ch = 0; ch < 2; ch++) {
            if (htim->Channel == (HAL_TIM_ACTIVE_CHANNEL_1 << ch)) {
                s_cap4_data.captured_value[ch] = HAL_TIM_ReadCapturedValue(htim, ch + 1);
                s_cap4_data.capture_done[ch] = 1;
                break;
            }
        }
    }
}

Cap_Device_T *get_cap_device(char *name)
{
    Cap_Device_T *cap_dev = NULL;

    if (0 == strcmp(name, "cap4"))
        cap_dev = &s_cap4_dev;

    return cap_dev;
}
