/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * Ultrasonic sensor task management module
 */

#include <stdio.h>
#include <string.h>
#include "main.h"
#include "ultrasonic_dev.h"
#include "app_manage.h"
#include "app_ultrasonic.h"
#include "match_mode.h"

#define TAG         "app ultrasonic:"


/* Task creation definitions
 * Work task: 256*4 Bytes, priority 3
 * Distance task: 512*4 Bytes, priority 4
 */
#define         ULTRASONIC_WORK_STK_SIZE            256
#define         ULTRASONIC_WORK_TASK_PRIO           3
rt_thread_t    ultrasonic_work_task_handler;

#define         ULTRASONIC_DISTANCE_STK_SIZE        512
#define         ULTRASONIC_DISTANCE_TASK_PRIO       4
rt_thread_t    ultrasonic_distance_task_handler;

static Ultrasonic_Device_T *ultrasonic01_dev = NULL;
static Ultrasonic_Device_T *ultrasonic02_dev = NULL;
static unsigned short s_ultrasonic01_distance = 0;
static unsigned short s_ultrasonic02_distance = 0;

/* Ultrasonic work task — reads filtered distance and uploads to host at 70ms interval */
static void ultrasonic_work_task_function(void *parameters)
{
    parameters = parameters;
    rt_tick_t last_time;
    last_time = rt_tick_get();

    uint16_t ultra01_distance = 0, ultra02_distance = 0;
    Rolling_Avg_Filter_T rolling_filter01 = {0};
    Rolling_Avg_Filter_T rolling_filter02 = {0};

    rolling_averageg_init(&rolling_filter01);
    rolling_averageg_init(&rolling_filter02);

    while(1) {
        if(ultrasonic01_dev && ultrasonic02_dev) {

            /* Read latest distance values (updated by distance task) */
            ultra01_distance = s_ultrasonic01_distance;
            ultra02_distance = s_ultrasonic02_distance;

            ultra01_distance = rolling_averageg_filter_int(&rolling_filter01, ultra01_distance);
            ultra02_distance = rolling_averageg_filter_int(&rolling_filter02, ultra02_distance);

            LOG_PRINT(LOG_OUT_INFO, "%s ultra01_distance:%d, ultra02_distance:%d\r\n", TAG,
                    ultra01_distance, ultra02_distance);

        }
        rt_thread_mdelay(70);
    }
}

/* Ultrasonic distance task — continuously reads raw distance values from sensors */
static void ultrasonic_distance_task_function(void *parameters)
{
    parameters = parameters;

    uint16_t ultra01_distance = 0, ultra02_distance = 0;

    if(ultrasonic01_dev) {
        if(ultrasonic01_dev->config(ultrasonic01_dev)){
            LOG_PRINT(LOG_OUT_ERROR, "%s Config error!\n",TAG);
            return;
        }
    }

    if(ultrasonic02_dev) {
        if(ultrasonic02_dev->config(ultrasonic02_dev)){
            LOG_PRINT(LOG_OUT_ERROR, "%s Config error!\n",TAG);
            return;
        }
    }
    while(1) {
        if(ultrasonic01_dev && ultrasonic02_dev) {
            ultra01_distance = ultrasonic01_dev->read_distance(ultrasonic01_dev);
            ultra02_distance = ultrasonic02_dev->read_distance(ultrasonic02_dev);
            s_ultrasonic01_distance = ultra01_distance;
            s_ultrasonic02_distance = ultra02_distance;

            LOG_PRINT(LOG_OUT_INFO, "%s raw:%s01_Dist:%d, 02_Dist:%d\n",TAG, s_ultrasonic01_distance, s_ultrasonic02_distance);
        }
        /* Max measurement distance is 1.5m, response time is ~140ms */
    }
}

static void task_create_function(void)
{
        {
        rt_thread_t t = rt_thread_create("ultrasonic_work_task_function", ultrasonic_work_task_function,
                                         RT_NULL, ULTRASONIC_WORK_STK_SIZE, ULTRASONIC_WORK_TASK_PRIO, 20);
        if (t != RT_NULL) rt_thread_startup(t);
    }
        {
        rt_thread_t t = rt_thread_create("ultrasonic_distance_task_function", ultrasonic_distance_task_function,
                                         RT_NULL, ULTRASONIC_DISTANCE_STK_SIZE, ULTRASONIC_DISTANCE_TASK_PRIO, 20);
        if (t != RT_NULL) rt_thread_startup(t);
    }
}


void app_ultrasonic_init(void)
{
    ultrasonic01_dev = get_ultrasonic_device("dypa21_01");
    if(!ultrasonic01_dev) {
        LOG_PRINT(LOG_OUT_ERROR, "%s Can not find dypa21_01!\n", TAG);
        return;
    }
    if(ultrasonic01_dev->init(ultrasonic01_dev)){
        LOG_PRINT(LOG_OUT_ERROR, "%s DYPA21_01 init error!\n", TAG);
        return;
    }

    ultrasonic02_dev = get_ultrasonic_device("dypa21_02");
    if(!ultrasonic02_dev) {
        LOG_PRINT(LOG_OUT_ERROR, "%s Can not find dypa21_02!\n", TAG);
        return;
    }
    if(ultrasonic02_dev->init(ultrasonic02_dev)){
        LOG_PRINT(LOG_OUT_ERROR, "%s DYPA21_02 init error!\n", TAG);
        return;
    }

    task_create_function();
}
