/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * Microsecond busy-wait using Pwm_Device_T (TIM6 free-running counter at 1 MHz)
 */
#include <stdio.h>
#include "delay_dev.h"
#include "pwm_dev.h"

static Pwm_Device_T *s_delay_timer = NULL;

void delay_us(uint32_t us)
{
    uint32_t told, tnow, cnt = 0;

    if (s_delay_timer == NULL) {
        s_delay_timer = get_pwm_device("tim6");
        if (s_delay_timer == NULL)
            return;
        s_delay_timer->init(s_delay_timer, 1);
        s_delay_timer->start(s_delay_timer);
    }

    s_delay_timer->get_count(s_delay_timer, &told);
    while (1) {
        s_delay_timer->get_count(s_delay_timer, &tnow);
        cnt += tnow - told;
        told = tnow;
        if (cnt >= us)
            break;
    }
}

void delay_ms(uint32_t ms)
{
    while (ms--) {
        delay_us(1000);
    }
}
