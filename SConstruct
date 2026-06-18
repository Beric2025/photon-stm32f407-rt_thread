#
# Copyright (c) 2026 beric-xiong
# SPDX-License-Identifier: MIT
#
# SConstruct - RT-Thread build entry for STM32F407
#
# Usage:
#   scons                    # build
#   scons -j8                # parallel build
#   scons -c                 # clean
#   scons --menuconfig       # graphical configuration
#   scons --target=cmake     # export CMakeLists.txt
#   scons --target=vsc       # export VS Code config
#

import os
import sys

# ============================================================
# RT-Thread root directory
# ============================================================
RTT_ROOT = os.path.join(os.getcwd(), 'third_party', 'rt-thread')
sys.path = sys.path + [os.path.join(RTT_ROOT, 'tools')]

import rtconfig
from building import *

# ============================================================
# Build environment
# ============================================================
env = Environment(
    tools=['mingw'],
    AS=rtconfig.AS,   ASFLAGS=rtconfig.AFLAGS,
    CC=rtconfig.CC,   CFLAGS=rtconfig.CFLAGS,
    CXX=rtconfig.CXX,
    AR=rtconfig.AR,   ARFLAGS='-rc',
    LINK=rtconfig.LINK, LINKFLAGS=rtconfig.LFLAGS,
    CPPPATH=rtconfig.CPPPATH,
)
env['CPPDEFINES'] = rtconfig.CPPDEFINES

Export('RTT_ROOT')
Export('rtconfig')

# ============================================================
# Banner
# ============================================================
print('')
print('=' * 60)
print('  RT-Thread STM32F407 Build System')
print('=' * 60)

# ============================================================
# Prepare building environment
#
# PrepareBuilding reads rtconfig.h from CWD and automatically:
#   1. Calls SConscript('SConscript')   -> BSP-level modules
#   2. Calls src/SConscript              -> kernel core
#      (auto-selects cpu_up/scheduler_up vs cpu_mp/scheduler_mp)
#   3. Calls libcpu/SConscript           -> Cortex-M4 CPU port
#   4. Calls components/SConscript       -> Finsh + other components
#
# SMP/multi-core is enabled/disabled by changing RT_CPUS_NR
# and RT_USING_SMP in rtconfig.h - no manual file-list edits needed.
# ============================================================
objs = PrepareBuilding(env, RTT_ROOT, has_libcpu=False)

# ============================================================
# Link executable
#
# DoBuilding handles Program creation, POST_ACTION (.bin/.hex/.dis),
# and all other post-build steps (dist, IDE export, cscope, etc.)
# ============================================================
TARGET = 'photon.elf'
DoBuilding(TARGET, objs)
