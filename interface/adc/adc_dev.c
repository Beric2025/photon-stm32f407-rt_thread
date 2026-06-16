/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * ADC device abstraction layer
 */

#include <string.h>
#include <stdio.h>
#include "main.h"
#include "adc_dev.h"
#include "bsp_adc.h"
#include "log_print.h"

#ifdef USE_OS
#include <rtthread.h>
#endif

#define TAG "adc dev:"

/* adc1 object */
#define ADC_BSP_OBJECT          (&g_adc1)
#define ADC_BSP_OBJECT_INIT     (bsp_adc1_init)

// #define ADC_BSP_OBJECT          NULL
// #define ADC_BSP_OBJECT_INIT     NULL


#define ADC1_DMA_SIZE           30
#define ADC1_CHANNEL_NUM        2
static unsigned short adc1_dma_buf[ADC1_DMA_SIZE * ADC1_CHANNEL_NUM];

#define ADC1_RING_BUF_SIZE      (ADC1_DMA_SIZE * ADC1_CHANNEL_NUM * 1)
static unsigned short s_adc1_ring_buf[ADC1_RING_BUF_SIZE];

typedef struct {
    ADC_HandleTypeDef *adc;
    void (*init)(void);
    unsigned char dma_on;
    unsigned short *dma_buf;
    unsigned short dma_buf_size;
    unsigned short *ring_buf;
    unsigned short ring_buf_size;
} Adc_Data_T;

/* function declarations */
static int adc_init(void *privatedata);
static int adc_start(void *privatedata);
static int adc_stop(void *privatedata);
static int adc_read_raw(void *privatedata, unsigned char channel, unsigned short *value);




/* ADC hardware configuration data */
static Adc_Data_T s_adc1_data = {
    .adc = ADC_BSP_OBJECT,
    .init = ADC_BSP_OBJECT_INIT,
    .dma_on = 1,
    .dma_buf = adc1_dma_buf,
    .dma_buf_size = ADC1_DMA_SIZE * ADC1_CHANNEL_NUM,
    .ring_buf = s_adc1_ring_buf,
    .ring_buf_size = 0
};

/* ADC device instance */
static Adc_Device_T s_adc1_dev = {
    .name = "adc1",
    .resolution = 16,
    .channel_count = ADC1_CHANNEL_NUM,
    .init = adc_init,
    .start = adc_start,
    .stop = adc_stop,
    .read_raw = adc_read_raw,
    .private_data = &s_adc1_data
};


static int adc_init(void *privatedata)
{
    if (privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s ADC init private data is null!\n", TAG);
        return -1;
    }

    Adc_Device_T *adev = (Adc_Device_T *)privatedata;
    Adc_Data_T *pdata = (Adc_Data_T *)adev->private_data;

    pdata->init();

    LOG_PRINT(LOG_OUT_DEBUG, "%s ADC init successful!\n", TAG);

    return 0;
}

static int adc_start(void *privatedata)
{
    if (privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s ADC start single private data is null!\n", TAG);
        return -1;
    }

    Adc_Device_T *adev = (Adc_Device_T *)privatedata;
    Adc_Data_T *pdata = (Adc_Data_T *)adev->private_data;

    if (pdata->dma_on) {
        if (HAL_ADC_Start_DMA(pdata->adc, (uint32_t *)(pdata->dma_buf), (uint32_t)(pdata->dma_buf_size)) != HAL_OK) {
            LOG_PRINT(LOG_OUT_ERROR, "%s ADC init DMA failed!\n", TAG);
            return -1;
        }
    }
    else {
        if (HAL_ADC_Start(pdata->adc) != HAL_OK) {
            LOG_PRINT(LOG_OUT_ERROR, "%s ADC start single failed!\n", TAG);
            return -1;
        }
    }

    return 0;
}
static int adc_stop(void *privatedata)
{
    if (privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s ADC stop private data is null!\n", TAG);
        return -1;
    }

    Adc_Device_T *adev = (Adc_Device_T *)privatedata;
    Adc_Data_T *pdata = (Adc_Data_T *)adev->private_data;

    if (pdata->dma_on) {
        if (HAL_ADC_Stop_DMA(pdata->adc) != HAL_OK) {
            LOG_PRINT(LOG_OUT_ERROR, "%s ADC stop DMA failed!\n", TAG);
            return -1;
        }
    } else {
        if (HAL_ADC_Stop(pdata->adc) != HAL_OK) {
            LOG_PRINT(LOG_OUT_ERROR, "%s ADC stop failed!\n", TAG);
            return -1;
        }
    }

    return 0;
}

static int adc_read_raw(void *privatedata, unsigned char channel, unsigned short *value)
{
    if (privatedata == NULL || value == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s ADC read raw private data is null!\n", TAG);
        return -1;
    }    

    Adc_Device_T *adev = (Adc_Device_T *)privatedata;
    Adc_Data_T *pdata = (Adc_Data_T *)adev->private_data;
    unsigned short avg[ADC1_CHANNEL_NUM] = {0};
    unsigned char i, j;
    signed char samples;

#ifdef USE_OS
    rt_enter_critical();
#endif
    if (pdata->ring_buf_size) {
        samples = pdata->ring_buf_size / ADC1_CHANNEL_NUM;
        for (j = 0; j < ADC1_CHANNEL_NUM; j++) {
            for (i = 0; i < samples; i++) {
                avg[j] += pdata->ring_buf[ADC1_CHANNEL_NUM * i + j];
            }
            avg[j] /= samples;
        }
    }
#ifdef USE_OS
    rt_exit_critical();
#endif

    if (channel >= ADC1_CHANNEL_NUM) {
        LOG_PRINT(LOG_OUT_ERROR, "%s ADC channel %d out of range!\n", TAG, channel);
        return -1;
    }

    *value = avg[channel];

    return 0;
}


/* ====================================================================
 * adc_dev_on_event — single entry point from BSP HAL callbacks
 *
 * Copies DMA buffer into ring buffer and re-arms conversion.
 * Runs in ISR context — keep it short.
 * ==================================================================== */
void adc_dev_on_event(void *hadc_void, Adc_Event_T event)
{
    (void)event;  /* currently only CONV_COMPLETE is handled */
    ADC_HandleTypeDef *hadc = (ADC_HandleTypeDef *)hadc_void;
    Adc_Data_T *pdata = NULL;

    if (hadc == s_adc1_data.adc) {
        pdata = &s_adc1_data;
    }

    if (pdata == NULL)
        return;

#ifdef USE_OS
    rt_interrupt_enter();
#endif
    if (s_adc1_data.ring_buf_size >= ADC1_RING_BUF_SIZE)
        s_adc1_data.ring_buf_size = 0;

    memcpy(s_adc1_data.ring_buf + s_adc1_data.ring_buf_size,
           s_adc1_data.dma_buf, s_adc1_data.dma_buf_size * 2);
    s_adc1_data.ring_buf_size += s_adc1_data.dma_buf_size;
#ifdef USE_OS
    rt_interrupt_leave();
#endif

    if (pdata->dma_on) {
        HAL_ADC_Start_DMA(pdata->adc, (uint32_t *)(pdata->dma_buf),
                            (uint32_t)(pdata->dma_buf_size));
    }
}

/**
 * get_adc_device - look up an ADC device descriptor by name
 * @name: device name string (e.g., "adc1")
 *
 * Return: pointer to the matching Adc_Device_T, or NULL if not found
 */
Adc_Device_T *get_adc_device(char *name)
{
    Adc_Device_T *adev = NULL;

    if (0 == strcmp(name, "adc1"))
        adev = &s_adc1_dev;

    return adev;
}
