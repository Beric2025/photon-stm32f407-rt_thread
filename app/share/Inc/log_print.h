/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef __LOG_PRINT_H__
#define __LOG_PRINT_H__

#include "project_config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LOG_OUT_DEBUG = 0,
    LOG_OUT_INFO,
    LOG_OUT_WARN,
    LOG_OUT_ERROR
} Log_Print_Level_E;

#if LOG_PRINT_ENABLE
#define LOG_PRINT(level, fmt, ...) \
    do { \
        if (level >= LOG_PRINT_LEVEL) { \
            log_print(level, fmt, ##__VA_ARGS__); \
        } \
    } while (0)
#else
#define LOG_PRINT(level, fmt, ...)
#endif

/**
 * @brief:  build version string from compile-time date/time
 */
void log_print_init(void);
/**
 * @brief:  print function, format and send log message via UART
 * @level:  log level
 * @fmt:  format string
 */
void log_print(int level, const char *fmt, ...);


#ifdef __cplusplus
}
#endif
#endif /*__ LOG_PRINT_H__ */
