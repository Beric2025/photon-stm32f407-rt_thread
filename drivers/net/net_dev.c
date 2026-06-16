/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * NET device abstraction layer
 */

#include <string.h>
#include "net_dev.h"

extern Net_Device_T *get_net1_dev(void);

Net_Device_T *get_net_device(char *name)
{
    Net_Device_T *pDev = get_net1_dev();
    if (0 == strcmp(name, pDev->name))
        return pDev;

    return NULL;
}
