/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * SPI device abstraction layer
 */

#include <string.h>
#include <stdio.h>
#include "main.h"
#include "spi_dev.h"
#include "bsp_spi.h"
#include "log_print.h"

#define TAG "spi dev:"


typedef struct {
    SPI_HandleTypeDef *spi;
    void (*init)(void);
} Spi_Data_T;

/* function declarations */
static int spi_init(void *privatedata);
static int spi_transfer(void *privatedata, unsigned char *buff, unsigned short length);
static int spi_write(void *privatedata, uint32_t regaddr, unsigned char *buff, unsigned short length);
static int spi_read(void *privatedata, uint32_t regaddr, unsigned char *buff, unsigned short length);

/* SPI hardware configuration data */
Spi_Data_T s_spi1_dev_data = {
    .spi = &g_spi1,
    .init = bsp_spi1_init,
};

/* SPI device instance */
static Spi_Device_T s_spi1_dev = {
    .name = "spi1",
    .init = spi_init,
    .transfer = spi_transfer,
    .write = spi_write,
    .read = spi_read,
    .private_data = &s_spi1_dev_data,
};


static int spi_init(void *privatedata)
{
    Spi_Device_T *spi_dev = NULL;

    if(privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s SPI init private data is null!\n", TAG);
        return -1;
    }

    spi_dev = (Spi_Device_T *)privatedata;
    Spi_Data_T *spi_data = (Spi_Data_T *)spi_dev->private_data;
    (void)spi_data;

    /* spi_data->spi->init(); */
    LOG_PRINT(LOG_OUT_DEBUG, "%s SPI init successful!\n", TAG);

    return 0;
}

static int spi_transfer(void *privatedata, unsigned char *buff, unsigned short length)
{
	HAL_StatusTypeDef reval = HAL_OK;
	
	if (privatedata == NULL) {
		LOG_PRINT(LOG_OUT_ERROR, "%s SPI transfer private data is null!\n", TAG);
		return -1;
	}

	Spi_Device_T *spi_dev = (Spi_Device_T *)privatedata;
	Spi_Data_T *spi_data = (Spi_Data_T *)spi_dev->private_data;
	
	reval = HAL_SPI_Transmit(spi_data->spi, buff, length, 200);

    if (reval == HAL_OK) {
        LOG_PRINT(LOG_OUT_DEBUG, "%s SPI write successful!\n", TAG);
        return 0;
    } else {
        LOG_PRINT(LOG_OUT_ERROR, "%s SPI write failed!\n", TAG);
        return -1;
    }
}

static int spi_write(void *privatedata, uint32_t regaddr, unsigned char *buff, unsigned short length)
{
    HAL_StatusTypeDef reval = HAL_OK;
    Spi_Device_T *spi_dev = NULL;
    Spi_Data_T *spi_data = NULL;
    unsigned char reg_addr_len = 8;
    unsigned char temp[4];

    if(privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s SPI write private data is null!\n", TAG);
        return -1;
    }

    spi_dev = (Spi_Device_T *)privatedata;
    spi_data = (Spi_Data_T *)spi_dev->private_data;

    temp[0] = (unsigned char)((regaddr >> 24) & 0xFF);
    temp[1] = (unsigned char)((regaddr >> 16) & 0xFF);
    temp[2] = (unsigned char)((regaddr >> 8) & 0xFF);
    temp[3] = (unsigned char)((regaddr >> 0) & 0xFF);

    if (reg_addr_len == 16) {
        HAL_SPI_Transmit(spi_data->spi, &temp[2], 1, 200);
    } else if (reg_addr_len == 24) {
        HAL_SPI_Transmit(spi_data->spi, &temp[1], 1, 200);
    } else if (reg_addr_len == 32) {
        HAL_SPI_Transmit(spi_data->spi, &temp[0], 1, 200);
    } else {
        HAL_SPI_Transmit(spi_data->spi, &temp[3], 1, 200);
    }

    reval = HAL_SPI_Transmit(spi_data->spi, buff, length, 200);

    if (reval == HAL_OK) {
        LOG_PRINT(LOG_OUT_DEBUG, "%s SPI write successful!\n", TAG);
        return 0;
    } else {
        LOG_PRINT(LOG_OUT_ERROR, "%s SPI write failed!\n", TAG);
        return -1;
    }
}

static int spi_read(void *privatedata, uint32_t regaddr, unsigned char *buff, unsigned short length)
{
    HAL_StatusTypeDef reval = HAL_OK;
    Spi_Device_T *spi_dev = NULL;
    Spi_Data_T *spi_data = NULL;
    unsigned char reg_addr_len = 8;
    unsigned char txData[16];
    unsigned char temp[4];

    if(privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s SPI read private data is null!\n", TAG);
        return -1;
    }

    spi_dev = (Spi_Device_T *)privatedata;
    spi_data = (Spi_Data_T *)spi_dev->private_data;


    if(length > sizeof(txData)) {
        LOG_PRINT(LOG_OUT_ERROR, "%s SPI read length is too long!\n", TAG);
        return -1;
    }


    temp[0] = (unsigned char)((regaddr >> 24) & 0xFF);
    temp[1] = (unsigned char)((regaddr >> 16) & 0xFF);
    temp[2] = (unsigned char)((regaddr >> 8) & 0xFF);
    temp[3] = (unsigned char)((regaddr >> 0) & 0xFF);

    if (reg_addr_len == 16) {
        HAL_SPI_Transmit(spi_data->spi, &temp[2], 1, 200);
    } else if (reg_addr_len == 24) {
        HAL_SPI_Transmit(spi_data->spi, &temp[1], 1, 200);
    } else if (reg_addr_len == 32) {
        HAL_SPI_Transmit(spi_data->spi, &temp[0], 1, 200);
    } else {
        HAL_SPI_Transmit(spi_data->spi, &temp[3], 1, 200);
    }

    reval = HAL_SPI_TransmitReceive(spi_data->spi, txData, buff, length, 200);
    if (reval == HAL_OK) {
        LOG_PRINT(LOG_OUT_DEBUG, "%s SPI read successful!\n", TAG);
        return 0;
    } else {
        LOG_PRINT(LOG_OUT_ERROR, "%s SPI read failed!\n", TAG);
        return -1;
    }
}

/**
 * get_spi_device - look up an SPI device descriptor by name
 * @name: device name string (e.g., "spi1")
 *
 * Return: pointer to the matching Spi_Device_T, or NULL if not found
 */
Spi_Device_T *get_spi_device(char *name)
{
    Spi_Device_T *spi_dev = NULL;

    if (0 == strcmp(name, "spi1"))
        spi_dev = &s_spi1_dev;

    return spi_dev;
}
