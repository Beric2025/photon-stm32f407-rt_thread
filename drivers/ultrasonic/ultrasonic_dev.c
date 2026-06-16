/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * ultrasonic device abstraction layer
 */

#include <string.h>
#include "ultrasonic_dev.h"

extern Ultrasonic_Device_T *get_dypa21_one_dev(void);
extern Ultrasonic_Device_T *get_dypa21_two_dev(void);

/*
 * Look up an ultrasonic device by name.
 * Returns pointer to the device structure, or NULL if not found.
 */
Ultrasonic_Device_T *get_ultrasonic_device(char *name)
{
    Ultrasonic_Device_T *dev = get_dypa21_one_dev();
    if(0 == strcmp(name, dev->name))
        return dev;

    dev = get_dypa21_two_dev();
    if(0 == strcmp(name, dev->name))
        return dev;

    return NULL;

}
