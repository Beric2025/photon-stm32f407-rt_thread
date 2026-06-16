/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef __NET_DEV_H_
#define __NET_DEV_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "main.h"

typedef struct {
    /* NET controller identifier (e.g. "net1") */
    char *name;

    /**
     * @brief:  initialize NET device
     * @privatedata:  NET handle (Net_Device_T *)
     * Return: 0 on success, -1 on failure
     */
    int (*init)(void *privatedata);

    /**
     * @brief:  configure NET device as client
     * @privatedata:  NET handle (Net_Device_T *)
     * Return: 0 on success, -1 on failure
     */
    int (*client_config)(void *privatedata);

    /**
     * @brief:  configure NET device as server
     * @privatedata:  NET handle (Net_Device_T *)
     * Return: 0 on success, -1 on failure
     */
    int (*server_config)(void *privatedata);

    /**
     * @brief:  send data via NET device
     * @privatedata: NET handle (Net_Device_T *)
     * @buff:        data buffer to send
     * @length:      number of bytes to send
     * Return: 0 on success, -1 on failure
     */
    int (*send)(void *privatedata, unsigned char *buff, unsigned short length);

    /**
     * @brief:  receive data from NET device
     * @privatedata: NET handle (Net_Device_T *)
     * @buff:        receive buffer
     * @size:        max bytes to receive
     * Return: number of bytes received, 0 if none available
     */
    unsigned short (*receive)(void *privatedata, unsigned char *buff, unsigned short size);

    /**
     * @brief:  reset NET device
     * @privatedata:  NET handle (Net_Device_T *)
     * Return: 0 on success, -1 on failure
     */
    int (*reset)(void *privatedata);

    /**
     * @brief:  reload NET device configuration
     * @privatedata:  NET handle (Net_Device_T *)
     * Return: 0 on success, -1 on failure
     */
    int (*reload)(void *privatedata);

    void *private_data;
} Net_Device_T;

/**
 * @brief:  get NET device structure by name
 * @name:   device name (e.g. "net1")
 * Return: pointer to Net_Device_T structure, NULL if not found
 */
Net_Device_T *get_net_device(char *name);

#ifdef __cplusplus
}
#endif

#endif  /* __NET_DEV_H_ */
