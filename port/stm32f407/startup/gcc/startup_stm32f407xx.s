/*******************************************************************************
 * File Name          : startup_stm32f407xx.s
 * Author             : MCD Application Team
 * Description        : STM32F407xx vector table for GCC toolchain.
 *                      This module performs:
 *                      - Set the initial SP
 *                      - Set the initial PC == Reset_Handler
 *                      - Set the vector table entries with the exceptions ISR address
 *                      - Branches to main()
 *                      After Reset the Cortex-M4 processor is in Thread mode,
 *                      priority is Privileged, and the Stack is set to Main.
 *******************************************************************************
 * @attention
 *
 * Copyright (c) 2017 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 *******************************************************************************
 */

  .syntax unified
  .cpu    cortex-m4
  .fpu    fpv4-sp-d16
  .thumb

/*******************************************************************************
 * Stack Configuration
 *******************************************************************************
 *  <h> Stack Configuration
 *    <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
 *  </h>
 */
  .equ  Stack_Size, 0x1600

  .section  .stack, "aw", %nobits
  .align  3
  .globl  __initial_sp
__initial_sp:
  .space  Stack_Size
  .size   __initial_sp, . - __initial_sp

/*******************************************************************************
 * Heap Configuration
 *******************************************************************************
 *  <h> Heap Configuration
 *    <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
 *  </h>
 */
  .equ  Heap_Size, 0x800

  .section  .heap, "aw", %nobits
  .align  3
  .globl  __heap_base
__heap_base:
  .space  Heap_Size
  .globl  __heap_limit
__heap_limit:
  .size   __heap_limit, . - __heap_limit

/*******************************************************************************
 * Vector Table — mapped to address 0 at reset
 ******************************************************************************/

  .section  .isr_vector, "a", %progbits
  .globl  __Vectors
  .globl  __Vectors_End
  .globl  __Vectors_Size

  .type   __Vectors, %object
  .size   __Vectors, . - __Vectors

__Vectors:
  .word  __initial_sp                    /* Top of Stack                  */
  .word  Reset_Handler                   /* Reset Handler                 */
  .word  NMI_Handler                     /* NMI Handler                   */
  .word  HardFault_Handler               /* Hard Fault Handler            */
  .word  MemManage_Handler               /* MPU Fault Handler             */
  .word  BusFault_Handler                /* Bus Fault Handler             */
  .word  UsageFault_Handler              /* Usage Fault Handler           */
  .word  0                               /* Reserved                      */
  .word  0                               /* Reserved                      */
  .word  0                               /* Reserved                      */
  .word  0                               /* Reserved                      */
  .word  SVC_Handler                     /* SVCall Handler                */
  .word  DebugMon_Handler                /* Debug Monitor Handler         */
  .word  0                               /* Reserved                      */
  .word  PendSV_Handler                  /* PendSV Handler                */
  .word  SysTick_Handler                 /* SysTick Handler               */

  /* External Interrupts */
  .word  WWDG_IRQHandler                 /* Window WatchDog               */
  .word  PVD_IRQHandler                  /* PVD through EXTI Line         */
  .word  TAMP_STAMP_IRQHandler           /* Tamper and TimeStamps         */
  .word  RTC_WKUP_IRQHandler             /* RTC Wakeup through EXTI       */
  .word  FLASH_IRQHandler                /* FLASH                         */
  .word  RCC_IRQHandler                  /* RCC                           */
  .word  EXTI0_IRQHandler                /* EXTI Line0                    */
  .word  EXTI1_IRQHandler                /* EXTI Line1                    */
  .word  EXTI2_IRQHandler                /* EXTI Line2                    */
  .word  EXTI3_IRQHandler                /* EXTI Line3                    */
  .word  EXTI4_IRQHandler                /* EXTI Line4                    */
  .word  DMA1_Stream0_IRQHandler         /* DMA1 Stream 0                 */
  .word  DMA1_Stream1_IRQHandler         /* DMA1 Stream 1                 */
  .word  DMA1_Stream2_IRQHandler         /* DMA1 Stream 2                 */
  .word  DMA1_Stream3_IRQHandler         /* DMA1 Stream 3                 */
  .word  DMA1_Stream4_IRQHandler         /* DMA1 Stream 4                 */
  .word  DMA1_Stream5_IRQHandler         /* DMA1 Stream 5                 */
  .word  DMA1_Stream6_IRQHandler         /* DMA1 Stream 6                 */
  .word  ADC_IRQHandler                  /* ADC1, ADC2 and ADC3s          */
  .word  CAN1_TX_IRQHandler              /* CAN1 TX                       */
  .word  CAN1_RX0_IRQHandler             /* CAN1 RX0                      */
  .word  CAN1_RX1_IRQHandler             /* CAN1 RX1                      */
  .word  CAN1_SCE_IRQHandler             /* CAN1 SCE                      */
  .word  EXTI9_5_IRQHandler              /* External Line[9:5]s           */
  .word  TIM1_BRK_TIM9_IRQHandler        /* TIM1 Break and TIM9           */
  .word  TIM1_UP_TIM10_IRQHandler        /* TIM1 Update and TIM10         */
  .word  TIM1_TRG_COM_TIM11_IRQHandler   /* TIM1 Trigger/Commutation/TIM11*/
  .word  TIM1_CC_IRQHandler              /* TIM1 Capture Compare          */
  .word  TIM2_IRQHandler                 /* TIM2                          */
  .word  TIM3_IRQHandler                 /* TIM3                          */
  .word  TIM4_IRQHandler                 /* TIM4                          */
  .word  I2C1_EV_IRQHandler              /* I2C1 Event                    */
  .word  I2C1_ER_IRQHandler              /* I2C1 Error                    */
  .word  I2C2_EV_IRQHandler              /* I2C2 Event                    */
  .word  I2C2_ER_IRQHandler              /* I2C2 Error                    */
  .word  SPI1_IRQHandler                 /* SPI1                          */
  .word  SPI2_IRQHandler                 /* SPI2                          */
  .word  USART1_IRQHandler               /* USART1                        */
  .word  USART2_IRQHandler               /* USART2                        */
  .word  USART3_IRQHandler               /* USART3                        */
  .word  EXTI15_10_IRQHandler            /* External Line[15:10]s         */
  .word  RTC_Alarm_IRQHandler            /* RTC Alarm (A and B)           */
  .word  OTG_FS_WKUP_IRQHandler          /* USB OTG FS Wakeup             */
  .word  TIM8_BRK_TIM12_IRQHandler       /* TIM8 Break and TIM12          */
  .word  TIM8_UP_TIM13_IRQHandler        /* TIM8 Update and TIM13         */
  .word  TIM8_TRG_COM_TIM14_IRQHandler   /* TIM8 Trigger/Commutation/TIM14*/
  .word  TIM8_CC_IRQHandler              /* TIM8 Capture Compare          */
  .word  DMA1_Stream7_IRQHandler         /* DMA1 Stream7                  */
  .word  FMC_IRQHandler                  /* FMC                           */
  .word  SDIO_IRQHandler                 /* SDIO                          */
  .word  TIM5_IRQHandler                 /* TIM5                          */
  .word  SPI3_IRQHandler                 /* SPI3                          */
  .word  UART4_IRQHandler                /* UART4                         */
  .word  UART5_IRQHandler                /* UART5                         */
  .word  TIM6_DAC_IRQHandler             /* TIM6 and DAC1&2 underrun      */
  .word  TIM7_IRQHandler                 /* TIM7                          */
  .word  DMA2_Stream0_IRQHandler         /* DMA2 Stream 0                 */
  .word  DMA2_Stream1_IRQHandler         /* DMA2 Stream 1                 */
  .word  DMA2_Stream2_IRQHandler         /* DMA2 Stream 2                 */
  .word  DMA2_Stream3_IRQHandler         /* DMA2 Stream 3                 */
  .word  DMA2_Stream4_IRQHandler         /* DMA2 Stream 4                 */
  .word  ETH_IRQHandler                  /* Ethernet                      */
  .word  ETH_WKUP_IRQHandler             /* Ethernet Wakeup through EXTI  */
  .word  CAN2_TX_IRQHandler              /* CAN2 TX                       */
  .word  CAN2_RX0_IRQHandler             /* CAN2 RX0                      */
  .word  CAN2_RX1_IRQHandler             /* CAN2 RX1                      */
  .word  CAN2_SCE_IRQHandler             /* CAN2 SCE                      */
  .word  OTG_FS_IRQHandler               /* USB OTG FS                    */
  .word  DMA2_Stream5_IRQHandler         /* DMA2 Stream 5                 */
  .word  DMA2_Stream6_IRQHandler         /* DMA2 Stream 6                 */
  .word  DMA2_Stream7_IRQHandler         /* DMA2 Stream 7                 */
  .word  USART6_IRQHandler               /* USART6                        */
  .word  I2C3_EV_IRQHandler              /* I2C3 event                    */
  .word  I2C3_ER_IRQHandler              /* I2C3 error                    */
  .word  OTG_HS_EP1_OUT_IRQHandler       /* USB OTG HS End Point 1 Out    */
  .word  OTG_HS_EP1_IN_IRQHandler        /* USB OTG HS End Point 1 In     */
  .word  OTG_HS_WKUP_IRQHandler          /* USB OTG HS Wakeup             */
  .word  OTG_HS_IRQHandler               /* USB OTG HS                    */
  .word  DCMI_IRQHandler                 /* DCMI                          */
  .word  0                               /* Reserved                      */
  .word  HASH_RNG_IRQHandler             /* Hash and Rng                  */
  .word  FPU_IRQHandler                  /* FPU                           */

__Vectors_End:
  .equ  __Vectors_Size, __Vectors_End - __Vectors

/*******************************************************************************
 * Reset Handler
 ******************************************************************************/

  .section  .text.Reset_Handler, "ax", %progbits
  .weak     Reset_Handler
  .type     Reset_Handler, %function
Reset_Handler:
  ldr   r0, =SystemInit
  blx   r0
  ldr   r0, =__libc_init_array
  blx   r0
  ldr   r0, =main
  bx    r0
  .size Reset_Handler, . - Reset_Handler

/*******************************************************************************
 * Default Exception / Interrupt Handlers
 ******************************************************************************/

  .section  .text.Default_Handler, "ax", %progbits
  .type     Default_Handler, %function
Default_Handler:
1:
  b    1b
  .size Default_Handler, . - Default_Handler

  .macro  def_irq_handler  handler_name
  .weak   \handler_name
  .thumb_set \handler_name, Default_Handler
  .endm

  /* System exception handlers */
  def_irq_handler  NMI_Handler
  def_irq_handler  HardFault_Handler
  def_irq_handler  MemManage_Handler
  def_irq_handler  BusFault_Handler
  def_irq_handler  UsageFault_Handler
  def_irq_handler  SVC_Handler
  def_irq_handler  DebugMon_Handler
  def_irq_handler  PendSV_Handler
  def_irq_handler  SysTick_Handler

  /* Peripheral interrupt handlers */
  def_irq_handler  WWDG_IRQHandler
  def_irq_handler  PVD_IRQHandler
  def_irq_handler  TAMP_STAMP_IRQHandler
  def_irq_handler  RTC_WKUP_IRQHandler
  def_irq_handler  FLASH_IRQHandler
  def_irq_handler  RCC_IRQHandler
  def_irq_handler  EXTI0_IRQHandler
  def_irq_handler  EXTI1_IRQHandler
  def_irq_handler  EXTI2_IRQHandler
  def_irq_handler  EXTI3_IRQHandler
  def_irq_handler  EXTI4_IRQHandler
  def_irq_handler  DMA1_Stream0_IRQHandler
  def_irq_handler  DMA1_Stream1_IRQHandler
  def_irq_handler  DMA1_Stream2_IRQHandler
  def_irq_handler  DMA1_Stream3_IRQHandler
  def_irq_handler  DMA1_Stream4_IRQHandler
  def_irq_handler  DMA1_Stream5_IRQHandler
  def_irq_handler  DMA1_Stream6_IRQHandler
  def_irq_handler  ADC_IRQHandler
  def_irq_handler  CAN1_TX_IRQHandler
  def_irq_handler  CAN1_RX0_IRQHandler
  def_irq_handler  CAN1_RX1_IRQHandler
  def_irq_handler  CAN1_SCE_IRQHandler
  def_irq_handler  EXTI9_5_IRQHandler
  def_irq_handler  TIM1_BRK_TIM9_IRQHandler
  def_irq_handler  TIM1_UP_TIM10_IRQHandler
  def_irq_handler  TIM1_TRG_COM_TIM11_IRQHandler
  def_irq_handler  TIM1_CC_IRQHandler
  def_irq_handler  TIM2_IRQHandler
  def_irq_handler  TIM3_IRQHandler
  def_irq_handler  TIM4_IRQHandler
  def_irq_handler  I2C1_EV_IRQHandler
  def_irq_handler  I2C1_ER_IRQHandler
  def_irq_handler  I2C2_EV_IRQHandler
  def_irq_handler  I2C2_ER_IRQHandler
  def_irq_handler  SPI1_IRQHandler
  def_irq_handler  SPI2_IRQHandler
  def_irq_handler  USART1_IRQHandler
  def_irq_handler  USART2_IRQHandler
  def_irq_handler  USART3_IRQHandler
  def_irq_handler  EXTI15_10_IRQHandler
  def_irq_handler  RTC_Alarm_IRQHandler
  def_irq_handler  OTG_FS_WKUP_IRQHandler
  def_irq_handler  TIM8_BRK_TIM12_IRQHandler
  def_irq_handler  TIM8_UP_TIM13_IRQHandler
  def_irq_handler  TIM8_TRG_COM_TIM14_IRQHandler
  def_irq_handler  TIM8_CC_IRQHandler
  def_irq_handler  DMA1_Stream7_IRQHandler
  def_irq_handler  FMC_IRQHandler
  def_irq_handler  SDIO_IRQHandler
  def_irq_handler  TIM5_IRQHandler
  def_irq_handler  SPI3_IRQHandler
  def_irq_handler  UART4_IRQHandler
  def_irq_handler  UART5_IRQHandler
  def_irq_handler  TIM6_DAC_IRQHandler
  def_irq_handler  TIM7_IRQHandler
  def_irq_handler  DMA2_Stream0_IRQHandler
  def_irq_handler  DMA2_Stream1_IRQHandler
  def_irq_handler  DMA2_Stream2_IRQHandler
  def_irq_handler  DMA2_Stream3_IRQHandler
  def_irq_handler  DMA2_Stream4_IRQHandler
  def_irq_handler  ETH_IRQHandler
  def_irq_handler  ETH_WKUP_IRQHandler
  def_irq_handler  CAN2_TX_IRQHandler
  def_irq_handler  CAN2_RX0_IRQHandler
  def_irq_handler  CAN2_RX1_IRQHandler
  def_irq_handler  CAN2_SCE_IRQHandler
  def_irq_handler  OTG_FS_IRQHandler
  def_irq_handler  DMA2_Stream5_IRQHandler
  def_irq_handler  DMA2_Stream6_IRQHandler
  def_irq_handler  DMA2_Stream7_IRQHandler
  def_irq_handler  USART6_IRQHandler
  def_irq_handler  I2C3_EV_IRQHandler
  def_irq_handler  I2C3_ER_IRQHandler
  def_irq_handler  OTG_HS_EP1_OUT_IRQHandler
  def_irq_handler  OTG_HS_EP1_IN_IRQHandler
  def_irq_handler  OTG_HS_WKUP_IRQHandler
  def_irq_handler  OTG_HS_IRQHandler
  def_irq_handler  DCMI_IRQHandler
  def_irq_handler  HASH_RNG_IRQHandler
  def_irq_handler  FPU_IRQHandler

  .end
