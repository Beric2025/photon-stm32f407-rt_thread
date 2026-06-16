/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * Battery monitoring task management module
 */

#include <stdio.h>
#include <string.h>
#include "main.h"
#include "app_manage.h"
#include "app_battery.h"
#include "match_mode.h"
#include "battery_dev.h"

#define TAG         "app battery:"


/* Task creation definitions
 * Task stack size: 512*4 Bytes
 * Task priority: 3
 * @rt_thread_t battery_work_task_handler
 */
#define         BATTERY_WORK_STK_SIZE              512
#define         BATTERY_WORK_TASK_PRIO             3
rt_thread_t    battery_work_task_handler;

static Battery_Device_T *batarry_dev = NULL;
static Battery_Info_T s_battery_status = {0};

/* Battery work task — reads battery status and uploads to host every 500ms */
static void battery_work_task_function(void *parameters)
{
    parameters = parameters;
    rt_tick_t las_time;
    las_time = rt_tick_get();

    unsigned short size = 0;
    unsigned short voltage;
    unsigned char capacity;
    short total_current;

    Rolling_Avg_Filter_T rollingFilterVoltage = {0};
    Rolling_Avg_Filter_T rollingFilterCapacity = {0};
    Rolling_Avg_Filter_T rollingFilterTotalCurrent = {0};

    rolling_averageg_init(&rollingFilterVoltage);
    rolling_averageg_init(&rollingFilterCapacity);
    rolling_averageg_init(&rollingFilterTotalCurrent);

    while(1) {
        if(batarry_dev) {
            memset(&s_battery_status, 0, sizeof(Battery_Info_T));

            /* Read battery data periodically at 500ms interval */
            size = batarry_dev->read(batarry_dev, (Battery_Info_T *)&s_battery_status);
            if(size) {
                voltage = rolling_averageg_filter_int(&rollingFilterVoltage, s_battery_status.voltage*100);
                capacity = rolling_averageg_filter_int(&rollingFilterCapacity, s_battery_status.capacity/s_battery_status.capacity_max*100);
                total_current = rolling_averageg_filter_int(&rollingFilterTotalCurrent, s_battery_status.total_current);

                LOG_PRINT(LOG_OUT_INFO, "%s voltage:%d, capacity:%d, capacity_max:%d, total_current:%d\r\n", TAG,
                    voltage, capacity, s_battery_status.capacity_max, total_current);
            }
        }
        rt_thread_mdelay(500);
    }
}

static void task_create_function(void)
{
    /* Create battery work task */
        {
        rt_thread_t t = rt_thread_create("battery_work_task_function", battery_work_task_function,
                                         RT_NULL, BATTERY_WORK_STK_SIZE, BATTERY_WORK_TASK_PRIO, 20);
        if (t != RT_NULL) rt_thread_startup(t);
    }
}

void app_battery_init(void)
{
    batarry_dev = get_battery_device("xingneng");
    if(!batarry_dev) {
        LOG_PRINT(LOG_OUT_ERROR, "%s Can not find xingneng!\n", TAG);
        return;
    }
    if(batarry_dev->init(batarry_dev)){
        LOG_PRINT(LOG_OUT_ERROR, "%sxingneng init error!\n",TAG);
        return;
    }

    task_create_function();
}
