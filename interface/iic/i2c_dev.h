/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef _I2C_DEV_H_
#define _I2C_DEV_H_

#ifdef __cplusplus
 	extern "C" {
#endif
#include <stdint.h>

typedef struct {
	/* I2C controller identifier (e.g. "i2c1", "i2c2") */
	char *name;

	/**
	 * @brief:  initialize I2C peripheral
	 * @privatedata:  I2C handle (I2c_Device_T *)
	 * @baudrate:        I2C bus baudrate
	 *
	 * Return: 0 on success, -1 on failure
	 */
	int (*init)(void *privatedata, uint32_t baudrate);

	/**
	 * @brief:  write data to I2C device register
	 * @privatedata:  I2C handle (I2c_Device_T *)
	 * @devaddr:        target device address
	 * @regaddr:        target register address
	 * @buff:           data buffer to write
	 * @length:         number of bytes to write
	 *
	 * Return: 0 on success, -1 on failure
	 */
	int (*write)(void *privatedata, unsigned short devaddr, unsigned short regaddr,
	             unsigned char *buff, unsigned short length);

	/**
	 * @brief:  read data from I2C device register
	 * @privatedata:  I2C handle (I2c_Device_T *)
	 * @devaddr:        target device address
	 * @regaddr:        target register address
	 * @buff:           buffer for received data
	 * @length:         number of bytes to read
	 *
	 * Return: 0 on failure, number of bytes read on success
	 */
	int (*read)(void *privatedata, unsigned short devaddr, unsigned short regaddr,
	            unsigned char *buff, unsigned short length);

	/**
	 * @brief:  transmit data to I2C device (no register address)
	 * @privatedata:  I2C handle (I2c_Device_T *)
	 * @devaddr:        target device address
	 * @buff:           data buffer to send
	 * @length:         number of bytes to send
	 *
	 * Return: 0 on success, -1 on failure
	 */
	int (*transmit)(void *privatedata, unsigned short devaddr, unsigned char *buff, 
				unsigned short length);

	/**
	 * @brief:  receive data from I2C device (no register address)
	 * @privatedata:  I2C handle (I2c_Device_T *)
	 * @devaddr:        target device address
	 * @buff:           buffer for received data
	 * @length:         length of the buffer
	 *
	 * Return: 0 on success, -1 on failure
	 */
	int (*receive)(void *privatedata, unsigned short devaddr, unsigned char *buff, 
				unsigned short length);

	void *private_data;
}I2c_Device_T;

/**
 * @brief:  get I2C device structure by name
 * @name:   device name (e.g. "i2c1", "i2c2")
 *
 * Return: pointer to I2c_Device_T structure, NULL if not found
 */
I2c_Device_T *get_i2c_device(char *name);

#ifdef __cplusplus
}
#endif

#endif	/* _I2C_DEV_H_*/
