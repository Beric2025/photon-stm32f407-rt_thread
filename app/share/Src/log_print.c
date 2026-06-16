/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * Log print module with level-based output (DEBUG/INFO/WARN/ERROR)
 */

#include "log_print.h"
#include "uart_dev.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define LOG_PRINT_BUF_SIZE 256

static const char *log_print_level_str[] = {
    "[DEBUG] ",
    "[INFO]  ",
    "[WARN]  ",
    "[ERROR] ",
};

Uart_Device_T *print = NULL;


void log_print_init(void)
{
    int ret;

    print = get_uart_device("uart1");
    if (!print) {
        return;
    }

    ret = print->init(print, 115200);
    if (ret != 0) {
        return;
    }
}

/**
 * log_print - Format and send log message via UART
 * @level: Log level (DEBUG/INFO/WARN/ERROR)
 * @fmt: Format string
 * @...: Variable arguments
 *
 * Return: None
 */
void log_print(int level, const char *fmt, ...)
{
    char buf[LOG_PRINT_BUF_SIZE];
    va_list args;
    int len;

    if (level < LOG_OUT_DEBUG || level > LOG_OUT_ERROR)
    {
        return;
    }

    len = snprintf(buf, sizeof(buf), "%s", log_print_level_str[level]);
    if (len < 0 || len >= (int)sizeof(buf))
    {
        return;
    }

    va_start(args, fmt);
    len = vsnprintf(buf + len, sizeof(buf) - len, fmt, args);
    va_end(args);
    if (len < 0)
    {
        return;
    }

    if (print) {
        print->send(print->private_data, buf, strlen(buf));
    }
}
