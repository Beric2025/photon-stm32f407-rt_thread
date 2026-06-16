# Photon — STM32 Embedded Framework

A layered, portable embedded firmware framework for the **STM32F407** MCU,
designed for clean separation between MCU-specific code and hardware-independent
device drivers. Internally codenamed **Photon**.

Based on **RT-Thread** for multitasking, **lwIP** for Ethernet networking, and
**STM32Cube HAL** for peripheral access — all abstracted behind a
Linux-kernel-style interface layer so application and driver code never touches
vendor HAL directly.

## Features

- **RT-Thread** RTOS with thread-safe peripheral access
- **lwIP** Ethernet networking (TCP/IP, custom protocol)
- **Finsh shell** — interactive command-line debug console via UART1
- **DFS** — device virtual file system (devfs)
- **Kconfig + menuconfig** — graphical kernel and component configuration
- **Motor control** — hub motor with speed/position/encoder/hall/fault
- **Sensing** — ultrasonic distance, ADC battery monitoring
- **Peripherals** — GPIO, UART, I²C, SPI, CAN, Ethernet, ADC, DAC, PWM, RNG, RTC, IWDG
- **IAP** — In-Application Programming for firmware self-update
- **Build-time version injection** via git describe

## Architecture

```
┌─────────────────────────┐
│  app/                   │  Application logic — no hardware dependency
├─────────────────────────┤
│  drivers/               │  Device drivers — HW access via interface/
├─────────────────────────┤
│  interface/             │  Abstract interfaces — pure C structs, zero MCU code
├─────────────────────────┤
│  port/                  │  Platform port — MCU-specific BSP implementation
│  ├─ stm32f407/          │
│  ├─ rtthread/           │
│  └─ lwip/               │
└─────────────────────────┘
```

## Design Rules

1. **interface/** defines only abstract types — no HAL, no RTOS, no MCU headers.
2. **drivers/** communicates with hardware exclusively through `interface/` structs.
3. **port/** implements the `interface/` contracts for a specific MCU.
4. **app/** orchestrates drivers; no direct hardware access.

Example — `interface/gpio/gpio_dev.h` defines `Gpio_Device_T` (a struct of function
pointers: `init`, `write_pin`, `read_pin`, `toggle_pin`, `ioctl`, …).
`port/stm32f407/bsp/bsp_gpio.c` implements the HAL calls. `drivers/` and `app/` only
see the struct — they never `#include "main.h"` or touch an MCU register.

## Hardware Requirements

| Component | Detail |
|---|---|
| **MCU** | STM32F407ZGTx (Cortex-M4F, 168 MHz, 1 MB Flash, 192 KB SRAM) |
| **Debug Probe** | ST-Link/V2 (or compatible) for flashing & debugging |
| **Ethernet PHY** | LAN8720 (RMII interface) |
| **Motor** | Hub motor with hall sensor feedback |
| **Sensors** | DYPA21 ultrasonic, ADC battery monitoring |
| **Expander** | PCF8574 I²C GPIO expander |

## Build

### Prerequisites

- `arm-none-eabi-gcc` (ARM GNU Toolchain, e.g. 15.2.rel1)
- `scons` — RT-Thread's build tool (`pip install scons`)
- `git` — for submodules and version injection

> **Note for Windows users**: Install `scons` via `pip install scons` in your
> Python environment. Make sure `arm-none-eabi-gcc` is on your `PATH`.

### Clone (with submodules)

```bash
git clone --recurse-submodules <repo-url>
# Or if already cloned:
git submodule update --init --recursive
```

### Quick Start

```bash
scons                    # Build the project
scons -j8                # Parallel build with 8 jobs
scons -c                 # Clean build outputs
```

### Configuration

The project has three layers of configuration:

#### 1. Kernel Configuration (`scons --menuconfig`)

RT-Thread uses a **Kconfig** system. Run `scons --menuconfig` to open the
graphical menu where you can enable/disable:

- Kernel features (IPC, memory management, hook functions)
- Finsh shell (command-line debug console)
- DFS (device virtual filesystem)
- Device drivers (serial, pin, etc.)
- C/C++ and POSIX layer options

Configuration is saved to `rtconfig.h` (auto-generated — do not edit by hand).
Project-specific overrides that survive `menuconfig` regeneration go in
`rtconfig_project.h`.

#### 2. Build Type (debug / release)

Set via the `BUILD` variable in `rtconfig.py`, or override at the command line:

```bash
# Debug build (default, -Og -g)
scons

# Release build (-Os)
scons BUILD=release
```

| Build Type | Optimization | Debug Symbols | Purpose |
|---|---|---|---|
| `debug` | `-Og` | Yes (`-g`) | Development: assertions, verbose logging |
| `release` | `-Os` | No | Production: size-optimized |

#### 3. Toolchain Configuration (`rtconfig.py`)

The toolchain and compiler flags are defined in `rtconfig.py`. Key settings:

| Variable | Description |
|---|---|
| `EXEC_PATH` | Path to toolchain binaries (or set `RTT_EXEC_PATH` env var) |
| `PREFIX` | Toolchain prefix (default: `arm-none-eabi-`) |
| `BUILD` | `debug` (`-Og -g`) or `release` (`-Os`) |
| `DEVICE` | MCU flags (`-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard`) |

### Version Injection

The build system automatically injects the git version as a compile-time macro:

| Git state | `APP_VERSION` value |
|---|---|
| On tag `v1.0.0` | `v1.0.0` |
| 3 commits after tag | `v1.0.0-3-gabc1234` |
| Uncommitted changes | `v1.0.0-3-gabc1234-dirty` |

**Using in code:**

```c
#include "app_version.h"

char ver[64];
build_version();
get_version(ver, sizeof(ver));
rt_kprintf("Firmware: %s\n", ver);
```

### Build Outputs

RT-Thread places final artifacts in the **project root** and intermediate object
files under `build/` — this is the standard convention for all RT-Thread BSPs.

| Location | File | Description |
|---|---|---|
| `./` (root) | `rtthread.elf` | ELF executable (debuggable) |
| `./` (root) | `rtthread.bin` | Raw binary for flashing |
| `./` (root) | `rtthread.hex` | Intel HEX for flashing |
| `./` (root) | `rtthread.map` | Linker map (memory layout, symbol addresses) |
| `build/kernel/` | `*.o` | Intermediate object files (safe to delete with `scons -c`) |

### Flashing

```bash
# Flash via OpenOCD (ST-Link + STM32F4x)
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
    -c "program rtthread.elf verify reset exit"
```

### IDE / VS Code

Open the workspace file ([Photon.code-workspace](Photon.code-workspace)) and use
the VS Code terminal to run `scons`. You can also export VS Code configuration:

```bash
scons --target=vsc         # Export VS Code config (includes include paths)
```

### Keil MDK

The Keil project file is kept for Windows debugging:
[keil/Photon.uvprojx](keil/Photon.uvprojx)

## Porting to a New MCU

1. Create `port/<your-mcu>/bsp/` with BSP implementations matching all required
   `interface/` contracts.
2. Provide `port/<your-mcu>/system/` with startup code, clock configuration, and
   `main.c` (calls `application_init()`).
3. Place `port/<your-mcu>/startup/gcc/` with the GCC startup assembly.
4. Add `<CHIP>_FLASH.ld` linker script in your port root.
5. Add a `port/<your-mcu>/SConscript` for the build system.
6. Update `rtconfig.py` with your MCU's flags and include paths.

See [docs/porting_guide.md](docs/porting_guide.md) for detailed instructions.

## Directory Layout

| Directory | Purpose |
|---|---|
| `app/app_core/` | Application tasks: LED, battery, motor, light, ultrasonic, network, version |
| `app/share/` | Shared utilities: log_print (debug logging), match_mode |
| `drivers/` | External peripheral drivers (9 device types) |
| `interface/` | Hardware abstraction interfaces (16 peripheral types) |
| `port/stm32f407/` | STM32F407 BSP port: HAL init, startup, system config |
| `port/rtthread/` | RT-Thread board init (`board.c`), Finsh console port (`finsh_port.c`) |
| `port/lwip/` | lwIP arch adaptation (`sys_arch.c`, `ethernetif.c`, `lwipopts.h`) |
| `third_party/rt-thread/` | RT-Thread kernel + components (git submodule) |
| `third_party/lwIP/` | lwIP TCP/IP stack (git submodule) |
| `third_party/STM32F4xx_HAL_Driver/` | STM32Cube F4 HAL drivers |
| `third_party/CMSIS/` | CMSIS Core + Device headers |
| `tools/` | Auxiliary scripts |
| `keil/` | Keil MDK project files |
| `docs/` | Documentation (porting guide, logo) |

## RT-Thread Shell (Finsh)

Connect a serial terminal to UART1 (115200 8N1) to access the Finsh shell:

```
   \ | /
- RT -     Thread Operating System
 / | \     5.1.0 build Jun 15 2026 14:00:00
 2006 - 2024 Copyright by RT-Thread team
msh >
```

Built-in commands:
- `help` — list available commands
- `ps` / `list_thread` — list all threads and their status
- `free` — show heap memory usage
- `version` — show RT-Thread version

## License

MIT License — see [LICENSE](LICENSE) for details.
