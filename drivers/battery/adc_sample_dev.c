/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * ADC-sampled battery device driver
 */

#include <string.h>
#include "app_manage.h"
#include "battery_dev.h"
#include "delay_dev.h"
#include "log_print.h"
#include "adc_dev.h"

#define TAG "adc_sample: "

/* ADC hardware parameters */
#define BATTERY_ADC_MAX_VALUE   0x0FFF           /* 12-bit ADC full-scale raw value */
#define BATTERY_ADC_FULL_SCALE  3.3f             /* ADC reference voltage (V) */

/* resistor divider network */
#define BATTERY_R10             10000.0f         /* R10 resistance (ohms) */
#define BATTERY_R15             10000.0f         /* R15 resistance (ohms) */
#define BATTERY_DIVIDER_RATIO   (BATTERY_R15 / (BATTERY_R10 + BATTERY_R15))  /* = 0.5 */

/* battery voltage thresholds */
#define BATTERY_VBAT_MAX        4.2f             /* fully charged voltage (V) */
#define BATTERY_VBAT_MIN        3.0f             /* under-voltage cutoff (V) */

/* ADC full-scale / divider ratio = 3.3V / 0.5 = 6.6V */
#define BATTERY_VOLTAGE_MULTIPLIER (BATTERY_ADC_FULL_SCALE / BATTERY_DIVIDER_RATIO)

/* battery level enum */
typedef enum {
    BATTERY_LEVEL_EMPTY = 0,
    BATTERY_LEVEL_LOW,
    BATTERY_LEVEL_MEDIUM,
    BATTERY_LEVEL_HIGH,
    BATTERY_LEVEL_FULL
} Battery_Level_T;

/* private data structure */
typedef struct {
    char *name;
    Adc_Device_T *adc;
    unsigned char init_flag;
} Battery_Data_T;

static float s_battery_voltage = 0.0f;

/* forward declarations */
static int battery_init(void *privatedata);
static int battery_read_voltage(void *privatedata, unsigned short *voltage);
static int battery_read_level(void *privatedata, unsigned char *level);
static int battery_read_capacity(void *privatedata, unsigned short *capacity);
static int battery_read_info(void *privatedata, Battery_Info_T *info);

/* battery private data instance */
static Battery_Data_T s_battery_data = {
    .name = "adc1",
    .adc = NULL,
    .init_flag = 0
};

/* battery device instance */
static Battery_Device_T s_battery_device = {
    .name = "adc_battery",
    .init = battery_init,
    .read_voltage = battery_read_voltage,
    .read_level = battery_read_level,
    .read_capacity = battery_read_capacity,
    .read = battery_read_info, 
    .private_data = &s_battery_data
};

/*
 * Initialize battery device: resolve and initialize the underlying ADC.
 * Returns 0 on success, -1 on failure.
 */
static int battery_init(void *privatedata)
{
    if (privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%sBattery init private data is null!\n", TAG);
        return -1;
    }

    Battery_Device_T *battery_dev = (Battery_Device_T *)privatedata;
    Battery_Data_T *battery_data = (Battery_Data_T *)battery_dev->private_data;

    if (battery_data->init_flag == 1) {
        LOG_PRINT(LOG_OUT_INFO, "%sBattery init already done!\n", TAG);
        return 0;
    }

    /* resolve ADC device by name */
    battery_data->adc = get_adc_device(battery_data->name);

    if (battery_data->adc == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%sFailed to get ADC instance\n", TAG);
        return -1;
    }

    /* initialize underlying ADC */
    if (battery_data->adc->init(battery_data->adc) != 0) {
        LOG_PRINT(LOG_OUT_ERROR, "%sFailed to initialize ADC\n", TAG);
        return -1;
    }

    battery_data->init_flag = 1;

    return 0;
}

/*
 * Read battery voltage from ADC and convert to mV.
 * Voltage = (ADC raw / 4095) * 3.3V / divider_ratio
 *         = (ADC raw / 4095) * 6.6V
 * Returns 0 on success, -1 on failure.
 */
static int battery_read_voltage(void *privatedata, unsigned short *voltage)
{
    if (privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%sBattery read private data is null!\n", TAG);
        return -1;
    }

    Battery_Device_T *battery_dev = (Battery_Device_T *)privatedata;
    Battery_Data_T *battery_data = (Battery_Data_T *)battery_dev->private_data;
    unsigned short data = 0;
    float vbat = 0.0f;

    if (battery_data->init_flag == 0) {
        LOG_PRINT(LOG_OUT_ERROR, "%sBattery device not initialized!\n", TAG);
        return -1;
    }

    /* read raw ADC buffer */
    if (battery_data->adc->read_raw(battery_data->adc, 0, &data) != 0) {
        LOG_PRINT(LOG_OUT_ERROR, "%sFailed to read battery voltage!\n", TAG);
        return -1;
    }

    if (data == 0) {
        *voltage = 0;
        return 0;
    }

    vbat = (float)data * BATTERY_VOLTAGE_MULTIPLIER / BATTERY_ADC_MAX_VALUE;

    /* convert to mV and cache */
    *voltage = (unsigned short)(vbat * 1000.0f);
    s_battery_voltage = *voltage;

    return 0;
}


/*
 * Read battery level as a percentage (0-100%).
 * Uses linear interpolation between VBAT_MIN (0%) and VBAT_MAX (100%).
 * Returns 0 on success, -1 on failure.
 */
static int battery_read_level(void *privatedata, unsigned char *level)
{
    if (privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%sBattery level private data is null!\n", TAG);
        return -1;
    }

    Battery_Device_T *battery_dev = (Battery_Device_T *)privatedata;
    Battery_Data_T *battery_data = (Battery_Data_T *)battery_dev->private_data;
    unsigned short voltage = 0;
    float vbat = 0.0f;
    unsigned char capacity_percent = 0;

    if (battery_data->init_flag == 0) {
        LOG_PRINT(LOG_OUT_ERROR, "%sBattery device not initialized\n", TAG);
        return -1;
    }

    if (s_battery_voltage == 0) {
        /* read battery voltage on first call */
        if (battery_read_voltage(battery_dev, &voltage) != 0) {
            LOG_PRINT(LOG_OUT_ERROR, "%sFailed to read battery voltage!\n", TAG);
            return -1;
        }
    } else {
        voltage = s_battery_voltage;
    }

    vbat = voltage / 1000.0f;

    /* clamp to valid voltage range and interpolate */
    if (vbat <= BATTERY_VBAT_MIN) {
        capacity_percent = 0;
    } else if (vbat >= BATTERY_VBAT_MAX) {
        capacity_percent = 100;
    } else {
        capacity_percent = (vbat - BATTERY_VBAT_MIN) / (BATTERY_VBAT_MAX - BATTERY_VBAT_MIN) * 100;
    }

    *level = (unsigned char)capacity_percent;

    return 0;
}

static int battery_read_capacity(void *privatedata, unsigned short *capacity)
{
    (void)privatedata;
    *capacity = 0;
    return 0;
}
static int battery_read_info(void *privatedata, Battery_Info_T *info)
{
    battery_read_voltage(privatedata, &info->voltage);
    battery_read_capacity(privatedata, &info->capacity);
    battery_read_level(privatedata, &info->electricity);
    
    return 0;
}

/*
 * Get the ADC-sampled battery device instance.
 */
Battery_Device_T *get_battery1_dev(void)
{
    return &s_battery_device;
}
