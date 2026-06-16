/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * battery device interface
 */

#include <string.h>
#include "battery_dev.h"

extern Battery_Device_T *get_xingneng_dev(void);


Battery_Device_T *get_battery_device(char *name)
{
    Battery_Device_T *dev = get_xingneng_dev();
    if (0 == strcmp(name, dev->name))
        return dev;

    return NULL;
}
