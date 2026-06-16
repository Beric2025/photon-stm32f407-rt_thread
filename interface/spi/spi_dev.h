/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef _SPI_DEV_H_
#define _SPI_DEV_H_

#ifdef __cplusplus
 extern "C" {
#endif
#include <stdint.h>

typedef struct {
	/* SPI controller identifier (e.g. "spi1", "spi2") */
	char *name;

	/**
	 * @brief:  initialize SPI peripheral
	 * @privatedata:  SPI handle (Spi_Device_T *)
	 *
	 * Return: 0 on success, -1 on failure
	 */
	int (*init)(void *privatedata);

	/**
	 * @brief:  full-duplex transmit/receive data (no register addressing)
	 * @privatedata:  SPI handle (Spi_Device_T *)
	 * @buff:         data buffer to send (also receives data on MISO)
	 * @length:         number of bytes to transfer
	 *
	 * Return: 0 on success, -1 on failure
	 */
	int (*transfer)(void *privatedata, unsigned char *buff, unsigned short length);

	/**
	 * @brief:  write data to SPI device register
	 * @privatedata:  SPI handle (Spi_Device_T *)
	 * @regaddr:        target register address
	 * @buff:           data buffer to write
	 * @length:         number of bytes to write
	 *
	 * Return: 0 on success, -1 on failure
	 */
	int (*write)(void *privatedata, uint32_t regaddr, unsigned char *buff, 
					unsigned short length);

	/**
	 * @brief:  read data from SPI device register
	 * @privatedata:  SPI handle (Spi_Device_T *)
	 * @regaddr:        target register address
	 * @buff:           buffer for received data
	 * @length:         number of bytes to read
	 *
	 * Return: 0 on success, -1 on failure
	 */
	int (*read)(void *privatedata, uint32_t regaddr, unsigned char *buff, 
					unsigned short length);

	void *private_data;
}Spi_Device_T;

/**
 * @brief:  get SPI device structure by name
 * @name:   device name (e.g. "spi1", "spi2")
 *
 * Return: pointer to Spi_Device_T structure, NULL if not found
 */
Spi_Device_T *get_spi_device(char *name);

#ifdef __cplusplus
}
#endif

#endif	/* _SPI_DEV_H_*/
