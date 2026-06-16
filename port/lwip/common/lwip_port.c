/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * lwIP port layer
 */

#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/init.h"
#include "ethernetif.h"
#include "lwip/timeouts.h"
#include "lwip/tcpip.h"
#include "delay_dev.h"
#include "eth_dev.h"
#include <stdio.h>
#include "log_print.h"
#include "lwip_port.h"


#define TAG "lwip port: "

/* lwIP needs a global errno */
int errno;



/* DHCP process states */
#define LWIP_DHCP_OFF                   (unsigned char) 0     /* DHCP off */
#define LWIP_DHCP_START                 (unsigned char) 1     /* DHCP start */
#define LWIP_DHCP_WAIT_ADDRESS          (unsigned char) 2     /* DHCP waiting for IP */
#define LWIP_DHCP_ADDRESS_ASSIGNED      (unsigned char) 3     /* DHCP address assigned */
#define LWIP_DHCP_TIMEOUT               (unsigned char) 4     /* DHCP timeout */
#define LWIP_DHCP_LINK_DOWN             (unsigned char) 5     /* DHCP link down */

/* Link states */
#define LWIP_LINK_OFF                   (unsigned char) 0     /* Link off */
#define LWIP_LINK_ON                    (unsigned char) 1     /* Link on */
#define LWIP_LINK_AGAIN                 (unsigned char) 2     /* Link re-established */

/* Max DHCP retries */
#define LWIP_MAX_DHCP_TRIES             (unsigned char) 4


Lwip_IP_Info_T g_lwip_dev;                                       /* lwIP control struct */
struct netif s_lwip_netif;                                  /* Global network interface */

static Ethernetif_Data_T s_eth_if_data = {                  /* Per-netif private data */
    .dev_name = "eth1",
    .name0    = 's',
    .name1    = 't',
};

#if LWIP_DHCP
uint32_t s_dhcp_fine_timer = 0;                             /* DHCP fine timer */
__IO unsigned char s_lwip_dhcp_state = LWIP_DHCP_OFF;             /* DHCP state */
#endif

/* Link thread config */
#define LWIP_LINK_TASK_PRIO             3                   /* Task priority */
#define LWIP_LINK_STK_SIZE              128 * 2             /* Task stack size */
static void lwip_link_thread( void * argument );             /* Link thread */

/* DHCP thread config */
#if LWIP_DHCP  
#define LWIP_DHCP_TASK_PRIO             4                   /* Task priority */
#define LWIP_DHCP_STK_SIZE              128 * 2             /* Task stack size */
static void lwip_periodic_handle(void *argument);            /* DHCP thread */
#endif
static void lwip_link_status_updated(struct netif *netif);   /* DHCP status callback */

#if 0
/**
 * @brief:  convert 32-bit IP address to 4-byte array (big-endian)
 * @ip:     32-bit IP address
 * @arr:    output byte array (4 bytes)
 */
static void lwip_ip_to_array(uint32_t ip, unsigned char *arr)
{
    arr[3] = (unsigned char)(ip >> 24);
    arr[2] = (unsigned char)(ip >> 16);
    arr[1] = (unsigned char)(ip >> 8);
    arr[0] = (unsigned char)(ip);
}
#endif
/**
 * @brief:  print current network configuration to console
 */
static void lwip_print_network_info(void)
{
    LOG_PRINT(LOG_OUT_INFO,"%sMAC:................%d.%d.%d.%d.%d.%d\r\n", TAG,
           g_lwip_dev.mac[0], g_lwip_dev.mac[1], g_lwip_dev.mac[2],
           g_lwip_dev.mac[3], g_lwip_dev.mac[4], g_lwip_dev.mac[5]);
    LOG_PRINT(LOG_OUT_INFO,"%sIP:........................%d.%d.%d.%d\r\n", TAG,
           g_lwip_dev.ip[0], g_lwip_dev.ip[1], g_lwip_dev.ip[2], g_lwip_dev.ip[3]);
    LOG_PRINT(LOG_OUT_INFO,"%sMASK:..........................%d.%d.%d.%d\r\n", TAG,
           g_lwip_dev.netmask[0], g_lwip_dev.netmask[1], g_lwip_dev.netmask[2], g_lwip_dev.netmask[3]);
    LOG_PRINT(LOG_OUT_INFO,"%sGATEWAY:..........................%d.%d.%d.%d\r\n", TAG,
           g_lwip_dev.gateway[0], g_lwip_dev.gateway[1], g_lwip_dev.gateway[2], g_lwip_dev.gateway[3]);
}

/* Set default IP address */
static void lwip_default_ip_set(Lwip_IP_Info_T *lwipinfo)
{
    /* MAC address */
    lwipinfo->mac[0] = LWIP_MAC0;
    lwipinfo->mac[1] = LWIP_MAC1;
    lwipinfo->mac[2] = LWIP_MAC2;
    lwipinfo->mac[3] = LWIP_MAC3;
    lwipinfo->mac[4] = LWIP_MAC4;
    lwipinfo->mac[5] = LWIP_MAC5;

    /* Default local IP */
    lwipinfo->ip[0] = LWIP_IP0;
    lwipinfo->ip[1] = LWIP_IP1;
    lwipinfo->ip[2] = LWIP_IP2;
    lwipinfo->ip[3] = LWIP_IP3;
    /* Default netmask */
    lwipinfo->netmask[0] = LWIP_NETMASK0;
    lwipinfo->netmask[1] = LWIP_NETMASK1;
    lwipinfo->netmask[2] = LWIP_NETMASK2;
    lwipinfo->netmask[3] = LWIP_NETMASK3;

    /* Default gateway */
    lwipinfo->gateway[0] = LWIP_GATEWAY0;
    lwipinfo->gateway[1] = LWIP_GATEWAY1;
    lwipinfo->gateway[2] = LWIP_GATEWAY2;
    lwipinfo->gateway[3] = LWIP_GATEWAY3;
    lwipinfo->dhcpstatus = LWIP_DHCP_OFF; /* DHCP disabled */
}

int lwip_echo_init(void)
{
    struct netif *netif_new;                         /* netif_add() return value */
    ip_addr_t ipaddr;                               /* IP address */
    ip_addr_t netmask;                              /* Netmask */
    ip_addr_t gw;                                   /* Default gateway */

    tcpip_init(NULL, NULL);

    lwip_default_ip_set(&g_lwip_dev);           /* Set default IP config */

#if LWIP_DHCP                                       /* Using DHCP */
    ip_addr_set_zero_ip4(&ipaddr);
    ip_addr_set_zero_ip4(&netmask);
    ip_addr_set_zero_ip4(&gw);
#else                                               /* Using static IP */
    IP4_ADDR(&ipaddr, g_lwip_dev.ip[0], g_lwip_dev.ip[1], g_lwip_dev.ip[2], g_lwip_dev.ip[3]);
    IP4_ADDR(&netmask, g_lwip_dev.netmask[0], g_lwip_dev.netmask[1], g_lwip_dev.netmask[2], g_lwip_dev.netmask[3]);
    IP4_ADDR(&gw, g_lwip_dev.gateway[0], g_lwip_dev.gateway[1], g_lwip_dev.gateway[2], g_lwip_dev.gateway[3]);
    lwip_print_network_info();
    g_lwip_dev.dhcpstatus = 0XFF;
#endif

    /* Add netif to the list and set as default */
    netif_new = netif_add(&s_lwip_netif, (const ip_addr_t *)&ipaddr, (const ip_addr_t *)&netmask, (const ip_addr_t *)&gw, &s_eth_if_data, &ethernetif_init, &tcpip_input);

    if (netif_new == NULL)
    {
        return -1;                                   /* Netif add failed */
    }
    else                                            /* Set netif as default and bring it up */
    {
        netif_set_default(&s_lwip_netif);           /* Set as default netif */

#if LWIP_NETIF_LINK_CALLBACK
        netif_set_link_callback(&s_lwip_netif, lwip_link_status_updated);
        /* PHY link status polling task */
        sys_thread_new("eth_link",
                       lwip_link_thread,            /* task entry function */
                       &s_lwip_netif,               /* task entry parameter */
                       LWIP_LINK_STK_SIZE,          /* task stack size */
                       LWIP_LINK_TASK_PRIO);        /* task priority */
#endif

    }
    g_lwip_dev.link_status = LWIP_LINK_OFF;          /* Link status = off */
#if LWIP_DHCP                                       /* If DHCP is enabled */
    g_lwip_dev.dhcpstatus = 0;                       /* DHCP status = 0 */
    /* DHCP polling task */
    sys_thread_new("eth_dhcp",
                   lwip_periodic_handle,            /* task entry function */
                   &s_lwip_netif,                   /* task entry parameter */
                   LWIP_DHCP_STK_SIZE,              /* task stack size */
                   LWIP_DHCP_TASK_PRIO);            /* task priority */
#endif
    return 0;                                       /* Success */
}

/**
 * @brief: Notify DHCP configuration status
 * @netif: Network interface
 */
static void lwip_link_status_updated(struct netif *netif)
{
    if (netif_is_up(netif))
    {
#if LWIP_DHCP
        /* Update DHCP state machine */
        s_lwip_dhcp_state = LWIP_DHCP_START;
#endif /* LWIP_DHCP */
        LOG_PRINT(LOG_OUT_DEBUG,"%sThe network cable is connected \r\n", TAG);
    }
    else
    {
#if LWIP_DHCP
        /* Update DHCP state machine */
        s_lwip_dhcp_state = LWIP_DHCP_LINK_DOWN;
#endif /* LWIP_DHCP */
        LOG_PRINT(LOG_OUT_DEBUG,"%sThe network cable is not connected \r\n", TAG);
    }
}


#if LWIP_DHCP   /* If DHCP is enabled */

/**
 * @brief: DHCP process
 * @argument: Task argument
 */
static void lwip_periodic_handle(void *argument)
{
    struct netif *netif = (struct netif *) argument;
    uint32_t ip = 0;
    uint32_t netmask = 0;
    uint32_t gw = 0;
    struct dhcp *dhcp;
    unsigned char iptxt[20];

    while (1)
    {
        switch (s_lwip_dhcp_state)
        {
            case LWIP_DHCP_START:
            {
                /* Clear IP, gateway and netmask */
                ip_addr_set_zero_ip4(&netif->ip_addr);
                ip_addr_set_zero_ip4(&netif->netmask);
                ip_addr_set_zero_ip4(&netif->gw);

                s_lwip_dhcp_state = LWIP_DHCP_WAIT_ADDRESS;

                LOG_PRINT(LOG_OUT_DEBUG,"%sState: Looking for DHCP server ...\r\n", TAG);
                dhcp_start(netif);
            }
            break;
            case LWIP_DHCP_WAIT_ADDRESS:
            {
                if (dhcp_supplied_address(netif))
                {
                    s_lwip_dhcp_state = LWIP_DHCP_ADDRESS_ASSIGNED;

                    /* Read new IP address */
                    ip = s_lwip_netif.ip_addr.addr;
                    netmask = s_lwip_netif.netmask.addr;
                    gw = s_lwip_netif.gw.addr;

                    sprintf((char *)iptxt, "%s", ip4addr_ntoa(netif_ip4_addr(netif)));
                    LOG_PRINT(LOG_OUT_INFO,"%sIP address assigned by a DHCP server: %s\r\n",TAG ,iptxt);

                    if (ip != 0)
                    {
                        g_lwip_dev.dhcpstatus = 2;         /* DHCP succeeded */

                        /* Parse DHCP-assigned addresses */
                        lwip_ip_to_array(ip, g_lwip_dev.ip);
                        lwip_ip_to_array(netmask, g_lwip_dev.netmask);
                        lwip_ip_to_array(gw, g_lwip_dev.gateway);
                        lwip_print_network_info();

                        if (g_lwip_dev.lwip_display_fn)
                            g_lwip_dev.lwip_display_fn(2);
                    }
                }
                else
                {
                    dhcp = (struct dhcp *)netif_get_client_data(netif, LWIP_NETIF_CLIENT_DATA_INDEX_DHCP);

                    /* DHCP timeout */
                    if (dhcp->tries > LWIP_MAX_DHCP_TRIES)
                    {
                        s_lwip_dhcp_state = LWIP_DHCP_TIMEOUT;
                        g_lwip_dev.dhcpstatus = 0XFF;
                        /* Fall back to static IP */
                        IP4_ADDR(&(s_lwip_netif.ip_addr), g_lwip_dev.ip[0], g_lwip_dev.ip[1], g_lwip_dev.ip[2], g_lwip_dev.ip[3]);
                        IP4_ADDR(&(s_lwip_netif.netmask), g_lwip_dev.netmask[0], g_lwip_dev.netmask[1], g_lwip_dev.netmask[2], g_lwip_dev.netmask[3]);
                        IP4_ADDR(&(s_lwip_netif.gw), g_lwip_dev.gateway[0], g_lwip_dev.gateway[1], g_lwip_dev.gateway[2], g_lwip_dev.gateway[3]);
                        netif_set_addr(netif, &s_lwip_netif.ip_addr, &s_lwip_netif.netmask, &s_lwip_netif.gw);

                        sprintf((char *)iptxt, "%s", ip4addr_ntoa(netif_ip4_addr(netif)));
                        printf ("DHCP Timeout !! \r\n");
                        printf ("Static IP address: %s\r\n", iptxt);

                        if (g_lwip_dev.lwip_display_fn)
                            g_lwip_dev.lwip_display_fn(2);
                    }
                }
            }
            break;
            case LWIP_DHCP_LINK_DOWN:
            {
                s_lwip_dhcp_state = LWIP_DHCP_OFF;
            }
            break;
            default: break;
        }

        /* wait 1000 ms */
        sys_msleep(1000);
    }
}
#endif

/**
 * @brief: Check ETH link status, update netif
 * @argument: netif
 */
static void lwip_link_thread( void *argument )
{
    struct netif *netif = (struct netif *) argument;
    Ethernetif_Data_T *if_data = (Ethernetif_Data_T *)netif->state;
    Eth_Device_T *eth = get_eth_device(if_data->dev_name);
    int link_again_num = 0;
    int link_up;

    if (eth == NULL) {
        return;
    }

    while(1)
    {
        link_up = eth->get_link_status(eth->private_data);

        if (!link_up)
        {
            if (link_again_num < 2)
                link_again_num++;

            if (link_again_num >= 2)                    /* Two consecutive detections confirm link down */
            {
                g_lwip_dev.link_status = LWIP_LINK_OFF;
#if LWIP_DHCP
                s_lwip_dhcp_state = LWIP_DHCP_LINK_DOWN;
                dhcp_stop(netif);
#endif
                eth->stop(eth->private_data);
                netif_set_down(netif);
                netif_set_link_down(netif);
            }
        }
        else                                            /* Cable connected */
        {
            link_again_num = 0;

            if (g_lwip_dev.link_status == LWIP_LINK_OFF)/* Bring up Ethernet and virtual netif */
            {
                g_lwip_dev.link_status = LWIP_LINK_ON;
                eth->start(eth->private_data);
                netif_set_up(netif);
                netif_set_link_up(netif);
            }
        }

        sys_msleep(100);
    }
}

