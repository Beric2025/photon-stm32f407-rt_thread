/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * TM512AC light strip driver
 */

#include <string.h>
#include <stdio.h>
#include <rtthread.h>
#include "app_manage.h"
#include "delay_dev.h"
#include "light_dev.h"
#include "gpio_dev.h"
#include "uart_dev.h"
#include "log_print.h"

#define TAG         "TM512AC:"

#define LIGHT_NUM           (100)        /* number of light strip LEDs */
#define EN_COLOR_MAX        (15)        /* maximum color index */

/* light strip color modes */
typedef enum color_mode
{
	EN_COLOR_OFF    = 0,       // 0. off
	EN_COLOR_RED     = 1,		// 1. red
	EN_COLOR_RED_BREATHE,		// 2. red breathe
	EN_COLOR_BLUE,				// 3. blue
	EN_COLOR_BLUE_BREATHE,		// 4. blue breathe
	EN_COLOR_GREEN = 5,			// 5. green
	EN_LIGHT_OFFF,				// 6. light off
	EN_COLOR_CYAN,				// 7. cyan
	EN_COLOR_CYAN_BREATHE,		// 8. cyan breathe
	EN_COLOR_YELLOW,			// 9. yellow
	EN_COLOR_YELLOW_BREATHE = 10, // 10. yellow breathe
	EN_COLOR_WHITE,				 // 11. white
	EN_COLOR_WHITE_BREATHE,	     // 12. white breathe
	EN_COLOR_PURPLE,			 // 13. purple
	EN_COLOR_PURPLE_BREATHE,	 // 14. purple breathe
} Color_Mode_E;

/* light strip RGB color state */
typedef struct
{
	unsigned char  red;
	unsigned char  blue;
	unsigned char  green;
} Rgb_T;
typedef struct
{
	unsigned char start_frame;
	Rgb_T rgb[LIGHT_NUM];
} Dmx512_Packet_T;

/* global variables */
Gpio_Device_T *s_gpio = NULL;
Gpio_Device_T *s_gpio_en = NULL;
Uart_Device_T *s_uart = NULL;
Dmx512_Packet_T s_dmx512_packet = {0};

Rgb_T  s_rgb[EN_COLOR_MAX] = {
	{0,   0,   0   },  // 0. off
	{255, 0,   0   },  // 1. red
	{255, 0,   0   },  // 2. red breathe
	{0,   255, 0   },  // 3. blue
	{0,   255, 0   },  // 4. blue breathe
	{0,   0,   255 },  // 5. green
	{0,   0,   0   },  // 6. light off
	{0,   255, 255 },  // 7. cyan
	{0,   255, 255 },  // 8. cyan breathe
	{255, 0,   255 },  // 9. yellow
	{255, 0,   255 },  // 10. yellow breathe
	{255, 255, 255 },  // 11. white
	{255, 255, 255 },  // 12. white breathe
	{255, 255,    0},  // 13. purple
	{255, 255,    0}   // 14. purple breathe
};

/* function declarations */
static int tm512ac_init(void *privatedata);
static int tm512ac_set_light_color(void *privatedata, unsigned char *color);
static int tm512ac_set_light_num_color(void *privatedata, unsigned short start,
	         unsigned short num, unsigned char *color);
static unsigned char tm512ac_read_light_color(void *privatedata);
static int tm512ac_light_off(void *privatedata);

/* initialization */
static Light_Device_T s_tm512ac_light_dev = {
    .name               = "tm512ac",
    .init               = tm512ac_init,
    .set_light_color    = tm512ac_set_light_color,
	.set_light_num_color = tm512ac_set_light_num_color,
    .read_light_color   = tm512ac_read_light_color,
	.light_off			= tm512ac_light_off
};
/*
 * Configure the port as a GPIO output.
 * Returns 0 on success.
 */
static int set_port_to_io(void)
{
    /* configure GPIO */
    s_gpio->ioctl(s_gpio, 1);

    return 0;
}
/*
 * Configure the port as a UART interface.
 * Returns 0 on success.
 */
static int set_port_to_uart(void)
{
    /* configure UART */
    s_uart->ioctl(s_uart, 1);

    return 0;
}
/*
 * Send data buffer over UART.
 * Returns 0 on success.
 */
static int uart_send_buf(char *buf, unsigned short size)
{
	s_gpio_en->write_pin(s_gpio_en, 1);		/* enable driver output */

    /* send data via UART */
    s_uart->send(s_uart, buf, size);

    return 0;
}
#if 0
/*
 * Receive data via UART.
 * Returns the number of bytes received.
 */
static unsigned short uart_receive_buf(char *buf)
{
	s_gpio_en->WriteGPIOPin(s_gpio_en, 0);		/* disable driver output */

    /* receive data via UART */
    return s_uart->UartReceive(s_uart, buf);
}

/*
 * Set the default signal level.
 * Returns 0 on success.
 */
static int tm512ac_default_level(void)
{
	set_port_to_io();                       /* configure GPIO mode */
	s_gpio->WriteGPIOPin(s_gpio, 1);        /* set high level */

    return 0;
}
#endif
/*
 * Generate the DMX512 break signal to start light transmission.
 * Returns 0 on success.
 */
static int tm512ac_turn_on(void)
{
#define  TIM_Delay_US_Time1     (100)
#define  TIM_Delay_US_Time2     (8)

	/* start signal */
	s_gpio->write_pin(s_gpio, 1);    /* set high level */
    set_port_to_io();                   /* configure GPIO mode */
    s_gpio->write_pin(s_gpio, 0);    /* set low level */
    delay_us(TIM_Delay_US_Time1);
    s_gpio->write_pin(s_gpio, 1);    /* set high level */
    delay_us(TIM_Delay_US_Time2);
    set_port_to_uart();

    return 0;
}

/*
 * Initialize the TM512AC device.
 * Returns 0 on success, -1 on failure.
 */
static int tm512ac_init(void *privatedata)
{
	(void)privatedata;
	int ret = 0;

	/* get enable pin */
	s_gpio_en = get_gpio_device("gpioi_pin1");
	if(s_gpio_en == NULL)	{
		LOG_PRINT(LOG_OUT_ERROR,"%sFind tm512ac driver pin error!\n",TAG);
		return -1;
	}

	ret = s_gpio_en->init(s_gpio_en);
	if(ret == -1)	{
		LOG_PRINT(LOG_OUT_ERROR,"%sFind tm512ac driver pin init error!\n",TAG);
		return -1;
	}

	/* initialize interfaces */
    s_gpio = get_gpio_device("gpioa_pin0");
	if(s_gpio == NULL)	{
		LOG_PRINT(LOG_OUT_ERROR,"%sFind tm512ac tx pin error!\n",TAG);
		return -1;
	}
    s_uart = get_uart_device("uart4");
	if(s_uart == NULL)	{
		LOG_PRINT(LOG_OUT_ERROR,"%sFind tm512ac uart error!\n",TAG);
		return -1;
	}

	ret = s_uart->init(s_uart, 500000);
	if(ret != 0) {
		LOG_PRINT(LOG_OUT_ERROR,"%sInit tm512ac uart error!\n",TAG);
		return -1;
	}

    return 0;
}

/*
 * Set all light strip LEDs to a single color.
 * Returns 0 on success, -1 on failure.
 */
static int tm512ac_set_light_color(void *privatedata, unsigned char *color)
{
    (void)privatedata;
    unsigned char color_code = *color;

    /* validate color range */
    if (color_code >= EN_COLOR_MAX)	{
		return -1;
	}

    /* fill all LEDs with the selected color */
    for (unsigned char i = 0;  i < LIGHT_NUM;  i++) {
		s_dmx512_packet.rgb[i].red   = s_rgb[color_code].red;
		s_dmx512_packet.rgb[i].green = s_rgb[color_code].green;
		s_dmx512_packet.rgb[i].blue  = s_rgb[color_code].blue;
	}

    /* send break signal */
    tm512ac_turn_on();
    /* send frame data */
	uart_send_buf((char *)&s_dmx512_packet, sizeof(s_dmx512_packet));

	/* resend to refresh display */
    tm512ac_turn_on();
	uart_send_buf((char *)&s_dmx512_packet, sizeof(s_dmx512_packet));

	delay_ms(4);		/* minimum 4ms interval between frames */

    return 0;
}
/*
 * Set a specified range of LEDs to a single color.
 * Returns 0 on success, -1 on failure.
 */
static int tm512ac_set_light_num_color(void *privatedata, unsigned short start,
	         unsigned short num, unsigned char *color)
{
	(void)privatedata;
	unsigned char color_code = *color;
	unsigned short total = start + num;

    /* validate color range */
    if (color_code >= EN_COLOR_MAX)	{
		return -1;
	}

    /* fill specified LEDs with the selected color */
    for (unsigned char i = start;  i < total;  i++) {
		s_dmx512_packet.rgb[i].red   = s_rgb[color_code].red;
		s_dmx512_packet.rgb[i].green = s_rgb[color_code].green;
		s_dmx512_packet.rgb[i].blue  = s_rgb[color_code].blue;
	}

    /* send break signal */
    tm512ac_turn_on();
    /* send frame data */
	uart_send_buf((char *)&s_dmx512_packet, sizeof(s_dmx512_packet));

	/* resend to refresh display */
    tm512ac_turn_on();
	uart_send_buf((char *)&s_dmx512_packet, sizeof(s_dmx512_packet));

	delay_ms(4);		/* minimum 4ms interval between frames */

    return 0;
}

/*
 * Read the current light strip color.
 * Returns the color code.
 */
static unsigned char tm512ac_read_light_color(void *privatedata)
{
    (void)privatedata;
    /* get current light strip color state */
    return 0;
}
/*
 * Turn off all light strip LEDs.
 * Returns 0 on success, -1 on failure.
 */
static int tm512ac_light_off(void *privatedata)
{
	return tm512ac_set_light_color(privatedata, EN_COLOR_OFF);
}

/*
 * Get the TM512AC light strip device handle.
 * Returns NULL on failure, or a pointer to the device structure on success.
 */
Light_Device_T *get_tm512ac_light_dev(void)
{
    return (Light_Device_T*)(&s_tm512ac_light_dev);
}
