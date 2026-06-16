/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef __BSP_FLASH_H
#define __BSP_FLASH_H

#include <stdint.h>

/* FLASH base address */
#define STM32_FLASH_BASE 0x08000000 /* STM32 FLASH base address */
#define STM32_FLASH_SIZE 0x0100000 /* STM32F407 FLASH total 1 MB */
#define FLASH_WAIT_TIME  50000     /* FLASH wait timeout */


/* -------------------------------------------------------------------------- */
/*  STM32F407 flash sector layout (non-uniform, 12 sectors)                   */
/*                                                                             */
/*  Sector  0:  0x08000000 - 0x08003FFF   16 KB                                */
/*  Sector  1:  0x08004000 - 0x08007FFF   16 KB                                */
/*  Sector  2:  0x08008000 - 0x0800BFFF   16 KB                                */
/*  Sector  3:  0x0800C000 - 0x0800FFFF   16 KB                                */
/*  Sector  4:  0x08010000 - 0x0801FFFF   64 KB                                */
/*  Sector  5:  0x08020000 - 0x0803FFFF  128 KB                                */
/*  Sector  6:  0x08040000 - 0x0805FFFF  128 KB                                */
/*  Sector  7:  0x08060000 - 0x0807FFFF  128 KB                                */
/*  Sector  8:  0x08080000 - 0x0809FFFF  128 KB                                */
/*  Sector  9:  0x080A0000 - 0x080BFFFF  128 KB                                */
/*  Sector 10:  0x080C0000 - 0x080DFFFF  128 KB                                */
/*  Sector 11:  0x080E0000 - 0x080FFFFF  128 KB                                */
/*  End:        0x08100000                                                     */
/* -------------------------------------------------------------------------- */

#define FLASH_SECTOR_0_ADDR     ((uint32_t)0x08000000)  /* sector  0:  16 KB */
#define FLASH_SECTOR_1_ADDR     ((uint32_t)0x08004000)  /* sector  1:  16 KB */
#define FLASH_SECTOR_2_ADDR     ((uint32_t)0x08008000)  /* sector  2:  16 KB */
#define FLASH_SECTOR_3_ADDR     ((uint32_t)0x0800C000)  /* sector  3:  16 KB */
#define FLASH_SECTOR_4_ADDR     ((uint32_t)0x08010000)  /* sector  4:  64 KB */
#define FLASH_SECTOR_5_ADDR     ((uint32_t)0x08020000)  /* sector  5: 128 KB */
#define FLASH_SECTOR_6_ADDR     ((uint32_t)0x08040000)  /* sector  6: 128 KB */
#define FLASH_SECTOR_7_ADDR     ((uint32_t)0x08060000)  /* sector  7: 128 KB */
#define FLASH_SECTOR_8_ADDR     ((uint32_t)0x08080000)  /* sector  8: 128 KB */
#define FLASH_SECTOR_9_ADDR     ((uint32_t)0x080A0000)  /* sector  9: 128 KB */
#define FLASH_SECTOR_10_ADDR    ((uint32_t)0x080C0000)  /* sector 10: 128 KB */
#define FLASH_SECTOR_11_ADDR    ((uint32_t)0x080E0000)  /* sector 11: 128 KB */
#define FLASH_END_ADDR          ((uint32_t)0x08100000)  /* flash end        */

#define FLASH_SECTOR_COUNT      12U


/* -------------------------------------------------------------------------- */
/*  flash partition layout (product-level config)                              */
/* -------------------------------------------------------------------------- */

#define BOARD_NUM_ADDR          FLASH_SECTOR_5_ADDR

#define BOOT_SECTOR_ADDR        FLASH_SECTOR_0_ADDR     /* 128 KB (sectors 0~4) */
#define BOOT_SECTOR_SIZE        0x20000
#define SETTING_SECTOR_ADDR     FLASH_SECTOR_5_ADDR     /* 128 KB (sector  5)  */
#define SETTING_SECTOR_SIZE     0x20000
#define APP_SECTOR_ADDR         FLASH_SECTOR_6_ADDR     /* 512 KB (sectors 6~9) */
#define APP_SECTOR_SIZE         0x80000
#define DOWNLOAD_SECTOR_ADDR    FLASH_SECTOR_10_ADDR    /* 256 KB (sectors 10~11) */
#define DOWNLOAD_SECTOR_SIZE    0x40000
#define APP_ERASE_SECTORS       APP_SECTOR_SIZE


/* -------------------------------------------------------------------------- */
/*  public API                                                                 */
/* -------------------------------------------------------------------------- */

/**
 * @brief: Get the flash sector number for a given address
 * @addr: flash address
 *
 * Return: 0~11 for valid sectors, 0xFF on invalid address
 */
unsigned char stmflash_get_flash_sector(uint32_t addr);

/**
 * @brief: Get the flash sector and bank for a given address
 * @addr: flash address
 * @bank: output parameter, returns the bank (FLASH_BANK_1)
 *
 * Return: 0~11 sector number, 0xFF on invalid address
 */
unsigned char stmflash_get_flash_sector_with_bank(uint32_t addr, uint32_t *bank);

/**
 * @brief: Erase the sector(s) then write data at the specified address
 * @note:  Flash erasure works on whole sectors — existing data in affected
 *         sectors is destroyed.
 * @writeaddr: start address (must be 4-byte aligned)
 * @buff:      data buffer pointer
 * @num:       number of 32-bit words to write
 *
 * Return: 0=success, 1=invalid address, 2=write error, 3=erase error, 4=timeout
 */
unsigned char stmflash_erase_and_write(uint32_t writeaddr, uint32_t *buff, uint32_t num);

/**
 * @brief: Read a 32-bit word from the specified address
 * @faddr: flash address
 *
 * Return: the 32-bit value read
 */
uint32_t stmflash_read_word(uint32_t faddr);

/**
 * @brief: Erase sectors covering the specified range
 * @note:  Flash erasure works on whole sectors — all sectors overlapping
 *         [writeaddr, writeaddr + num*4) are erased.
 * @writeaddr: start address (must be 4-byte aligned)
 * @num:       number of 32-bit words in the range
 *
 * Return: 0=success, 1=invalid address, 3=erase error, 4=timeout
 */
unsigned char stmflash_erase(uint32_t writeaddr, uint32_t num);

/**
 * @brief: Write data to flash (sector must be pre-erased)
 * @writeaddr: start address (must be 4-byte aligned)
 * @buff:      data buffer pointer
 * @num:       number of 32-bit words to write
 *
 * Return: 0=success, 1=invalid address, 2=write error
 */
unsigned char stmflash_write(uint32_t writeaddr, uint32_t *buff, uint32_t num);

/**
 * @brief: Read data of the specified length from the given address
 * @raddr:  start address
 * @buff:   data buffer pointer (output)
 * @length: number of 32-bit words to read
 */
void stmflash_read(uint32_t raddr, uint32_t *buff, uint32_t length);


#endif /* __BSP_FLASH_H */
