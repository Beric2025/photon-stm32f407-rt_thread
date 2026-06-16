/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef _TCP_ECHO_H
#define _TCP_ECHO_H

#ifdef __cplusplus
 	extern "C" {
#endif

/* TCP network configuration */
#define TCP_LOCAL_PORT                      8234                /* Local TCP port */
#define TCP_REMOTE_IP                       "192.168.9.70"      /* Remote host IP address */
#define TCP_REMOTE_PORT                     5959                /* Remote TCP port */

/**
 * @brief:  network data buffer descriptor
 */
typedef struct {
    unsigned char *buff;                                      /* Data buffer pointer */
    unsigned short length;                                    /* Data length in bytes */
} Net_Data_T;

/**
 * @brief:  initialize TCP client and start client thread
 */
void tcp_client_init(void);

/**
 * @brief:  initialize TCP server and start server thread
 */
void tcp_server_init(void);

/**
 * @brief:  append data to the transmit buffer for sending
 * @buff:   data buffer pointer
 * @length: number of bytes to send
 */
void send_buffer(unsigned char *buff, unsigned short length);

/**
 * @brief:  retrieve received data from the receive buffer
 * @buff:   output buffer for received data
 * @size:   max bytes to receive
 * @*length: number of bytes to receive
 *
 * Return: 0 on success, -1 on failure
 */
int get_buffer(unsigned char *buff, unsigned short size, unsigned short *length);

/**
 * @brief:  get current TCP socket handle
 *
 * Return: socket handle, -1 if not connected
 */
int get_net_socket(void);

#ifdef __cplusplus
}
#endif

#endif	/* _TCP_ECHO_H */
