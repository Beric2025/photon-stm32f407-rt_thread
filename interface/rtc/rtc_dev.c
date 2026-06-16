/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * RTC device abstraction layer
 */

#include <string.h>
#include <stdio.h>
#include "main.h"
#include "rtc_dev.h"
#include "bsp_rtc.h"
#include "log_print.h"

#define TAG "rtc dev:"

typedef struct {
    RTC_HandleTypeDef *hrtc;
    void (*init)(void);
} Rtc_Data_T;

/* function declarations */
static int rtc_init(void *privatedata);
static int rtc_get_time(void *privatedata, uint32_t *timestamp);
static int rtc_set_time(void *privatedata, uint32_t timestamp);

/* RTC hardware data */
static Rtc_Data_T s_rtc_data = {
    .hrtc = &g_rtc,
    .init = bsp_rtc_init,
};

/* RTC device instance */
static Rtc_Device_T s_rtc_dev = {
    .name = "rtc",
    .init = rtc_init,
    .get_time = rtc_get_time,
    .set_time = rtc_set_time,
    .private_data = &s_rtc_data,
};


static int rtc_init(void *privatedata)
{
    if (privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s RTC init private data is null!\n", TAG);
        return -1;
    }

    Rtc_Device_T *rtc_dev = (Rtc_Device_T *)privatedata;
    Rtc_Data_T *rtc_data = (Rtc_Data_T *)rtc_dev->private_data;

    rtc_data->init();

    LOG_PRINT(LOG_OUT_DEBUG, "%s RTC init successful!\n", TAG);

    return 0;
}

static int rtc_get_time(void *privatedata, uint32_t *timestamp)
{
    if (privatedata == NULL || timestamp == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s RTC get time private data is null!\n", TAG);
        return -1;
    }

    Rtc_Device_T *rtc_dev = (Rtc_Device_T *)privatedata;
    Rtc_Data_T *rtc_data = (Rtc_Data_T *)rtc_dev->private_data;

    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    HAL_RTC_GetTime(rtc_data->hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(rtc_data->hrtc, &sDate, RTC_FORMAT_BIN);

    /* Seconds since 2000-01-01 (valid 2000~2099, leap-year aware) */
    static const unsigned short mdays[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
    uint32_t days = sDate.Date - 1;
    days += mdays[sDate.Month - 1];
    if (sDate.Month > 2 && (sDate.Year % 4 == 0))
        days++;
    days += sDate.Year * 365 + (sDate.Year + 3) / 4;

    *timestamp = days * 86400 + sTime.Hours * 3600
               + sTime.Minutes * 60 + sTime.Seconds;

    return 0;
}

static int rtc_set_time(void *privatedata, uint32_t timestamp)
{
    if (privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s RTC set time private data is null!\n", TAG);
        return -1;
    }

    Rtc_Device_T *rtc_dev = (Rtc_Device_T *)privatedata;
    Rtc_Data_T *rtc_data = (Rtc_Data_T *)rtc_dev->private_data;

    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    sTime.Seconds = timestamp % 60;
    sTime.Minutes = (timestamp / 60) % 60;
    sTime.Hours   = (timestamp / 3600) % 24;

    /* Convert days-since-2000 back to year/month/day */
    uint32_t days = timestamp / 86400;
    uint32_t y = (days * 4 + 3) / 1461;   /* 1461 = 365*4 + 1 */
    days -= y * 365 + (y + 3) / 4;

    static const unsigned char md[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    uint32_t m;
    for (m = 0; m < 12; m++) {
        unsigned char lim = md[m];
        if (m == 1 && (y % 4 == 0)) lim = 29;
        if (days < lim) break;
        days -= lim;
    }
    sDate.Year  = y;
    sDate.Month = m + 1;
    sDate.Date  = days + 1;

    if (HAL_RTC_SetTime(rtc_data->hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
        return -1;
    if (HAL_RTC_SetDate(rtc_data->hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
        return -1;

    return 0;
}

Rtc_Device_T *get_rtc_device(char *name)
{
    Rtc_Device_T *rtc_dev = NULL;

    if (0 == strcmp(name, "rtc"))
        rtc_dev = &s_rtc_dev;

    return rtc_dev;
}
