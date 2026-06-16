/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * BSP Ethernet — STM32F407 RMII 100Mbit.
 *
 * This file owns ALL hardware-specific code:
 *   - MAC / DMA initialisation (HAL_ETH_Init)
 *   - PHY initialisation & auto-negotiation (eth_phy_*)
 *   - RX buffer pool bridging (→ eth_dev_rx_*)
 *   - Eth_Device_T vtable implementation
 *   - HAL MSP (GPIO, clocks, NVIC) & interrupt callbacks
 *
 * Dependencies (allowed):  stm32f4xx_hal, eth_dev, eth_phy, pcf8574, FreeRTOS
 * Dependencies (forbidden): none — this IS the lowest hardware layer.
 */

#include "main.h"                     /* → stm32f4xx_hal.h, HAL config           */
#include "bsp_eth.h"                  /* pin macros, extern declarations         */
#include "eth_dev.h"                  /* Eth_Device_T, buffer/frame API          */
#include "eth_phy.h"                  /* PHY register abstraction                */
#include "pcf8574.h"                  /* GPIO expander (PHY hardware reset)      */
#include "delay_dev.h"                /* delay_ms()                              */
#include "log_print.h"                /* LOG_PRINT()                             */

#include "stm32f4xx_hal_eth.h"        /* ETH_HandleTypeDef, HAL_ETH_*()          */
#include "stm32f4xx_hal_cortex.h"     /* HAL_MPU_ConfigRegion()                  */

#include "lwip/sys.h"                 /* sys_sem_t, sys_sem_new/free/signal_isr  */

#include <string.h>

#define TAG "eth bsp: "

/* ---- Compile-time optional MPU guard ---- */
#if defined(ETH_MPU_CONFIG) && (ETH_MPU_CONFIG == 1)
# include "stm32f4xx_hal_cortex.h"
#endif

/* ====================================================================
 * DMA descriptors — must reside in DMA-accessible SRAM (not CCM)
 * ==================================================================== */

ETH_DMADescTypeDef  g_ethDmaRxDscrTab[ETH_RX_DESC_CNT]
    __attribute__((section(".eth_dma_rx_desc"), aligned(4)));

ETH_DMADescTypeDef  g_ethDmaTxDscrTab[ETH_TX_DESC_CNT]
    __attribute__((section(".eth_dma_tx_desc"), aligned(4)));

ETH_HandleTypeDef   g_eth_handler;

/* ====================================================================
 * BSP private data structure (one per controller instance)
 * ==================================================================== */

#define ETH_DMA_TRANSMIT_TIMEOUT      20U

typedef struct {
    ETH_HandleTypeDef     *eth;               /* HAL handle                  */
    unsigned char        (*bsp_init)(uint8_t *mac_addr);  /* self init func  */
    uint8_t               mac_addr[6];        /* MAC address                 */

    /* PHY */
    Eth_Chip_Object_T     eth_chip;           /* PHY device object           */

    /* TX configuration (persisted across sends) */
    ETH_TxPacketConfig    tx_config;

    /* RTOS synchronisation */
    sys_sem_t             rx_sem;             /* signaled on RX frame ready  */

    /* User callbacks (registered via Eth_Device_T vtable) */
    eth_rx_callback_t     rx_cb;
    void                 *rx_cb_user_data;
    eth_link_callback_t   link_cb;
    void                 *link_cb_user_data;
} Eth_Data_T;

/* ---- Singleton instance ---- */
static Eth_Data_T s_eth1_data = {
    .eth      = &g_eth_handler,
    .bsp_init = bsp_eth_init,
};

/* ====================================================================
 * Forward declarations — vtable methods
 * ==================================================================== */

static int  eth1_init(void *privatedata, uint8_t *mac_addr);
static int  eth1_deinit(void *privatedata);
static int  eth1_send(void *privatedata, uint8_t *data, uint16_t length);
static int  eth1_ioctl(void *privatedata, uint32_t cmd, void *arg);
static int  eth1_wait_rx(void *privatedata, uint32_t timeout_ms);
static int  eth1_register_rx_callback(void *privatedata, eth_rx_callback_t cb, void *user_data);
static int  eth1_register_link_callback(void *privatedata, eth_link_callback_t cb, void *user_data);
static int  eth1_start(void *privatedata);
static int  eth1_stop(void *privatedata);
static int  eth1_get_link_status(void *privatedata);

/* ---- Singleton device instance ---- */
static Eth_Device_T s_eth1_dev = {
    .name                    = "eth1",
    .private_data            = &s_eth1_data,
    .init                    = eth1_init,
    .deinit                  = eth1_deinit,
    .send                    = eth1_send,
    .ioctl                   = eth1_ioctl,
    .wait_rx                 = eth1_wait_rx,
    .register_rx_callback    = eth1_register_rx_callback,
    .register_link_callback  = eth1_register_link_callback,
    .start                   = eth1_start,
    .stop                    = eth1_stop,
    .get_link_status         = eth1_get_link_status,
};

/* ====================================================================
 * PHY helpers (hardware-specific: pcf8574 GPIO expander)
 * ==================================================================== */

static void sys_intx_disable(void)  { __ASM volatile("cpsid i"); }
static void sys_intx_enable(void)   { __ASM volatile("cpsie i"); }

/**
 * Hardware-reset the PHY via a dedicated IO-expander pin, then wait
 * for the PHY to come back ready.  The reset polarity depends on the
 * board design — the pcf8574 driver handles both cases.
 */
static void eth_phy_reset(Eth_Data_T *ed)
{
    uint32_t regval;

    sys_intx_disable();

    /* Read PHY register 2 to determine current polarity state */
    HAL_ETH_ReadPHYRegister(ed->eth, ETH_CHIP_ADDR, 2, &regval);

#ifdef ETH_PHY_RESET_VIA_GPIO
    /* Direct MCU GPIO: PD3 */
    if ((regval & 0xFFF) == 0xFFF) {
        HAL_GPIO_WritePin(ETH_PHY_RST_PORT, ETH_PHY_RST_PIN, GPIO_PIN_SET);
        delay_ms(100);
        HAL_GPIO_WritePin(ETH_PHY_RST_PORT, ETH_PHY_RST_PIN, GPIO_PIN_RESET);
        delay_ms(100);
    } else {
        HAL_GPIO_WritePin(ETH_PHY_RST_PORT, ETH_PHY_RST_PIN, GPIO_PIN_RESET);
        delay_ms(100);
        HAL_GPIO_WritePin(ETH_PHY_RST_PORT, ETH_PHY_RST_PIN, GPIO_PIN_SET);
        delay_ms(100);
    }
#else
    /* PCF8574 I2C GPIO expander: pin P7 */
    if ((regval & 0xFFF) == 0xFFF) {
        pcf8574_write_bit(ETH_RESET_IO, 1);
        delay_ms(100);
        pcf8574_write_bit(ETH_RESET_IO, 0);
        delay_ms(100);
    } else {
        pcf8574_write_bit(ETH_RESET_IO, 0);
        delay_ms(100);
        pcf8574_write_bit(ETH_RESET_IO, 1);
        delay_ms(100);
    }
#endif

    sys_intx_enable();
}

/**
 * Read a raw PHY register through the HAL, bypassing the eth_phy
 * abstraction layer (used for quick link-status checks).
 */
static uint32_t eth_read_phy_reg(Eth_Data_T *ed, unsigned short reg_addr)
{
    uint32_t regval;
    HAL_ETH_ReadPHYRegister(ed->eth, ETH_CHIP_ADDR, reg_addr, &regval);
    return regval;
}

/* ====================================================================
 * PHY IO callbacks — bridge eth_phy abstraction ↔ HAL MDIO
 * ==================================================================== */

static int  eth_phy_io_init(void)    { return 0; }
static int  eth_phy_io_deinit(void)  { return 0; }

static int  eth_phy_io_read_reg(uint32_t dev_addr, uint32_t reg_addr,
                                uint32_t *p_reg_val)
{
    if (HAL_ETH_ReadPHYRegister(&g_eth_handler,
                                dev_addr, reg_addr, (uint32_t *)p_reg_val) != HAL_OK)
        return -1;
    return 0;
}

static int  eth_phy_io_write_reg(uint32_t dev_addr, uint32_t reg_addr,
                                 uint32_t reg_val)
{
    if (HAL_ETH_WritePHYRegister(&g_eth_handler,
                                 dev_addr, reg_addr, reg_val) != HAL_OK)
        return -1;
    return 0;
}

static int  eth_phy_io_get_tick(void)
{
    return (int)HAL_GetTick();
}

static Eth_Chip_Ioctl_T eth_chip_io_ctx = {
    .init     = eth_phy_io_init,
    .deinit   = eth_phy_io_deinit,
    .writereg = eth_phy_io_write_reg,
    .readreg  = eth_phy_io_read_reg,
    .gettick  = eth_phy_io_get_tick,
};

/* ====================================================================
 * MPU configuration (compiled-in only when ETH_MPU_CONFIG=1)
 * ==================================================================== */

static void bsp_eth_mpu_config(void)
{
#if defined(ETH_MPU_CONFIG) && (ETH_MPU_CONFIG == 1)

    MPU_Region_InitTypeDef MPU_InitStruct = {0};

    HAL_MPU_Disable();

    /* Region 0: DMA descriptor tables (Tx + Rx), 256 bytes */
    MPU_InitStruct.Enable            = MPU_REGION_ENABLE;
    MPU_InitStruct.Number            = MPU_REGION_NUMBER0;
    MPU_InitStruct.BaseAddress       = (uint32_t)g_ethDmaTxDscrTab;
    MPU_InitStruct.Size              = MPU_REGION_SIZE_256B;
    MPU_InitStruct.SubRegionDisable  = 0x00;
    MPU_InitStruct.TypeExtField      = MPU_TEX_LEVEL1;
    MPU_InitStruct.AccessPermission  = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.DisableExec       = MPU_INSTRUCTION_ACCESS_ENABLE;
    MPU_InitStruct.IsShareable       = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.IsCacheable       = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsBufferable      = MPU_ACCESS_NOT_BUFFERABLE;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    /* Region 1: RX buffer pool (.eth_rx_buf section) */
    extern uint8_t __eth_rx_buf_start;      /* linker-defined symbol */
    MPU_InitStruct.Number            = MPU_REGION_NUMBER1;
    MPU_InitStruct.BaseAddress       = (uint32_t)&__eth_rx_buf_start;
    MPU_InitStruct.Size              = MPU_REGION_SIZE_16KB;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

#endif /* ETH_MPU_CONFIG */
}

/* ====================================================================
 * BSP public API — called once at startup
 * ==================================================================== */

/**
 * Initialise ETH hardware: MPU, HAL_ETH_Init, register callbacks.
 * After this call the ETH peripheral is ready; PHY init follows
 * in eth1_init().
 */
unsigned char bsp_eth_init(uint8_t *mac_addr)
{
    if (mac_addr == NULL) {
        return 1;
    }

    bsp_eth_mpu_config();

#ifdef ETH_PHY_RESET_VIA_GPIO
    /* PD3: push-pull output, default high (PHY not in reset) */
    ETH_PHY_RST_CLK_EN();
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin   = ETH_PHY_RST_PIN;
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_WritePin(ETH_PHY_RST_PORT, ETH_PHY_RST_PIN, GPIO_PIN_SET);
    HAL_GPIO_Init(ETH_PHY_RST_PORT, &gpio);
#endif

    g_eth_handler.Instance         = ETH;
    g_eth_handler.gState           = HAL_ETH_STATE_RESET;

    ETH_InitTypeDef *init          = &g_eth_handler.Init;
    init->MACAddr                  = mac_addr;
    init->MediaInterface           = HAL_ETH_RMII_MODE;
    init->TxDesc                   = g_ethDmaTxDscrTab;
    init->RxDesc                   = g_ethDmaRxDscrTab;
    init->RxBuffLen                = ETH_MAX_PACKET_SIZE;

    if (HAL_ETH_Init(&g_eth_handler) != HAL_OK) {
        return 1;
    }

    /* Register custom RX buffer management */
    HAL_ETH_RegisterRxAllocateCallback(&g_eth_handler, eth_dev_rx_buf_alloc);
    HAL_ETH_RegisterRxLinkCallback(&g_eth_handler,    eth_dev_rx_buf_link);

    /* Publish the device — lwIP / app code can now call get_eth_device("eth1") */
    eth_dev_register(&s_eth1_dev);

    return 0;
}

/* ====================================================================
 * HAL MSP — GPIO, clocks, NVIC
 * ==================================================================== */

void HAL_ETH_MspInit(ETH_HandleTypeDef *heth)
{
    if (heth->Instance != ETH) { return; }

    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_ETH_CLK_ENABLE();

    ETH_CLK_GPIO_CLK_ENABLE();
    ETH_MDIO_GPIO_CLK_ENABLE();
    ETH_CRS_GPIO_CLK_ENABLE();
    ETH_MDC_GPIO_CLK_ENABLE();
    ETH_RXD0_GPIO_CLK_ENABLE();
    ETH_RXD1_GPIO_CLK_ENABLE();
    ETH_TX_EN_GPIO_CLK_ENABLE();
    ETH_TXD0_GPIO_CLK_ENABLE();
    ETH_TXD1_GPIO_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};
    gpio.Mode      = GPIO_MODE_AF_PP;
    gpio.Pull      = GPIO_NOPULL;
    gpio.Speed     = GPIO_SPEED_FREQ_HIGH;
    gpio.Alternate = GPIO_AF11_ETH;

#define INIT_PIN(port, pin) do { gpio.Pin = (pin); HAL_GPIO_Init((port), &gpio); } while(0)

    INIT_PIN(ETH_CLK_GPIO_PORT,   ETH_CLK_GPIO_PIN);    /* PA1  */
    INIT_PIN(ETH_MDIO_GPIO_PORT,  ETH_MDIO_GPIO_PIN);   /* PA2  */
    INIT_PIN(ETH_CRS_GPIO_PORT,   ETH_CRS_GPIO_PIN);    /* PA7  */
    INIT_PIN(ETH_MDC_GPIO_PORT,   ETH_MDC_GPIO_PIN);    /* PC1  */
    INIT_PIN(ETH_RXD0_GPIO_PORT,  ETH_RXD0_GPIO_PIN);   /* PC4  */
    INIT_PIN(ETH_RXD1_GPIO_PORT,  ETH_RXD1_GPIO_PIN);   /* PC5  */
    INIT_PIN(ETH_TX_EN_GPIO_PORT, ETH_TX_EN_GPIO_PIN);  /* PG11 */
    INIT_PIN(ETH_TXD0_GPIO_PORT,  ETH_TXD0_GPIO_PIN);   /* PG13 */
    INIT_PIN(ETH_TXD1_GPIO_PORT,  ETH_TXD1_GPIO_PIN);   /* PG14 */

#undef INIT_PIN

    HAL_NVIC_SetPriority(ETH_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(ETH_IRQn);
}

void HAL_ETH_MspDeInit(ETH_HandleTypeDef *heth)
{
    if (heth->Instance != ETH) { return; }

    HAL_NVIC_DisableIRQ(ETH_IRQn);

    HAL_GPIO_DeInit(ETH_CLK_GPIO_PORT,   ETH_CLK_GPIO_PIN);
    HAL_GPIO_DeInit(ETH_MDIO_GPIO_PORT,  ETH_MDIO_GPIO_PIN);
    HAL_GPIO_DeInit(ETH_CRS_GPIO_PORT,   ETH_CRS_GPIO_PIN);
    HAL_GPIO_DeInit(ETH_MDC_GPIO_PORT,   ETH_MDC_GPIO_PIN);
    HAL_GPIO_DeInit(ETH_RXD0_GPIO_PORT,  ETH_RXD0_GPIO_PIN);
    HAL_GPIO_DeInit(ETH_RXD1_GPIO_PORT,  ETH_RXD1_GPIO_PIN);
    HAL_GPIO_DeInit(ETH_TX_EN_GPIO_PORT, ETH_TX_EN_GPIO_PIN);
    HAL_GPIO_DeInit(ETH_TXD0_GPIO_PORT,  ETH_TXD0_GPIO_PIN);
    HAL_GPIO_DeInit(ETH_TXD1_GPIO_PORT,  ETH_TXD1_GPIO_PIN);

#ifdef ETH_PHY_RESET_VIA_GPIO
    HAL_GPIO_DeInit(ETH_PHY_RST_PORT, ETH_PHY_RST_PIN);
#endif

    __HAL_RCC_ETH_CLK_DISABLE();
}

/* ====================================================================
 * HAL interrupt & callback glue
 * ==================================================================== */

void ETH_IRQHandler(void)
{
    HAL_ETH_IRQHandler(&g_eth_handler);
}

void HAL_ETH_RxAllocateCallback(uint8_t **buff)
{
    eth_dev_rx_buf_alloc(buff);
}

void HAL_ETH_RxLinkCallback(void **pStart, void **pEnd,
                            uint8_t *buff, uint16_t length)
{
    eth_dev_rx_buf_link(pStart, pEnd, buff, length);
}

void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef *heth)
{
    (void)heth;
    if (eth_dev_rx_done()) {
        sys_sem_signal_isr(&s_eth1_data.rx_sem);
    }
}

void HAL_ETH_ErrorCallback(ETH_HandleTypeDef *heth)
{
    (void)heth;
    LOG_PRINT(LOG_OUT_ERROR, "%sETH DMA error!\n", TAG);
}

/* ====================================================================
 * Eth_Device_T vtable implementation
 * ==================================================================== */

/**
 * Full hardware initialisation: MAC, PHY, auto-negotiation.
 * Executed once at netif-add time (called from ethernetif_init → low_level_init).
 */
static int eth1_init(void *privatedata, uint8_t *mac_addr)
{
    if (!privatedata) {
        LOG_PRINT(LOG_OUT_ERROR, "%sinit: null private data\n", TAG);
        return -1;
    }

    Eth_Device_T *eth_dev = (Eth_Device_T *)privatedata;
    Eth_Data_T   *ed      = (Eth_Data_T *)eth_dev->private_data;

    if (!ed) {
        LOG_PRINT(LOG_OUT_ERROR, "%sinit: null eth data\n", TAG);
        return -1;
    }

    memcpy(ed->mac_addr, mac_addr, 6);

    /* ---- MAC / DMA init (HAL_ETH_Init → MSP → GPIO/clk/NVIC) ---- */
    if (ed->bsp_init(ed->mac_addr) != 0) {
        LOG_PRINT(LOG_OUT_ERROR, "%sinit: BSP init failed\n", TAG);
        return -1;
    }

    /* ---- MDIO clock (must be set before any PHY register access) ---- */
    HAL_ETH_SetMDIOClockRange(ed->eth);

    /* ---- PHY hardware reset ---- */
    eth_phy_reset(ed);

    /* ---- TX descriptor config (persisted) ---- */
    memset(&ed->tx_config, 0, sizeof(ETH_TxPacketConfig));
    ed->tx_config.Attributes  = ETH_TX_PACKETS_FEATURES_CSUM
                              | ETH_TX_PACKETS_FEATURES_CRCPAD;
    ed->tx_config.ChecksumCtrl = ETH_CHECKSUM_IPHDR_PAYLOAD_INSERT_PHDR_CALC;
    ed->tx_config.CRCPadCtrl   = ETH_CRC_PAD_INSERT;

    /* ---- RX buffer pool ---- */
    eth_dev_rx_pool_init();

    /* ---- RX semaphore ---- */
    if (sys_sem_new(&ed->rx_sem, 0) != ERR_OK) {
        LOG_PRINT(LOG_OUT_ERROR, "%sinit: semaphore create failed\n", TAG);
        return -1;
    }

    /* ---- PHY init & auto-negotiation ---- */
    eth_phy_register_bus_io(&ed->eth_chip, &eth_chip_io_ctx);
    eth_phy_init(&ed->eth_chip);
    eth_phy_start_auto_nego(&ed->eth_chip);

    delay_ms(2000);

    int phy_link_state = eth_phy_get_link_state(&ed->eth_chip);
    if (phy_link_state == ETH_CHIP_STATUS_READ_ERROR) {
        LOG_PRINT(LOG_OUT_ERROR, "%sinit: PHY link state read error\n", TAG);
        return -1;
    }

    uint32_t duplex, speed;
    switch (phy_link_state) {
    case ETH_CHIP_STATUS_100MBITS_FULLDUPLEX:
        duplex = ETH_FULLDUPLEX_MODE;  speed = ETH_SPEED_100M;  break;
    case ETH_CHIP_STATUS_100MBITS_HALFDUPLEX:
        duplex = ETH_HALFDUPLEX_MODE;  speed = ETH_SPEED_100M;  break;
    case ETH_CHIP_STATUS_10MBITS_FULLDUPLEX:
        duplex = ETH_FULLDUPLEX_MODE;  speed = ETH_SPEED_10M;   break;
    case ETH_CHIP_STATUS_10MBITS_HALFDUPLEX:
        duplex = ETH_HALFDUPLEX_MODE;  speed = ETH_SPEED_10M;   break;
    default:
        duplex = ETH_FULLDUPLEX_MODE;  speed = ETH_SPEED_100M;  break;
    }

    /* ---- Configure MAC speed/duplex ---- */
    ETH_MACConfigTypeDef mac_config;
    HAL_ETH_GetMACConfig(ed->eth, &mac_config);
    mac_config.DuplexMode = duplex;
    mac_config.Speed      = speed;
    HAL_ETH_SetMACConfig(ed->eth, &mac_config);

    /* ---- Start DMA ---- */
    HAL_ETH_Start_IT(ed->eth);

    /* ---- Sanity check: can we talk to the PHY? ---- */
    {
        int timeout = 1000;
        while (!eth_read_phy_reg(ed, ed->eth_chip.physcsr) && timeout > 0) {
            delay_ms(1);
            timeout--;
        }
        if (timeout <= 0) {
            LOG_PRINT(LOG_OUT_ERROR, "%sinit: MCU-PHY communication failed\n", TAG);
        }
    }

    /* ---- Notify upper layers ---- */
    if (ed->link_cb) {
        ed->link_cb(ed->link_cb_user_data, 1, (int)speed, (int)duplex);
    }

    LOG_PRINT(LOG_OUT_DEBUG, "%sinit: success\n", TAG);
    return 0;
}

static int eth1_deinit(void *privatedata)
{
    if (!privatedata) return -1;
    Eth_Data_T *ed = (Eth_Data_T *)((Eth_Device_T *)privatedata)->private_data;
    if (!ed || !ed->eth) return -1;

    HAL_ETH_Stop_IT(ed->eth);
    eth_phy_deinit(&ed->eth_chip);

    if (sys_sem_valid(&ed->rx_sem)) {
        sys_sem_free(&ed->rx_sem);
        sys_sem_set_invalid(&ed->rx_sem);
    }
    return 0;
}

static int eth1_send(void *privatedata, uint8_t *data, uint16_t length)
{
    if (!privatedata) {
        LOG_PRINT(LOG_OUT_ERROR, "%ssend: null private data\n", TAG);
        return -1;
    }

    Eth_Data_T *ed = (Eth_Data_T *)((Eth_Device_T *)privatedata)->private_data;

    ETH_BufferTypeDef txb[ETH_TX_DESC_CNT];
    memset(txb, 0, sizeof(txb));
    txb[0].buffer = data;
    txb[0].len    = length;
    txb[0].next   = NULL;

    ed->tx_config.Length   = length;
    ed->tx_config.TxBuffer = txb;

    if (HAL_ETH_Transmit(ed->eth, &ed->tx_config,
                         ETH_DMA_TRANSMIT_TIMEOUT) != HAL_OK) {
        LOG_PRINT(LOG_OUT_ERROR, "%ssend: HAL_ETH_Transmit failed\n", TAG);
        return -1;
    }
    return 0;
}

static int eth1_ioctl(void *privatedata, uint32_t cmd, void *arg)
{
    (void)privatedata; (void)cmd; (void)arg;
    return 0;
}

static int eth1_wait_rx(void *privatedata, uint32_t timeout_ms)
{
    if (!privatedata) return -1;

    Eth_Data_T *ed = (Eth_Data_T *)((Eth_Device_T *)privatedata)->private_data;

    if (sys_arch_sem_wait(&ed->rx_sem, timeout_ms) == SYS_ARCH_TIMEOUT) {
        return 0;
    }

    int count = 0;
    for (;;) {
        void *frame = eth_dev_rx_dequeue();
        if (!frame) break;

        if (ed->rx_cb) {
            void *seg = frame;
            while (seg) {
                ed->rx_cb(ed->rx_cb_user_data,
                          eth_dev_rx_seg_data(seg),
                          eth_dev_rx_seg_length(seg));
                seg = eth_dev_rx_seg_next(seg);
            }
        }

        eth_dev_rx_recycle(frame);
        count++;
    }
    return count;
}

static int eth1_register_rx_callback(void *privatedata,
                                     eth_rx_callback_t cb, void *user_data)
{
    if (!privatedata) return -1;
    Eth_Data_T *ed = (Eth_Data_T *)((Eth_Device_T *)privatedata)->private_data;
    ed->rx_cb          = cb;
    ed->rx_cb_user_data = user_data;
    return 0;
}

static int eth1_register_link_callback(void *privatedata,
                                       eth_link_callback_t cb, void *user_data)
{
    if (!privatedata) return -1;
    Eth_Data_T *ed = (Eth_Data_T *)((Eth_Device_T *)privatedata)->private_data;
    ed->link_cb          = cb;
    ed->link_cb_user_data = user_data;
    return 0;
}

static int eth1_start(void *privatedata)
{
    if (!privatedata) return -1;
    Eth_Data_T *ed = (Eth_Data_T *)((Eth_Device_T *)privatedata)->private_data;
    if (!ed || !ed->eth) return -1;

    HAL_ETH_Start_IT(ed->eth);
    return 0;
}

static int eth1_stop(void *privatedata)
{
    if (!privatedata) return -1;
    Eth_Data_T *ed = (Eth_Data_T *)((Eth_Device_T *)privatedata)->private_data;
    if (!ed || !ed->eth) return -1;

    HAL_ETH_Stop_IT(ed->eth);
    return 0;
}

static int eth1_get_link_status(void *privatedata)
{
    if (!privatedata) return -1;
    Eth_Data_T *ed = (Eth_Data_T *)((Eth_Device_T *)privatedata)->private_data;
    if (!ed || !ed->eth) return -1;

    uint32_t regval;
    if (HAL_ETH_ReadPHYRegister(ed->eth, ETH_CHIP_ADDR,
                                ETH_CHIP_BSR, &regval) != HAL_OK)
        return -1;
    return (regval & ETH_CHIP_BSR_LINK_STATUS) ? 1 : 0;
}
