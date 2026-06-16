/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * app version
 */

#include <stdio.h>
#include <string.h>
#include "app_version.h"

static char s_version[64];

int build_version(void)
{
    char month_str[16] = {0};
    char date_str[16]  = {0};
    char time_str[16]  = {0};
    int year = 2000;
    int month = 1;
    int day = 1;
    int hour  = 1;
    int minute  = 1;
    int second  = 1;
    char *month_names[12] = {"Jan","Feb","Mar", "Apr", "May","Jun","Jul", "Aug", "Sep","Oct","Nov","Dec"};

    strncpy(date_str, __DATE__, sizeof(date_str)-1);
    strncpy(time_str, __TIME__, sizeof(time_str)-1);

    if (sscanf(date_str, "%3s %d %d", month_str, &day, &year) != 3) {
        return -1;
    }
    if (sscanf(time_str, "%d:%d:%d", &hour, &minute, &second) != 3) {
        return -1;
    }

    for (int i = 0; i < 12; i++) {
        if (strncmp(month_str, month_names[i], 3) == 0) {
            month = i + 1;
            break;
        }
    }

    /* Format: <APP_VERSION>.<YYYYMMDDhhmm>
       e.g. v0.0.1-a1b2c3d.202602131430                     */
    memset(s_version, 0, sizeof(s_version));
    snprintf(s_version, sizeof(s_version), "%s.%04d%02d%02d%02d%02d",
             APP_VERSION, year, month, day, hour, minute);
    return 0;
}

int get_version(char *verbuffer, int buffersize)
{
    int ver_len = (int)strlen(s_version);

    if (buffersize < ver_len + 1) {
        return -1;
    }

    memcpy(verbuffer, s_version, ver_len + 1);
    return ver_len;
}
