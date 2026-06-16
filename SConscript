#
# Copyright (c) 2026 beric-xiong
# SPDX-License-Identifier: MIT
#
# Root SConscript - orchestrates all BSP-level modules.
# Called automatically by building.py PrepareBuilding().
#
# Kernel (src/), libcpu, and components (Finsh etc.) are compiled
# automatically by building.py via their own SConscripts - they
# are NOT listed here.
#

import os
from building import *

objs = []
cwd = GetCurrentDir()

# ============================================================
# Sub-modules (each with its own SConscript)
# ============================================================
sub_modules = [
    'port',
    'drivers',
    'app',
    'interface',
    os.path.join('third_party', 'STM32F4xx_HAL_Driver'),
    # lwIP is a git submodule without its own SConscript —
    # its core sources are compiled by port/SConscript instead.
]

for mod in sub_modules:
    sconscript = os.path.join(cwd, mod, 'SConscript')
    if os.path.isfile(sconscript):
        objs.extend(SConscript(sconscript))
    else:
        print('WARNING: SConscript not found: ' + sconscript)

Return('objs')
