/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef __APP_MOTOR_H_
#define __APP_MOTOR_H_

#ifdef __cplusplus
	extern "C" {
#endif

#include "app_manage.h"

typedef struct
{
    unsigned char  motor_id;         /* motor ID (1 or 2) */
    unsigned char  motor_cmd;        /* motor command */
    int motor_speed;                 /* motor speed */
} Motor_Info_T;

typedef enum
{
    MOTOR_ID_1 = 1,                  /* motor 1 */
    MOTOR_ID_2,                      /* motor 2 */
} Motor_Id_E;

typedef enum
{
    MOTOR_CMD_SPEED = 1              /* speed control command */
} Motor_Cmd_E;

extern rt_mq_t g_motor_queue;      /* motor control command queue */

/**
 * @brief:  initialize motor control task
 */
void app_motor_init(void);

#ifdef __cplusplus
}
#endif

#endif  /* __APP_MOTOR_H_ */
