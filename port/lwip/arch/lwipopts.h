/**
  ******************************************************************************
  * @file    LwIP/LwIP_HTTP_Server_Netconn_RTOS/Inc/lwipopts.h
  * @author  MCD Application Team
  * @brief   lwIP Options Configuration.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__


/* NO_SYS: 0 = with OS emulation layer. Different code compiled based on this */
#define NO_SYS                          0

/* ---------- Memory options ---------- */
/* Memory alignment: 4-byte aligned  */
#define MEM_ALIGNMENT                   4

/* Heap memory size */
#define MEM_SIZE                        (20*1024)

/* MEMP_NUM_PBUF: Number of mempool pbufs */
#define MEMP_NUM_PBUF                   15
/* MEMP_NUM_UDP_PCB: Number of UDP PCBs */
#define MEMP_NUM_UDP_PCB                4
/* MEMP_NUM_TCP_PCB: Number of TCP PCBs */
#define MEMP_NUM_TCP_PCB                4
/* MEMP_NUM_TCP_PCB_LISTEN: Number of listening TCP PCBs */
#define MEMP_NUM_TCP_PCB_LISTEN         2
/* MEMP_NUM_TCP_SEG: Number of queued TCP segments */
#define MEMP_NUM_TCP_SEG                120
/* MEMP_NUM_SYS_TIMEOUT: Number of simulated timeouts */
#define MEMP_NUM_SYS_TIMEOUT            6


/* ---------- Pbuf options ---------- */
/* PBUF_POOL_SIZE: Number of buffers in the pbuf pool */
#define PBUF_POOL_SIZE                  20
/* PBUF_POOL_BUFSIZE: Size of each pbuf in the pool */
#define PBUF_POOL_BUFSIZE               LWIP_MEM_ALIGN_SIZE(TCP_MSS+40+PBUF_LINK_ENCAPSULATION_HLEN+PBUF_LINK_HLEN)


/* ---------- TCP options ---------- */
#define LWIP_TCP                        1
#define TCP_TTL                         255

/* Controls whether TCP queues out-of-order segments. Set to 0 if low on memory. */
#define TCP_QUEUE_OOSEQ                 0

/* TCP MSS */
#define TCP_MSS                         (1500 - 40)   /* TCP_MSS = (Ethernet MTU - IP header size - TCP header size) */

/* TCP sender buffer space (bytes) */
#define TCP_SND_BUF                     (11*TCP_MSS)

/* TCP_SND_QUEUELEN: TCP send buffer queue. Must be at least 2 * TCP_SND_BUF / TCP_MSS */
#define TCP_SND_QUEUELEN                (8* TCP_SND_BUF/TCP_MSS)

/* TCP receive window */
#define TCP_WND                         (2*TCP_MSS)


/* ---------- ICMP options ---------- */
#define LWIP_ICMP                       1


/* ---------- DHCP options ---------- */
/* Set LWIP_DHCP to 1 to enable DHCP */
#define LWIP_DHCP                       0


/* ---------- UDP options ---------- */
#define LWIP_UDP                        1
#define UDP_TTL                         255


/* ---------- Statistics options ---------- */
#define LWIP_STATS                      0
#define LWIP_PROVIDE_ERRNO              1

/* ---------- Link callback options ---------- */
/* LWIP_NETIF_LINK_CALLBACK==1: Enable callback on link status change */
#define LWIP_NETIF_LINK_CALLBACK        1
/* ---------- Checksum options ---------- */

/* 
The STM32F4x7 allows computing and verifying the IP, UDP, TCP and ICMP checksums by hardware:
 - To use this feature let the following define uncommented.
 - To disable it and process by CPU comment the  the checksum.
*/
#define CHECKSUM_BY_HARDWARE 


#ifdef CHECKSUM_BY_HARDWARE
  /* CHECKSUM_GEN_IP==0: Generate checksums by hardware for outgoing IP packets.*/
  #define CHECKSUM_GEN_IP                 0
  /* CHECKSUM_GEN_UDP==0: Generate checksums by hardware for outgoing UDP packets.*/
  #define CHECKSUM_GEN_UDP                0
  /* CHECKSUM_GEN_TCP==0: Generate checksums by hardware for outgoing TCP packets.*/
  #define CHECKSUM_GEN_TCP                0 
  /* CHECKSUM_CHECK_IP==0: Check checksums by hardware for incoming IP packets.*/
  #define CHECKSUM_CHECK_IP               0
  /* CHECKSUM_CHECK_UDP==0: Check checksums by hardware for incoming UDP packets.*/
  #define CHECKSUM_CHECK_UDP              0
  /* CHECKSUM_CHECK_TCP==0: Check checksums by hardware for incoming TCP packets.*/
  #define CHECKSUM_CHECK_TCP              0
  /* CHECKSUM_CHECK_ICMP==0: Check checksums by hardware for incoming ICMP packets.*/
  #define CHECKSUM_GEN_ICMP               0
#else
  /* CHECKSUM_GEN_IP==1: Generate checksums in software for outgoing IP packets.*/
  #define CHECKSUM_GEN_IP                 1
  /* CHECKSUM_GEN_UDP==1: Generate checksums in software for outgoing UDP packets.*/
  #define CHECKSUM_GEN_UDP                1
  /* CHECKSUM_GEN_TCP==1: Generate checksums in software for outgoing TCP packets.*/
  #define CHECKSUM_GEN_TCP                1
  /* CHECKSUM_CHECK_IP==1: Check checksums in software for incoming IP packets.*/
  #define CHECKSUM_CHECK_IP               1
  /* CHECKSUM_CHECK_UDP==1: Check checksums in software for incoming UDP packets.*/
  #define CHECKSUM_CHECK_UDP              1
  /* CHECKSUM_CHECK_TCP==1: Check checksums in software for incoming TCP packets.*/
  #define CHECKSUM_CHECK_TCP              1
  /* CHECKSUM_CHECK_ICMP==1: Check checksums by hardware for incoming ICMP packets.*/
  #define CHECKSUM_GEN_ICMP               1
#endif


/* ---------- Netconn options ---------- */
/**
 * LWIP_NETCONN==1: Enable Netconn API (requires api_lib.c)
 */
#define LWIP_NETCONN                    1

/* ---------- Socket options ---------- */
/**
 * LWIP_SOCKET==1: Enable Socket API (requires sockets.c)
 */
#define LWIP_SOCKET                     1

/*
   ------------------------------------
   ---------- httpd options ----------
   ------------------------------------
*/
/** Set this to 1 to include "fsdata_custom.c" instead of "fsdata.c" for the
 * file system (to prevent changing the file included in CVS) */
#define HTTPD_USE_CUSTOM_FSDATA         1
/* ---------- OS options ---------- */

#define TCPIP_THREAD_NAME               "TCP/IP"
#define TCPIP_THREAD_STACKSIZE          1000
#define TCPIP_MBOX_SIZE                 6
#define DEFAULT_UDP_RECVMBOX_SIZE       6
#define DEFAULT_TCP_RECVMBOX_SIZE       6
#define DEFAULT_ACCEPTMBOX_SIZE         6
#define DEFAULT_THREAD_STACKSIZE        500
#define TCPIP_THREAD_PRIO               5
#define LWIP_SO_RCVTIMEO                1

#endif /* __LWIPOPTS_H__ */
