/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef _FLASH_DEV_H_
#define _FLASH_DEV_H_

#ifdef __cplusplus
 	extern "C" {
#endif
#include <stdint.h>


typedef struct {
	/* Flash device identifier (e.g. "flash0") */
	char *name;

	/**
	 * @brief:  initialize flash peripheral
	 * @privatedata:  flash handle (Flash_Device_T *)
	 *
	 * Return: 0 on success, -1 on failure
	 */
	int (*init)(void *privatedata);

	/**
	 * @brief:  get flash sector number for a given address
	 * @privatedata:  flash handle (Flash_Device_T *)
	 * @addr:         flash address
	 *
	 * Return: sector number (0~15), 0xFF on invalid address
	 */
	int (*get_sector)(void *privatedata, uint32_t addr);

	/**
	 * @brief:  get flash sector and bank for a given address
	 * @privatedata:  flash handle (Flash_Device_T *)
	 * @addr:         flash address
	 * @bank:         output, returns FLASH_BANK_1 or FLASH_BANK_2
	 *
	 * Return: sector number (0~7), 0xFF on invalid address
	 */
	int (*get_sector_with_bank)(void *privatedata, uint32_t addr, uint32_t *bank);

	/**
	 * @brief:  erase flash data starting from the given address
	 * @privatedata:  flash handle (Flash_Device_T *)
	 * @writeaddr:    start address (must be 32-byte aligned)
	 * @num:          number of 32-bit words to erase
	 *
	 * Return: 0 on success, -1 on failure
	 */
	int (*erase)(void *privatedata, uint32_t writeaddr, uint32_t num);

	/**
	 * @brief:  write data to flash (sector must be pre-erased)
	 * @privatedata:  flash handle (Flash_Device_T *)
	 * @writeaddr:    start address (must be 32-byte aligned)
	 * @buff:         data buffer pointer
	 * @num:          number of 32-bit words to write
	 *
	 * Return: 0 on success, -1 on failure
	 */
	int (*write)(void *privatedata, uint32_t writeaddr, uint32_t *buff, uint32_t num);

	/**
	 * @brief:  read data from flash
	 * @privatedata:  flash handle (Flash_Device_T *)
	 * @readaddr:     start address
	 * @buff:         buffer for received data
	 * @length:       number of 32-bit words to read
	 *
	 * Return: 0 on success, -1 on failure
	 */
	int (*read)(void *privatedata, uint32_t readaddr, uint32_t *buff, uint32_t length);

	/**
	 * @brief:  read a single 32-bit word from flash
	 * @privatedata:  flash handle (Flash_Device_T *)
	 * @readaddr:     flash address
	 *
	 * Return: 32-bit value read from flash
	 */
	uint32_t (*read_word)(void *privatedata, uint32_t readaddr);

	/**
	 * @brief:  erase sector then write data to flash
	 * @privatedata:  flash handle (Flash_Device_T *)
	 * @writeaddr:    start address (must be 32-byte aligned)
	 * @buff:         data buffer pointer
	 * @num:          number of 32-bit words to write
	 *
	 * Return: 0 on success, -1 on failure
	 */
	int (*erase_and_write)(void *privatedata, uint32_t writeaddr, uint32_t *buff, uint32_t num);

	void *private_data;
} Flash_Device_T;

/**
 * @brief:  get flash device structure by name
 * @name:   device name (e.g. "flash0")
 *
 * Return: pointer to Flash_Device_T structure, NULL if not found
 */
Flash_Device_T *get_flash_device(char *name);

#ifdef __cplusplus
}
#endif

#endif	/* _FLASH_DEV_H_ */
