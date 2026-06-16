/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef __APP_MANAGE_H_
#define __APP_MANAGE_H_

#ifdef __cplusplus
	extern "C" {
#endif

#include <rtthread.h>
#include "log_print.h"

extern uint8_t is_os_running;      /* OS running status: 0-not running, 1-running */

/**
 * @brief:  initialize all application tasks and modules
 */
void application_init(void);

#ifdef __cplusplus
}
#endif

#endif  /* __APP_MANAGE_H_ */
