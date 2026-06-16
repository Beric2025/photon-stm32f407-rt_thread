/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * Low-power device abstraction layer
 */

#include <string.h>
#include <stdio.h>
#include "main.h"
#include "lp_dev.h"
#include "log_print.h"

#define TAG "lp dev:"

typedef struct {
    unsigned char inited;
} Lp_Data_T;

/* function declarations */
static int lp_enter(void *privatedata, unsigned char mode);
static int lp_set_wakeup(void *privatedata, uint32_t source, unsigned char enable);

/* LP hardware data */
static Lp_Data_T s_lp_data = {
    .inited = 0,
};

/* LP device instance */
static LowPower_Device_T s_lp_dev = {
    .name = "lp",
    .enter = lp_enter,
    .set_wakeup = lp_set_wakeup,
    .private_data = &s_lp_data,
};


static int lp_enter(void *privatedata, unsigned char mode)
{
    if (privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s LP enter private data is null!\n", TAG);
        return -1;
    }

    (void)privatedata;

    switch (mode) {
    case LP_MODE_SLEEP:
        HAL_SuspendTick();
        HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
        break;
    case LP_MODE_STOP:
        HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
        break;
    case LP_MODE_STANDBY:
        HAL_PWR_EnterSTANDBYMode();
        break;
    default:
        LOG_PRINT(LOG_OUT_ERROR, "%s LP unknown mode %d!\n", TAG, mode);
        return -1;
    }

    return 0;
}

static int lp_set_wakeup(void *privatedata, uint32_t source, unsigned char enable)
{
    (void)privatedata;

    switch (source) {
    case LP_WAKEUP_EXTI:
        if (enable) {
            HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);
        } else {
            HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);
        }
        break;
    case LP_WAKEUP_RTC:
        /* RTC wakeup is configured in RTC BSP when alarm is set */
        break;
    default:
        LOG_PRINT(LOG_OUT_ERROR, "%s LP unknown wakeup source %u!\n", TAG, source);
        return -1;
    }

    return 0;
}

LowPower_Device_T *get_lp_device(char *name)
{
    LowPower_Device_T *lp_dev = NULL;

    if (0 == strcmp(name, "lp"))
        lp_dev = &s_lp_dev;

    return lp_dev;
}
