/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * DYP-A21 ultrasonic sensor driver
 */

#include <string.h>
#include <stdio.h>
#include <rtthread.h>
#include "app_manage.h"
#include "delay_dev.h"
#include "ultrasonic_dev.h"
#include "gpio_dev.h"
#include "uart_dev.h"
#include "log_print.h"

#define TAG         "DYPA21: "

/* station address */
#define DYPA21_DFAULT_STATION          (0x01)
/* function codes */
#define DYPA21_READ_FUNCTIONCODE       (0x03)
#define DYPA21_WRITE_FUNCTIONCODE      (0x06)

/* register address definitions */
typedef enum
{
    DYPA21_REG_PROCESS_DISTANCE_ADDR           = 0x0100,
    DYPA21_REG_REAL_TIME_DISTANCE_ADDR         = 0x0101,
    DYPA21_REG_TEMPRETURE_ADDR                 = 0x0102,
    DYPA21_REG_ECHO_TIME_ADDR                  = 0x010A,

    DYPA21_REG_SLAVE_ADDR                      = 0x0200,
    DYPA21_REG_BAUDRATE_ADDR                   = 0x0201,
    DYPA21_REG_SWITCH_OUTPUT_POLARITY_ADDR     = 0x0205,
    DYPA21_REG_SET_SWITCH_OUTPUT_LIMIT_ADDR    = 0x0206,
    DYPA21_REG_ANGLE_LEVEL_ADDR                = 0x0208,
    DYPA21_REG_VALUE_UNIT_ADDR                 = 0x0209,
    DYPA21_REG_POWER_NOISE_LEVEL_ADDR          = 0x021A,
    DYPA21_REG_RANGE_LEVEL_ADDR                = 0x021F,
    DYPA21_REG_SWITCH_ECHO_TIME_ADDR           = 0x023A,
    DYPA21_REG_SWITCH_HOLD_TIME_ADDR           = 0x023B
}Dypa21_Reg_Config_T;

/* private data structure */
typedef struct {
    char *gpio_name;
    char *uart_name;
    unsigned char slave_addr;
    Gpio_Device_T *gpio;
    Uart_Device_T *uart;
    unsigned short timeout;
}Dypa21_Data_T;

/* function declarations */
static int dypa21_init(void *privatedata);
static int dypa21_config(void *privatedata);
static int dypa21_set(void *privatedata, unsigned char *pData, unsigned short size);
static unsigned short dypa21_read_distance(void *privatedata);

/* private data instances */
static Dypa21_Data_T s_dypa21_one_data = {
    .gpio_name   = "gpioh_pin15",
    .uart_name   = "uart2",
    .slave_addr  = DYPA21_DFAULT_STATION,
    .timeout    = 140*2,
};
static Dypa21_Data_T s_dypa21_two_data = {
    .gpio_name   = "gpioi_pin0",
    .uart_name   = "uart3",
    .slave_addr  = DYPA21_DFAULT_STATION,
    .timeout    = 140*2,
};

/* device instances */
static Ultrasonic_Device_T s_dypa21_one_dev = {
    .name           = "dypa21_01",
    .init           = dypa21_init,
    .config         = dypa21_config,
    .set            = dypa21_set,
    .read_distance  = dypa21_read_distance,
    .private_data    = &s_dypa21_one_data
};
static Ultrasonic_Device_T s_dypa21_two_dev = {
    .name           = "dypa21_02",
    .init           = dypa21_init,
    .config         = dypa21_config,
    .set            = dypa21_set,
    .read_distance  = dypa21_read_distance,
    .private_data    = &s_dypa21_two_data
};

/*
 * CRC-16-Modbus algorithm.
 * Returns the CRC value (low byte first, high byte second).
 */
static unsigned short crc16_modbus(unsigned char *ptr, int len)
{
    int i;
    unsigned short crc = 0xffff;
    while(len--)
    {
        crc ^= (unsigned short)(*ptr++);
        for(i = 0; i < 8; ++i)
        {
            if(crc & 0x0001)
            {
                crc = (crc >> 1) ^ 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }
    return crc;
}

/*
 * Verify the CRC of received data.
 * Returns 0 on success, -1 on failure.
 */
static int data_crc(unsigned char *data, int size)
{
    int ret = 0;
    unsigned short crc_value = 0;

    crc_value = crc16_modbus(data, size-2);
    if(crc_value != (data[size-2]<<0 | data[size-1]<<8))
        ret = -1;

    return ret;
}

/*
 * Send data over RS485.
 * Switches to transmit mode, sends data, waits for completion, then switches
 * back to receive mode.
 * Returns 0 on success, -1 on failure.
 */
static int rs485_send(Dypa21_Data_T *dev, unsigned char *data, unsigned short size)
{
    Dypa21_Data_T *dypa21_data = dev;
    int ret = 0;
    uint32_t delay_time_ms;

    /* switch to transmit mode */
    dypa21_data->gpio->write_pin(dypa21_data->gpio, 1);
    ret = dypa21_data->uart->send(dypa21_data->uart, (char *)data, size);

    // rt_tick_t ticks = xTaskGetTickCount();//test

    if(ret == 0) {
        // ensure data is fully transmitted before switching
        // 10 bits per byte (1 start + 8 data + 1 stop); add 2ms margin for reliability
        // formula = (size * 10 * 1000) / baudrate + 2ms
        delay_time_ms = (size * 10 * 1000) / 115200 + 2;
        if(delay_time_ms < 2) delay_time_ms = 2; // minimum delay 2ms

        // wait to ensure data is fully sent
        delay_ms(delay_time_ms);

        // uint32_t milliseconds = (xTaskGetTickCount() - ticks)* portTICK_PERIOD_MS;//test
        // printf("[%d] milliseconds\n", milliseconds);//test


        /* switch back to receive mode */
        dypa21_data->gpio->write_pin(dypa21_data->gpio, 0);

        return 0;
    } else {
        // switch to receive mode even on send failure
        delay_ms(2);
        dypa21_data->gpio->write_pin(dypa21_data->gpio, 0);
        return -1;
    }
}

/*
 * Receive data over RS485.
 * Returns the number of bytes received.
 */
static unsigned short rs485_receive(Dypa21_Data_T *dev, unsigned char *data, unsigned short size)
{
    Dypa21_Data_T *dypa21_data = dev;

    /* switch to receive mode (handled in rs485_send) */
    // dypa21_data->gpio->write_pin(dypa21_data->gpio, 0);

    return dypa21_data->uart->receive(dypa21_data->uart, (char *)data, size);
}

/*
 * Read a register value from the device via Modbus.
 * Returns the register value on success, 0xFFFF on failure.
 */
static unsigned short dypa21_read_reg(Dypa21_Data_T *dev, unsigned char slave_addr,
    unsigned char functionCode, unsigned short reg)
{
    #define DYPA21_READ_REG_LEN    (8)
    #define DYPA21_READ_OK_LEN     (7)
    #define MAX_RECV_BUFFER_SIZE   (80)
    // #define DYPA21_READ_TIMEOUT    (42)

    unsigned char send_data[DYPA21_READ_REG_LEN] = {0};
    unsigned char recv_data[MAX_RECV_BUFFER_SIZE] = {0};
    unsigned short recv_index = 0;  // use unsigned short to prevent overflow
    unsigned short index = 0;
    unsigned short crc_value = 0, value = 0;
    unsigned short rx_size = 0;     // use a more appropriate variable type
    unsigned char retry = 3;
    unsigned char timeout = 0;
    int ret = 0;
    unsigned char expected_len;

    // parameter validation
    if (dev == NULL) {
        return 0xFFFF;  /* return error value */
    }

    send_data[index++] = slave_addr;
    send_data[index++] = functionCode;
    send_data[index++] = (unsigned char)(reg >> 8);
    send_data[index++] = (unsigned char)(reg & 0xFF);
    send_data[index++] = 0x00;
    send_data[index++] = 0x01;
    crc_value = crc16_modbus(send_data, index);
    send_data[index++] = (unsigned char)(crc_value & 0xFF);
    send_data[index++] = (unsigned char)(crc_value >> 8);

    while(retry--) {
        // clear receive buffer
        memset(recv_data, 0, sizeof(recv_data));
        recv_index = 0;  // reset receive index

        // send data and check result
        ret = rs485_send(dev, send_data, index);
        if (ret != 0) {
            LOG_PRINT(LOG_OUT_ERROR, "DYPA21 ReadReg Rs485Send error, retry:%d\n", retry);
            continue;  // send failed, retry
        }

        timeout = dev->timeout/5;  // set timeout counter

        // wait for response
        while(timeout--) {
            if (recv_index >= MAX_RECV_BUFFER_SIZE) {
                // buffer overflow, discard and restart
                recv_index = 0;
                memset(recv_data, 0, sizeof(recv_data));
            }

            // receive data
            rx_size = rs485_receive(dev, &recv_data[recv_index], sizeof(recv_data) - recv_index);
            if(rx_size > 0) {
                recv_index += rx_size;

                // search for a valid Modbus response
                // Modbus RTU response format: [slave_addr(1)][func_code(1)][byte_count(1)][data(n)][CRC(2)]
                // minimum read register response length: 1+1+1+2+2 = 7 bytes
                if(recv_index >= DYPA21_READ_OK_LEN) {
                    // search for possible response start position
                    for(unsigned short i = 0; i <= recv_index - DYPA21_READ_OK_LEN; i++) {
                        // check if it matches the expected response format
                        if(recv_data[i] == send_data[0] &&     // slave address match
                           recv_data[i+1] == send_data[1] &&  // function code match
                           recv_data[i+2] == 2) {            // byte count is 2

                            expected_len = i + 5 + 2; // offset + header(addr+func+count+data) + CRC
                            if(expected_len <= recv_index) { // ensure enough data for CRC check
                                // verify data CRC
                                ret = data_crc(&recv_data[i], expected_len);
                                if(ret == 0) {
                                    // CRC passed, extract value
                                    value = (unsigned short)(recv_data[i+3] << 8) | recv_data[i+4];

                                    // remove processed data
                                    if(i + expected_len < recv_index) {
                                        memmove(recv_data, &recv_data[i + expected_len],
                                                recv_index - (i + expected_len));
                                        recv_index -= (i + expected_len);
                                    } else {
                                        recv_index = 0;
                                    }

                                    return value;  // success
                                }
                            }
                        }
                    }
                }
            }
            rt_thread_mdelay(5);
        }
        // LZYC_PRINTF("DYPA21 Read timeout,retry:%d\n", retry);
    }

    // all retries exhausted
    // LZYC_PRINTF("DYPA21 ReadReg failed after all retries\n");
    return 0xFFFF;  // return error value indicating failure
}

/*
 * Write a register value to the device via Modbus.
 * Returns 0 on success, -1 on failure.
 */
static int dypa21_write_reg(Dypa21_Data_T *dev, unsigned char slave_addr,
    unsigned char functionCode, unsigned short reg, unsigned short vlaue)
{
#define DYPA21_WRITE_REQ_LEN            (8)
#define WRITE_MAX_RECV_BUFFER_SIZE      (32)
#define DYPA21_WRITE_TIMEOUT            (100)

    unsigned char send_data[DYPA21_WRITE_REQ_LEN] = {0};
    unsigned char recv_data[WRITE_MAX_RECV_BUFFER_SIZE] = {0};
    unsigned short recv_index = 0;      // use unsigned short to prevent overflow
    unsigned short index = 0;
    unsigned short crc_value = 0;
    unsigned short rx_size = 0;         // use a more appropriate variable type
    unsigned char retry = 3;
    unsigned char timeout = DYPA21_WRITE_TIMEOUT;       // write operation timeout
    int send_ret;
    unsigned short calc_crc, received_crc;

    // parameter validation
    if (dev == NULL) {
        return -1;
    }

    send_data[index++] = slave_addr;
    send_data[index++] = functionCode;
    send_data[index++] = (unsigned char)(reg >> 8);
    send_data[index++] = (unsigned char)(reg & 0xFF);
    send_data[index++] = (unsigned char)(vlaue >> 8);
    send_data[index++] = (unsigned char)(vlaue & 0xFF);
    crc_value = crc16_modbus(send_data, index);
    send_data[index++] = (unsigned char)(crc_value & 0xFF);
    send_data[index++] = (unsigned char)(crc_value >> 8);


    while(retry--) {
        // clear receive buffer
        memset(recv_data, 0, sizeof(recv_data));
        recv_index = 0;

        // send data and check result
        send_ret = rs485_send(dev, send_data, index);
        if (send_ret != 0) {
            LOG_PRINT(LOG_OUT_ERROR, "DYPA21 WriteReg Rs485Send error, retry:%d\n", retry);
            continue;  // send failed, retry
        }

        timeout = DYPA21_WRITE_TIMEOUT;  // set timeout counter

        // wait for response
        while(timeout--) {
            if (recv_index >= WRITE_MAX_RECV_BUFFER_SIZE) {
                // buffer overflow, discard and restart
                recv_index = 0;
                memset(recv_data, 0, sizeof(recv_data));
            }

            // receive data
            rx_size = rs485_receive(dev, &recv_data[recv_index], sizeof(recv_data) - recv_index);
            if(rx_size > 0) {
                recv_index += rx_size;

                // check for valid write response
                if(recv_index >= DYPA21_WRITE_REQ_LEN) {
                    // search for possible response start position
                    for(unsigned short i = 0; i <= recv_index - DYPA21_WRITE_REQ_LEN; i++) {
                        // check if it matches the expected write response format
                        if(recv_data[i] == send_data[0] &&     // slave address match
                           recv_data[i+1] == send_data[1] &&  // function code match
                           recv_data[i+2] == send_data[2] &&  // register high byte match
                           recv_data[i+3] == send_data[3] &&  // register low byte match
                           recv_data[i+4] == send_data[4] &&  // value high byte match
                           recv_data[i+5] == send_data[5]) {  // value low byte match

                            // verify response CRC (first 6 bytes)
                            calc_crc = crc16_modbus(&recv_data[i], DYPA21_WRITE_REQ_LEN - 2);
                            received_crc = ((unsigned short)recv_data[i+6] << 0 | ((unsigned short)recv_data[i+7] << 8));

                            if(calc_crc == received_crc) {
                                // CRC passed, write successful
                                return 0;
                            } else {
                                // CRC failed, try next possible position
                                LOG_PRINT(LOG_OUT_ERROR, "DYPA21 WriteReg CRC Error\n");
                            }
                        }
                    }
                }
            }
            rt_thread_mdelay(5);
        }
        // LZYC_PRINTF("DYPA21 Write timeout,retry:%d\n", retry);
    }

    return -1;
}

#if 1
/*
 * Scan the bus to discover the slave address.
 * Returns 0 on success, -1 on failure.
 */
static int dypa21_get_slave_addr(Dypa21_Data_T *dev)
{
    unsigned char addr = DYPA21_DFAULT_STATION;
    unsigned char addr_max = 0xFE;
    unsigned short value = 0;

    for (addr = 1; addr < addr_max; addr++)  {
        value = dypa21_read_reg(dev, addr, DYPA21_READ_FUNCTIONCODE, DYPA21_REG_SLAVE_ADDR);
        if(addr == (unsigned char)value) {
            dev->slave_addr = addr;
            return 0;
        }
        rt_thread_mdelay(10);
    }

    dev->slave_addr = DYPA21_DFAULT_STATION;
    return -1;

}

/*
 * Set the sensor angle level.
 * level: 1 to 4 (default 4). Higher level means wider angle.
 *    1 - cone ~50 deg, horizontal ~50 deg, vertical ~65 deg
 *    2 - cone ~55 deg, horizontal ~55 deg, vertical ~70 deg
 *    3 - cone ~65 deg, horizontal ~60 deg, vertical ~75 deg
 *    4 - cone ~70 deg, horizontal ~65 deg, vertical ~90 deg
 * Returns 0 on success, -1 on failure.
 */
static int dypa21_set_angle_level(Dypa21_Data_T *dev, unsigned char level)
{
    int ret = 0;

    ret = dypa21_write_reg(dev, dev->slave_addr, DYPA21_WRITE_FUNCTIONCODE,
        DYPA21_REG_ANGLE_LEVEL_ADDR, level);
    if (ret == 0)
        return 0;
    else
        return -1;
}
#endif

/*
 * Set the sensor range level.
 * level: 1 to 5 (default 5).
 *    1 - ~50cm, real-time response 15-80ms, peak response 190-500ms
 *    2 - ~150cm, real-time response 20-90ms, peak response 230-550ms
 *    3 - ~250cm, real-time response 25-100ms, peak response 250-600ms
 *    4 - ~350cm, real-time response 35-110ms, peak response 280-650ms
 *    5 - ~500cm, real-time response 40-140ms, peak response 320-750ms
 * Returns 0 on success, -1 on failure.
 */
static int dypa21_set_range_level(Dypa21_Data_T *dev, unsigned char level)
{
    int ret = 0;

    ret = dypa21_write_reg(dev, dev->slave_addr, DYPA21_WRITE_FUNCTIONCODE,
        DYPA21_REG_RANGE_LEVEL_ADDR, level);
    if (ret == 0)
        return 0;
    else
        return -1;

}

/*
 * Initialize the DYP-A21 device.
 * Obtains and initializes the GPIO control pin and UART interface.
 * Returns 0 on success, -1 on failure.
 */
static int dypa21_init(void *privatedata)
{
    Ultrasonic_Device_T *dev = (Ultrasonic_Device_T *)privatedata;
    Dypa21_Data_T *dypa21_data = (Dypa21_Data_T *)dev->private_data;

    int ret = 0;

    /* get GPIO device */
    dypa21_data->gpio = get_gpio_device(dypa21_data->gpio_name);
    if(dypa21_data->gpio == NULL)	{
        LOG_PRINT(LOG_OUT_ERROR, "%sFind %s driver pin error!\n",TAG, dypa21_data->gpio_name);
        return -1;
    }

    ret = dypa21_data->gpio->init(dypa21_data->gpio);
    if(ret == -1)	{
        LOG_PRINT(LOG_OUT_ERROR, "%sFind %s driver pin init error!\n",TAG, dypa21_data->gpio_name);
        return -1;
    }

    /* get UART device */
    dypa21_data->uart = get_uart_device(dypa21_data->uart_name);
    if(dypa21_data->uart == NULL)	{
        LOG_PRINT(LOG_OUT_ERROR, "%sFind %s uart error!\n",TAG, dypa21_data->uart_name);
        return -1;
    }

    ret = dypa21_data->uart->init(dypa21_data->uart, 115200);
    if(ret != 0) {
        LOG_PRINT(LOG_OUT_ERROR, "%sInit %s uart error!\n",TAG, dypa21_data->uart_name);
        return -1;
    }

    return 0;


}

/*
 * Configure the DYP-A21 device.
 * Discovers the slave address and sets range and angle levels.
 * Returns 0 on success, -1 on failure.
 */
static int dypa21_config(void *privatedata)
{
    Ultrasonic_Device_T *dev = (Ultrasonic_Device_T *)privatedata;
    Dypa21_Data_T *dypa21_data = (Dypa21_Data_T *)dev->private_data;

    #define DYPA21_TIMEOUT_COEFFICIENT 	(2)

    int ret = 0;

   /* get slave address */
   ret = dypa21_get_slave_addr(dypa21_data);
   if(ret != 0) {
        LOG_PRINT(LOG_OUT_ERROR, "%sGet station error!\n",TAG);
        // return -1;
    }

    /* set range level 2: ~150cm, real-time response 20-90ms, peak response 230-550ms */
    ret = dypa21_set_range_level(dypa21_data, 2);
    if(ret != 0) {
        LOG_PRINT(LOG_OUT_ERROR, "%s%sSet range level 2 error!\n",TAG, dev->name);
        // return -1;
    }else{
        dypa21_data->timeout = 140 * DYPA21_TIMEOUT_COEFFICIENT;
    }

    /* set angle level 4: cone ~70 deg, horizontal ~65 deg, vertical ~90 deg */
    ret = dypa21_set_angle_level(dypa21_data, 4);
    if(ret != 0) {
        LOG_PRINT(LOG_OUT_ERROR, "%sSet angle level 4 error!\n",TAG);
        // return -1;
    }

    return 0;
}

/*
 * Send configuration data to the device via communication protocol.
 * Returns 0 on success, -1 on failure.
 */
static int dypa21_set(void *privatedata, unsigned char *pData, unsigned short size)
{
    (void)privatedata;
    (void)pData;
    (void)size;
    return 0;
}

/*
 * Read the real-time distance from the sensor.
 * Returns the distance value, or 0 on failure.
 */
static unsigned short dypa21_read_distance(void *privatedata)
{
    Ultrasonic_Device_T *dev = (Ultrasonic_Device_T *)privatedata;
    Dypa21_Data_T *dypa21_data = (Dypa21_Data_T *)dev->private_data;
    unsigned short distance = 0;

    distance =  dypa21_read_reg(dypa21_data, dypa21_data->slave_addr, DYPA21_READ_FUNCTIONCODE,
        DYPA21_REG_REAL_TIME_DISTANCE_ADDR);

    if(distance == 0xffff) distance = 0;

    return distance;
}

/*
 * Get device 1 instance.
 * Returns pointer to the ultrasonic device structure, or NULL on failure.
 */
Ultrasonic_Device_T *get_dypa21_one_dev(void)
{
    return (Ultrasonic_Device_T*)(&s_dypa21_one_dev);
}

/*
 * Get device 2 instance.
 * Returns pointer to the ultrasonic device structure, or NULL on failure.
 */
Ultrasonic_Device_T *get_dypa21_two_dev(void)
{
    return (Ultrasonic_Device_T*)(&s_dypa21_two_dev);
}
