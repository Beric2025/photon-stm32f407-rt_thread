/*******************************************************************************
 * Linker script for STM32F407ZGTx
 * Flash: 1024 KB, SRAM: 128 KB, CCMRAM: 64 KB
 ******************************************************************************/

MEMORY
{
  FLASH    (rx)  : ORIGIN = 0x08000000, LENGTH = 1024K
  SRAM     (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
  CCMRAM   (rw)  : ORIGIN = 0x10000000, LENGTH = 64K
}

/* Highest address of the user stack (grows down from end of SRAM) */
_estack     = ORIGIN(SRAM) + LENGTH(SRAM);    /* end of SRAM */
_min_stack  = _estack - __stack_size__;       /* bottom of stack */

/* Heap starts after .bss in SRAM, grows upward toward stack */
_heap_start = _end;
_heap_end   = _min_stack;

ENTRY(Reset_Handler)

SECTIONS
{
  /*****************************************************************************
   * Vector table — must be at beginning of FLASH
   *****************************************************************************/
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } >FLASH

  /*****************************************************************************
   * Program code and constants
   *****************************************************************************/
  .text :
  {
    . = ALIGN(4);
    *(.text)                 /* .text sections (code) */
    *(.text*)                /* .text* sections (code) */
    *(.glue_7)               /* glue arm to thumb code */
    *(.glue_7t)              /* glue thumb to arm code */
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;              /* global symbol at end of code */
  } >FLASH

  /*****************************************************************************
   * Constant data
   *****************************************************************************/
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)               /* .rodata sections (constants) */
    *(.rodata*)              /* .rodata* sections (constants) */
    . = ALIGN(4);
  } >FLASH

  /*****************************************************************************
   * Pre-init / init / fini arrays
   *****************************************************************************/
  .preinit_array :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >FLASH

  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >FLASH

  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >FLASH

  /*****************************************************************************
   * Initialised data — stored in FLASH, copied to SRAM at startup
   *****************************************************************************/
  _sidata = LOADADDR(.data);

  .data :
  {
    . = ALIGN(4);
    _sdata = .;              /* start of data section in SRAM */
    *(.data)
    *(.data*)
    . = ALIGN(4);
    _edata = .;              /* end of data section in SRAM */
  } >SRAM AT> FLASH

  /*****************************************************************************
   * Uninitialised data (BSS)
   *****************************************************************************/
  .bss :
  {
    . = ALIGN(4);
    _sbss = .;               /* start of bss section */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;               /* end of bss section */
    __bss_end__ = _ebss;
  } >SRAM

  /*****************************************************************************
   * Heap and stack — defined here for size tracking, allocated in SRAM
   *****************************************************************************/
  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + __heap_size__;
    . = . + __stack_size__;
    . = ALIGN(8);
  } >SRAM

  /*****************************************************************************
   * CCMRAM section — user must explicitly place data here with
   * __attribute__((section(".ccmram")))
   *****************************************************************************/
  .ccmram (NOLOAD) :
  {
    . = ALIGN(4);
    *(.ccmram)
    *(.ccmram*)
    . = ALIGN(4);
  } >CCMRAM

  /*****************************************************************************
   * Remove standard library info sections to keep binary size down
   *****************************************************************************/
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}

/* Exported symbols expected by startup / newlib */
__stack_size__  = DEFINED(__stack_size__)  ? __stack_size__  : 0x1600;
__heap_size__   = DEFINED(__heap_size__)   ? __heap_size__   : 0x800;
