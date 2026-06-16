/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * hub motor device driver
 */

#include <string.h>
#include <stdio.h>
#include "main.h"
#include <rtthread.h>
#include "app_manage.h"
#include "motor_dev.h"
#include "can_dev.h"
#include "delay_dev.h"
#include "log_print.h"

#define TAG         "Hub_motor: "

/* macro definitions */
#define HUB_MOTOR_NODE_ID         1       /* default node ID */
#define SDO_TX_ID_BASE            0x600   /* SDO TX base ID (0x600 + Node_ID) */
#define SDO_RX_ID_BASE            0x580   /* SDO RX base ID (0x580 + Node_ID) */
#define TPDO1_ID_BASE             0x180   /* TPDO1 base ID (0x180 + Node_ID) */
#define TPDO2_ID_BASE             0x280   /* TPDO2 base ID (0x280 + Node_ID) */

/* SDO command definitions */
#define SDO_DOWNLOAD_REQUEST      0x20    /* download request (write) */
#define SDO_UPLOAD_REQUEST        0x40    /* upload request (read) */
#define SDO_DOWNLOAD_RESPONSE     0x60    /* download response (write success) */
#define SDO_UPLOAD_RESPONSE       0x40    /* upload response (with data) */
#define SDO_ABORT_COMMUNICATION   0x80    /* communication aborted */

/* communication macros */
#define SDO_MAX_RETRY             3       /* max retry count */
#define SDO_READ_TIMEOUT          50      /* read timeout (ms) */
#define SDO_FRAME_SIZE            8       /* SDO frame size fixed at 8 bytes */

/* motor status definitions */
#define MOTOR_STATUS_READY        0x0001  /* motor ready */
#define MOTOR_STATUS_ENABLED      0x0002  /* motor enabled */
#define MOTOR_STATUS_ALARM        0x0008  /* alarm active */
#define MOTOR_STATUS_ESTOP        0x0020  /* emergency stop */

/* CANopen object dictionary */
typedef enum {
    // motor parameters (Table F1-1)
    CANOPEN_OBJ_MOTOR_MODEL_CODE              = 0x6410,  // motor model code
    CANOPEN_OBJ_FEEDBACK_TYPE                 = 0x6410,  // feedback type (SubIndex 0x02)
    CANOPEN_OBJ_ENCODER_RESOLUTION            = 0x6410,  // encoder resolution (SubIndex 0x03)
    CANOPEN_OBJ_MOTOR_POLE_PAIRS              = 0x6410,  // motor pole pairs (SubIndex 0x05)
    CANOPEN_OBJ_MOTOR_EXCITATION_MODE         = 0x6410,  // excitation mode (SubIndex 0x06)
    CANOPEN_OBJ_MOTOR_EXCITATION_CURRENT      = 0x6410,  // excitation current (SubIndex 0x07)
    CANOPEN_OBJ_MOTOR_EXCITATION_TIME         = 0x6410,  // excitation time (SubIndex 0x08)
    CANOPEN_OBJ_MOTOR_OVERLOAD_CURRENT        = 0x6410,  // overload current (SubIndex 0x09)
    CANOPEN_OBJ_MOTOR_OVERLOAD_TIME_CONST     = 0x6410,  // overload time constant (SubIndex 0x0A)
    CANOPEN_OBJ_MOTOR_MAX_CURRENT             = 0x6410,  // max current (SubIndex 0x0B)
    CANOPEN_OBJ_MOTOR_ROTATION_DIRECTION      = 0x6410,  // rotation direction (SubIndex 0x13)
    CANOPEN_OBJ_MOTOR_RATED_SPEED             = 0x6410,  // rated speed (SubIndex 0x1A)
    CANOPEN_OBJ_MOTOR_RATED_POWER             = 0x6410,  // rated power (SubIndex 0x1B)
    CANOPEN_OBJ_MOTOR_HALL_ANGLE              = 0x6410,  // hall sensor angle (SubIndex 0x1F)
    CANOPEN_OBJ_CURRENT_MOTOR_MODEL           = 0x6410,  // current motor model code (SubIndex 0x16)

    // operation mode control parameters (Table F1-2)
    CANOPEN_OBJ_OPERATION_MODE                = 0x6060,  // operation mode (SubIndex 0x00)
    CANOPEN_OBJ_CONTROL_WORD                  = 0x6040,  // control word (SubIndex 0x00)
    CANOPEN_OBJ_EFFECTIVE_OPERATION_MODE      = 0x6061,  // effective operation mode (SubIndex 0x00)
    CANOPEN_OBJ_DRIVE_ERROR_STATUS_WORD       = 0x2601,  // drive error status word (SubIndex 0x00)
    CANOPEN_OBJ_ESTOP_COMMAND                 = 0x605A,  // emergency stop command (SubIndex 0x11)
    CANOPEN_OBJ_ABSOLUTE_TARGET_POSITION      = 0x607A,  // absolute target position (SubIndex 0x00)
    CANOPEN_OBJ_RELATIVE_TARGET_POSITION      = 0x607B,  // relative target position (SubIndex 0x00)
    CANOPEN_OBJ_TRAPEZOIDAL_VELOCITY          = 0x6081,  // trapezoidal velocity (SubIndex 0x00)
    CANOPEN_OBJ_TRAPEZOIDAL_VELOCITY_RPM      = 0x6082,  // trapezoidal velocity rpm (SubIndex 0x00)
    CANOPEN_OBJ_TRAPEZOIDAL_ACCELERATION      = 0x6083,  // trapezoidal acceleration (SubIndex 0x00)
    CANOPEN_OBJ_TRAPEZOIDAL_DECELERATION      = 0x6084,  // trapezoidal deceleration (SubIndex 0x00)
    CANOPEN_OBJ_QUICK_STOP_DECELERATION       = 0x605A,  // quick stop deceleration (SubIndex 0x01)
    CANOPEN_OBJ_TARGET_SPEED_RPM              = 0x2FF0,  // target speed rpm (SubIndex 0x09)
    CANOPEN_OBJ_TARGET_SPEED                  = 0x60FF,  // target speed (SubIndex 0x00)
    CANOPEN_OBJ_TARGET_CURRENT                = 0x60F6,  // target current (SubIndex 0x08)
    CANOPEN_OBJ_TARGET_CURRENT_LIMIT          = 0x6073,  // target current limit (SubIndex 0x00)
    CANOPEN_OBJ_MAX_SPEED_LIMIT_RPM           = 0x6080,  // max speed limit rpm (SubIndex 0x00)
    CANOPEN_OBJ_POSITION_FOLLOWING_ERROR_WINDOW = 0x6065, // position following error window (SubIndex 0x00)
    CANOPEN_OBJ_POSITION_COMPENSATION_ENABLE  = 0x60FB,  // position compensation enable for mode 3 (SubIndex 0x88)
    CANOPEN_OBJ_POSITION_FEEDFORWARD          = 0x60FB,  // position/speed feedforward (SubIndex 0x02)
    CANOPEN_OBJ_POSITION_GAIN_0               = 0x60FB,  // position loop proportional gain 0 (SubIndex 0x01)
    CANOPEN_OBJ_SPEED_GAIN_0                  = 0x60F9,  // speed loop proportional gain 0 (SubIndex 0x01)
    CANOPEN_OBJ_SPEED_INTEGRAL_GAIN_0         = 0x60F9,  // speed loop integral gain 0 (SubIndex 0x02)
    CANOPEN_OBJ_CURRENT_POSITION_ZERO_CMD     = 0x607C,  // current position zero command (SubIndex 0x02)
    CANOPEN_OBJ_LOCK_AXIS_NOISE_REDUCTION_EN  = 0x5000,  // lock axis noise reduction enable (SubIndex 0x50)
    CANOPEN_OBJ_LOCK_AXIS_DELAY_TIME          = 0x5000,  // lock axis delay time (SubIndex 0x51)
    CANOPEN_OBJ_SPRING_COMPENSATION_COEFF     = 0x60F9,  // spring compensation coefficient (SubIndex 0x8C)
    CANOPEN_OBJ_SPRING_COMPENSATION_BASE      = 0x60F9,  // spring compensation base (SubIndex 0x8E)
    CANOPEN_OBJ_DRIVER_OVERTEMP_ALARM_POINT   = 0x3000,  // driver over-temperature alarm point (SubIndex 0x15)

    // monitoring data (Table F1-3)
    CANOPEN_OBJ_ACTUAL_POSITION               = 0x6063,  // actual position (SubIndex 0x00)
    CANOPEN_OBJ_ACTUAL_SPEED_RPM              = 0x60F9,  // actual speed rpm (SubIndex 0x18)
    CANOPEN_OBJ_ACTUAL_SPEED_001RPM           = 0x60F9,  // actual speed 0.001rpm (SubIndex 0x19)
    CANOPEN_OBJ_ACTUAL_SPEED_SAMPLE_PERIOD    = 0x60F9,  // actual speed sample period (SubIndex 0x1A)
    CANOPEN_OBJ_ACTUAL_SPEED_INTERNAL         = 0x606C,  // actual speed internal (SubIndex 0x00)
    CANOPEN_OBJ_ACTUAL_CURRENT_IQ             = 0x6078,  // actual current Iq (SubIndex 0x00)
    CANOPEN_OBJ_ACTUAL_CURRENT_IIT            = 0x60F6,  // IIt actual current (SubIndex 0x32)
    CANOPEN_OBJ_ACTUAL_DC_BUS_VOLTAGE         = 0x60F7,  // actual DC bus voltage (SubIndex 0x12)
    CANOPEN_OBJ_ACTUAL_DRIVER_TEMP            = 0x60F7,  // actual driver temperature (SubIndex 0x0B)
    CANOPEN_OBJ_POSITION_FOLLOWING_ERROR      = 0x60F4,  // position following error (SubIndex 0x00)

    // IO and data acquisition (Table F1-4)
    CANOPEN_OBJ_DIGITAL_INPUT_HW_STATE        = 0x2010,  // digital input hardware state (SubIndex 0x0A)
    CANOPEN_OBJ_DIGITAL_INPUT_POLARITY        = 0x2010,  // digital input polarity (SubIndex 0x01)
    CANOPEN_OBJ_DIGITAL_INPUT_SIMULATE        = 0x2010,  // digital input simulate (SubIndex 0x02)
    CANOPEN_OBJ_DIGITAL_INPUT_VIRTUAL_STATE   = 0x2010,  // digital input virtual state (SubIndex 0x0B)

    // communication parameters (Table F1-5)
    CANOPEN_OBJ_UART_BAUD_RATE_SETTING        = 0x2FE0,  // UART baud rate setting (SubIndex 0x01)
    CANOPEN_OBJ_UART_PROTOCOL_SELECT          = 0x2FE0,  // UART protocol select (SubIndex 0x0F)
    CANOPEN_OBJ_RS485_CAN_STATION_ID          = 0x2FE0,  // RS485/CAN station ID (SubIndex 0x0F)
    CANOPEN_OBJ_COMM_LOST_SHUTDOWN_EN         = 0x4100,  // communication lost shutdown enable (SubIndex 0x10)
    CANOPEN_OBJ_COMM_LOST_SHUTDOWN_DELAY      = 0x4100,  // communication lost shutdown delay (SubIndex 0x11)
    CANOPEN_OBJ_SIMPLE_PDO_FUNC_1             = 0x4700,  // simple PDO function 1 (SubIndex 0x01)
    CANOPEN_OBJ_TXPDO1_INHIBIT_TIME           = 0x1800,  // TX-PDO1 inhibit time (SubIndex 0x03)
    CANOPEN_OBJ_SIMPLE_PDO_FUNC_2             = 0x4700,  // simple PDO function 2 (SubIndex 0x02)
    CANOPEN_OBJ_TXPDO2_INHIBIT_TIME           = 0x1801,  // TX-PDO2 inhibit time (SubIndex 0x03)

    // save parameters (Table F1-6)
    CANOPEN_OBJ_SAVE_S1_PARAM_CMD             = 0x2FE5,  // save S1 parameter group command (SubIndex 0x01)
    CANOPEN_OBJ_SAVE_S2_PARAM_CMD             = 0x2FE5,  // save S2 parameter group command (SubIndex 0x02)
    CANOPEN_OBJ_SAVE_S3_PARAM_CMD             = 0x2FE5,  // save S3 parameter group command (SubIndex 0x03)
    CANOPEN_OBJ_SAVE_S4_PARAM_CMD             = 0x2FE5,  // save S4 parameter group command (SubIndex 0x04)
    CANOPEN_OBJ_SAVE_S5_PARAM_CMD             = 0x2FE5,  // save S5 parameter group command (SubIndex 0x05)

    // device information (Table F1-7)
    CANOPEN_OBJ_MACHINE_MODEL_SERIES          = 0x3000,  // machine model series (SubIndex 0x3A)
    CANOPEN_OBJ_MACHINE_MODEL_VOLT_CUR        = 0x3000,  // machine model voltage/current class (SubIndex 0x3B)
    CANOPEN_OBJ_MACHINE_MODEL_FB_BUS          = 0x3000,  // machine model feedback/bus (SubIndex 0x3C)
    CANOPEN_OBJ_MACHINE_MODEL_RECIPE          = 0x3000,  // machine model recipe (SubIndex 0x3D)
    CANOPEN_OBJ_FIRMWARE_DATE                 = 0x2500,  // firmware date (SubIndex 0xF1)
    CANOPEN_OBJ_HARDWARE_VERSION              = 0x2500,  // hardware version (SubIndex 0xF3)

    // special functions (Table F1-8)
    CANOPEN_OBJ_SSTOP_ENABLE                  = 0x5000,  // S-stop enable (SubIndex 0x20)
    CANOPEN_OBJ_SSTOP_CURRENT_LIMIT           = 0x5000,  // S-stop current limit (SubIndex 0x28)
    CANOPEN_OBJ_S_CURVE_SPEED_POINT           = 0x5000,  // S-curve speed point (SubIndex 0x02)
    CANOPEN_OBJ_S_CURVE_DURATION              = 0x5000,  // S-curve duration (SubIndex 0x04)
    CANOPEN_OBJ_ERROR_SHUTDOWN_MODE_USE       = 0x270F,  // error shutdown mode selection (SubIndex 0x10)
    CANOPEN_OBJ_ERROR_SHUTDOWN_MODE_LOW       = 0x270F,  // error shutdown mode low 8 bits (SubIndex 0x11)
    CANOPEN_OBJ_ERROR_SHUTDOWN_MODE_HIGH      = 0x270F,  // error shutdown mode high 8 bits (SubIndex 0x12)
} Canopen_Obj_Index_E;

/* sub-index enumeration */
typedef enum {
    CANOPEN_SUBINDEX_00           = 0x00,  // general sub-index
    CANOPEN_SUBINDEX_01           = 0x01,  // sub-index 1
    CANOPEN_SUBINDEX_02           = 0x02,  // sub-index 2
    CANOPEN_SUBINDEX_03           = 0x03,  // sub-index 3
    CANOPEN_SUBINDEX_04           = 0x04,  // sub-index 4
    CANOPEN_SUBINDEX_05           = 0x05,  // sub-index 5
    CANOPEN_SUBINDEX_06           = 0x06,  // sub-index 6
    CANOPEN_SUBINDEX_07           = 0x07,  // sub-index 7
    CANOPEN_SUBINDEX_08           = 0x08,  // sub-index 8
    CANOPEN_SUBINDEX_09           = 0x09,  // sub-index 9
    CANOPEN_SUBINDEX_0A           = 0x0A,  // sub-index A
    CANOPEN_SUBINDEX_0B           = 0x0B,  // sub-index B
    CANOPEN_SUBINDEX_0C           = 0x0C,  // sub-index C
    CANOPEN_SUBINDEX_0D           = 0x0D,  // sub-index D
    CANOPEN_SUBINDEX_0E           = 0x0E,  // sub-index E
    CANOPEN_SUBINDEX_0F           = 0x0F,  // sub-index F
    CANOPEN_SUBINDEX_10           = 0x10,  // sub-index 10
    CANOPEN_SUBINDEX_11           = 0x11,  // sub-index 11
    CANOPEN_SUBINDEX_12           = 0x12,  // sub-index 12
    CANOPEN_SUBINDEX_13           = 0x13,  // sub-index 13
    CANOPEN_SUBINDEX_14           = 0x14,  // sub-index 14
    CANOPEN_SUBINDEX_15           = 0x15,  // sub-index 15
    CANOPEN_SUBINDEX_16           = 0x16,  // sub-index 16
    CANOPEN_SUBINDEX_17           = 0x17,  // sub-index 17
    CANOPEN_SUBINDEX_18           = 0x18,  // sub-index 18
    CANOPEN_SUBINDEX_19           = 0x19,  // sub-index 19
    CANOPEN_SUBINDEX_1A           = 0x1A,  // sub-index 1A
    CANOPEN_SUBINDEX_1B           = 0x1B,  // sub-index 1B
    CANOPEN_SUBINDEX_1C           = 0x1C,  // sub-index 1C
    CANOPEN_SUBINDEX_1D           = 0x1D,  // sub-index 1D
    CANOPEN_SUBINDEX_1E           = 0x1E,  // sub-index 1E
    CANOPEN_SUBINDEX_1F           = 0x1F,  // sub-index 1F
    CANOPEN_SUBINDEX_20           = 0x20,  // sub-index 20
    CANOPEN_SUBINDEX_28           = 0x28,  // sub-index 28
    CANOPEN_SUBINDEX_32           = 0x32,  // sub-index 32
    CANOPEN_SUBINDEX_3A           = 0x3A,  // sub-index 3A
    CANOPEN_SUBINDEX_3B           = 0x3B,  // sub-index 3B
    CANOPEN_SUBINDEX_3C           = 0x3C,  // sub-index 3C
    CANOPEN_SUBINDEX_3D           = 0x3D,  // sub-index 3D
    CANOPEN_SUBINDEX_50           = 0x50,  // sub-index 50
    CANOPEN_SUBINDEX_51           = 0x51,  // sub-index 51
    CANOPEN_SUBINDEX_88           = 0x88,  // sub-index 88
    CANOPEN_SUBINDEX_8C           = 0x8C,  // sub-index 8C
    CANOPEN_SUBINDEX_8E           = 0x8E,  // sub-index 8E
    CANOPEN_SUBINDEX_91           = 0x91,  // sub-index 91
    CANOPEN_SUBINDEX_92           = 0x92,  // sub-index 92
    CANOPEN_SUBINDEX_93           = 0x93,  // sub-index 93
    CANOPEN_SUBINDEX_94           = 0x94,  // sub-index 94
    CANOPEN_SUBINDEX_98           = 0x98,  // sub-index 98
    CANOPEN_SUBINDEX_99           = 0x99,  // sub-index 99
    CANOPEN_SUBINDEX_9A           = 0x9A,  // sub-index 9A
    CANOPEN_SUBINDEX_9B           = 0x9B,  // sub-index 9B
    CANOPEN_SUBINDEX_9C           = 0x9C,  // sub-index 9C
    CANOPEN_SUBINDEX_9D           = 0x9D,  // sub-index 9D
    CANOPEN_SUBINDEX_9F           = 0x9F,  // sub-index 9F
    CANOPEN_SUBINDEX_A1           = 0xA1,  // sub-index A1
    CANOPEN_SUBINDEX_B1           = 0xB1,  // sub-index B1
    CANOPEN_SUBINDEX_B2           = 0xB2,  // sub-index B2
    CANOPEN_SUBINDEX_B3           = 0xB3,  // sub-index B3
    CANOPEN_SUBINDEX_B4           = 0xB4,  // sub-index B4
    CANOPEN_SUBINDEX_B8           = 0xB8,  // sub-index B8
    CANOPEN_SUBINDEX_BC           = 0xBC,  // sub-index BC
    CANOPEN_SUBINDEX_E1           = 0xE1,  // sub-index E1
    CANOPEN_SUBINDEX_E2           = 0xE2,  // sub-index E2
    CANOPEN_SUBINDEX_F1           = 0xF1,  // sub-index F1
    CANOPEN_SUBINDEX_F3           = 0xF3,  // sub-index F3
    CANOPEN_SUBINDEX_F4           = 0xF4,  // sub-index F4
    CANOPEN_SUBINDEX_F5           = 0xF5,  // sub-index F5
    CANOPEN_SUBINDEX_F6           = 0xF6,  // sub-index F6
    CANOPEN_SUBINDEX_F7           = 0xF7,  // sub-index F7
    CANOPEN_SUBINDEX_F9           = 0xF9,  // sub-index F9
    CANOPEN_SUBINDEX_FA           = 0xFA,  // sub-index FA
    CANOPEN_SUBINDEX_FB           = 0xFB,  // sub-index FB
    CANOPEN_SUBINDEX_FC           = 0xFC,  // sub-index FC
    CANOPEN_SUBINDEX_FD           = 0xFD,  // sub-index FD
    CANOPEN_SUBINDEX_FE           = 0xFE,  // sub-index FE
    CANOPEN_SUBINDEX_FF           = 0xFF,  // sub-index FF
} Canopen_Subindex_E;

/* SDO communication structure */
typedef struct {
    uint8_t command;                        /* SDO command */
    uint16_t index;                         /* object index */
    uint8_t sub_index;                      /* object sub-index */
    uint32_t data;                          /* data (max 4 bytes) */
    uint8_t data_size;                      /* data size (1, 2, or 4 bytes) */
} Sdo_Request_T;

/* private data structure */
typedef struct {
    char *can_name;
    Can_Device_T *can_dev;
    uint8_t node_id;                        /* node ID */
    uint32_t position;                      /* current position */
    int32_t speed;                          /* current speed */
    uint16_t status_word;                   /* status word */
    uint16_t actual_current;                /* actual current */
    uint32_t error_status;                  /* error status */
    uint8_t pdo_enable1;                    /* PDO1 enable status */
    uint8_t pdo_enable2;                    /* PDO2 enable status */
    float voltage;                          /* voltage */
    float temperature;                      /* temperature */
    float distance;                         /* distance */
    float power;                            /* power */
    int error_code;                         /* error code */
} Hub_Motor_Data_T;

/* function declarations */
static int hub_motor_init(void *privatedata);
static int hub_motor_set_speed(void *privatedata, int speed);
static int hub_motor_set_position(void *privatedata, uint32_t position);
static int hub_motor_read_speed(void *privatedata, int *speed);
static int hub_motor_read_position(void *privatedata, uint32_t *position);
static int hub_motor_read_error_code(void *privatedata, int *errorcode);
static int hub_motor_read(void *privatedata, Motor_Status_T *status);

/* SDO communication function declarations */
static int sdo_communicate(Hub_Motor_Data_T *motor_data, Sdo_Request_T *request, uint8_t *response_data, uint8_t *response_size);

/* private data */
static Hub_Motor_Data_T s_hub_motor_data = {
    .can_name = "can1",
    .node_id = HUB_MOTOR_NODE_ID,
    .position = 0,
    .speed = 0,
    .status_word = 0,
    .actual_current = 0,
    .error_status = 0,
    .pdo_enable1 = 0,
    .pdo_enable2 = 0,
    .voltage = 0,
    .temperature = 0,
    .distance = 0,
    .power = 0,
    .error_code = 0
};

/* motor device instance */
static Motor_Device_T s_hub_motor_dev = {
    .name = "hub_motor",
    .init = hub_motor_init,
    .set_speed = hub_motor_set_speed,
    .set_position = hub_motor_set_position,
    .read_speed = hub_motor_read_speed,
    .read_position = hub_motor_read_position,
    .read_error_code = hub_motor_read_error_code,
    .read = hub_motor_read,
    .private_data = &s_hub_motor_data,
};

/*
 * SDO communication wrapper (based on ModbusRead implementation approach).
 * Returns 0 on success, other values on failure.
 */
static int sdo_communicate(Hub_Motor_Data_T *motor_data, Sdo_Request_T *request, uint8_t *response_data, uint8_t *response_size)
{
    uint8_t send_buf[SDO_FRAME_SIZE] = {0};
    uint8_t recv_buf[SDO_FRAME_SIZE] = {0};
    uint8_t retry = SDO_MAX_RETRY;
    uint8_t timeout = SDO_READ_TIMEOUT;
    int ret = 0;
    uint8_t expected_response_cmd = 0;
    uint8_t data_offset = 0;

    if((motor_data == NULL) || (request == NULL) || (response_data == NULL) || (response_size == NULL)) {
        return 1;
    }

    // build send frame
    send_buf[0] = request->command;
    send_buf[1] = request->index & 0xFF;           // Index LSB
    send_buf[2] = (request->index >> 8) & 0xFF;    // Index MSB
    send_buf[3] = request->sub_index;               // SubIndex

    // set CCS and data length fields based on data size
    if(request->command & SDO_DOWNLOAD_REQUEST) {
        // write: fill data
        switch(request->data_size) {
            case 4:
                send_buf[4] = request->data & 0xFF;
                send_buf[5] = (request->data >> 8) & 0xFF;
                send_buf[6] = (request->data >> 16) & 0xFF;
                send_buf[7] = (request->data >> 24) & 0xFF;
                expected_response_cmd = SDO_DOWNLOAD_RESPONSE;
                break;
            case 2:
                send_buf[4] = request->data & 0xFF;
                send_buf[5] = (request->data >> 8) & 0xFF;
                expected_response_cmd = SDO_DOWNLOAD_RESPONSE;
                break;
            case 1:
            default:
                send_buf[4] = request->data & 0xFF;
                expected_response_cmd = SDO_DOWNLOAD_RESPONSE;
                break;
        }
    } else if(request->command & SDO_UPLOAD_REQUEST) {
        // read: no data needed
        expected_response_cmd = SDO_UPLOAD_RESPONSE;
    }

    // retry until success or max retries
    while(retry--) {
        // clear receive buffer
        memset(recv_buf, 0, sizeof(recv_buf));

        // send data and wait for response
        ret = motor_data->can_dev->send(motor_data->can_dev, send_buf, SDO_FRAME_SIZE);
        if(ret != 0) {
            LOG_PRINT(LOG_OUT_ERROR, "%sSDO Send error, retry: %d\n", TAG, retry);
            continue;  // send failed, retry
        }

        timeout = SDO_READ_TIMEOUT;  // reset timeout counter

        /* wait for response */
        while(timeout--) {
            // poll for receive
            unsigned short rx_len = motor_data->can_dev->receive(motor_data->can_dev, recv_buf,sizeof(recv_buf));

            if(rx_len > 0) {
                // check if valid response
                uint8_t response_cmd = recv_buf[0];

                // verify command matches expected
                if((response_cmd & 0xE0) == expected_response_cmd) {
                    // verify node ID matches
                    // note: production code may filter CAN ID; simplified here

                    // fill response data, determine offset and size
                    if(expected_response_cmd == SDO_DOWNLOAD_RESPONSE) {
                        // write response: confirm success
                        return 0;
                    } else {
                        // read response: extract data
                        uint8_t bytes_to_copy = 0;

                        // determine data length from response command
                        if((response_cmd & 0x1F) == 0x03) {  // 4 bytes
                            bytes_to_copy = 4;
                            data_offset = 4;
                        } else if((response_cmd & 0x1F) == 0x07) {  // 3 bytes (uncommon)
                            bytes_to_copy = 3;
                            data_offset = 5;
                        } else if((response_cmd & 0x1F) == 0x0B) {  // 2 bytes
                            bytes_to_copy = 2;
                            data_offset = 4;
                        } else if((response_cmd & 0x1F) == 0x0F) {  // 1 byte
                            bytes_to_copy = 1;
                            data_offset = 4;
                        } else {
                            // default to 4 bytes
                            bytes_to_copy = 4;
                            data_offset = 4;
                        }

                        // copy data to output buffer
                        if(response_data != NULL && response_size != NULL) {
                            for(int i = 0; i < bytes_to_copy; i++) {
                                response_data[i] = recv_buf[data_offset + i];
                            }
                            *response_size = bytes_to_copy;
                        }

                        return 0;  // success
                    }
                } else if(response_cmd == SDO_ABORT_COMMUNICATION) {
                    LOG_PRINT(LOG_OUT_ERROR, "%sSDO communication aborted: 0x%02X\n", TAG, response_cmd);
                    return 3;  // communication aborted error
                }
            }

            rt_thread_mdelay(2);  // short delay
        }

        LOG_PRINT(LOG_OUT_DEBUG, "%sSDO read timeout, retry: %d\n", TAG, retry);
    }

    LOG_PRINT(LOG_OUT_ERROR, "%sSDO communication failed after 3 retries\n", TAG);
    return 2;  // timeout or max retries exceeded
}

/*
 * Set the motor control mode.
 * Returns 0 on success, other values on failure.
 */
static int hub_motor_set_control_mode(Motor_Device_T *motor_dev, uint8_t mode)
{
    if(motor_dev == NULL || motor_dev->private_data == NULL) {
        return -1;
    }

    Hub_Motor_Data_T *motor_data = (Hub_Motor_Data_T *)motor_dev->private_data;

    // use standard SDO communication wrapper
    Sdo_Request_T request = {
        .command = SDO_DOWNLOAD_REQUEST | 0x2F,  // 1-byte write
        .index = CANOPEN_OBJ_OPERATION_MODE,     // operation mode object (0x6060)
        .sub_index = CANOPEN_SUBINDEX_00,         // sub-index 0
        .data = mode,                            // operation mode
        .data_size = 1                            // 1-byte data
    };

    uint8_t response[SDO_FRAME_SIZE] = {0};
    uint8_t response_size = 0;

    int result = sdo_communicate(motor_data, &request, response, &response_size);

    return result;
}

/*
 * Clear the motor error status.
 * Returns 0 on success, other values on failure.
 */
static int hub_motor_clear_error(Motor_Device_T *motor_dev)
{
    if(motor_dev == NULL || motor_dev->private_data == NULL) {
        return -1;
    }

    Hub_Motor_Data_T *motor_data = (Hub_Motor_Data_T *)motor_dev->private_data;

    // send fault reset command
    Sdo_Request_T request = {
        .command = SDO_DOWNLOAD_REQUEST | 0x2B,  // 2-byte write
        .index = CANOPEN_OBJ_CONTROL_WORD,       // control word object (0x6040)
        .sub_index = CANOPEN_SUBINDEX_00,         // sub-index 0
        .data = 0x0086,                          // fault reset value
        .data_size = 2                            // 2-byte data
    };

    uint8_t response[SDO_FRAME_SIZE] = {0};
    uint8_t response_size = 0;

    int result = sdo_communicate(motor_data, &request, response, &response_size);

    return result;
}

/*
 * Read the motor status word.
 * Returns 0 on success, other values on failure.
 */
static int hub_motor_read_status_word(Motor_Device_T *motor_dev, uint16_t *status_word)
{
    if(motor_dev == NULL || motor_dev->private_data == NULL || status_word == NULL) {
        return -1;
    }

    Hub_Motor_Data_T *motor_data = (Hub_Motor_Data_T *)motor_dev->private_data;

    // use standard SDO communication wrapper
    Sdo_Request_T request = {
        .command = SDO_UPLOAD_REQUEST,           // upload request
        .index = CANOPEN_OBJ_CONTROL_WORD,       // control word object (0x6040)
        .sub_index = CANOPEN_SUBINDEX_00,         // sub-index 0
        .data = 0,                               // not used
        .data_size = 0                            // not used
    };

    uint8_t response[SDO_FRAME_SIZE] = {0};
    uint8_t response_size = 0;

    int result = sdo_communicate(motor_data, &request, response, &response_size);

    if(result == 0 && response_size >= 2) {
        *status_word = (response[0] | (response[1] << 8));
        return 0;
    }

    return -1;
}

// /*
//  * Read the driver error code.
//  * Returns 0 on success, other values on failure.
//  */
// static int hub_motor_read_driver_error_code(Motor_Device_T *motor_dev, uint16_t *error_code)
// {
//     if(motor_dev == NULL || motor_dev->private_data == NULL || error_code == NULL) {
//         return -1;
//     }
//
//     Hub_Motor_Data_T *motor_data = (Hub_Motor_Data_T *)motor_dev->private_data;
//
//     // read error code object (0x603F)
//     Sdo_Request_T request = {
//         .command = SDO_UPLOAD_REQUEST,           // upload request
//         .index = 0x603F,                         // error code object
//         .sub_index = CANOPEN_SUBINDEX_00,         // sub-index 0
//         .data = 0,                               // not used
//         .data_size = 0                            // not used
//     };
//
//     uint8_t response[SDO_FRAME_SIZE] = {0};
//     uint8_t response_size = 0;
//
//     int result = sdo_communicate(motor_data, &request, response, &response_size);
//
//     if(result == 0 && response_size >= 2) {
//         *error_code = (response[0] | (response[1] << 8));
//         return 0;
//     }
//
//     return -1;
// }
// /*
//  * Set the motor node ID.
//  * Returns 0 on success, other values on failure.
//  */
// static int hub_motor_set_node_id(Motor_Device_T *motor_dev, uint8_t node_id)
// {
//     if(motor_dev == NULL || motor_dev->private_data == NULL) {
//         return -1;
//     }
//
//     Hub_Motor_Data_T *motor_data = (Hub_Motor_Data_T *)motor_dev->private_data;
//     motor_data->node_id = node_id;
//
//     return 0;
// }

// /*
//  * Read the actual motor position.
//  * Returns 0 on success, other values on failure.
//  */
// static int hub_motor_read_actual_position(Motor_Device_T *motor_dev, uint32_t *position)
// {
//     if(motor_dev == NULL || motor_dev->private_data == NULL || position == NULL) {
//         return -1;
//     }
//
//     Hub_Motor_Data_T *motor_data = (Hub_Motor_Data_T *)motor_dev->private_data;
//
//     // use standard SDO communication wrapper
//     Sdo_Request_T request = {
//         .command = SDO_UPLOAD_REQUEST,           // upload request
//         .index = CANOPEN_OBJ_ACTUAL_POSITION,    // actual position object (0x6063)
//         .sub_index = CANOPEN_SUBINDEX_00,         // general sub-index
//         .data = 0,                               // not used
//         .data_size = 0                            // not used
//     };
//
//     uint8_t response[SDO_FRAME_SIZE] = {0};
//     uint8_t response_size = 0;
//
//     int result = sdo_communicate(motor_data, &request, response, &response_size);
//
//     if(result == 0 && response_size >= 4) {
//         *position = (response[0] | (response[1] << 8) | (response[2] << 16) | (response[3] << 24));
//         motor_data->position = *position;
//         return 0;
//     }
//
//     return -1;
// }

// /*
//  * Read the actual motor speed.
//  * Returns 0 on success, other values on failure.
//  */
// static int hub_motor_read_actual_speed(Motor_Device_T *motor_dev, int32_t *speed)
// {
//     if(motor_dev == NULL || motor_dev->private_data == NULL || speed == NULL) {
//         return -1;
//     }
//
//     Hub_Motor_Data_T *motor_data = (Hub_Motor_Data_T *)motor_dev->private_data;
//
//     // use standard SDO communication wrapper
//     Sdo_Request_T request = {
//         .command = SDO_UPLOAD_REQUEST,           // upload request
//         .index = CANOPEN_OBJ_ACTUAL_SPEED_INTERNAL, // actual speed object (0x606C)
//         .sub_index = CANOPEN_SUBINDEX_00,         // general sub-index
//         .data = 0,                               // not used
//         .data_size = 0                            // not used
//     };
//
//     uint8_t response[SDO_FRAME_SIZE] = {0};
//     uint8_t response_size = 0;
//
//     int result = sdo_communicate(motor_data, &request, response, &response_size);
//
//     if(result == 0 && response_size >= 4) {
//         *speed = (response[0] | (response[1] << 8) | (response[2] << 16) | (response[3] << 24));
//         motor_data->speed = *speed;
//         return 0;
//     }
//
//     return -1;
// }

// /*
//  * Set the target speed of the motor.
//  * Returns 0 on success, other values on failure.
//  */
// static int hub_motor_set_target_speed(Motor_Device_T *motor_dev, int32_t target_speed)
// {
//     if(motor_dev == NULL || motor_dev->private_data == NULL) {
//         return -1;
//     }
//
//     Hub_Motor_Data_T *motor_data = (Hub_Motor_Data_T *)motor_dev->private_data;
//
//     // use standard SDO communication wrapper
//     Sdo_Request_T request = {
//         .command = SDO_DOWNLOAD_REQUEST | 0x23,  // 4-byte write
//         .index = CANOPEN_OBJ_TARGET_SPEED,       // target speed object (0x60FF)
//         .sub_index = CANOPEN_SUBINDEX_00,         // general sub-index
//         .data = target_speed,                     // data
//         .data_size = 4                            // 4-byte data
//     };
//
//     uint8_t response[SDO_FRAME_SIZE] = {0};
//     uint8_t response_size = 0;
//
//     int result = sdo_communicate(motor_data, &request, response, &response_size);
//
//     if(result == 0) {
//         return 0;
//     }
//
//     return -1;
// }

/*
 * Enable the motor.
 * Returns 0 on success, other values on failure.
 */
static int hub_motor_enable(Motor_Device_T *motor_dev)
{
    if(motor_dev == NULL || motor_dev->private_data == NULL) {
        return -1;
    }

    Hub_Motor_Data_T *motor_data = (Hub_Motor_Data_T *)motor_dev->private_data;

    // use standard SDO communication wrapper
    Sdo_Request_T request = {
        .command = SDO_DOWNLOAD_REQUEST | 0x2B,  // 2-byte write
        .index = CANOPEN_OBJ_CONTROL_WORD,       // control word object (0x6040)
        .sub_index = CANOPEN_SUBINDEX_00,         // general sub-index
        .data = 0x000F,                          // enable value
        .data_size = 2                            // 2-byte data
    };

    uint8_t response[SDO_FRAME_SIZE] = {0};
    uint8_t response_size = 0;

    int result = sdo_communicate(motor_data, &request, response, &response_size);

    if(result == 0) {
        return 0;
    }

    return -1;
}

/*
 * Disable the motor.
 * Returns 0 on success, other values on failure.
 */
__attribute__((unused)) static int hub_motor_disable(Motor_Device_T *motor_dev)
{
    if(motor_dev == NULL || motor_dev->private_data == NULL) {
        return -1;
    }

    Hub_Motor_Data_T *motor_data = (Hub_Motor_Data_T *)motor_dev->private_data;

    // use standard SDO communication wrapper
    Sdo_Request_T request = {
        .command = SDO_DOWNLOAD_REQUEST | 0x2B,  // 2-byte write
        .index = CANOPEN_OBJ_CONTROL_WORD,       // control word object (0x6040)
        .sub_index = CANOPEN_SUBINDEX_00,         // general sub-index
        .data = 0x0000,                          // disable value
        .data_size = 2                            // 2-byte data
    };

    uint8_t response[SDO_FRAME_SIZE] = {0};
    uint8_t response_size = 0;

    int result = sdo_communicate(motor_data, &request, response, &response_size);

    if(result == 0) {
        return 0;
    }

    return -1;
}
#if 0
/*
 * Enable PDO function.
 * Returns 0 on success, other values on failure.
 */
static int hub_motor_enable_pdo(Motor_Device_T *motor_dev, uint8_t pdo_num, uint8_t enable)
{
    if(motor_dev == NULL || motor_dev->private_data == NULL) {
        return -1;
    }

    Hub_Motor_Data_T *motor_data = (Hub_Motor_Data_T *)motor_dev->private_data;

    Sdo_Request_T request;

    if(pdo_num == 1) {
        // enable PDO1 (object address 0x470001)
        request.command = SDO_DOWNLOAD_REQUEST | 0x2F;  // 1-byte write
        request.index = CANOPEN_OBJ_SIMPLE_PDO_FUNC_1;  // simple PDO function 1 (0x4700)
        request.sub_index = CANOPEN_SUBINDEX_01;         // sub-index 1
        request.data = enable ? 0x01 : 0x00;            // enable/disable
        request.data_size = 1;                           // 1-byte data
    } else if(pdo_num == 2) {
        // enable PDO2 (object address 0x470002)
        request.command = SDO_DOWNLOAD_REQUEST | 0x2F;  // 1-byte write
        request.index = CANOPEN_OBJ_SIMPLE_PDO_FUNC_1;  // simple PDO function 1 (0x4700)
        request.sub_index = CANOPEN_SUBINDEX_02;         // sub-index 2
        request.data = enable ? 0x01 : 0x00;            // enable/disable
        request.data_size = 1;                           // 1-byte data
    } else {
        return -1;
    }

    uint8_t response[SDO_FRAME_SIZE] = {0};
    uint8_t response_size = 0;

    int result = sdo_communicate(motor_data, &request, response, &response_size);

    if(result == 0) {
        if(pdo_num == 1) {
            motor_data->pdo_enable1 = enable;
        } else if(pdo_num == 2) {
            motor_data->pdo_enable2 = enable;
        }
        return 0;
    }

    return -1;
}
#endif

/*
 * Hub motor initialization.
 * Returns 0 on success, other values on failure.
 */
static int hub_motor_init(void *privatedata)
{
    Motor_Device_T *motor_dev = (Motor_Device_T *)privatedata;
    Hub_Motor_Data_T *data = (Hub_Motor_Data_T *)motor_dev->private_data;
    uint8_t retry = 3;
    int result = 0;

    // get CAN device
    data->can_dev = get_can_device(data->can_name);
    if(data->can_dev == NULL) {
        LOG_PRINT(LOG_OUT_ERROR, "%sFind %s error!\n",TAG, data->can_name);
        return -1;
    }

    // initialize CAN device
    if(data->can_dev->init(data->can_dev) != 0) {
        LOG_PRINT(LOG_OUT_ERROR, "%s%s init error!\n",TAG, data->can_name);
        return -1;
    }

    // step-by-step initialization
    // 1. clear error
    while(retry--) {
        result = hub_motor_clear_error(motor_dev);
        if(result == 0) {
            break;
        }
        rt_thread_mdelay(10);
    }

    if(result != 0) {
        LOG_PRINT(LOG_OUT_ERROR, "%sClear motor error failed!\n", TAG);
        return -1;
    }

    // 2. wait for clear to complete
    rt_thread_mdelay(10);

    // 3. set operation mode to speed mode
    result = hub_motor_set_control_mode(motor_dev, 3); // speed mode is typically 0x03
    if(result != 0) {
        LOG_PRINT(LOG_OUT_ERROR, "%sSet control mode failed!\n", TAG);
        return -1;
    }

    // 4. set target speed to 0
    result = hub_motor_set_speed(motor_dev, 0);
    if(result != 0) {
        LOG_PRINT(LOG_OUT_ERROR, "%sSet initial speed to 0 failed!\n", TAG);
        return -1;
    }

    // 5. enable motor
    retry = 3;
    while(retry--) {
        result = hub_motor_enable(motor_dev);
        if(result == 0) {
            break;
        }
        rt_thread_mdelay(10);
    }

    if(result != 0) {
        LOG_PRINT(LOG_OUT_ERROR, "%sEnable motor failed!\n", TAG);
        return -1;
    }

    // 6. short delay for enable to take effect
    rt_thread_mdelay(100);

    // 7. verify enable status
    uint16_t status_word;
    result = hub_motor_read_status_word(motor_dev, &status_word);
    if(result != 0) {
        LOG_PRINT(LOG_OUT_ERROR, "%sRead status word failed!\n", TAG);
        return -1;
    }

    // verify enable status (bits 3 and 4 should be set)
    if(!(status_word & 0x0040) || !(status_word & 0x0020)) {
        LOG_PRINT(LOG_OUT_ERROR, "%sMotor not enabled properly! Status: 0x%04X\n", TAG, status_word);
        return -1;
    }

    LOG_PRINT(LOG_OUT_DEBUG, "%sHub motor init success!\n", TAG);
    return 0;
}

/*
 * Set the motor speed.
 * Returns 0 on success, other values on failure.
 */
static int hub_motor_set_speed(void *privatedata, int speed)
{
    Hub_Motor_Data_T *motor_data = (Hub_Motor_Data_T *)privatedata;

    // use standard SDO communication wrapper
    Sdo_Request_T request = {
        .command = SDO_DOWNLOAD_REQUEST | 0x23,  // 4-byte write
        .index = CANOPEN_OBJ_TARGET_SPEED,       // target speed object (0x60FF)
        .sub_index = CANOPEN_SUBINDEX_00,         // general sub-index
        .data = speed,                           // data
        .data_size = 4                            // 4-byte data
    };

    uint8_t response[SDO_FRAME_SIZE] = {0};
    uint8_t response_size = 0;

    int result = sdo_communicate(motor_data, &request, response, &response_size);

    if(result == 0) {
        motor_data->speed = speed;
        return 0;
    }

    return -1;
}

/*
 * Set the motor position.
 * Returns 0 on success, other values on failure.
 */
static int hub_motor_set_position(void *privatedata, uint32_t position)
{
    Hub_Motor_Data_T *motor_data = (Hub_Motor_Data_T *)privatedata;

    // use standard SDO communication wrapper
    Sdo_Request_T request = {
        .command = SDO_DOWNLOAD_REQUEST | 0x23,  // 4-byte write
        .index = CANOPEN_OBJ_ABSOLUTE_TARGET_POSITION, // absolute target position object (0x607A)
        .sub_index = CANOPEN_SUBINDEX_00,         // general sub-index
        .data = position,                        // data
        .data_size = 4                            // 4-byte data
    };

    uint8_t response[SDO_FRAME_SIZE] = {0};
    uint8_t response_size = 0;

    int result = sdo_communicate(motor_data, &request, response, &response_size);

    if(result == 0) {
        motor_data->position = position;
        return 0;
    }

    return -1;
}

/*
 * Read the motor speed.
 * Returns 0 on success, other values on failure.
 */
static int hub_motor_read_speed(void *privatedata, int *speed)
{
    Hub_Motor_Data_T *motor_data = (Hub_Motor_Data_T *)privatedata;

    // use standard SDO communication wrapper
    Sdo_Request_T request = {
        .command = SDO_UPLOAD_REQUEST,           // upload request
        .index = CANOPEN_OBJ_ACTUAL_SPEED_INTERNAL, // actual speed object (0x606C)
        .sub_index = CANOPEN_SUBINDEX_00,         // general sub-index
        .data = 0,                               // not used
        .data_size = 0                            // not used
    };

    uint8_t response[SDO_FRAME_SIZE] = {0};
    uint8_t response_size = 0;

    int result = sdo_communicate(motor_data, &request, response, &response_size);

    if(result == 0 && response_size >= 4) {
        int32_t new_speed = (response[0] | (response[1] << 8) | (response[2] << 16) | (response[3] << 24));
        motor_data->speed = new_speed;
        *speed = new_speed;
        return 0;
    }

    return -1;
}

/*
 * Read the motor position.
 * Returns 0 on success, other values on failure.
 */
static int hub_motor_read_position(void *privatedata, uint32_t *position)
{
    Hub_Motor_Data_T *motor_data = (Hub_Motor_Data_T *)privatedata;

    // use standard SDO communication wrapper
    Sdo_Request_T request = {
        .command = SDO_UPLOAD_REQUEST,           // upload request
        .index = CANOPEN_OBJ_ACTUAL_POSITION,    // actual position object (0x6063)
        .sub_index = CANOPEN_SUBINDEX_00,         // general sub-index
        .data = 0,                               // not used
        .data_size = 0                            // not used
    };

    uint8_t response[SDO_FRAME_SIZE] = {0};
    uint8_t response_size = 0;

    int result = sdo_communicate(motor_data, &request, response, &response_size);

    if(result == 0 && response_size >= 4) {
        uint32_t new_position = (response[0] | (response[1] << 8) | (response[2] << 16) | (response[3] << 24));
        motor_data->position = new_position;
        *position = new_position;
        return 0;
    }

    return -1;
}

/*
 * Read the motor error code.
 * Returns 0 on success, other values on failure.
 */
static int hub_motor_read_error_code(void *privatedata, int *errorcode)
{
    Hub_Motor_Data_T *motor_data = (Hub_Motor_Data_T *)privatedata;

    // use standard SDO communication wrapper
    Sdo_Request_T request = {
        .command = SDO_UPLOAD_REQUEST,           // upload request
        .index = 0x603F,                         // error code object (0x603F)
        .sub_index = CANOPEN_SUBINDEX_00,         // general sub-index
        .data = 0,                               // not used
        .data_size = 0                            // not used
    };

    uint8_t response[SDO_FRAME_SIZE] = {0};
    uint8_t response_size = 0;

    int result = sdo_communicate(motor_data, &request, response, &response_size);

    if(result == 0 && response_size >= 2) {
        uint16_t new_error_code = (response[0] | (response[1] << 8));
        motor_data->error_code = new_error_code;
        *errorcode = new_error_code;
        return 0;
    }

    return -1;
}

/*
 * Read motor data (generic).
 * Returns the length of data read.
 */
static int hub_motor_read(void *privatedata, Motor_Status_T *status)
{
    (void)privatedata;
    (void)status;
    return 0;
}




/*
 * Get the hub motor device structure.
 * Returns the device structure pointer.
 */
Motor_Device_T *get_hub_motor_dev(char *name)
{
    Motor_Device_T *dev = NULL;

    if(0 == strcmp(name, "hub_motor"))
        dev = &s_hub_motor_dev;

    return dev;
}


