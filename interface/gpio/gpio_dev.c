/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * GPIO device abstraction layer
 */

#include <string.h>
#include <stdio.h>
#include "main.h"
#include "gpio_dev.h"
#include "bsp_gpio.h"
#include "log_print.h"

#define TAG "gpio dev:"


typedef struct {
    GPIO_TypeDef    *gpiox;
    uint16_t        gpio_pin;
    void            (*init)(void);
    unsigned char int_flag;                                 /* interrupt flag (0 = idle, 1 = pending) */
} Gpio_Data_T;

/* function declarations */
static int gpio_init(void *privatedata);
static int write_gpio_pin(void *privatedata, unsigned char value);
static unsigned char read_gpio_pin(void *privatedata);
static int toggle_gpio_pin(void *privatedata);
static int gpio_ioctl(void *privatedata, uint32_t flag);
static int get_gpio_pin_int(void *privatedata, unsigned char *value);
static int clear_gpio_pin_int(void *privatedata);

/* GPIO hardware configuration data */
Gpio_Data_T s_gpioa_pin0_data = {
    .gpiox = GPIOA,
    .gpio_pin = GPIO_PIN_0,
	.init = bsp_gpioa_pin0_init 
};
Gpio_Data_T s_gpiob_pin0_data = {
    .gpiox = GPIOB,
    .gpio_pin = GPIO_PIN_0,
	.init = bsp_gpiob_pin0_init
};
Gpio_Data_T s_gpiob_pin1_data = {
    .gpiox = GPIOB,
    .gpio_pin = GPIO_PIN_1,
	.init = bsp_gpiob_pin1_init
};
Gpio_Data_T s_gpiof_pin8_data = {
    .gpiox = GPIOF,
    .gpio_pin = GPIO_PIN_8,
	.init = bsp_gpiof_pin8_init
};
Gpio_Data_T s_gpioh_pin15_data = {
    .gpiox = GPIOH,
    .gpio_pin = GPIO_PIN_15,
	.init = bsp_gpioh_pin15_init 
};
Gpio_Data_T s_gpioi_pin0_data = {
    .gpiox = GPIOI,
    .gpio_pin = GPIO_PIN_0,
	.init = bsp_gpioi_pin0_init 
};
Gpio_Data_T s_gpioi_pin1_data = {
    .gpiox = GPIOI,
    .gpio_pin = GPIO_PIN_1,
	.init = bsp_gpioi_pin1_init 
};

/* GPIO device instances */
static Gpio_Device_T s_gpioa_pin0 = {
    .name = "gpioa_pin0",
    .init = gpio_init,
    .write_pin = write_gpio_pin,
    .read_pin = read_gpio_pin,
    .ioctl    = gpio_ioctl,
    .private_data = &s_gpioa_pin0_data,
};
static Gpio_Device_T s_gpiob_pin0 = {
    .name = "gpiob_pin0",
    .init = gpio_init,
    .write_pin = write_gpio_pin,
    .read_pin = read_gpio_pin,
    .toggle_pin = toggle_gpio_pin,
    .ioctl = gpio_ioctl,
    .get_interruput = get_gpio_pin_int,
    .clear_interruput = clear_gpio_pin_int,
    .private_data = &s_gpiob_pin0_data,
};
static Gpio_Device_T s_gpiob_pin1 = {
    .name = "gpiob_pin1",
    .init = gpio_init,
    .write_pin = write_gpio_pin,
    .read_pin = read_gpio_pin,
    .toggle_pin = toggle_gpio_pin,
    .ioctl = gpio_ioctl,
    .get_interruput = get_gpio_pin_int,
    .clear_interruput = clear_gpio_pin_int,
    .private_data = &s_gpiob_pin1_data,
};
static Gpio_Device_T s_gpiof_pin8 = {
    .name = "gpiof_pin8",
    .init = gpio_init,
    .write_pin = write_gpio_pin,
    .private_data = &s_gpiof_pin8_data,
};
static Gpio_Device_T s_gpioh_pin15 = {
    .name = "gpioh_pin15",
    .init = gpio_init,
    .write_pin = write_gpio_pin,
    .read_pin = read_gpio_pin,
    .private_data = &s_gpioh_pin15_data,
}; 
static Gpio_Device_T s_gpioi_pin0 = {
    .name = "gpioi_pin0",
    .init = gpio_init,
    .write_pin = write_gpio_pin,
    .read_pin = read_gpio_pin,
    .private_data = &s_gpioi_pin0_data,
}; 
static Gpio_Device_T s_gpioi_pin1 = {
    .name = "gpioi_pin1",
    .init = gpio_init,
    .write_pin = write_gpio_pin,
    .read_pin = read_gpio_pin,
    .private_data = &s_gpioi_pin1_data,
}; 

static int gpio_init(void *privatedata)
{
    if(privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s GPIO init private data is null!\n", TAG);
        return -1;
    }

    Gpio_Device_T *gpio_dev = (Gpio_Device_T *)privatedata;
    Gpio_Data_T *gpio_data = (Gpio_Data_T *)gpio_dev->private_data;

    gpio_data->init();

    LOG_PRINT(LOG_OUT_DEBUG, "%s GPIO init successful!\n", TAG);

    return 0;
}
static int write_gpio_pin(void *privatedata, unsigned char value)
{
    if(privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s Write GPIO pin private data is null!\n", TAG);
        return -1;
    }

    Gpio_Device_T *gpio_dev = (Gpio_Device_T *)privatedata;
    Gpio_Data_T *gpio_data = (Gpio_Data_T *)gpio_dev->private_data;

    HAL_GPIO_WritePin(gpio_data->gpiox, gpio_data->gpio_pin, (GPIO_PinState)value);

    LOG_PRINT(LOG_OUT_DEBUG, "%s Write GPIO pin successful!\n", TAG);

	return 0;
}
static unsigned char read_gpio_pin(void *privatedata)
{
    if(privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s Read GPIO pin private data is null!\n", TAG);
        return 0;
    }

    Gpio_Device_T *gpio_dev = (Gpio_Device_T *)privatedata;
    Gpio_Data_T *gpio_data = (Gpio_Data_T *)gpio_dev->private_data;

    LOG_PRINT(LOG_OUT_DEBUG, "%s Read GPIO pin successful!\n", TAG);
    return (unsigned char)HAL_GPIO_ReadPin(gpio_data->gpiox, gpio_data->gpio_pin);
}

static int toggle_gpio_pin(void *privatedata)
{
    if(privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s Toggle GPIO pin private data is null!\n", TAG);
        return -1;
    }

    Gpio_Device_T *gpio_dev = (Gpio_Device_T *)privatedata;
    Gpio_Data_T *gpio_data = (Gpio_Data_T *)gpio_dev->private_data;

    HAL_GPIO_TogglePin(gpio_data->gpiox, gpio_data->gpio_pin);

    LOG_PRINT(LOG_OUT_DEBUG, "%s Toggle GPIO pin successful!\n", TAG);
    return 0;
}
static int gpio_ioctl(void *privatedata, uint32_t flag)
{
    if(privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s GPIO ioctl private data is null!\n", TAG);
        return -1;
    }

    Gpio_Device_T *gpio_dev = (Gpio_Device_T *)privatedata;
    // Gpio_Data_T *gpio_data = (Gpio_Data_T *)gpio_dev->private_data;

    flag = flag;

    if(0 == strcmp(gpio_dev->name , "gpioa_pin0")) {

        GPIO_InitTypeDef GPIO_InitStruct = {0};

        /*Configure GPIO pin : LAN_RST_Pin */
        GPIO_InitStruct.Pin = GPIO_PIN_0;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }

    LOG_PRINT(LOG_OUT_DEBUG, "%s GPIO ioctl successful!\n", TAG);

	return 0;
}

/*
 * Read the interrupt flag for a GPIO pin.
 * Returns 0 on success; *value is set to the current flag.
 */
static int get_gpio_pin_int(void *privatedata, unsigned char *value)
{
    (void)value;
    if(privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s GPIO init private data is null!\n", TAG);
        return -1;
    }

    Gpio_Device_T *gpio_dev = (Gpio_Device_T *)privatedata;
    Gpio_Data_T *gpio_data = (Gpio_Data_T *)gpio_dev->private_data;

    *value = gpio_data->int_flag; 


    LOG_PRINT(LOG_OUT_DEBUG, "%s Get gpio interruput flag successful!\n", TAG);

    return 0;

}
/*
 * Clear the interrupt flag for a GPIO pin.
 * Returns 0 on success.
 */
static int clear_gpio_pin_int(void *privatedata)
{    
    if(privatedata == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%s: GPIO init private data is null!\n", TAG);
        return -1;
    }

    Gpio_Device_T *gpio_dev = (Gpio_Device_T *)privatedata;
    Gpio_Data_T *gpio_data = (Gpio_Data_T *)gpio_dev->private_data;

    gpio_data->int_flag = 0; 

    LOG_PRINT(LOG_OUT_DEBUG, "%s: Clear gpio interruput flag successful!\n", TAG);

    return 0;
}

/* ====================================================================
 * gpio_dev_on_event — entry point from BSP HAL_GPIO_EXTI_Callback
 *
 * Maps a GPIO pin to the registered device(s) and sets their interrupt
 * flag.  Since HAL_GPIO_EXTI_Callback only provides the pin (not the
 * port), board-specific logic must map the pin+port combination here.
 * ==================================================================== */
void gpio_dev_on_event(uint16_t gpio_pin)
{
    /* Board-specific pin-to-device mapping — extend as needed.
     * Example:
     *   if (gpio_pin == GPIO_PIN_0) s_gpiob_pin0_data.int_flag = 1;
     */
    (void)gpio_pin;
}

Gpio_Device_T *get_gpio_device(char *name)
{
    Gpio_Device_T *gpio_dev = NULL;

    if(0 == strcmp(name , "gpioa_pin0"))
        gpio_dev = &s_gpioa_pin0;

    if(0 == strcmp(name , "gpiob_pin0"))
        gpio_dev = &s_gpiob_pin0;

    if(0 == strcmp(name , "gpiob_pin1"))
        gpio_dev = &s_gpiob_pin1;
    
    if(0 == strcmp(name , "gpiof_pin8"))
        gpio_dev = &s_gpiof_pin8;

    if(0 == strcmp(name , "gpioh_pin15"))
        gpio_dev = &s_gpioh_pin15;

    if(0 == strcmp(name , "gpioi_pin0"))
        gpio_dev = &s_gpioi_pin0;

    if(0 == strcmp(name , "gpioi_pin1"))
        gpio_dev = &s_gpioi_pin1;

    return gpio_dev;

}
