/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * motor device abstraction layer
 */

#include <string.h>
#include "motor_dev.h"

extern Motor_Device_T *get_hub_motor_dev(char *name);

/*
 * Look up a motor device by name.
 * Returns pointer to the device structure, or NULL if not found.
 */
Motor_Device_T *get_motor_device(char *name)
{
    Motor_Device_T *dev = get_hub_motor_dev(name);
    if(0 == strcmp(name, dev->name))
        return dev;

    return NULL;
}
