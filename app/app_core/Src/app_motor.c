/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * Motor control task management module
 */

#include <stdio.h>
#include <string.h>
#include "main.h"
#include "motor_dev.h"
#include "app_motor.h"

#define TAG         "app motor:"


rt_mq_t g_motor_queue;

/* Task creation definitions
 * Motor work: 256*4 Bytes, priority 6
 * Motor status: 256*4 Bytes, priority 6
 */
#define         MOTOR_WORK_STK_SIZE                 256
#define         MOTOR_WORK_TASK_PRIO                6
rt_thread_t    motor_work_task_handler;
#define         MOTOR_STATUS_STK_SIZE               256
#define         MOTOR_STATUS_TASK_PRIO              6
rt_thread_t    motor_status_task_handler;

static Motor_Device_T *motor1_dev = NULL;
static Motor_Device_T *motor2_dev = NULL;

static void queue_create_function(void)
{
    g_motor_queue = rt_mq_create("q", sizeof(Motor_Info_T), 6, RT_IPC_FLAG_FIFO);
}

/* Motor work task — receives motor commands via queue and sets speed */
static void motor_work_task_function(void *parameters)
{
    parameters = parameters;

    Motor_Info_T motor1_info = {0};
    Motor_Info_T motor2_info = {0};

    while(1) {
        rt_mq_recv(g_motor_queue, &motor1_info, sizeof(Motor_Info_T), RT_WAITING_FOREVER);
        rt_mq_recv(g_motor_queue, &motor2_info, sizeof(Motor_Info_T), RT_WAITING_FOREVER);
        if(motor1_info.motor_id && motor2_info.motor_id) {
            if(motor1_info.motor_id == MOTOR_ID_1) {
                switch (motor1_info.motor_cmd) {
                    case MOTOR_CMD_SPEED:
                        motor1_dev->set_speed(motor1_dev, motor1_info.motor_speed);
                        break;
                    default:
                        break;
                }
            }
            if(motor2_info.motor_id == MOTOR_ID_2) {
                switch (motor2_info.motor_cmd) {
                    case MOTOR_CMD_SPEED:
                        motor2_dev->set_speed(motor2_dev, motor2_info.motor_speed);
                        break;
                    default:
                        break;
                }
            }
        }
    }
}

/* Motor status task — reads motor speed/position and uploads periodically */
static void motor_status_task_function(void *parameters)
{
    parameters = parameters;
    rt_tick_t xLastWakeTime;
    xLastWakeTime = rt_tick_get();

    Motor_Info_T motor1_info = {0};
    Motor_Info_T motor2_info = {0};

    while(1) {
        if(motor1_info.motor_id && motor2_info.motor_id) {
            /* Read motor speed and actual position */
            /* Upload to host */
        }
        rt_thread_mdelay(20);
    }
}

static void task_create_function(void)
{
        {
        rt_thread_t t = rt_thread_create("motor_work_task_function", motor_work_task_function,
                                         RT_NULL, MOTOR_WORK_STK_SIZE, MOTOR_WORK_TASK_PRIO, 20);
        if (t != RT_NULL) rt_thread_startup(t);
    }
        {
        rt_thread_t t = rt_thread_create("motor_status_task_function", motor_status_task_function,
                                         RT_NULL, MOTOR_STATUS_STK_SIZE, MOTOR_STATUS_TASK_PRIO, 20);
        if (t != RT_NULL) rt_thread_startup(t);
    }
}


void app_motor_init(void)
{
    motor1_dev = get_motor_device("hub_motor1");
    if(!motor1_dev) {
        LOG_PRINT(LOG_OUT_ERROR, "%s Can not find hub_motor1!\n", TAG);
        return;
    }

    if(motor1_dev->init(motor1_dev)){
        LOG_PRINT(LOG_OUT_ERROR, "%s hub_motor1 init error!\n", TAG);
        return;
    }

    motor2_dev = get_motor_device("hub_motor2");
    if(!motor2_dev) {
        LOG_PRINT(LOG_OUT_ERROR, "%s Can not find hub_motor2!\n", TAG);
        return;
    }

    if(motor2_dev->init(motor2_dev)){
        LOG_PRINT(LOG_OUT_ERROR, "%s hub_motor2 init error!\n", TAG);
        return;
    }

    queue_create_function();
    task_create_function();
}
