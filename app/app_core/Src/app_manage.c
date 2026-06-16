/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * Task management module — RT-Thread version
 */

#include <stdio.h>
#include <string.h>
#include <rtthread.h>
#include "main.h"
#include "app_manage.h"
#include "app_version.h"
#include "lwip_port.h"
#include "app_led.h"
#include "app_network.h"
#include "app_light.h"
#include "app_motor.h"
#include "app_battery.h"
#include "app_ultrasonic.h"
#include "app_adc_battery.h"
#include "bsp_can.h"

#define TAG         "app manage: "


/* OS running status: 0-not running, 1-running */
uint8_t is_os_running = 0;


static void device_init(void)
{
    char version[100] = {0};

    log_print_init();

    build_version();
    get_version(version, sizeof(version));
    LOG_PRINT(LOG_OUT_INFO, "%sApp Ver:%s\n", TAG, version);
}


static void task_init(void)
{
    device_init();
    app_network_init();
    rt_enter_critical();
    app_led_init();
    app_light_init();
    app_motor_init();
    app_battery_init();
    app_adc_battery_init();
    app_ultrasonic_init();
    rt_exit_critical();
}


static void start_task_function(void *parameters)
{
    (void)parameters;

    task_init();

    /* Self-delete when done */
    rt_thread_delete(rt_thread_self());
}


void application_init(void)
{
    rt_thread_t tid;

    tid = rt_thread_create("init", start_task_function, RT_NULL,
                           512, 0, 20);
    if (tid != RT_NULL) {
        rt_thread_startup(tid);
    }

    is_os_running = 1;
}
