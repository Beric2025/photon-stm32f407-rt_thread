/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * Project-specific overrides — survives menuconfig regeneration.
 *
 * When scons --menuconfig regenerates rtconfig.h, it auto-includes
 * this file at the end (via #include "rtconfig_project.h").
 *
 * Put settings here that have NO corresponding Kconfig option.
 */

#ifndef RTCONFIG_PROJECT_H__
#define RTCONFIG_PROJECT_H__

/* ============================================================
 * Memory
 * ============================================================ */
#define RT_HEAP_SIZE                (10 * 1024)   /* 10KB heap */

/* ============================================================
 * lwIP / HAL — already handled by port config headers
 * ============================================================ */

#endif /* RTCONFIG_PROJECT_H__ */
