/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef _DELAY_DEV_H_
#define _DELAY_DEV_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>


/**
 * @brief: microsecond delay (busy-wait)
 * @us: delay time in microseconds
 */
void delay_us(uint32_t us);

/**
 * @brief: millisecond delay (busy-wait)
 * @ms: delay time in milliseconds
 */
void delay_ms(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif /* _DELAY_DEV_H_ */
