/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * light device abstraction layer
 */

#include <string.h>
#include "light_dev.h"

extern Light_Device_T *get_tm512ac_light_dev(void);

/*
 * Look up a light device by name.
 * Returns pointer to the device structure, or NULL if not found.
 */
Light_Device_T *get_light_device(char *name)
{
    Light_Device_T *dev = get_tm512ac_light_dev();
    if(0 == strcmp(name, dev->name))
        return dev;

    return NULL;

}
