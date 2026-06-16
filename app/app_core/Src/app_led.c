/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * LED indicator task management module
 */

#include <stdio.h>
#include <string.h>
#include "main.h"
#include "gpio_dev.h"
#include "app_manage.h"
#include "app_led.h"

#define TAG         "app led: "


/* 
* Task creation definitions
* Task stack size: 100*4 Bytes 
* Task priority: 1 (higher than idle task)
* @rt_thread_t led_work_task_handle
*/
#define         LED_WORK_STK_SIZE 		    		100     
#define         LED_WORK_TASK_PRIO		    		1
rt_thread_t    led_work_task_handler;

/* LED GPIO definition */
static Gpio_Device_T *power_led_dev = NULL;
static Gpio_Device_T *run_led_dev = NULL;

static void led_work_task_function(void *parameters)
{
	parameters = parameters;

	while(1) {
		if(run_led_dev) {
            run_led_dev->toggle_pin(run_led_dev);
		}
		rt_thread_mdelay(500);
	}
}

static void task_create_function(void)
{
    {
        rt_thread_t t = rt_thread_create("LEDWorkTaskFunction", led_work_task_function,
                                         RT_NULL, LED_WORK_STK_SIZE, LED_WORK_TASK_PRIO, 20);
        if (t != RT_NULL) rt_thread_startup(t);
    }
}

void app_led_init(void)
{
    /* Power LED */
	power_led_dev = get_gpio_device("gpiob_pin1");
	if(!power_led_dev) {
		LOG_PRINT(LOG_OUT_ERROR, "%sFind gpiob_pin1 error!\n",TAG);
        return;
    }

	if(power_led_dev->init(power_led_dev)){
		LOG_PRINT(LOG_OUT_ERROR, "%sgpiob_pin1 init error!\n",TAG);
		return;
	}
    power_led_dev->write_pin(power_led_dev, 1);

    /* Run LED */
	run_led_dev = get_gpio_device("gpioe_pin6");
	if(!run_led_dev) {
		LOG_PRINT(LOG_OUT_ERROR, "%sFind gpioe_pin6 error!\n",TAG);
        return;
    }

	if(run_led_dev->init(run_led_dev)){
		LOG_PRINT(LOG_OUT_ERROR, "%sgpioe_pin6 init error!\n",TAG);
		return;
	}

	task_create_function();
}



