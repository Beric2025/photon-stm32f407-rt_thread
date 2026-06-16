/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * xingneng battery device driver
 */

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stddef.h>
#include <rtthread.h>
#include "app_manage.h"
#include "battery_dev.h"
#include "gpio_dev.h"
#include "uart_dev.h"
#include "delay_dev.h"
#include "log_print.h"

#define TAG             "xingnenng:"
#define XINGNENG_BAT    5

/* station address */
#define BATTERY_STATION                 0x0B
#define BATTERY_FUNCTION_CODE           0x03

/* battery register addresses */
#define BATTERY_REG_current_L_ADDR      0x0400
#define BATTERY_REG_VOLTAGE_MAX_ADDR    0x0800
#define BATTERY_REG_TEMP_MAX_ADDR       0x0C00
#define BATTERY_REG_AFE_STATUS_ADDR     0x1000
#define BATTERY_REG_CHARGE_SWITCH_ADDR  0xFC00

/* Modbus request frame */
typedef struct
{
    unsigned char slave_addr;
    unsigned char function_code;
    union {
        struct {
            unsigned short start_addr;
            unsigned short quantity;
        } read_regs;

        struct {
            unsigned short start_addr;
            unsigned short value;
        } write_single_reg;

        struct {
            unsigned short start_addr;
            unsigned char byte_count;
            unsigned char values[246];
        } write_multiple_regs;
    };
    unsigned short crc16;
} Modbus_Request_T;

/* battery status structure */
typedef struct
{
    int current;
    uint32_t remaining_capacity;
    uint32_t full_charge_capacity;
    uint32_t charge_current;
    uint32_t charging_voltage;
    uint32_t pack_voltage;
    uint32_t battery_voltage;
    unsigned short cycle_count;
    unsigned short average_time_to_empty;
    unsigned short average_time_to_full;
    unsigned short soc;
    unsigned short soh;
    unsigned short battery_status;
    unsigned short battery_alarm;
    unsigned short battery_safety;
    unsigned short voltage_max;
    unsigned short voltage_min;
    unsigned short Voltage1;
    unsigned short voltage2;
    unsigned short voltage3;
    unsigned short voltage4;
    unsigned short voltage5;
    unsigned short voltage6;
    unsigned short voltage7;
    unsigned short voltage8;
    unsigned short voltage9;
    unsigned short voltage10;
    unsigned short temperature_max;
    unsigned short temperature_min;
    unsigned short afe_status;
    unsigned short afe_safety;
    unsigned short cell_balan;
    unsigned short charge_switch;
    unsigned short discharge_switch;
    unsigned short bms_reset;
    unsigned short battery_id;
} Battery_Status_T;

/* private data structure */
typedef struct {
    char *gpio_name;
    char *uart_name;
    Gpio_Device_T *gpio;
    Uart_Device_T *uart;
} Xingneng_Data_T;

/* field descriptor for data-driven register parsing */
typedef struct {
    unsigned char offset;       /* byte offset in response data */
    unsigned char size;         /* 2 = 16-bit, 4 = 32-bit */
    size_t        field_offset; /* offsetof(Battery_Status_T, field) */
} Field_Desc_T;

/* main register field table: maps Modbus response bytes to Battery_Status_T fields */
static const Field_Desc_T s_main_fields[] = {
    {0,  4, offsetof(Battery_Status_T, current)},
    {4,  4, offsetof(Battery_Status_T, remaining_capacity)},
    {8,  4, offsetof(Battery_Status_T, full_charge_capacity)},
    {12, 4, offsetof(Battery_Status_T, charge_current)},
    {16, 4, offsetof(Battery_Status_T, charging_voltage)},
    {20, 4, offsetof(Battery_Status_T, pack_voltage)},
    {24, 4, offsetof(Battery_Status_T, battery_voltage)},
    {28, 2, offsetof(Battery_Status_T, cycle_count)},
    {30, 2, offsetof(Battery_Status_T, average_time_to_empty)},
    {32, 2, offsetof(Battery_Status_T, average_time_to_full)},
    {34, 2, offsetof(Battery_Status_T, soc)},
    {36, 2, offsetof(Battery_Status_T, soh)},
    {38, 2, offsetof(Battery_Status_T, battery_status)},
    {40, 2, offsetof(Battery_Status_T, battery_alarm)},
    {42, 2, offsetof(Battery_Status_T, battery_safety)},
};

#define MAIN_FIELDS_COUNT  (sizeof(s_main_fields) / sizeof(s_main_fields[0]))

/* forward declarations */
static int xingneng_init(void *privatedata);
static int xingneng_send(void *privatedata, unsigned char *buff, unsigned short length);
static int xingneng_read(void *privatedata, Battery_Info_T *info);

/* private data */
static Xingneng_Data_T s_xingneng_data = {
    .gpio_name = "gpiof_pin8",
    .uart_name = "uart2",
};

/* device instance */
static Battery_Device_T s_xingneng_dev = {
    .name           = "xingneng",
    .init           = xingneng_init,
    .send           = xingneng_send,
    .read           = xingneng_read,
    .private_data   = &s_xingneng_data
};

/* CRC16 Modbus checksum */
static unsigned short crc16_modbus(unsigned char *ptr, int len)
{
    uint32_t i;
    unsigned short crc = 0xFFFF;

    while (len--) {
        crc ^= *ptr++;
        for (i = 0; i < 8; ++i) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc = (crc >> 1);
        }
    }

    return crc;
}

/* RS485 send, auto-switch RX/TX via GPIO */
static int rs485_send(Xingneng_Data_T *datadev, unsigned char *buff, unsigned short length)
{
    int ret;
    uint32_t delay_time_ms;

    datadev->gpio->write_pin(datadev->gpio, 1);
    ret = datadev->uart->send(datadev->uart, (char *)buff, length);

    /* wait for TX to complete before switching back to RX */
    delay_time_ms = (length * 10 * 1000) / 9600 + 2;
    if (delay_time_ms < 2) delay_time_ms = 2;
    delay_ms(delay_time_ms);

    datadev->gpio->write_pin(datadev->gpio, 0);

    return (ret == 0) ? 0 : -1;
}

/* RS485 receive */
static unsigned short rs485_receive(Xingneng_Data_T *datadev, unsigned char *buff, unsigned short size)
{
    return datadev->uart->receive(datadev->uart, (char *)buff, size);
}

/* Modbus read registers (function code 0x03) */
static unsigned char modbus_read(Xingneng_Data_T *datadev, Modbus_Request_T *request,
    unsigned char *response, unsigned short *size)
{
    #define READ_LEN                8
    #define ERROR_LEN               5
    #define READ_TIMEOUT            30
    #define MAX_RECV_BUFFER_SIZE    256

    unsigned char send_buf[READ_LEN];
    unsigned char recv_buf[MAX_RECV_BUFFER_SIZE];
    unsigned short recv_index = 0;
    unsigned short rx_len = 0;
    unsigned short crc16_cal = 0;
    unsigned short crc16_recv = 0;
    unsigned char index = 0;
    unsigned char retry = 3;
    unsigned short timeout;
    unsigned char frame_start;
    int ret = 0;

    if ((request == NULL) || (response == NULL) || (size == NULL)) {
        return 1;
    }

    /* build request frame */
    send_buf[index++] = request->slave_addr;
    send_buf[index++] = request->function_code;
    send_buf[index++] = (request->read_regs.start_addr >> 8) & 0xFF;
    send_buf[index++] = request->read_regs.start_addr  & 0xFF;
    send_buf[index++] = (request->read_regs.quantity >> 8) & 0xFF;
    send_buf[index++] = request->read_regs.quantity & 0xFF;

    crc16_cal = crc16_modbus(send_buf, index);
    send_buf[index++] = (crc16_cal >> 0) & 0xFF;
    send_buf[index++] = (crc16_cal >> 8) & 0xFF;

    while (retry--) {
        memset(recv_buf, 0, sizeof(recv_buf));
        recv_index = 0;

        ret = rs485_send(datadev, send_buf, index);
        if (ret != 0) {
            LOG_PRINT(LOG_OUT_ERROR,"%s xinghang ReadReg rs485_send error, retry:%d\n", TAG, retry);
            continue;
        }

        timeout = READ_TIMEOUT;

        while (timeout--) {
            if (recv_index >= MAX_RECV_BUFFER_SIZE) {
                recv_index = 0;
                memset(recv_buf, 0, sizeof(recv_buf));
            }

            rx_len = rs485_receive(datadev, &recv_buf[recv_index], sizeof(recv_buf) - recv_index);
            if (rx_len > 0) {
                recv_index += rx_len;
                if (recv_index < ERROR_LEN) {
                    continue;
                }

                /* scan for valid Modbus frame start */
                for (frame_start = 0; frame_start <= recv_index - ERROR_LEN; frame_start++) {
                    if (recv_buf[frame_start] != request->slave_addr) {
                        continue;
                    }

                    if (recv_buf[frame_start + 1] != request->function_code) {
                        if (recv_buf[frame_start + 1] == (request->function_code | 0x80)) {
                            LOG_PRINT(LOG_OUT_ERROR,"%s Modbus exception: 0x%02X\n", TAG, recv_buf[frame_start + 2]);
                            return 4;
                        }
                        continue;
                    }

                    /* compute expected frame length */
                    unsigned char expectedLen;
                    if (recv_buf[frame_start + 1] & 0x80) {
                        expectedLen = 5;    /* exception response */
                    } else {
                        expectedLen = 3 + recv_buf[frame_start + 2] + 2;  /* addr + func + byte_count + data + crc */
                    }

                    if (frame_start + expectedLen > recv_index) {
                        continue;
                    }

                    /* verify CRC */
                    crc16_recv = (recv_buf[frame_start + expectedLen - 1] << 8) | recv_buf[frame_start + expectedLen - 2];
                    crc16_cal = crc16_modbus(&recv_buf[frame_start], expectedLen - 2);

                    if (crc16_recv != crc16_cal) {
                        LOG_PRINT(LOG_OUT_ERROR,"%s CRC error: 0x%04X != 0x%04X\n", TAG, crc16_recv, crc16_cal);
                        continue;
                    }

                    /* valid frame received */
                    *size = expectedLen;
                    memcpy(response, &recv_buf[frame_start], expectedLen);
                    return 0;
                }
            }

            rt_thread_mdelay(5);
        }
    }

    return 2;
}

/* get 16-bit value from byte buffer */
static unsigned short get_short_data(unsigned char *buff, unsigned char bigendian)
{
    if (bigendian) {
        return (buff[0] << 8) | buff[1];
    }
    return (buff[1] << 8) | buff[0];
}

/* parse Modbus response data into battery status using field descriptor table */
static void parse_fields(Battery_Status_T *status, const unsigned char *data,
    unsigned char byte_count, const Field_Desc_T *fields, int count)
{
    for (int i = 0; i < count; i++) {
        if (fields[i].offset + fields[i].size > byte_count) break;

        if (fields[i].size == 4) {
            uint32_t val32 = get_short_data((unsigned char *)&data[fields[i].offset], 1)
                               | (get_short_data((unsigned char *)&data[fields[i].offset + 2], 1) << 16);
            *(uint32_t *)((char *)status + fields[i].field_offset) = val32;
        } else {
            unsigned short val16 = get_short_data((unsigned char *)&data[fields[i].offset], 1);
            *(unsigned short *)((char *)status + fields[i].field_offset) = val16;
        }
    }
}

/* initialize xingneng device */
static int xingneng_init(void *privatedata)
{
    Battery_Device_T *pdev = (Battery_Device_T *)privatedata;
    Xingneng_Data_T *xingneng_data = (Xingneng_Data_T *)pdev->private_data;

    /* GPIO init */
    xingneng_data->gpio = get_gpio_device(xingneng_data->gpio_name);
    if (xingneng_data->gpio == NULL) {
        LOG_PRINT(LOG_OUT_ERROR,"%s Find %s driver pin error!\n", TAG, xingneng_data->gpio_name);
        return -1;
    }

    if (xingneng_data->gpio->init(xingneng_data->gpio) == -1) {
        LOG_PRINT(LOG_OUT_ERROR,"%s Find %s driver pin init error!\n", TAG, xingneng_data->gpio_name);
        return -1;
    }

    /* UART init */
    xingneng_data->uart = get_uart_device(xingneng_data->uart_name);
    if (xingneng_data->uart == NULL) {
        LOG_PRINT(LOG_OUT_ERROR,"%s Find %s uart error!\n", TAG, xingneng_data->uart_name);
        return -1;
    }

    if (xingneng_data->uart->init(xingneng_data->uart, 9600) != 0) {
        LOG_PRINT(LOG_OUT_ERROR,"%s Init %s uart error!\n", TAG, xingneng_data->uart_name);
        return -1;
    }

    return 0;
}

/* send data to battery device (not implemented) */
static int xingneng_send(void *privatedata, unsigned char *buff, unsigned short length)
{
    (void)privatedata;
    (void)buff;
    (void)length;
    return 0;
}

/* read battery status and fill info structure */
static int xingneng_read(void *privatedata, Battery_Info_T *info)
{
    Battery_Device_T *pdev = (Battery_Device_T *)privatedata;
    Xingneng_Data_T *xingneng_data = (Xingneng_Data_T *)pdev->private_data;
    Battery_Status_T bat_status = {0};

    /* read battery main info: 0x0400, 22 registers */
    Modbus_Request_T req = {
        .slave_addr = BATTERY_STATION,
        .function_code = BATTERY_FUNCTION_CODE,
        .read_regs.start_addr = BATTERY_REG_current_L_ADDR,
        .read_regs.quantity = 22,
    };

    unsigned char response_buf[256] = {0};
    unsigned short response_size = 0;
    unsigned char result = modbus_read(xingneng_data, &req, response_buf, &response_size);

    if (result == 0) {
        parse_fields(&bat_status, &response_buf[3], response_buf[2], s_main_fields, MAIN_FIELDS_COUNT);
    } else {
        switch (result) {
            case 2: LOG_PRINT(LOG_OUT_ERROR,"%s Timeout error\n", TAG); break;
            case 3: LOG_PRINT(LOG_OUT_ERROR,"%s CRC error\n", TAG); break;
            case 4: LOG_PRINT(LOG_OUT_ERROR,"%s Slave exception\n", TAG); break;
        }
    }

    /* calculate battery percentage */
    float capacity = bat_status.remaining_capacity;
    float capacity_full = bat_status.full_charge_capacity * 0.99f;
    float capacity_low = bat_status.full_charge_capacity * 0.05f;
    float powerStatus = (capacity >= capacity_full) ? 100.0f :
                        ((capacity <= capacity_low) ? 0.0f :
                        ((capacity - capacity_low) / (capacity_full - capacity_low) * 100.0f));

    info->total_current = bat_status.current;
    info->voltage = bat_status.pack_voltage;
    info->electricity = (unsigned char)ceil(powerStatus);
    info->FET = 0;
    info->protection = 0;
    info->error_code = 0;
    info->factory = XINGNENG_BAT;
    info->timeout = 0;
    info->capacity = capacity / 1000.0f;
    info->capacity_max = bat_status.full_charge_capacity / 1000.0f;

    return 0;
}

/* get xingneng battery device instance */
Battery_Device_T *get_xingneng_dev(void)
{
    return &s_xingneng_dev;
}
