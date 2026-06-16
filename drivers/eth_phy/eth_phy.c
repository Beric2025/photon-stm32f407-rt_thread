/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * PHY low-level driver interface
 */

#include "eth_phy.h"


#define ETH_CHIP_SW_RESET_TO    ((uint32_t)500U)
#define ETH_PHYInit_TO          ((uint32_t)2000U)
#define ETH_CHIP_MAX_DEV_ADDR   ((uint32_t)31U)

/* Known PHY chip identification: Reg2 + Reg3 values */
#define YT8512C_AND_RTL8201BL_PHYREGISTER2      0x0000
#define SR8201F_PHYREGISTER2                    0x001C
#define LAN8720A_PHYREGISTER2                   0x0007

/* PHY chip info table: runtime auto-detection maps Reg2/Reg3 to these entries */
typedef struct {
	int phy_type;
	unsigned short physcsr;
	unsigned short speed_status;
	unsigned short duplex_status;
} PhyChipInfo;

static const PhyChipInfo phy_chip_table[] = {
	{ LAN8720,  0x1F, 0x0004, 0x0010 },
	{ SR8201F,  0x00, 0x2020, 0x0100 },
	{ YT8512C,  0x11, 0x4010, 0x2000 },
	{ RTL8201,  0x10, 0x0022, 0x0004 },
};

static const PhyChipInfo *phy_chip_lookup(int phy_type)
{
	uint32_t i;
	for (i = 0; i < sizeof(phy_chip_table) / sizeof(phy_chip_table[0]); i++)
	{
		if (phy_chip_table[i].phy_type == phy_type)
			return &phy_chip_table[i];
	}
	return &phy_chip_table[0]; /* fallback to first entry */
}


int  eth_phy_register_bus_io(Eth_Chip_Object_T *obj, Eth_Chip_Ioctl_T *io_ctx)
{
	if (!obj || !io_ctx->readreg || !io_ctx->writereg || !io_ctx->gettick)
	{
		return ETH_CHIP_STATUS_ERROR;
	}

	obj->io.init = io_ctx->init;
	obj->io.deinit = io_ctx->deinit;
	obj->io.readreg = io_ctx->readreg;
	obj->io.writereg = io_ctx->writereg;
	obj->io.gettick = io_ctx->gettick;

	return ETH_CHIP_STATUS_OK;
}

int eth_phy_init(Eth_Chip_Object_T *obj)
{
	uint32_t tickstart = 0, regvalue = 0, addr = 0;
	int status = ETH_CHIP_STATUS_OK;

	/* Runtime PHY auto-detection via Register 2 and 3 */
	obj->io.readreg(addr, PHY_REGISTER2, &regvalue);

	switch (regvalue)
	{
		case YT8512C_AND_RTL8201BL_PHYREGISTER2:
			obj->io.readreg(addr, PHY_REGISTER3, &regvalue);
			if (regvalue == 0x128)
				obj->phy_type = YT8512C;
			else
				obj->phy_type = RTL8201;
			break;
		case SR8201F_PHYREGISTER2:
			obj->phy_type = SR8201F;
			break;
		case LAN8720A_PHYREGISTER2:
			obj->phy_type = LAN8720;
			break;
		default:
			obj->phy_type = YT8512C; /* fallback */
			break;
	}

	/* Populate PHY-specific register info from lookup table */
	{
		const PhyChipInfo *info = phy_chip_lookup(obj->phy_type);
		obj->physcsr       = info->physcsr;
		obj->speed_status  = info->speed_status;
		obj->duplex_status = info->duplex_status;
	}

	if (obj->is_initialized == 0)
	{
		if (obj->io.init != 0)
		{
			/* MDC clock */
			obj->io.init();
		}

		obj->devaddr = ETH_CHIP_MAX_DEV_ADDR + 1;

		/* Probe for PHY address */
		for (addr = 0; addr <= ETH_CHIP_MAX_DEV_ADDR; addr ++)
		{
			if (obj->io.readreg(addr, obj->physcsr, &regvalue) < 0)
			{
				status = ETH_CHIP_STATUS_READ_ERROR;
				continue;
			}
			if ((regvalue & ETH_CHIP_PHY_COUNT) == addr)
			{
				obj->devaddr = addr;
				status = ETH_CHIP_STATUS_OK;
				break;
			}
		}

		if (obj->devaddr > ETH_CHIP_MAX_DEV_ADDR)
		{
			status = ETH_CHIP_STATUS_ADDRESS_ERROR;
		}

		if (status == ETH_CHIP_STATUS_OK)
		{
			if (obj->io.writereg(obj->devaddr, ETH_CHIP_BCR, ETH_CHIP_BCR_SOFT_RESET) >= 0)
			{
				if (obj->io.readreg(obj->devaddr, ETH_CHIP_BCR, &regvalue) >= 0)
				{
					tickstart = obj->io.gettick();

					/* Wait for reset complete or timeout */
					while (regvalue & ETH_CHIP_BCR_SOFT_RESET)
					{
						if ((obj->io.gettick() - tickstart) <= ETH_CHIP_SW_RESET_TO)
						{
							if (obj->io.readreg(obj->devaddr, ETH_CHIP_BCR, &regvalue) < 0)
							{
								status = ETH_CHIP_STATUS_READ_ERROR;
								break;
							}
						}
						else
						{
							status = ETH_CHIP_STATUS_RESET_TIMEOUT;
							break;
						}
					}
				}
				else
				{
					status = ETH_CHIP_STATUS_READ_ERROR;
				}
			}
			else
			{
				status = ETH_CHIP_STATUS_WRITE_ERROR;
			}
		}
	}

	if (status == ETH_CHIP_STATUS_OK)
	{
		tickstart =  obj->io.gettick();

		/* Wait 2s for init */
		while ((obj->io.gettick() - tickstart) <= ETH_PHYInit_TO)
		{
		}
		obj->is_initialized = 1;
	}

	return status;
}

int eth_phy_deinit(Eth_Chip_Object_T *obj)
{
	if (obj->is_initialized)
	{
		if (obj->io.deinit != 0)
		{
			if (obj->io.deinit() < 0)
			{
				return ETH_CHIP_STATUS_ERROR;
			}
		}

		obj->is_initialized = 0;
	}

	return ETH_CHIP_STATUS_OK;
}

int eth_phy_disable_power_down_mode(Eth_Chip_Object_T *obj)
{
	uint32_t readval = 0;
	int status = ETH_CHIP_STATUS_OK;

	if (obj->io.readreg(obj->devaddr, ETH_CHIP_BCR, &readval) >= 0)
	{
		readval &= ~ETH_CHIP_BCR_POWER_DOWN;

		if (obj->io.writereg(obj->devaddr, ETH_CHIP_BCR, readval) < 0)
		{
			status =  ETH_CHIP_STATUS_WRITE_ERROR;
		}
	}
	else
	{
		status = ETH_CHIP_STATUS_READ_ERROR;
	}

	return status;
}

int eth_phy_enable_power_down_mode(Eth_Chip_Object_T *obj)
{
	uint32_t readval = 0;
	int status = ETH_CHIP_STATUS_OK;

	if (obj->io.readreg(obj->devaddr, ETH_CHIP_BCR, &readval) >= 0)
	{
		readval |= ETH_CHIP_BCR_POWER_DOWN;

		if (obj->io.writereg(obj->devaddr, ETH_CHIP_BCR, readval) < 0)
		{
			status =  ETH_CHIP_STATUS_WRITE_ERROR;
		}
	}
	else
	{
		status = ETH_CHIP_STATUS_READ_ERROR;
	}

	return status;
}

int eth_phy_start_auto_nego(Eth_Chip_Object_T *obj)
{
	uint32_t readval = 0;
	int status = ETH_CHIP_STATUS_OK;

	if (obj->io.readreg(obj->devaddr, ETH_CHIP_BCR, &readval) >= 0)
	{
		readval |= ETH_CHIP_BCR_AUTONEGO_EN;

		if (obj->io.writereg(obj->devaddr, ETH_CHIP_BCR, readval) < 0)
		{
			status =  ETH_CHIP_STATUS_WRITE_ERROR;
		}
	}
	else
	{
		status = ETH_CHIP_STATUS_READ_ERROR;
	}

	return status;
}

int eth_phy_get_link_state(Eth_Chip_Object_T *obj)
{
	uint32_t readval = 0;

	if (obj->io.readreg(obj->devaddr, obj->physcsr, &readval) < 0)
	{
		return ETH_CHIP_STATUS_READ_ERROR;
	}

	if (((readval & obj->speed_status) != obj->speed_status) && ((readval & obj->duplex_status) != 0))
	{
		return ETH_CHIP_STATUS_100MBITS_FULLDUPLEX;
	}
	else if (((readval & obj->speed_status) != obj->speed_status))
	{
		return ETH_CHIP_STATUS_100MBITS_HALFDUPLEX;
	}
	else if (((readval & ETH_CHIP_BCR_DUPLEX_MODE) != ETH_CHIP_BCR_DUPLEX_MODE))
	{
		return ETH_CHIP_STATUS_10MBITS_FULLDUPLEX;
	}
	else
	{
		return ETH_CHIP_STATUS_10MBITS_HALFDUPLEX;
	}
}

int eth_phy_set_link_state(Eth_Chip_Object_T *obj, uint32_t link_state)
{
	uint32_t bcrvalue = 0;
	int status = ETH_CHIP_STATUS_OK;

	if (obj->io.readreg(obj->devaddr, ETH_CHIP_BCR, &bcrvalue) >= 0)
	{
		bcrvalue &= ~(ETH_CHIP_BCR_AUTONEGO_EN | ETH_CHIP_BCR_SPEED_SELECT | ETH_CHIP_BCR_DUPLEX_MODE);

		if (link_state == ETH_CHIP_STATUS_100MBITS_FULLDUPLEX)
		{
			bcrvalue |= (ETH_CHIP_BCR_SPEED_SELECT | ETH_CHIP_BCR_DUPLEX_MODE);
		}
		else if (link_state == ETH_CHIP_STATUS_100MBITS_HALFDUPLEX)
		{
			bcrvalue |= ETH_CHIP_BCR_SPEED_SELECT;
		}
		else if (link_state == ETH_CHIP_STATUS_10MBITS_FULLDUPLEX)
		{
			bcrvalue |= ETH_CHIP_BCR_DUPLEX_MODE;
		}
		else
		{
			status = ETH_CHIP_STATUS_ERROR;
		}
	}
	else
	{
		status = ETH_CHIP_STATUS_READ_ERROR;
	}

	if(status == ETH_CHIP_STATUS_OK)
	{
		if(obj->io.writereg(obj->devaddr, ETH_CHIP_BCR, bcrvalue) < 0)
		{
			status = ETH_CHIP_STATUS_WRITE_ERROR;
		}
	}

	return status;
}

int eth_phy_enable_loopback_mode(Eth_Chip_Object_T *obj)
{
	uint32_t readval = 0;
	int status = ETH_CHIP_STATUS_OK;

	if (obj->io.readreg(obj->devaddr, ETH_CHIP_BCR, &readval) >= 0)
	{
		readval |= ETH_CHIP_BCR_LOOPBACK;

		if (obj->io.writereg(obj->devaddr, ETH_CHIP_BCR, readval) < 0)
		{
			status = ETH_CHIP_STATUS_WRITE_ERROR;
		}
	}
	else
	{
		status = ETH_CHIP_STATUS_READ_ERROR;
	}

	return status;
}

int eth_phy_disable_loopback_mode(Eth_Chip_Object_T *obj)
{
	uint32_t readval = 0;
	int status = ETH_CHIP_STATUS_OK;

	if (obj->io.readreg(obj->devaddr, ETH_CHIP_BCR, &readval) >= 0)
	{
		readval &= ~ETH_CHIP_BCR_LOOPBACK;

		if (obj->io.writereg(obj->devaddr, ETH_CHIP_BCR, readval) < 0)
		{
			status =  ETH_CHIP_STATUS_WRITE_ERROR;
		}
	}
	else
	{
		status = ETH_CHIP_STATUS_READ_ERROR;
	}

	return status;
}
