/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * Finsh shell console port — binds Finsh to UART1 via the existing
 * Uart_Device_T interface layer.
 */

#include <rtthread.h>
#include <string.h>
#include "uart_dev.h"

/* UART1 device handle obtained via get_uart_device("uart1") */
static Uart_Device_T *finsh_uart = NULL;

/* ============================================================
 * Finsh console output — rt_hw_console_output()
 *
 * Called by the Finsh shell to print characters. Uses the existing
 * UART device abstraction so no direct HAL calls are needed.
 * ============================================================ */
void rt_hw_console_output(const char *str)
{
    if (finsh_uart == NULL) {
        finsh_uart = get_uart_device("uart1");
        if (finsh_uart == NULL) return;
    }

    rt_size_t len = rt_strlen(str);
    finsh_uart->send(finsh_uart, (char *)str, (unsigned short)len);
}

/* ============================================================
 * Finsh console input — rt_hw_console_getchar()
 *
 * Called by Finsh to read a single character. Blocking until
 * data arrives. Returns '\0' if no device.
 * ============================================================ */
char rt_hw_console_getchar(void)
{
    char ch = 0;

    if (finsh_uart == NULL) {
        finsh_uart = get_uart_device("uart1");
        if (finsh_uart == NULL) return '\0';
    }

    /* Spin until data is available */
    while (finsh_uart->receive(finsh_uart, &ch, 1) == 0) {
        /* busy-wait */
    }

    return ch;
}
