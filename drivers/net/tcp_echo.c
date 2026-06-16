/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * TCP echo client/server implementation
 */

#include <rtthread.h>
#include <stdint.h>
#include <stdio.h>
#include <lwip/sockets.h>
#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/api.h"
#include "tcp_echo.h"
#include "main.h"
#include "log_print.h"

#define TAG  "tcp echo:"

#define TURN_ON_KEEPALIVE_AND_TIMEOUT  1

#define LWIP_RX_BUFSIZE                     1024*6
#define LWIP_SEND_THREAD_PRIO               5

#define LWIP_TX_BUFSIZE                     512

#ifdef TURN_ON_KEEPALIVE_AND_TIMEOUT
#define TCP_KEEPALIVE_IDLE                  10
#define TCP_KEEPALIVE_INTERVAL              5
#define TCP_KEEPALIVE_COUNT                 3
#define TCP_RECV_TIMEOUT_SEC                2
#endif

/* RX data buffer */
static uint8_t s_rx_buff[LWIP_RX_BUFSIZE];
Net_Data_T s_rx_data = {
    .buff = s_rx_buff,
    .length = 0
};

/* TX data buffer */
static uint8_t s_tx_buff[LWIP_TX_BUFSIZE];

Net_Data_T s_txData = {
    .buff = s_tx_buff,
    .length = 0
};

/* Data send flag */
int s_tcp_socket = -1;
static int s_connected = -1;
static unsigned char *s_tbuf = NULL;
sys_thread_t tcp_client_thread_handle;

/* Function declarations */
static void tcp_send_thread(void *arg);


void lwip_send_thread(void)
{
    sys_thread_new("tcp_send_thread", tcp_send_thread, NULL, 512, LWIP_SEND_THREAD_PRIO);
}


void clear_client_buffer(void)
{
    if (s_tcp_socket != -1) {
        closesocket(s_tcp_socket);
        s_tcp_socket = -1;
    }
    if (s_tbuf) {
        rt_free(s_tbuf);
        s_tbuf = NULL;
    }
}

/**
 * @brief: TCP client thread with keepalive and timeout support
 * @parameters: thread parameters (unused)
 *
 * Handles socket creation, connection, keepalive configuration,
 * data reception, and automatic reconnection on disconnect.
 */
void tcp_client_thread(void *parameters)
{
    parameters = parameters;

    struct sockaddr_in clientAddr = {0};
    err_t err;
    int recvDataLen;
    unsigned char *tbuf;
    struct timeval tv;
    int keepalive, keepidle, keepintvl, keepcnt;

    lwip_send_thread();

    while (1) {
sock_start:
        clientAddr.sin_family           = AF_INET;
        clientAddr.sin_port             = htons(TCP_LOCAL_PORT);
        clientAddr.sin_addr.s_addr      = inet_addr(TCP_REMOTE_IP);

        s_tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
        memset(&(clientAddr.sin_zero), 0, sizeof(clientAddr.sin_zero));

        tbuf = (unsigned char *)rt_malloc(LWIP_RX_BUFSIZE);

        /* Connect to remote IP address */
        err = connect(s_tcp_socket, (struct sockaddr *)&clientAddr, sizeof(struct sockaddr));

        if (err == -1) {
            LOG_PRINT(LOG_OUT_INFO, "%s State:Connection Fail!\n", TAG);
            closesocket(s_tcp_socket);
            s_tcp_socket = -1;
            rt_free( tbuf);
            rt_thread_mdelay(100);
            goto sock_start;
        }

        LOG_PRINT(LOG_OUT_INFO, "%s State:Connection Successful!\n", TAG);

        /* Enable TCP keepalive */
        keepalive = 1;
        setsockopt(s_tcp_socket, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(int));

        keepidle = TCP_KEEPALIVE_IDLE;
        setsockopt(s_tcp_socket, IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, sizeof(int));

        keepintvl = TCP_KEEPALIVE_INTERVAL;
        setsockopt(s_tcp_socket, IPPROTO_TCP, TCP_KEEPINTVL, &keepintvl, sizeof(int));

        keepcnt = TCP_KEEPALIVE_COUNT;
        setsockopt(s_tcp_socket, IPPROTO_TCP, TCP_KEEPCNT, &keepcnt, sizeof(int));

        /* Set receive timeout */
        tv.tv_sec = TCP_RECV_TIMEOUT_SEC;
        tv.tv_usec = 0;
        setsockopt(s_tcp_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        while (1)  {
            recvDataLen = recv(s_tcp_socket, tbuf, LWIP_RX_BUFSIZE, 0);
            if (recvDataLen <= 0 ) {
                LOG_PRINT(LOG_OUT_INFO, "%s State:Disconnect! (recv=%d)\n",TAG , recvDataLen);
                closesocket(s_tcp_socket);
                s_tcp_socket = -1;
                rt_free(tbuf);
                rt_thread_mdelay(100);
                goto sock_start;
            }

            /* Data received */
            if(recvDataLen > 0) {
                rt_enter_critical();
                if(s_rx_data.length + recvDataLen > LWIP_RX_BUFSIZE) {
                    s_rx_data.length = 0;
                }
                memcpy(&s_rx_data.buff[s_rx_data.length], tbuf, recvDataLen);
                s_rx_data.length += recvDataLen;
                rt_exit_critical();
            }

            rt_thread_mdelay(10);
        }
    }
}


/**
 * @brief: TCP server thread
 * @parameters: thread parameters (unused)
 *
 * Creates a listening TCP server, accepts client connections,
 * and receives data from connected clients.
 */
static void tcp_server_thread(void *parameters)
{
    parameters = parameters;

    int sock = -1;
    char *recv_data;
    struct sockaddr_in server_addr,client_addr;
    socklen_t sin_size;
    int recv_data_len;

    LOG_PRINT(LOG_OUT_INFO, "%s client port:%d\n\n",TAG ,TCP_LOCAL_PORT);

    lwip_send_thread();

    recv_data = (char *)rt_malloc(512);
    if (recv_data == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s No memory\n",TAG);
        goto __exit;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)  {
        LOG_PRINT(LOG_OUT_ERROR, "%s Socket error\n",TAG);
        goto __exit;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(TCP_LOCAL_PORT);
    memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
        LOG_PRINT(LOG_OUT_ERROR, "%s Unable to bind\n",TAG);
        goto __exit;
    }

    if (listen(sock, 5) == -1) {
        LOG_PRINT(LOG_OUT_ERROR, "%s Listen error\n",TAG);
        goto __exit;
    }

    while(1) {
        sin_size = sizeof(struct sockaddr_in);

        s_connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);

        LOG_PRINT(LOG_OUT_INFO, "%s new client s_connected from (%s, %d)\n",TAG ,
                inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        {
            int flag = 1;

            setsockopt(s_connected,
                        IPPROTO_TCP,
                        TCP_NODELAY,
                        (void *) &flag,
                        sizeof(int));
        }

        while(1) {
            recv_data_len = recv(s_connected, recv_data, 512, 0);

            if (recv_data_len <= 0)
                break;

            LOG_PRINT(LOG_OUT_INFO, "%s recv %d len data\n",TAG ,recv_data_len);

            // write(s_connected,recv_data,recv_data_len);
            // tcpecho_send(recv_data, recv_data_len);//test

        }
        if (s_connected >= 0)  closesocket(s_connected);

        s_connected = -1;
    }
__exit:
    if (sock >= 0) closesocket(sock);
    if (recv_data) free(recv_data);
}

void tcp_client_init(void)
{
    tcp_client_thread_handle = sys_thread_new("tcp_client_thread", \
        tcp_client_thread, NULL, 512, 4);
}

void tcp_server_init(void)
{
    sys_thread_new("tcp_server_thread", tcp_server_thread, NULL, 512, 4);
}

/**
 * @brief: TCP send task
 * @parameters: thread parameters (unused)
 *
 * Monitors s_txData for data to send and writes to the TCP socket.
 * Clears the buffer on successful send or after 3 consecutive failures.
 */
void tcp_send_thread(void *parameters)
{
    parameters = parameters;

    err_t err;
    unsigned char cnt = 0;

    // while (1)  {
        while (1)  {
            /* Data to send */
            if((s_tcp_socket != -1) && (s_txData.length > 0)) {
                rt_enter_critical();
                err = write(s_tcp_socket, s_txData.buff, s_txData.length);
                if (err > 0) {
                    s_txData.length = 0;
                }
                else {
                    if(cnt >= 3) {
                        cnt = 0;
                        s_txData.length = 0;
                    }
                    else {
                        cnt++;
                    }
                }
                rt_exit_critical();
            }

            rt_thread_mdelay(10);
        }

    //     closesocket(s_tcp_socket);
    // }
}

/**
 * @brief: enqueue data into the TX buffer
 * @buff:   data pointer
 * @length: data length
 *
 * Copies data into the TX ring buffer protected by a critical section.
 */
void send_buffer(unsigned char *buff, unsigned short length)
{
    if(s_tcp_socket != -1) {
        rt_enter_critical();
        if(s_txData.length + length > LWIP_TX_BUFSIZE) {
            s_txData.length = 0;
        }
        memcpy(&s_txData.buff[s_txData.length], buff, length);
        s_txData.length += length;
        rt_exit_critical();
    }
}

int get_buffer(unsigned char *buff, unsigned short size, unsigned short *length)
{
    int ret = 0;

    rt_enter_critical();
    if(size >= s_rx_data.length) {
        memcpy(buff, s_rx_data.buff, s_rx_data.length);
        *length = s_rx_data.length;
        s_rx_data.length = 0;
    }
    else {
        ret = -1;
    }
    rt_exit_critical();

    if(-1 == ret) {
        LOG_PRINT(LOG_OUT_ERROR, "%s Get buffer size is small\n", TAG);
    }

    return ret;
}

static void tcp_create_task(void)
{
    tcp_client_init();
}

static void tcp_delete_task(void)
{
    /* Free memory */
    clear_client_buffer();
    /* Delete task */
    rt_thread_delete((rt_thread_t)tcp_client_thread_handle.thread_handle);
}

/**
 * @brief: link status change callback
 * @status: 1 = link up, 0 = link down
 *
 * Creates or deletes the TCP client task based on link status.
 */
void tcp_link_callback(unsigned char status)
{
    if(status){
        tcp_create_task();
    }
    else {
        tcp_delete_task();
    }
}

/**
 * @brief: get current TCP socket handle
 *
 * Return: socket file descriptor, or -1 if not connected
 */
int get_net_socket(void)
{
    return s_tcp_socket;
}
