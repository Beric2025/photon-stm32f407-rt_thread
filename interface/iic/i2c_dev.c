/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * I2C management
 */

#include <string.h>
#include <stdio.h>
#include "main.h"
#include "i2c_dev.h"
#include "bsp_i2c.h"
#include "my_i2c.h"
#include "log_print.h"

#define TAG "i2c dev"


/* Private data structure */
typedef struct {
    /* 0: HAL mode, 1: software bit-bang mode */
    unsigned char mode;                     
    I2C_HandleTypeDef *i2c;
	void (*init)(uint32_t baudrate);
    void (*my_init)(uint32_t baudrate);
}I2c_Data_T;

/* Function declarations */
static int i2c_init(void *privatedata, uint32_t baudrate);
static int i2c_write(void *privatedata, unsigned short devaddr, unsigned short regaddr, 
                    unsigned char *buff, unsigned short length);
static int i2c_read(void *privatedata, unsigned short devaddr, unsigned short regaddr, 
                    unsigned char *buff, unsigned short length);
static int i2c_transmit(void *privatedata, unsigned short devaddr, unsigned char *buff, 
                    unsigned short length);
static int i2c_receive(void *privatedata, unsigned short devaddr, unsigned char *buff, 
                    unsigned short length);

/* Initialization */
I2c_Data_T s_i2c2_dev_data = {
    .mode = 0,
    .i2c = &g_i2c2,
	.init = bsp_i2c2_init,
    .my_init = my_i2c_init
};

/* Device structure definition */
static I2c_Device_T s_i2c2_dev = {
    .name = "i2c2",
    .init = i2c_init,
    .write = i2c_write,
    .read = i2c_read,
    .transmit = i2c_transmit,
    .receive = i2c_receive,
    .private_data = &s_i2c2_dev_data,
};

static int i2c_init(void *privatedata, uint32_t baudrate)
{
    if(privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s: I2C init private data is null!\n", TAG);
        return -1;
    }

    I2c_Device_T *i2c_dev = (I2c_Device_T *)privatedata;
    I2c_Data_T *i2c_data = (I2c_Data_T *)i2c_dev->private_data;

    if(i2c_data->mode == 0)
        i2c_data->init(baudrate);
    else
        i2c_data->my_init(baudrate);

    LOG_PRINT(LOG_OUT_DEBUG, "%s: I2C init successful!\n", TAG);
    return 0;
}

static int i2c_write(void *privatedata, unsigned short devaddr, unsigned short regaddr, 
                    unsigned char *buff, unsigned short length)
{
    if(privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s: I2C write private data is null!\n", TAG);
        return -1;
    }

    I2c_Device_T *i2c_dev = (I2c_Device_T *)privatedata;
    I2c_Data_T *i2c_data = (I2c_Data_T *)i2c_dev->private_data;

    if(i2c_data->mode == 0) {
        HAL_StatusTypeDef reval = HAL_OK;

        unsigned char tmp[256];
        if((uint32_t)(length + 1) > sizeof(tmp)) {
            LOG_PRINT(LOG_OUT_ERROR, "%s: I2C write buffer too small!\n", TAG);
            return -1;
        }
        tmp[0] = (unsigned char )regaddr & 0xFF ;
        memcpy(&tmp[1], buff, length);
        reval = HAL_I2C_Master_Transmit(i2c_data->i2c, devaddr, tmp, length+1, 100);
        if(reval == HAL_OK) {
            LOG_PRINT(LOG_OUT_DEBUG, "%s: I2C write successful!\n", TAG);
            return 0;
        }
        else {
            LOG_PRINT(LOG_OUT_ERROR, "%s: I2C write failed!\n", TAG);
            return -1;
        }
    }
    else {
        my_i2c_write_multi_byte((devaddr>>1)<<1, regaddr, buff, length);        
        LOG_PRINT(LOG_OUT_DEBUG, "%s: I2C write successful!\n", TAG);
        return 0;
    }
}
static int i2c_read(void *privatedata, unsigned short devaddr, unsigned short regaddr, 
                    unsigned char *buff, unsigned short length)
{
    if(privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s: I2C read private data is null!\n", TAG);
        return -1;
    }

    I2c_Device_T *i2c_dev = (I2c_Device_T *)privatedata;
    I2c_Data_T *i2c_data = (I2c_Data_T *)i2c_dev->private_data;

    if(i2c_data->mode == 0) {
        HAL_StatusTypeDef reval = HAL_OK;

        reval = HAL_I2C_Master_Transmit(i2c_data->i2c, devaddr, (unsigned char *)&regaddr, 1, 100);
        if(reval != HAL_OK) {
            LOG_PRINT(LOG_OUT_ERROR, "%s: I2C read failed to transmit register address!\n", TAG);
            return -1;
        }

        reval = HAL_I2C_Master_Receive(i2c_data->i2c, devaddr, buff, length, 100);
        if(reval == HAL_OK) {
            LOG_PRINT(LOG_OUT_DEBUG, "%s: I2C read successful!\n", TAG);
            return 0;
        }
        else {
            LOG_PRINT(LOG_OUT_ERROR, "%s: I2C read failed!\n", TAG);
            return -1;
        }
    }
    else {
        my_i2c_read_multi_byte((devaddr>>1)<<1, regaddr, buff, length);
        LOG_PRINT(LOG_OUT_DEBUG, "%s: I2C read successful!\n", TAG);
        return 0;
    }
}
static int i2c_transmit(void *privatedata, unsigned short devaddr, unsigned char *buff, 
                        unsigned short length)
{
    if(privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s: I2C transmit private data is null!\n", TAG);
        return -1;
    }

    I2c_Device_T *i2c_dev = (I2c_Device_T *)privatedata;
    I2c_Data_T *i2c_data = (I2c_Data_T *)i2c_dev->private_data;

    if(i2c_data->mode == 0) {
        HAL_StatusTypeDef reval = HAL_OK;

        reval = HAL_I2C_Master_Transmit(i2c_data->i2c, devaddr, buff, length, 100);
        if(reval == HAL_OK) {
            LOG_PRINT(LOG_OUT_DEBUG, "%s: I2C transmit successful!\n", TAG);
            return 0;
        }
        else {
            LOG_PRINT(LOG_OUT_ERROR, "%s: I2C transmit failed!\n", TAG);
            return -1;
        }
    }
    else {
        my_i2c_transmit_multi_byte((devaddr>>1)<<1, buff, length);
        LOG_PRINT(LOG_OUT_DEBUG, "%s: I2C transmit successful!\n", TAG);
        return 0;
    }
}
static int i2c_receive(void *privatedata, unsigned short devaddr, unsigned char *buff, 
                        unsigned short length)
{
    if(privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s: I2C receive private data is null!\n", TAG);
        return -1;
    }

    I2c_Device_T *i2c_dev = (I2c_Device_T *)privatedata;
    I2c_Data_T *i2c_data = (I2c_Data_T *)i2c_dev->private_data;

    if(i2c_data->mode == 0) {
        HAL_StatusTypeDef reval = HAL_OK;

        reval = HAL_I2C_Master_Receive(i2c_data->i2c, devaddr, buff, length, 100);
        if(reval == HAL_OK) {
            LOG_PRINT(LOG_OUT_DEBUG, "%s: I2C receive successful!\n", TAG);
            return 0;
        }
        else {
            LOG_PRINT(LOG_OUT_ERROR, "%s: I2C receive failed!\n", TAG);
            return -1;
        }
    }
    else {
        my_i2c_receive_multi_byte((devaddr>>1)<<1, buff, length);
        LOG_PRINT(LOG_OUT_DEBUG, "%s: I2C receive successful!\n", TAG);
        return 0;
    }
}

I2c_Device_T *get_i2c_device(char *name)
{
    I2c_Device_T *i2c_dev = NULL;

    if(0 == strcmp(name , "i2c2"))
      i2c_dev = &s_i2c2_dev;

    return i2c_dev;
}
