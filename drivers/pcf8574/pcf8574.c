/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * pcf8574
 */

#include "pcf8574.h"
#include "i2c_dev.h"

static I2c_Device_T *i2c_dev = NULL;

unsigned char pcf8574_init(void)
{

    unsigned char temp = 0;
    int ret = 0;
    // GPIO_InitTypeDef gpio_init_struct;
    // PCF8574_GPIO_CLK_ENABLE();                            /* Enable GPIOB clock */

    // gpio_init_struct.Pin = PCF8574_GPIO_PIN;              /* PB12 */
    // gpio_init_struct.Mode = GPIO_MODE_INPUT;              /* Input */
    // gpio_init_struct.Pull = GPIO_PULLUP;                  /* Pull-up */
    // gpio_init_struct.Speed = GPIO_SPEED_HIGH;             /* High speed */
    // HAL_GPIO_Init(PCF8574_GPIO_PORT, &gpio_init_struct);  /* Init GPIO */
    // iic_init();                                           /* I2C init */

    i2c_dev = get_i2c_device("i2c2");
    if(i2c_dev == NULL) {
        return 1;
    }
    ret = i2c_dev->init(i2c_dev, 100000);
    if(ret != 0) {
        return 1;
    }

    /* Check if PCF8574 is present */
    // iic_start();
    // iic_send_byte(PCF8574_ADDR);                          /* Send address */
    // temp = iic_wait_ack();                                /* Wait ACK to detect PCF8574 presence */
    // iic_stop();                                           /* Generate stop condition */
    // pcf8574_write_byte(0XFF);                             /* Default all IOs to high */
    // return temp;
    ret = i2c_dev->transmit(i2c_dev, PCF8574_ADDR, &temp, 1);
    if(ret != 0) {
        return 1;
    }
    pcf8574_write_byte(0XFF);                             /* All IO default to high */
    return 0;
}

unsigned char pcf8574_read_byte(void)
{
    unsigned char temp = 0;
	int ret = 0;

    // iic_start();
    // iic_send_byte(PCF8574_ADDR | 0X01);                   /* Enter receive mode */
    // iic_wait_ack();
    // temp = iic_read_byte(0);
    // iic_stop();                                           /* Generate stop condition */

    ret = i2c_dev->receive(i2c_dev, PCF8574_ADDR | 0X01, &temp, 1);
    if(ret != 0) {
        return 0;
    }

    return temp;
}

void pcf8574_write_byte(unsigned char data)
{
    // iic_start();
    // iic_send_byte(PCF8574_ADDR | 0X00);   /* Send device address 0x40, write mode */
    // iic_wait_ack();
    // iic_send_byte(data);                  /* Send byte */
    // iic_wait_ack();
    // iic_stop();                           /* Generate stop condition  */
    // delay_ms(10);

    int ret = 0;

    ret = i2c_dev->transmit(i2c_dev, PCF8574_ADDR | 0X00, &data, 1);
    if(ret != 0) {
        return ;
    }
}


void pcf8574_write_bit(unsigned char bit, unsigned char sta)
{
    unsigned char data;

    data = pcf8574_read_byte();
    if (sta == 0)
    {
        data &= ~(1 << bit);
    }
    else
    {
        data |= 1 << bit;
    }
    pcf8574_write_byte(data);
}


unsigned char pcf8574_read_bit(unsigned char bit)
{
    unsigned char data;

    data = pcf8574_read_byte();
    if (data & (1 << bit))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


