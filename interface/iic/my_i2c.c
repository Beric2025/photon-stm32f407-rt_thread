/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * Software I2C bus driver implementation
 */

#include "my_i2c.h"
#include "delay_dev.h"

static void iic_init(void)
{
	IIC_SCL_H;
	IIC_SDA_H;
}

static void iic_start(void)
{
	IIC_SDA_H;
	/* SDA_OUT(); */
	IIC_SCL_H;
	delay_us(4);
	IIC_SDA_L;
	delay_us(4);
}

static void iic_stop(void)
{
	IIC_SDA_L;
	/* SDA_OUT(); */
	IIC_SCL_H;
	delay_us(4);
	IIC_SDA_H;
}

static char iic_wait_ack(void)
{
	unsigned char timeout=0;

	IIC_SDA_H;
	/* SDA_IN(); */
	IIC_SCL_H;

	while(READ_SDA){
		timeout++;
		if(timeout>250){
			iic_stop();
			return 1;
		}
		delay_us(2);
	}
	IIC_SCL_L;
	delay_us(20);
	return 0;
}

static void iic_send_byte(unsigned char txd)
{
    unsigned char t;

	/* SDA_OUT(); */
    IIC_SCL_L;
    for(t=0;t<8;t++){
		if(txd&0x80)
			IIC_SDA_H;
		else
			IIC_SDA_L;
    	txd<<=1;
		delay_us(2);
		IIC_SCL_H;
		delay_us(2);
		IIC_SCL_L;
    }
}

static unsigned char iic_read_byte(unsigned char ack)
{
	unsigned char i;
	unsigned char receive;

	receive = 0;
	/* SDA_IN(); */
    IIC_SCL_L;
    delay_us(4);
	for(i=0;i<8;i++){
		IIC_SCL_H;
		delay_us(4);
        receive<<=1;
        if(READ_SDA)receive++;
		delay_us(2);
		IIC_SCL_L;
		delay_us(4);
    }
    if(!ack){
		IIC_SDA_H;
		/* SDA_OUT(); */
		IIC_SCL_H;
		delay_us(2);
		IIC_SCL_L;
	}
	else{
		/* SDA_OUT(); */
		IIC_SDA_L;
		IIC_SCL_H;
		delay_us(2);
		IIC_SCL_L;
		IIC_SDA_H;
	}
    return receive;
}

void my_i2c_init(uint32_t baudrate)
{
	(void)baudrate;
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	__HAL_RCC_GPIOH_CLK_ENABLE();

	HAL_GPIO_WritePin(IIC_SCL_GPIO_Port, IIC_SCL_Pin|IIC_SDA_Pin, GPIO_PIN_SET);

	GPIO_InitStruct.Pin = IIC_SCL_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(IIC_SCL_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = IIC_SDA_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	HAL_GPIO_Init(IIC_SDA_GPIO_Port, &GPIO_InitStruct);

	iic_init();
}


void my_i2c_write_multi_byte(uint8_t devaddr,uint8_t regaddr, uint8_t *data, uint32_t length)
{
	unsigned char i;

	iic_start();
	iic_send_byte(devaddr);
	iic_wait_ack();
    iic_send_byte(regaddr);
	iic_wait_ack();
	/* delay_us(20); */
	for(i=0;i<length;i++){
		iic_send_byte(*data++);
		iic_wait_ack();
	}
    iic_stop();
	/* delay_ms(5); */
}


void my_i2c_transmit_multi_byte(uint8_t devaddr, uint8_t *data, uint32_t length)
{
	unsigned char i;

	iic_start();
	iic_send_byte(devaddr);
	iic_wait_ack();
	/* delay_us(20); */
	for(i=0;i<length;i++){
		iic_send_byte(*data++);
		iic_wait_ack();
	}
    iic_stop();
	/* delay_ms(5); */
}


void my_i2c_read_multi_byte(uint8_t devaddr, uint8_t regaddr, uint8_t *data, uint32_t length)
{
	unsigned char i;

	iic_start();
	iic_send_byte(devaddr);
	iic_wait_ack();
	iic_send_byte(regaddr);
	iic_wait_ack();
	iic_stop();
	delay_us(10);
	iic_start();
	iic_send_byte(devaddr|1);
	iic_wait_ack();
	for(i=0;i<length-1;i++){
		*data++ =iic_read_byte(1);
    }
	*data = iic_read_byte(0);
	iic_stop();
	/* delay_ms(5); */
}

void my_i2c_receive_multi_byte(uint8_t devaddr, uint8_t *data, uint32_t length)
{
	unsigned char i;

	iic_start();
	iic_send_byte(devaddr|1);
	iic_wait_ack();
	for(i=0;i<length-1;i++){
		*data++ =iic_read_byte(1);
    }
	*data = iic_read_byte(0);
	iic_stop();
	/* delay_ms(5); */
}
