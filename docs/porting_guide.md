# Porting Guide

This guide explains how to port this framework to a new MCU platform.

## Architecture Overview

```
app/            Application logic — platform-independent
drivers/        Device drivers — use interface/ abstractions
interface/      Abstract interfaces — pure C structs, no MCU headers
port/<mcu>/     Platform port — MCU-specific BSP implementation
third_party/    Unmodified third-party code (RT-Thread, lwIP, CMSIS, HAL)
```

## Design Rule

The `interface/` layer defines abstract device operations via structs of function pointers (e.g., `Gpio_Device_T`, `Uart_Device_T`).
The `port/<mcu>/bsp/` layer implements these interfaces for a specific MCU.
Upper layers never call HAL or MCU registers directly.

## Porting Steps

### 1. Create Port Directory

```
port/<your-mcu>/
├── bsp/                  # BSP implementations
│   ├── bsp_gpio.c/h      # GPIO HAL init
│   ├── bsp_uart.c/h      # UART HAL init
│   ├── bsp_i2c.c/h       # I2C HAL init
│   ├── bsp_spi.c/h       # SPI HAL init
│   ├── bsp_can.c/h       # CAN HAL init
│   ├── bsp_eth.c/h       # Ethernet HAL init
│   └── bsp_flash.c/h     # Internal flash HAL
├── system/               # System files
│   ├── main.c            # Entry point (calls application_init())
│   ├── main.h            # Main header (includes HAL)
│   ├── gpio.c/h          # GPIO IRQ handlers
│   ├── stm32f4xx_hal_conf.h  # HAL configuration
│   ├── stm32f4xx_hal_msp.c   # HAL MSP callbacks
│   ├── stm32f4xx_hal_timebase_tim.c  # HAL timebase (TIM6 — independent from RTOS tick)
│   ├── stm32f4xx_it.c/h  # Interrupt handlers
│   ├── syscalls.c        # Retargeted syscalls (printf, etc.)
│   └── system_stm32f4xx.c  # System init (clock config)
├── startup/              # Startup assembly (toolchain-specific subdirectories)
│   ├── startup_*.s       # Common startup (Keil / IAR)
│   └── gcc/              # GCC-specific startup
│       └── startup_*.s
└── <CHIP>_FLASH.ld       # Linker script
```

Additionally, create `port/rtthread/` with RT-Thread board initialization:

```
port/rtthread/
├── board.c               # RT-Thread board init (rt_hw_board_init)
├── board.h               # Board header (heap region, clock defines)
└── finsh_port.c          # Finsh shell console port (UART bridge)
```

### 2. Implement BSP Functions

Each BSP file must provide init functions that configure the MCU peripherals.
See `port/stm32f407/bsp/bsp_gpio.c` for an example.

Key pattern:
```c
// bsp_gpio.c - platform-specific GPIO initialization
#include "main.h"  // your MCU HAL

void bsp_gpiob_pin0_init(void)
{
    // Enable clocks, configure pins with your HAL
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}
```

### 3. Implement System Files

- `main.c`: Call `HAL_Init()`, clock config, then `application_init()`
- `stm32f4xx_hal_msp.c`: HAL MSP callbacks
- `stm32f4xx_it.c`: Interrupt service routines (SysTick → RT-Thread tick, other IRQs)
- `system_stm32f4xx.c`: System clock configuration (`SystemClock_Config()`)

### 4. Implement RT-Thread Board Init

Create `port/rtthread/board.c` with these essential functions:

```c
#include <rthw.h>
#include <rtthread.h>
#include "board.h"

// SysTick ISR — feeds RT-Thread tick counter
void SysTick_Handler(void)
{
    rt_interrupt_enter();
    rt_tick_increase();
    rt_interrupt_leave();
}

// Hardware init — called by RT-Thread startup
void rt_hw_board_init(void)
{
    HAL_Init();
    SystemClock_Config();
    SysTick_Config(SystemCoreClock / RT_TICK_PER_SECOND);

    // RT-Thread heap (use available SRAM region)
    rt_system_heap_init(HEAP_BEGIN, HEAP_END);

    // Auto-init components registered via INIT_BOARD_EXPORT
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif

    // Console device for Finsh shell
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
}
```

Define `HEAP_BEGIN` and `HEAP_END` in `port/rtthread/board.h`:

```c
#define STM32_SRAM_SIZE         (192 * 1024)
#define STM32_SRAM_END          (0x20000000 + STM32_SRAM_SIZE)
#define HEAP_BEGIN              ((void *)(0x20000000 + 128 * 1024))
#define HEAP_END                ((void *)STM32_SRAM_END)
```

### 5. Configure Build System (SCons)

The build system uses **SCons** with RT-Thread's `building.py` framework.
Create a `port/SConscript`:

```python
import os
from building import *

cwd = GetCurrentDir()
objs = []

# System files
system_src = Glob(os.path.join(cwd, '<your-mcu>', 'system', '*.c'))
# BSP peripheral drivers
bsp_src = Glob(os.path.join(cwd, '<your-mcu>', 'bsp', '*.c'))
# RT-Thread board init
rtthread_src = Glob(os.path.join(cwd, 'rtthread', '*.c'))
# lwIP port adaptation
lwip_port_src = Glob(os.path.join(cwd, 'lwip', 'arch', '*.c'))
lwip_port_src += Glob(os.path.join(cwd, 'lwip', 'common', '*.c'))
# Startup assembly
startup_asm = Glob(os.path.join(cwd, '<your-mcu>', 'startup', 'gcc', '*.s'))

all_src = system_src + bsp_src + rtthread_src + lwip_port_src + startup_asm
group = DefineGroup('Port', all_src, depend=[''])
objs.append(group)
Return('objs')
```

**Toolchain configuration** — update `rtconfig.py`:

```python
ARCH = 'arm'
CPU  = 'cortex-m4'    # or your CPU family
CROSS_TOOL = 'gcc'
PREFIX = 'arm-none-eabi-'

# MCU flags
DEVICE = ' -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard'
DEVICE += ' -ffunction-sections -fdata-sections'

CFLAGS  = DEVICE + ' -Wall -Wextra'
LFLAGS  = DEVICE + ' -Wl,--gc-sections,-Map=rtthread.map,-cref,-u,Reset_Handler'
LFLAGS += ' -T port/<your-mcu>/<CHIP>_FLASH.ld'

# Include paths
CPPPATH = [
    'port/<your-mcu>/system',
    'port/<your-mcu>/bsp',
    'port/rtthread',
    'port/lwip/arch',
    'port/lwip/common',
    'third_party/rt-thread/include',
    'third_party/rt-thread/libcpu/arm/cortex-m4',
    'third_party/CMSIS/Include',
    'third_party/CMSIS/Device/ST/<YOUR_FAMILY>/Include',
    'third_party/<YOUR_HAL>/Inc',
    'interface',
    # ... all interface/ subdirectories
    'drivers',
    # ... all drivers/ subdirectories
    'app',
    'app/app_core/Inc',
    'app/share/Inc',
]
```

### 6. Configure RT-Thread

RT-Thread uses Kconfig for kernel configuration. The root `Kconfig` sources
RT-Thread's own Kconfig tree and adds project-specific options.

**Key configuration files:**

| File | Purpose |
|---|---|
| `Kconfig` | Kconfig menu tree (used by `scons --menuconfig`) |
| `rtconfig.h` | Auto-generated by menuconfig — do NOT edit manually |
| `rtconfig.py` | Toolchain flags and include paths (you edit this) |
| `rtconfig_project.h` | Project overrides that survive menuconfig regeneration |

**When porting to a new MCU, adjust these in `rtconfig.h` (via menuconfig):**

- `RT_NAME_MAX` — max thread/object name length (default: 12)
- `RT_CPUS_NR` — number of CPU cores (1 for single-core, set >1 for SMP)
- `RT_TICK_PER_SECOND` — system tick frequency (default: 1000)
- `RT_THREAD_PRIORITY_MAX` — number of priority levels (default: 32)
- `IDLE_THREAD_STACK_SIZE` — idle thread stack (default: 256)
- `RT_MAIN_THREAD_STACK_SIZE` — main thread stack (default: 1024)

**Finsh shell** — implement `rt_hw_console_output()` and `rt_hw_console_getchar()`
in `port/rtthread/finsh_port.c` to bridge Finsh to your UART device:

```c
void rt_hw_console_output(const char *str)
{
    // Send string to your debug UART
    uart_send(str, rt_strlen(str));
}

char rt_hw_console_getchar(void)
{
    char ch;
    // Blocking read one character from debug UART
    while (uart_recv(&ch, 1) == 0) { }
    return ch;
}
```

### 7. Configure lwIP (Network Stack)

The lwIP adaptation layer lives in `port/lwip/arch/`:

| File | Purpose |
|---|---|
| `lwipopts.h` | lwIP compile-time options (memory, protocols, debugging) |
| `cc.h` | Compiler/platform types and macros |
| `sys_arch.c/h` | RTOS adaptation (mutex, semaphore, mailbox, thread) |
| `ethernetif.c/h` | Ethernet interface driver (MAC ↔ lwIP glue) |

When porting to a new MCU with a different Ethernet MAC:
1. Update `ethernetif.c` to use the new MAC HAL
2. Verify `sys_arch.c` is compatible with RT-Thread (it uses RT-Thread semaphore/mutex/mailbox APIs)
3. Review `lwipopts.h` memory settings against your MCU's RAM

### 8. Implement Interface Functions

The `*_dev.c` files in `interface/` define abstract device operations via structs of function pointers. During initialization, your BSP functions are registered into these structs, wiring the abstract interface to your hardware.

For each interface module your application uses:

1. Read the `*_dev.h` header to understand the expected function signatures
2. Implement the corresponding BSP functions in `port/<your-mcu>/bsp/`
3. Call the registration/init function (e.g., `gpio_dev_init()`) from `main.c` or `application_init()`

Example — GPIO registration flow:
1. `interface/gpio/gpio_dev.h` declares `Gpio_Device_T` with function pointer fields
2. `port/stm32f407/bsp/bsp_gpio.c` implements the hardware-specific init functions
3. `port/stm32f407/system/gpio.c` wires BSP functions into `Gpio_Device_T` structs
4. `port/stm32f407/system/main.c` calls `gpio_dev_init()` at startup

## Reference Platform

See `port/stm32f407/` for a complete working example using:
- STM32F407ZGTx MCU (Cortex-M4F, 168 MHz, 1 MB Flash, 192 KB SRAM)
- STM32Cube F4 HAL
- RT-Thread RTOS
- lwIP TCP/IP stack

## Adding New Hardware Drivers

1. Create `drivers/<device>/` directory
2. Define device abstraction (`*_dev.h`) — reference `interface/gpio/gpio_dev.h` for the pattern
3. Implement driver (`*_dev.c`) — call hardware exclusively through `interface/` modules
4. Register in `app/` initialization

## Design Checklist

- [ ] `interface/` files contain NO `#include "main.h"` or any HAL header
- [ ] `drivers/` files use only `interface/` abstractions for hardware access
- [ ] `app/` files are platform-independent
- [ ] All MCU-specific code is under `port/<mcu>/`
- [ ] BSP functions are the ONLY place touching HAL/registers
- [ ] `port/rtthread/board.c` provides `rt_hw_board_init()` and `SysTick_Handler()`
- [ ] `port/rtthread/finsh_port.c` bridges UART to Finsh console
- [ ] `rtconfig.py` has correct MCU flags and include paths for your chip
- [ ] `Kconfig` sources `$RTT_DIR/Kconfig` for full RT-Thread configuration tree
