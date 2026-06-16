/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * Board-level initialization for STM32F407 + RT-Thread
 *
 * Uses SysTick as the RT-Thread system tick source. The HAL timebase
 * (TIM6) is kept independent — HAL_InitTick / HAL_SuspendTick /
 * HAL_ResumeTick remain in stm32f4xx_hal_timebase_tim.c unchanged.
 */

#include <rthw.h>
#include <rtthread.h>
#include "board.h"
#include "stm32f4xx_hal.h"

/* Forward declaration from main.c */
extern void SystemClock_Config(void);

/* Extern for SystemCoreClock — defined by CMSIS */
extern uint32_t SystemCoreClock;

/* ============================================================
 * SysTick interrupt — feeds RT-Thread tick counter
 * ============================================================ */
void SysTick_Handler(void)
{
    rt_interrupt_enter();
    rt_tick_increase();
    rt_interrupt_leave();
}

/* ============================================================
 * Hardware initialisation — called by rtthread_startup()
 * ============================================================ */
void rt_hw_board_init(void)
{
    /* HAL library init (reset peripherals, flash interface, SysTick config) */
    HAL_Init();

    /* Clock configuration: HSE → PLL → 168 MHz */
    SystemClock_Config();

    /* Configure SysTick for RT-Thread tick (1ms = RT_TICK_PER_SECOND = 1000) */
    SysTick_Config(SystemCoreClock / RT_TICK_PER_SECOND);

    /* Set SysTick interrupt priority (lowest) */
    NVIC_SetPriority(SysTick_IRQn, 0xFF);

    /* Initialize RT-Thread heap */
    rt_system_heap_init(HEAP_BEGIN, HEAP_END);

    /* Initialize hardware drivers registered via INIT_BOARD_EXPORT */
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif

#ifdef RT_USING_DEVICE
    /* Initialize the console device for Finsh (requires device framework) */
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
#endif
}
