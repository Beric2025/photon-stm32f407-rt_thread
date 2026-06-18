#
# Copyright (c) 2026 beric-xiong
# SPDX-License-Identifier: MIT
#
# RT-Thread toolchain configuration for STM32F407
# Based on the standard RT-Thread BSP template for stm32f4xx.
#

import os

# ============================================================
# Architecture identifiers (required by RT-Thread build system)
# ============================================================
ARCH = 'arm'
CPU  = 'cortex-m4'

# ============================================================
# Toolchain type
# ============================================================
CROSS_TOOL = 'gcc'
PLATFORM   = 'gcc'

# ============================================================
# BSP library config
# ============================================================
BSP_LIBRARY_TYPE = None

# ============================================================
# Override from environment
# ============================================================
if os.getenv('RTT_CC'):
    CROSS_TOOL = os.getenv('RTT_CC')

if os.getenv('RTT_ROOT'):
    RTT_ROOT = os.getenv('RTT_ROOT')

# ============================================================
# Toolchain executable path and prefix
#
# EXEC_PATH is the compiler execute path. Set a default, then
# allow the RTT_EXEC_PATH environment variable to override it —
# this is the standard RT-Thread convention.
#
# If neither the default nor RTT_EXEC_PATH is valid, the build
# system will auto-detect the toolchain from the system PATH.
# ============================================================
if CROSS_TOOL == 'gcc':
    EXEC_PATH = r'D:/Toolchain/arm-gnu-toolchain-15.2.rel1/bin'
elif CROSS_TOOL == 'keil':
    EXEC_PATH = r'C:/Keil_v5'
elif CROSS_TOOL == 'iar':
    EXEC_PATH = r'C:/Program Files (x86)/IAR Systems/Embedded Workbench 8.3'
else:
    EXEC_PATH = ''

if os.getenv('RTT_EXEC_PATH'):
    EXEC_PATH = os.getenv('RTT_EXEC_PATH')

PREFIX = 'arm-none-eabi-'

# ============================================================
# Build tools
# ============================================================
CC       = PREFIX + 'gcc'
CXX      = PREFIX + 'g++'
AS       = PREFIX + 'gcc'
AR       = PREFIX + 'ar'
LINK     = PREFIX + 'gcc'
SIZE     = PREFIX + 'size'
OBJDUMP  = PREFIX + 'objdump'
OBJCPY   = PREFIX + 'objcopy'

TARGET_EXT = 'elf'

# ============================================================
# Build type
# ============================================================
from SCons.Script import ARGUMENTS
BUILD = ARGUMENTS.get('BUILD', 'debug')

# ============================================================
# MCU architecture flags (Cortex-M4F with FPU)
# ============================================================
DEVICE = ' -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard'
DEVICE += ' -ffunction-sections -fdata-sections'

CFLAGS  = DEVICE + ' -Wall -Wextra'
AFLAGS  = ' -c' + DEVICE + ' -x assembler-with-cpp -Wa,-mimplicit-it=thumb '
LFLAGS  = DEVICE + ' -Wl,--gc-sections,-Map=photon.map,-cref,-u,Reset_Handler'
LFLAGS += ' -T port/stm32f407/STM32F407ZGTx_FLASH.ld'
LFLAGS += ' --specs=nano.specs --specs=nosys.specs'

CXXFLAGS = CFLAGS

# ============================================================
# Optimization level
# ============================================================
if BUILD == 'debug':
    CFLAGS += ' -Og -g'
    AFLAGS += ' -g'
else:
    CFLAGS += ' -Os'

# ============================================================
# Global include paths
# ============================================================
CPPPATH = [
    'port/stm32f407/system',
    'port/stm32f407/bsp',
    'port/rtthread',
    'port/lwip',
    'port/lwip/arch',
    'port/lwip/common',
    'third_party/CMSIS/Include',
    'third_party/CMSIS/Device/ST/STM32F4xx/Include',
    'third_party/rt-thread/include',
    'third_party/rt-thread/libcpu/arm/cortex-m4',
    'third_party/rt-thread/components/finsh',
    'third_party/lwIP/src/include',
    'third_party/lwIP',
    'third_party/STM32F4xx_HAL_Driver/Inc',
    'third_party/STM32F4xx_HAL_Driver/Inc/Legacy',
    'interface',
    'interface/gpio', 'interface/uart', 'interface/can',
    'interface/flash', 'interface/iic', 'interface/spi',
    'interface/ethernet', 'interface/adc', 'interface/tim',
    'interface/wdg', 'interface/rtc', 'interface/dac',
    'interface/i2s', 'interface/lp', 'interface/rng',
    'drivers',
    'drivers/battery', 'drivers/light', 'drivers/motor',
    'drivers/ultrasonic', 'drivers/iap', 'drivers/eth_phy',
    'drivers/net', 'drivers/delay', 'drivers/pcf8574',
    'app',
    'app/app_core/Inc', 'app/share/Inc',
]

CPPPATH = [p.replace('\\', '/') for p in CPPPATH]

# ============================================================
# Global preprocessor definitions
# ============================================================
CPPDEFINES = [
    'STM32F407xx',
    'USE_HAL_DRIVER',
    'USE_PWR_LDO_SUPPLY',
]

# ============================================================
# Post-build actions
# ============================================================
POST_ACTION = OBJCPY + ' -O binary $TARGET photon.bin\n'
POST_ACTION += OBJCPY + ' -O ihex  $TARGET photon.hex\n'
POST_ACTION += SIZE + ' $TARGET \n'
