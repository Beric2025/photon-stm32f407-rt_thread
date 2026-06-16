/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * IAP (In-Application Programming) boot management
 */

#include <stdio.h>
#include <string.h>
#include <rtthread.h>
#include "main.h"
#include "app_manage.h"
#include "iap.h"
#include "bsp_flash.h"
#include "flash_dev.h"
#include "delay_dev.h"
#include "log_print.h"


Boot_Param_T boot_param;
uint32_t g_update_file_total_size;


/*
 * Write boot parameter data to SETTING flash sector.
 * Retries up to 5 times on failure.
 */
static void write_setting_boot(Boot_Param_T *param)
{
    uint32_t setting_addr = SETTING_SECTOR_ADDR;
    uint8_t ret = 0, cnt = 5;
    uint32_t *buf = (uint32_t *)param;
    Flash_Device_T *flash = get_flash_device("flash0");

    if (!flash) {
        LOG_PRINT(LOG_OUT_ERROR, "write flag fail: no flash device!\n");
        return;
    }

    while(cnt--) {
        ret = flash->erase_and_write(flash, setting_addr, buf, sizeof(Boot_Param_T)/4);
        if(ret == 0)  {
            LOG_PRINT(LOG_OUT_DEBUG, "write flag ok!\n");
            break;
        }
        delay_ms(10);
    }

    if(ret){
        LOG_PRINT(LOG_OUT_ERROR, "write flag fail!\n");
    }
}

/*
 * Set the update flag in boot parameters and write to flash.
 * This signals the bootloader to enter firmware update mode on next reset.
 */
void set_update_flag(void)
{
    boot_param.status = UPDATE_PROGRAM;
    boot_param.download_size = g_update_file_total_size;
    boot_param.power = POWER_SETUP_ON;
    write_setting_boot(&boot_param);
}

/*
 * Read boot parameter data from SETTING flash sector.
 */
static void read_setting_boot(Boot_Param_T *param)
{
    uint32_t setting_addr = SETTING_SECTOR_ADDR;
    Flash_Device_T *flash = get_flash_device("flash0");

    if (!flash) {
        LOG_PRINT(LOG_OUT_ERROR, "read setting boot fail: no flash device!\n");
        return;
    }

    /* read update status */
    param->status = flash->read_word(flash, setting_addr);
    delay_ms(5);
    /* read firmware file size */
    param->download_size = flash->read_word(flash, setting_addr + 4);
    delay_ms(5);
    /* read external power control flag */
    param->power = flash->read_word(flash, setting_addr + 8);
    delay_ms(5);
}

/*
 * Read the external power state from boot parameters.
 * Returns current power state (POWER_SETUP_ON or POWER_SETUP_OFF).
 */
unsigned char read_boot_power_status(void)
{
    read_setting_boot(&boot_param);
    return boot_param.power;
}

/*
 * Set the external power state in boot parameters.
 * Only writes to flash if the state has changed.
 */
void set_boot_power(unsigned char status)
{
    read_setting_boot(&boot_param);
    if(boot_param.power != status) {
        boot_param.power = status;
        write_setting_boot(&boot_param);
    }
}

/*
 * Update boot status, file size, and power state in flash.
 * Parameters:
 *   status   - update status (START_PROGRAM / UPDATE_PROGRAM / UPDATE_SUCCESS)
 *   file_size - firmware file size in bytes
 *   power    - external power state (POWER_SETUP_ON / POWER_SETUP_OFF)
 */
void update_boot_status(unsigned char status, uint32_t file_size, unsigned char power)
{
    read_setting_boot(&boot_param);
    if((boot_param.status != status) || (boot_param.power != power)) {
        boot_param.status = status;
        boot_param.download_size = file_size;
        boot_param.power = power;
        write_setting_boot(&boot_param);
    }
}

/*
 * Perform a software reset of the MCU.
 */
void reset_mcu(void)
{
    LOG_PRINT(LOG_OUT_INFO, "MCU reset\n");
    __set_FAULTMASK(1);
    NVIC_SystemReset();
}


/*
 * Erase a range of flash sectors.
 * Returns 0 on success, -1 on failure.
 */
int iap_erase_buf(uint32_t start_addr, uint32_t size)
{
    Flash_Device_T *flash = get_flash_device("flash0");
    if (!flash) return -1;

    return flash->erase(flash, start_addr, size);
}

/*
 * Write data buffer to flash.
 * Returns 0 on success, -1 on failure.
 */
int iap_write_buf(uint32_t start_addr, uint32_t *buf, uint32_t size)
{
    Flash_Device_T *flash = get_flash_device("flash0");
    if (!flash) return -1;

    return flash->write(flash, start_addr, buf, size);
}

/*
 * Mark the firmware update as completed.
 * Sets the update flag and triggers an MCU reset.
 */
void iap_update_file_completed(void)
{
    LOG_PRINT(LOG_OUT_INFO, "Updata data process completed, Reset MCU!\n");
    set_update_flag();
}
