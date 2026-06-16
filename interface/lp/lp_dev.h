/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef _LP_DEV_H_
#define _LP_DEV_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>


/* power modes (MCU-agnostic, BSP maps to hardware-specific modes) */
#define LP_MODE_SLEEP    0   /* light sleep, CPU off, peripherals on */
#define LP_MODE_STOP     1   /* deep sleep, most clocks off, EXTI/RTC wakeup */
#define LP_MODE_STANDBY  2   /* lowest power, only pin/RTC alarm wakeup */

/* wakeup sources */
#define LP_WAKEUP_EXTI   0   /* external pin interrupt */
#define LP_WAKEUP_RTC    1   /* RTC alarm */


typedef struct {
    /* device identifier (e.g. "lp") */
    char *name;

    /**
     * @brief:  enter low-power mode
     * @privatedata:  pointer to this LowPower_Device_T
     * @mode:         LP_MODE_SLEEP / LP_MODE_STOP / LP_MODE_STANDBY
     *
     * Return: 0 on success, -1 on failure
     */
    int (*enter)(void *privatedata, unsigned char mode);

    /**
     * @brief:  configure wakeup source
     * @privatedata:  pointer to this LowPower_Device_T
     * @source:       LP_WAKEUP_EXTI / LP_WAKEUP_RTC
     * @enable:       1 = enable, 0 = disable
     *
     * Return: 0 on success, -1 on failure
     */
    int (*set_wakeup)(void *privatedata, uint32_t source, unsigned char enable);

    void *private_data;
} LowPower_Device_T;

/**
 * @brief:  get low-power device structure by name
 * @name:   device name (e.g. "lp")
 *
 * Return: pointer to LowPower_Device_T structure, NULL if not found
 */
LowPower_Device_T *get_lp_device(char *name);

#ifdef __cplusplus
}
#endif

#endif /* _LP_DEV_H_ */
