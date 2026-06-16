/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef __APP_LIGHT_H_
#define __APP_LIGHT_H_

#ifdef __cplusplus
	extern "C" {
#endif

#include "app_manage.h"

extern rt_mq_t g_light_queue;      /* light control command queue */

/**
 * @brief:  initialize light control task
 */
void app_light_init(void);

/**
 * @brief:  suspend light control task
 */
void app_light_suspend(void);

/**
 * @brief:  resume light control task
 */
void app_light_resume(void);

#ifdef __cplusplus
}
#endif

#endif  /* __APP_LIGHT_H_ */
