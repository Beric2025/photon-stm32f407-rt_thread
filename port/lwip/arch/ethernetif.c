/**
  ******************************************************************************
  * @file    LwIP/LwIP_HTTP_Server_Netconn_RTOS/Src/ethernetif.c
  * @author  MCD Application Team
  * @brief   This file implements Ethernet network interface drivers for lwIP
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "lwip/opt.h"
#include "lwip/prot/etharp.h"
#include "lwip/etharp.h"
#include "lwip/pbuf.h"
#include "lwip/netif.h"
#include "lwip/snmp.h"
#include "lwip/sys.h"
#include "lwip_port.h"
#include "ethernetif.h"
#include "eth_dev.h"
#include <string.h>

/* Private define ------------------------------------------------------------*/
#define TIME_WAITING_FOR_INPUT                 0   /* lwIP: 0 = infinite wait */
#define INTERFACE_THREAD_STACK_SIZE            ( 512 )
#define NETIF_IN_TASK_PRIORITY                  ( 2 )

#define LINK_SPEED_OF_YOUR_NETIF_IN_BPS         100000000U

#define ETH_TX_BUF_SIZE                         1536U

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  RX callback registered with eth_dev. Called from eth_dev's wait_rx
  *         context (task context, not ISR) for each received frame.
  * @param  user_data: netif pointer
  * @param  data:      received frame data
  * @param  length:    frame length in bytes
  */
static void ethernet_rx_callback(void *user_data, uint8_t *data, uint16_t length)
{
    struct netif *netif = (struct netif *)user_data;
    struct pbuf *p;

    p = pbuf_alloc(PBUF_RAW, length, PBUF_POOL);
    if (p)
    {
        pbuf_take(p, data, length);
        if (netif->input(p, netif) != ERR_OK)
        {
            pbuf_free(p);
        }
    }
}

/**
  * @brief  In this function, the hardware should be initialized.
  *         Called from ethernetif_init().
  * @param  netif the already initialized lwip network interface structure
  *         for this ethernetif
  */
static void low_level_init(struct netif *netif)
{
    Ethernetif_Data_T *if_data = (Ethernetif_Data_T *)netif->state;
    Eth_Device_T *eth = get_eth_device(if_data->dev_name);

    if (eth == NULL)
    {
        return;
    }

    /* Set MAC address; must not conflict with other devices on the network */
    netif->hwaddr[0] = g_lwip_dev.mac[0];
    netif->hwaddr[1] = g_lwip_dev.mac[1];
    netif->hwaddr[2] = g_lwip_dev.mac[2];
    netif->hwaddr[3] = g_lwip_dev.mac[3];
    netif->hwaddr[4] = g_lwip_dev.mac[4];
    netif->hwaddr[5] = g_lwip_dev.mac[5];
    netif->hwaddr_len = ETHARP_HWADDR_LEN;

    netif->mtu = 1500;

    /* Netif flags: enable, broadcast, ARP */
    netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

    /* Register RX callback with device abstraction layer */
    eth->register_rx_callback(eth->private_data, ethernet_rx_callback, netif);

    /* Initialize hardware through device abstraction layer */
    if (eth->init(eth->private_data, netif->hwaddr) != 0)
    {
        return;
    }

    /* Create the task that handles ETH RX */
    sys_thread_new("eth_thread",
                   ethernetif_input,
                   netif,
                   INTERFACE_THREAD_STACK_SIZE,
                   NETIF_IN_TASK_PRIORITY);
}

/**
  * @brief  This function should do the actual transmission of the packet.
  * @param  netif the lwip network interface structure for this ethernetif
  * @param  p     the MAC packet to send
  * @return ERR_OK if the packet could be sent, ERR_IF on error
  */
static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
    Ethernetif_Data_T *if_data = (Ethernetif_Data_T *)netif->state;
    Eth_Device_T *eth = get_eth_device(if_data->dev_name);
    int ret;

    if (eth == NULL)
    {
        return ERR_IF;
    }

    if (p->next == NULL)
    {
        /* Single pbuf: pass payload directly */
        ret = eth->send(eth->private_data, (uint8_t *)p->payload, p->tot_len);
    }
    else
    {
        /* Chained pbufs: flatten to contiguous buffer */
        static uint8_t tx_buf[ETH_TX_BUF_SIZE];
        pbuf_copy_partial(p, tx_buf, p->tot_len, 0);
        ret = eth->send(eth->private_data, tx_buf, p->tot_len);
    }

    return (ret == 0) ? ERR_OK : ERR_IF;
}

/**
  * @brief  This function is the ethernetif_input task. It blocks waiting
  *         for RX frames and processes them through the registered callback.
  * @param  argument: netif pointer
  */
void ethernetif_input(void *argument)
{
    struct netif *netif = (struct netif *)argument;
    Ethernetif_Data_T *if_data = (Ethernetif_Data_T *)netif->state;
    Eth_Device_T *eth = get_eth_device(if_data->dev_name);

    if (eth == NULL)
    {
        return;
    }

    for (;;)
    {
        eth->wait_rx(eth->private_data, TIME_WAITING_FOR_INPUT);
    }
}

/**
  * @brief  Should be called at the beginning of the program to set up the
  *         network interface. It calls low_level_init() to do the
  *         actual setup of the hardware.
  *         This function should be passed as a parameter to netif_add().
  * @param  netif the lwip network interface structure for this ethernetif
  * @return ERR_OK if the loopif is initialized
  *         ERR_MEM if private data couldn't be allocated
  *         any other err_t on error
  */
err_t ethernetif_init(struct netif *netif)
{
    LWIP_ASSERT("netif != NULL", (netif != NULL));

#if LWIP_NETIF_HOSTNAME
    netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

    MIB2_INIT_NETIF(netif, snmp_ifType_ethernet_csmacd, LINK_SPEED_OF_YOUR_NETIF_IN_BPS);

    Ethernetif_Data_T *if_data = (Ethernetif_Data_T *)netif->state;
    netif->name[0] = if_data->name0;
    netif->name[1] = if_data->name1;

    netif->output = etharp_output;
    netif->linkoutput = low_level_output;

    low_level_init(netif);

    return ERR_OK;
}


