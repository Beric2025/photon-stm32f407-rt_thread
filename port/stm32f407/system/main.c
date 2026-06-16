/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * Main entry — with RT_USING_USER_MAIN, main() is called twice:
 *   1. Boot → main() → rtthread_startup() → scheduler (never returns).
 *   2. Scheduler runs "main" thread → main() → application init.
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "gpio.h"
#include <rtthread.h>
#include "app_manage.h"

/* Declared in components.c */
extern int rtthread_startup(void);

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* Boot guard: first call starts RT-Thread, second call does app init */
static int _rt_booted = 0;

/**
  * @brief  The application entry point.
  *
  * On first call (boot), starts RT-Thread kernel and scheduler.
  * On second call (from RT-Thread main thread), runs application init.
  *
  * @retval int
  */
int main(void)
{
    if (_rt_booted == 0) {
        _rt_booted = 1;
        /* Boot path: never returns (scheduler takes over) */
        rtthread_startup();
        return 0;
    }

    /* User-main path: scheduler is running, init app and peripherals */
    MX_GPIO_Init();
    application_init();

    /* Main thread exits — RT-Thread keeps running other threads */
    return 0;
}

/**
  * @brief System Clock Configuration (HSE → PLL → 168 MHz)
  * @note  Called from rt_hw_board_init(), not directly from main().
  * @retval None
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 4;
    RCC_OscInitStruct.PLL.PLLN = 168;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief  Period elapsed callback (TIM6 HAL timebase)
  * @note   TIM6 is reserved for HAL timebase. Invoked from HAL_TIM_IRQHandler().
  *         Calls HAL_IncTick() to maintain uwTick for HAL delay/timeout APIs.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6)
    {
        HAL_IncTick();
    }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number */
}
#endif /* USE_FULL_ASSERT */
