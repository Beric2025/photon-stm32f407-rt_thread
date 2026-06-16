/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * Network task management module
 */

#include <stdio.h>
#include <string.h>
#include "main.h"
#include "net_dev.h"
#include "app_manage.h"
#include "app_network.h"

#define TAG         "net_work:"



/* Network task config */
#define         NET_WORK_STK_SIZE               1024    /* Stack size: 1024*4 bytes */
#define         NET_WORK_TASK_PRIO              1       /* Task priority: 1 */
rt_thread_t    net_work_task_handler;                   /* Task handle */

static Net_Device_T *net_dev = NULL;                     /* Ethernet device pointer */
static unsigned char s_msg[1024*6] = {0};                /* Receive buffer */

static void net_work_task_function(void *parameters)
{
	parameters = parameters;

	uint16_t len = 0;

	if(net_dev) {
		/* Configure as client */
		net_dev->client_config(net_dev);

		/* Configure as server */
		net_dev->server_config(net_dev);
	}

	while(1) {
		if(net_dev) {
			len = net_dev->receive(net_dev, s_msg, sizeof(s_msg));
			if(len) {
				/* Start timer, enable heartbeat detection after packet stream stabilizes */


				// DataParseProcess(s_msg, len);
			}
		}
		rt_thread_mdelay(1);		/* 1ms delay, ensure idle task gets CPU time */
	}
}


static void task_create_function(void)
{
    /* Create network data processing task */
        {
        rt_thread_t t = rt_thread_create("net_work_task_function", net_work_task_function,
                                         RT_NULL, NET_WORK_STK_SIZE, NET_WORK_TASK_PRIO, 20);
        if (t != RT_NULL) rt_thread_startup(t);
    }
}


void app_network_init(void)
{
	net_dev = get_net_device("net1");
	if(!net_dev) {
		LOG_PRINT(LOG_OUT_ERROR, "%s Find tcp  error!\n",TAG);
        return;
    }

	if(net_dev->init(net_dev)){
		LOG_PRINT(LOG_OUT_ERROR, "%s TCP init error!\n",TAG);
		return;
	}

	task_create_function();
}



