/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef __ETH_PHY_H
#define __ETH_PHY_H

#ifdef __cplusplus
 extern "C" {
#endif
#include <stdint.h>

/* PHY register map */
#define ETH_CHIP_BCR                            ((unsigned short)0x0000U)
#define ETH_CHIP_BSR                            ((unsigned short)0x0001U)
#define PHY_REGISTER2                           ((unsigned short)0x0002U)
#define PHY_REGISTER3                           ((unsigned short)0x0003U)

/* BCR register values (typically not modified) */
#define ETH_CHIP_BCR_SOFT_RESET                 ((unsigned short)0x8000U)
#define ETH_CHIP_BCR_LOOPBACK                   ((unsigned short)0x4000U)
#define ETH_CHIP_BCR_SPEED_SELECT               ((unsigned short)0x2000U)
#define ETH_CHIP_BCR_AUTONEGO_EN                ((unsigned short)0x1000U)
#define ETH_CHIP_BCR_POWER_DOWN                 ((unsigned short)0x0800U)
#define ETH_CHIP_BCR_ISOLATE                    ((unsigned short)0x0400U)
#define ETH_CHIP_BCR_RESTART_AUTONEGO           ((unsigned short)0x0200U)
#define ETH_CHIP_BCR_DUPLEX_MODE                ((unsigned short)0x0100U)

/* BSR register values (typically not modified) */
#define ETH_CHIP_BSR_100BASE_T4                 ((unsigned short)0x8000U)
#define ETH_CHIP_BSR_100BASE_TX_FD              ((unsigned short)0x4000U)
#define ETH_CHIP_BSR_100BASE_TX_HD              ((unsigned short)0x2000U)
#define ETH_CHIP_BSR_10BASE_T_FD                ((unsigned short)0x1000U)
#define ETH_CHIP_BSR_10BASE_T_HD                ((unsigned short)0x0800U)
#define ETH_CHIP_BSR_100BASE_T2_FD              ((unsigned short)0x0400U)
#define ETH_CHIP_BSR_100BASE_T2_HD              ((unsigned short)0x0200U)
#define ETH_CHIP_BSR_EXTENDED_STATUS            ((unsigned short)0x0100U)
#define ETH_CHIP_BSR_AUTONEGO_CPLT              ((unsigned short)0x0020U)
#define ETH_CHIP_BSR_REMOTE_FAULT               ((unsigned short)0x0010U)
#define ETH_CHIP_BSR_AUTONEGO_ABILITY           ((unsigned short)0x0008U)
#define ETH_CHIP_BSR_LINK_STATUS                ((unsigned short)0x0004U)
#define ETH_CHIP_BSR_JABBER_DETECT              ((unsigned short)0x0002U)
#define ETH_CHIP_BSR_EXTENDED_CAP               ((unsigned short)0x0001U)

/* PHY process status */
#define  ETH_CHIP_STATUS_READ_ERROR             ((int)-5)
#define  ETH_CHIP_STATUS_WRITE_ERROR            ((int)-4)
#define  ETH_CHIP_STATUS_ADDRESS_ERROR          ((int)-3)
#define  ETH_CHIP_STATUS_RESET_TIMEOUT          ((int)-2)
#define  ETH_CHIP_STATUS_ERROR                  ((int)-1)
#define  ETH_CHIP_STATUS_OK                     ((int) 0)
#define  ETH_CHIP_STATUS_LINK_DOWN              ((int) 1)
#define  ETH_CHIP_STATUS_100MBITS_FULLDUPLEX    ((int) 2)
#define  ETH_CHIP_STATUS_100MBITS_HALFDUPLEX    ((int) 3)
#define  ETH_CHIP_STATUS_10MBITS_FULLDUPLEX     ((int) 4)
#define  ETH_CHIP_STATUS_10MBITS_HALFDUPLEX     ((int) 5)
#define  ETH_CHIP_STATUS_AUTONEGO_NOTDONE       ((int) 6)

/* PHY address -- set by user */
#define ETH_CHIP_ADDR                           ((unsigned short)0x0000U)
/* Number of PHY registers */
#define ETH_CHIP_PHY_COUNT                      ((unsigned short)0x001FU)

/* Supported PHY chip types */
#define LAN8720                                 0
#define SR8201F                                 1
#define YT8512C                                 2
#define RTL8201                                 3



/**
 * @brief:  PHY bus init callback (e.g. configure MDC clock)
 *
 * Return: 0 on success, -1 on failure
 */
typedef int  (*eth_chip_init_func)(void);

/**
 * @brief:  PHY bus deinit callback
 *
 * Return: 0 on success, -1 on failure
 */
typedef int  (*eth_chip_deinit_func)(void);

/**
 * @brief:  read PHY register via MDIO bus
 * @addr:   PHY device address (0-31)
 * @reg:    register offset
 * @p_val:  output register value pointer
 *
 * Return: 0 on success, -1 on failure
 */
typedef int  (*eth_chip_read_reg_func)(uint32_t addr, uint32_t reg, uint32_t *p_val);

/**
 * @brief:  write PHY register via MDIO bus
 * @addr:   PHY device address (0-31)
 * @reg:    register offset
 * @val:    register value to write
 *
 * Return: 0 on success, -1 on failure
 */
typedef int  (*eth_chip_write_reg_func)(uint32_t addr, uint32_t reg, uint32_t val);

/**
 * @brief:  get system tick for timeout measurement
 *
 * Return: current tick value (ms)
 */
typedef int  (*eth_chip_get_tick_func)(void);


/**
 * PHY bus IO function table
 */
typedef struct {
	/* bus init callback (e.g. MDC clock config) */
	eth_chip_init_func     init;
	/* bus deinit callback */
	eth_chip_deinit_func   deinit;
	/* write PHY register callback */
	eth_chip_write_reg_func writereg;
	/* read PHY register callback */
	eth_chip_read_reg_func  readreg;
	/* get system tick callback (for timeout) */
	eth_chip_get_tick_func  gettick;
} Eth_Chip_Ioctl_T;

/**
 * PHY device object, holds hardware address and IO context
 */
typedef struct {
	/* detected PHY device address (0-31) */
	uint32_t  devaddr;
	/* initialization flag (0 = not init, 1 = init done) */
	uint32_t  is_initialized;
	/* bus IO function table */
	Eth_Chip_Ioctl_T  io;
	/* user private data pointer */
	void         *p_data;
	/* PHY chip type (LAN8720, SR8201F, YT8512C, RTL8201) */
	int           phy_type;
	/* PHY-specific status register address */
	unsigned short physcsr;
	/* PHY-specific speed status bit mask */
	unsigned short speed_status;
	/* PHY-specific duplex status bit mask */
	unsigned short duplex_status;
} Eth_Chip_Object_T;


/**
 * @brief:  register bus IO functions to PHY device object
 * @obj:  device object pointer
 * @io_ctx: IO function struct pointer
 * Return:  ETH_CHIP_STATUS_OK on success, ETH_CHIP_STATUS_ERROR on missing function
 */
int eth_phy_register_bus_io(Eth_Chip_Object_T *obj, Eth_Chip_Ioctl_T *io_ctx);

/**
 * @brief:  initialize PHY chip, probe address, soft reset and wait ready
 * @obj:  device object pointer
 * Return:  ETH_CHIP_STATUS_OK on success, or error status code
 */
int eth_phy_init(Eth_Chip_Object_T *obj);

/**
 * @brief:  deinitialize PHY chip and release hardware
 * @obj:  device object pointer
 * Return:  ETH_CHIP_STATUS_OK on success, ETH_CHIP_STATUS_ERROR on failure
 */
int eth_phy_deinit(Eth_Chip_Object_T *obj);

/**
 * @brief:  disable PHY power-down mode (clear BCR power-down bit)
 * @obj:  device object pointer
 * Return:  ETH_CHIP_STATUS_OK on success, or read/write error code
 */
int eth_phy_disable_power_down_mode(Eth_Chip_Object_T *obj);

/**
 * @brief:  enable PHY power-down mode (set BCR power-down bit)
 * @obj:  device object pointer
 * Return:  ETH_CHIP_STATUS_OK on success, or read/write error code
 */
int eth_phy_enable_power_down_mode(Eth_Chip_Object_T *obj);

/**
 * @brief:  start PHY auto-negotiation (set BCR auto-nego enable bit)
 * @obj:  device object pointer
 * Return:  ETH_CHIP_STATUS_OK on success, or read/write error code
 */
int eth_phy_start_auto_nego(Eth_Chip_Object_T *obj);

/**
 * @brief:  get current PHY link state (speed/duplex)
 * @obj:  device object pointer
 * Return:  link state code (100M/10M full/half duplex), or ETH_CHIP_STATUS_READ_ERROR
 */
int eth_phy_get_link_state(Eth_Chip_Object_T *obj);

/**
 * @brief:  set PHY link speed and duplex mode
 * @obj:  device object pointer
 * @link_state:  link state code (100M/10M full/half duplex)
 * Return:  ETH_CHIP_STATUS_OK on success, or error code
 */
int eth_phy_set_link_state(Eth_Chip_Object_T *obj, uint32_t link_state);

/**
 * @brief:  enable PHY loopback mode (set BCR loopback bit)
 * @obj:  device object pointer
 * Return:  ETH_CHIP_STATUS_OK on success, or read/write error code
 */
int eth_phy_enable_loopback_mode(Eth_Chip_Object_T *obj);

/**
 * @brief:  disable PHY loopback mode (clear BCR loopback bit)
 * @obj:  device object pointer
 * Return:  ETH_CHIP_STATUS_OK on success, or read/write error code
 */
int eth_phy_disable_loopback_mode(Eth_Chip_Object_T *obj);

#ifdef __cplusplus
}
#endif

#endif
