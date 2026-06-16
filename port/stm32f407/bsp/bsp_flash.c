/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * STM32F4xx internal flash driver (BSP layer)
 */

#include "bsp_flash.h"
#include "stm32f4xx_hal.h"


/* sector start addresses — defined in bsp_flash.h, single source of truth */
static const uint32_t s_sector_start[FLASH_SECTOR_COUNT + 1] = {
    FLASH_SECTOR_0_ADDR,
    FLASH_SECTOR_1_ADDR,
    FLASH_SECTOR_2_ADDR,
    FLASH_SECTOR_3_ADDR,
    FLASH_SECTOR_4_ADDR,
    FLASH_SECTOR_5_ADDR,
    FLASH_SECTOR_6_ADDR,
    FLASH_SECTOR_7_ADDR,
    FLASH_SECTOR_8_ADDR,
    FLASH_SECTOR_9_ADDR,
    FLASH_SECTOR_10_ADDR,
    FLASH_SECTOR_11_ADDR,
    FLASH_END_ADDR,
};


/* -------------------------------------------------------------------------- */
/*  internal helpers                                                          */
/* -------------------------------------------------------------------------- */

/*
 * Find the flash sector index for a given address.
 * Return: 0~11 on success, 0xFF on invalid address.
 */
static unsigned char find_sector(uint32_t addr)
{
    unsigned char i;

    for (i = 0; i < FLASH_SECTOR_COUNT; i++) {
        if (addr >= s_sector_start[i] && addr < s_sector_start[i + 1]) {
            return i;
        }
    }
    return 0xFF;
}

/*
 * Unlock flash, execute the operation, lock flash.
 * Caller passes a function pointer — keeps lock/unlock logic centralized.
 */
static unsigned char flash_op_begin(void)
{
    if (HAL_FLASH_Unlock() != HAL_OK) {
        return 1;   /* unlock failed */
    }
    return 0;
}

static void flash_op_end(void)
{
    HAL_FLASH_Lock();
}


/* -------------------------------------------------------------------------- */
/*  public API                                                                */
/* -------------------------------------------------------------------------- */

/*
 * Get the flash sector number for a given address.
 * Return: 0~11 for valid sectors, 0xFF on invalid address.
 */
unsigned char stmflash_get_flash_sector(uint32_t addr)
{
    return find_sector(addr);
}

/*
 * Get the flash sector and bank for a given address.
 * STM32F407 has only one bank — always returns FLASH_BANK_1.
 * Return: 0~11 sector number, 0xFF on invalid address.
 */
unsigned char stmflash_get_flash_sector_with_bank(uint32_t addr, uint32_t *bank)
{
    unsigned char sector = stmflash_get_flash_sector(addr);

    if (bank != NULL) {
        *bank = FLASH_BANK_1;   /* F407 single bank */
    }
    return sector;
}

/*
 * Read a single 32-bit word from the specified flash address.
 * Return: the 32-bit value read.
 */
uint32_t stmflash_read_word(uint32_t faddr)
{
    return *(__IO uint32_t *)faddr;
}

/*
 * Read data of the specified length from the given address.
 * @raddr:  start address
 * @buff:   data buffer pointer (output)
 * @length: number of 32-bit words to read
 */
void stmflash_read(uint32_t raddr, uint32_t *buff, uint32_t length)
{
    uint32_t i;

    for (i = 0; i < length; i++) {
        buff[i] = stmflash_read_word(raddr);
        raddr += 4;
    }
}

/*
 * Erase sectors covering the specified range.
 *
 * Flash erasure works on whole sectors only — all sectors that overlap
 * [writeaddr, writeaddr + num*4) are erased. This destroys all data within
 * those sectors, not just the requested range.
 *
 * @writeaddr: start address (must be flash-mapped address)
 * @num:       number of 32-bit words in the range
 *
 * Return: 0=success, 1=invalid address, 3=erase error, 4=timeout
 */
unsigned char stmflash_erase(uint32_t writeaddr, uint32_t num)
{
    unsigned char start_sector;
    unsigned char end_sector;
    uint32_t        sector_error;
    HAL_StatusTypeDef status;
    FLASH_EraseInitTypeDef erase_init;
    unsigned char ret;

    if (num == 0) {
        return 0;   /* nothing to erase */
    }

    start_sector = find_sector(writeaddr);
    if (start_sector == 0xFF) {
        return 1;
    }

    end_sector = find_sector(writeaddr + (num * 4) - 1);
    if (end_sector == 0xFF) {
        return 1;
    }

    /* unlock → erase → lock */
    ret = flash_op_begin();
    if (ret != 0) {
        return 3;
    }

    erase_init.TypeErase    = FLASH_TYPEERASE_SECTORS;
    erase_init.Banks        = FLASH_BANK_1;
    erase_init.Sector       = start_sector;
    erase_init.NbSectors    = end_sector - start_sector + 1;
    erase_init.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    status = HAL_FLASHEx_Erase(&erase_init, &sector_error);
    if (status != HAL_OK) {
        ret = (status == HAL_TIMEOUT) ? 4 : 3;
    }

    flash_op_end();
    return ret;
}

/*
 * Write data to flash (sector must be pre-erased).
 *
 * @writeaddr: start address (must be word-aligned)
 * @buff:      data buffer pointer
 * @num:       number of 32-bit words to write
 *
 * Return: 0=success, 1=invalid address, 2=write error
 */
unsigned char stmflash_write(uint32_t writeaddr, uint32_t *buff, uint32_t num)
{
    uint32_t        i;
    HAL_StatusTypeDef status;
    unsigned char ret;

    if (num == 0 || buff == NULL) {
        return 0;
    }

    /* validate range */
    if (find_sector(writeaddr) == 0xFF) {
        return 1;
    }

    ret = flash_op_begin();
    if (ret != 0) {
        return 2;
    }

    ret = 0;
    for (i = 0; i < num; i++) {
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
                                   writeaddr + i * 4,
                                   (uint64_t)buff[i]);
        if (status != HAL_OK) {
            ret = 2;
            break;
        }
    }

    flash_op_end();
    return ret;
}

/*
 * Erase sector(s) then write data at the specified address.
 *
 * Convenience wrapper: erases all sectors overlapping the target range,
 * then writes the data.  Existing data in affected sectors is destroyed.
 *
 * @writeaddr: start address (must be word-aligned)
 * @buff:      data buffer pointer
 * @num:       number of 32-bit words to write
 *
 * Return: 0=success, 1=invalid address, 2=write error, 3=erase error, 4=timeout
 */
unsigned char stmflash_erase_and_write(uint32_t writeaddr, uint32_t *buff, uint32_t num)
{
    unsigned char ret;

    ret = stmflash_erase(writeaddr, num);
    if (ret != 0) {
        return ret;
    }

    ret = stmflash_write(writeaddr, buff, num);
    return ret;
}
