/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * TCP net device abstraction layer (lwIP raw API)
 */

#include <string.h>
#include <stdio.h>
#include <rtthread.h>
#include "app_manage.h"
#include "delay_dev.h"
#include "net_dev.h"
#include "sys_arch.h"
#include "tcp_echo.h"
#include "lwip_port.h"

#define TAG "net1 dev:"

/* Private data structure */
typedef struct {
    int (*init)(void);
    void (*client_init)(void);
    void (*server_init)(void);
    void (*send)(unsigned char *data, unsigned short length);
    int (*receive)(unsigned char *data, unsigned short size, unsigned short *length);
} Internet_Data_T;

/* Function declarations */
static int net1_init(void *privatedata);
static int net1_client_config(void *privatedata);
static int net1_server_config(void *privatedata);
static int net1_send(void *privatedata, unsigned char *buff, unsigned short length);
static unsigned short net1_receive(void *privatedata, unsigned char *buff, unsigned short size);

static Internet_Data_T s_net1_data = {
    .init           = lwip_echo_init,
    .client_init    = tcp_client_init,
    .server_init    = tcp_server_init,
    .send           = send_buffer,
    .receive        = get_buffer,
};

/* Private data and device instance */
static Net_Device_T s_net1_dev = {
    .name           = "net1",
    .init           = net1_init,
    .client_config  = net1_client_config,
    .server_config  = net1_server_config,
    .send           = net1_send,
    .receive        = net1_receive,
    .private_data    = &s_net1_data
};

/* ---- Net_Device_T interface implementations ---- */

static int net1_init(void *privatedata)
{
    int ret = 0;

    if (privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s Init private data is null!\n", TAG);
        return -1;
    }  

    Net_Device_T *dev = (Net_Device_T *)privatedata;
    Internet_Data_T *net_data = (Internet_Data_T *)dev->private_data;

    ret = net_data->init();

    if(ret == -1) {
        LOG_PRINT(LOG_OUT_ERROR, "%s Init error!\n", TAG);
    }

    return ret;
}

static int net1_client_config(void *privatedata)
{
    if (privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s Client private data is null!\n", TAG);
        return -1;
    } 

    Net_Device_T *dev = (Net_Device_T *)privatedata;
    Internet_Data_T *net_data = (Internet_Data_T *)dev->private_data;

    net_data->client_init();


    return 0;
}

static int net1_server_config(void *privatedata)
{
    if (privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s Server private data is null!\n", TAG);
        return -1;
    } 

    Net_Device_T *dev = (Net_Device_T *)privatedata;
    Internet_Data_T *net_data = (Internet_Data_T *)dev->private_data;

    net_data->server_init();
    return 0;
}

static int net1_send(void *privatedata, unsigned char *buff, unsigned short length)
{
    if (privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s Send private data is null!\n", TAG);
        return -1;
    } 

    Net_Device_T *dev = (Net_Device_T *)privatedata;
    Internet_Data_T *net_data = (Internet_Data_T *)dev->private_data;

    net_data->send(buff, length);

    return 0;
}

static unsigned short net1_receive(void *privatedata, unsigned char *buff, unsigned short size)
{
    unsigned short ret_len = 0;
    int ret = 0;

    if (privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s Receive private data is null!\n", TAG);
        return 0;
    }     

    Net_Device_T *dev = (Net_Device_T *)privatedata;
    Internet_Data_T *net_data = (Internet_Data_T *)dev->private_data;

    ret = net_data->receive(buff, size, &ret_len);
    if(ret == -1) {
        LOG_PRINT(LOG_OUT_ERROR, "%s Receive error!\n", TAG);
        return 0;
    }
    return ret_len;
}

/* ---- Public API ---- */

Net_Device_T *get_net1_dev(void)
{
    return (Net_Device_T *)(&s_net1_dev);
}
