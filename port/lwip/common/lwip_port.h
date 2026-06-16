/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef _LWIP_PORT_H
#define _LWIP_PORT_H

#ifdef __cplusplus
 	extern "C" {
#endif

/* Default MAC address: B8:AE:1D:00:04:00 */
#define LWIP_MAC0    ((unsigned char) 0xB8)
#define LWIP_MAC1    ((unsigned char) 0xAE)
#define LWIP_MAC2    ((unsigned char) 0x1D)
#define LWIP_MAC3    ((unsigned char) 0x00)
#define LWIP_MAC4    ((unsigned char) 0x04)
#define LWIP_MAC5    ((unsigned char) 0x00)

/* Default static IP: 192.168.1.30 */
#define LWIP_IP0     ((unsigned char) 192)
#define LWIP_IP1     ((unsigned char) 168)
#define LWIP_IP2     ((unsigned char) 1)
#define LWIP_IP3     ((unsigned char) 30)

/* Default netmask: 255.255.255.0 */
#define LWIP_NETMASK0  ((unsigned char) 255)
#define LWIP_NETMASK1  ((unsigned char) 255)
#define LWIP_NETMASK2  ((unsigned char) 255)
#define LWIP_NETMASK3  ((unsigned char) 0)

/* Default gateway: 192.168.1.1 */
#define LWIP_GATEWAY0  ((unsigned char) 192)
#define LWIP_GATEWAY1  ((unsigned char) 168)
#define LWIP_GATEWAY2  ((unsigned char) 1)
#define LWIP_GATEWAY3  ((unsigned char) 1)

/**
 * @brief:  display callback function prototype
 * @index:  display index for selecting which information to show
 */
typedef void (*display_function)(unsigned char index);

/** lwIP control struct */
typedef struct
{
    unsigned char mac[6];                                     /* MAC address */
    unsigned char ip[4];                                      /* Local IP address */
    unsigned char netmask[4];                                 /* Subnet mask */
    unsigned char gateway[4];                                 /* Default gateway address */
    unsigned char dhcpstatus;                                 /* DHCP status:
                                                                 0 = not acquired,
                                                                 2 = acquired,
                                                                 0xFF = failed / static IP */
    unsigned char link_status;                                /* Ethernet link status:
                                                                 0 = link down,
                                                                 1 = link up */
    display_function lwip_display_fn;                         /* Display callback function pointer */
}Lwip_IP_Info_T;

/** lwIP control struct instance */
extern Lwip_IP_Info_T g_lwip_dev;

/**
 * @brief:  initialize lwIP protocol stack and Ethernet interface
 *
 * Return: 0 on success, non-zero on failure
 */
int lwip_echo_init(void);

#ifdef __cplusplus
}
#endif

#endif	/* _LWIP_PORT_H */
