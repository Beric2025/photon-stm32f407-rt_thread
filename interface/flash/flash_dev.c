/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * Flash management
 */

#include <string.h>
#include <stdio.h>
#include "main.h"
#include "flash_dev.h"
#include "bsp_flash.h"
#include "log_print.h"

#define TAG "flash dev: "


/* Private data structure */
typedef struct {
    unsigned char mode;

    unsigned char (*get_sector)(uint32_t addr);
    unsigned char (*get_sector_with_bank)(uint32_t addr, uint32_t *bank);
    unsigned char (*erase)(uint32_t writeaddr, uint32_t num);
    unsigned char (*write)(uint32_t writeaddr, uint32_t *buff, uint32_t num);
    void          (*read)(uint32_t raddr, uint32_t *buff, uint32_t length);
    uint32_t  (*read_word)(uint32_t faddr);
    unsigned char (*erase_and_write)(uint32_t writeaddr, uint32_t *buff, uint32_t num);
} Flash_Data_T;

/* Function declarations */
static int flash_init(void *privatedata);
static int flash_get_sector(void *privatedata, uint32_t addr);
static int flash_get_sector_with_bank(void *privatedata, uint32_t addr, uint32_t *bank);
static int flash_erase(void *privatedata, uint32_t writeaddr, uint32_t num);
static int flash_write(void *privatedata, uint32_t writeaddr, uint32_t *buff, uint32_t num);
static int flash_read(void *privatedata, uint32_t readaddr, uint32_t *buff, uint32_t length);
static uint32_t flash_read_word(void *privatedata, uint32_t readaddr);
static int flash_erase_and_write(void *privatedata, uint32_t writeaddr, uint32_t *buff, uint32_t num);

/* Initialization */
static Flash_Data_T s_flash0_dev_data = {
    .mode                = 0,
    .get_sector          = stmflash_get_flash_sector,
    .get_sector_with_bank = stmflash_get_flash_sector_with_bank,
    .erase               = stmflash_erase,
    .write               = stmflash_write,
    .read                = stmflash_read,
    .read_word           = stmflash_read_word,
    .erase_and_write     = stmflash_erase_and_write,
};

/* Device structure definition */
static Flash_Device_T s_flash0_dev = {
    .name = "flash0",
    .init = flash_init,
    .get_sector = flash_get_sector,
    .get_sector_with_bank = flash_get_sector_with_bank,
    .erase = flash_erase,
    .write = flash_write,
    .read = flash_read,
    .read_word = flash_read_word,
    .erase_and_write = flash_erase_and_write,
    .private_data = &s_flash0_dev_data,
};

static int flash_init(void *privatedata)
{
    if(privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s: flash init private data is null!\n", TAG);
        return -1;
    }
		
		privatedata = privatedata;

//    Flash_Device_T *flash_dev = (Flash_Device_T *)privatedata;
//    Flash_Data_T *flash_data = (Flash_Data_T *)flash_dev->private_data;

    /* Internal flash requires no HAL initialization */
    LOG_PRINT(LOG_OUT_DEBUG, "%s: flash init successful!\n", TAG);
    return 0;
}

static int flash_get_sector(void *privatedata, uint32_t addr)
{
    if(privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s: flash get sector private data is null!\n", TAG);
        return -1;
    }

    Flash_Device_T *flash_dev = (Flash_Device_T *)privatedata;
    Flash_Data_T *flash_data = (Flash_Data_T *)flash_dev->private_data;

    unsigned char sector = flash_data->get_sector(addr);
    if(sector == 0xFF) {
        LOG_PRINT(LOG_OUT_ERROR, "%s: flash get sector invalid address!\n", TAG);
        return -1;
    }

    LOG_PRINT(LOG_OUT_DEBUG, "%s: flash get sector successful!\n", TAG);
    return sector;
}

static int flash_get_sector_with_bank(void *privatedata, uint32_t addr, uint32_t *bank)
{
    if(privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s: flash get sector with bank private data is null!\n", TAG);
        return -1;
    }

    Flash_Device_T *flash_dev = (Flash_Device_T *)privatedata;
    Flash_Data_T *flash_data = (Flash_Data_T *)flash_dev->private_data;

    unsigned char sector = flash_data->get_sector_with_bank(addr, bank);
    if(sector == 0xFF) {
        LOG_PRINT(LOG_OUT_ERROR, "%s: flash get sector with bank invalid address!\n", TAG);
        return -1;
    }

    LOG_PRINT(LOG_OUT_DEBUG, "%s: flash get sector with bank successful!\n", TAG);
    return sector;
}

static int flash_erase(void *privatedata, uint32_t writeaddr, uint32_t num)
{
    if(privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s: flash erase private data is null!\n", TAG);
        return -1;
    }

    Flash_Device_T *flash_dev = (Flash_Device_T *)privatedata;
    Flash_Data_T *flash_data = (Flash_Data_T *)flash_dev->private_data;

    unsigned char ret = flash_data->erase(writeaddr, num);
    if(ret != 0) {
        LOG_PRINT(LOG_OUT_ERROR, "%s: flash erase failed, ret=%d!\n", TAG, ret);
        return -1;
    }

    LOG_PRINT(LOG_OUT_DEBUG, "%s: flash erase successful!\n", TAG);
    return 0;
}

static int flash_write(void *privatedata, uint32_t writeaddr, uint32_t *buff, uint32_t num)
{
    if(privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s: flash write private data is null!\n", TAG);
        return -1;
    }

    Flash_Device_T *flash_dev = (Flash_Device_T *)privatedata;
    Flash_Data_T *flash_data = (Flash_Data_T *)flash_dev->private_data;

    unsigned char ret = flash_data->write(writeaddr, buff, num);
    if(ret != 0) {
        LOG_PRINT(LOG_OUT_ERROR, "%s: flash write failed, ret=%d!\n", TAG, ret);
        return -1;
    }

    LOG_PRINT(LOG_OUT_DEBUG, "%s: flash write successful!\n", TAG);
    return 0;
}

static int flash_read(void *privatedata, uint32_t readaddr, uint32_t *buff, uint32_t length)
{
    if(privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s: flash read private data is null!\n", TAG);
        return -1;
    }

    Flash_Device_T *flash_dev = (Flash_Device_T *)privatedata;
    Flash_Data_T *flash_data = (Flash_Data_T *)flash_dev->private_data;

    flash_data->read(readaddr, buff, length);

    LOG_PRINT(LOG_OUT_DEBUG, "%s: flash read successful!\n", TAG);
    return 0;
}

static uint32_t flash_read_word(void *privatedata, uint32_t readaddr)
{
    if(privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s: flash read word private data is null!\n", TAG);
        return 0;
    }

    Flash_Device_T *flash_dev = (Flash_Device_T *)privatedata;
    Flash_Data_T *flash_data = (Flash_Data_T *)flash_dev->private_data;

    uint32_t val = flash_data->read_word(readaddr);

    LOG_PRINT(LOG_OUT_DEBUG, "%s: flash read word successful!\n", TAG);
    return val;
}

static int flash_erase_and_write(void *privatedata, uint32_t writeaddr, uint32_t *buff, uint32_t num)
{
    if(privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s: flash erase and write private data is null!\n", TAG);
        return -1;
    }

    Flash_Device_T *flash_dev = (Flash_Device_T *)privatedata;
    Flash_Data_T *flash_data = (Flash_Data_T *)flash_dev->private_data;

    unsigned char ret = flash_data->erase_and_write(writeaddr, buff, num);
    if(ret != 0) {
        LOG_PRINT(LOG_OUT_ERROR, "%s: flash erase and write failed, ret=%d!\n", TAG, ret);
        return -1;
    }

    LOG_PRINT(LOG_OUT_DEBUG, "%s: flash erase and write successful!\n", TAG);
    return 0;
}

Flash_Device_T *get_flash_device(char *name)
{
    Flash_Device_T *flash_dev = NULL;

    if(0 == strcmp(name , "flash0"))
      flash_dev = &s_flash0_dev;

    return flash_dev;
}
