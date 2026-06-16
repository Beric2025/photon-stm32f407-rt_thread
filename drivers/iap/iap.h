/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef _IAP_H_
#define _IAP_H_

#ifdef __cplusplus
 extern "C" {
#endif
#include <stdint.h>

#define START_PROGRAM           0                               /* modify new program */
#define UPDATE_PROGRAM          1                               /* update new program */
#define UPDATE_SUCCESS          2                               /* update success */

#define POWER_SETUP_ON          1                               /* turn on external power */
#define POWER_SETUP_OFF         0                               /* turn off external power */

typedef struct {
    uint32_t status;            /* boot status */
    uint32_t download_size;     /* download file size */
    uint32_t power;             /* external power state */
} Boot_Param_T;

extern Boot_Param_T boot_param;
extern uint32_t g_update_file_total_size;

/**
 * @brief:  set update flag and reset MCU to enter bootloader
 */
void set_update_flag(void);

/**
 * @brief:  software reset MCU
 */
void reset_mcu(void);

/**
 * @brief:  erase flash sector range
 * @start_addr:  start address of flash to erase
 * @size:        number of 32-bit words to erase
 * Return: 0 on success, -1 on failure
 */
int iap_erase_buf(uint32_t start_addr, uint32_t size);

/**
 * @brief:  write data to flash (sector must be pre-erased)
 * @start_addr:  start address of flash to write
 * @buf:         pointer to data buffer
 * @size:        number of 32-bit words to write
 * Return: 0 on success, -1 on failure
 */
int iap_write_buf(uint32_t start_addr, uint32_t *buf, uint32_t size);

/**
 * @brief:  mark firmware update as completed
 */
void iap_update_file_completed(void);

/**
 * @brief:  set external power state
 * @status:  power state (POWER_SETUP_ON / POWER_SETUP_OFF)
 */
void set_boot_power(unsigned char status);

/**
 * @brief:  read external power state
 * Return: current power state
 */
unsigned char read_boot_power_status(void);

/**
 * @brief:  update boot status with file info
 * @status:    update status
 * @file_size: firmware file size
 * @power:     power state
 */
void update_boot_status(unsigned char status, uint32_t file_size, unsigned char power);

#ifdef __cplusplus
}
#endif

#endif /* _IAP_H_ */
