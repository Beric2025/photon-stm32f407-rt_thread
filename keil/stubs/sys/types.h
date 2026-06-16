/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * Minimal POSIX type stubs for ARM Compiler (Keil MDK).
 *
 * ARMCC does not provide <sys/types.h> — RT-Thread's POSIX
 * compatibility layer (common/include/sys/unistd.h, dirent.h,
 * etc.) needs these definitions.
 *
 * GCC+newlib provide their own <sys/types.h>, so this file is
 * placed under keil/stubs/ and only added to the Keil include
 * path — SCons never sees it.
 */

#ifndef __SYS_TYPES_H__
#define __SYS_TYPES_H__

#include <stdint.h>
#include <limits.h>

typedef int          pid_t;
typedef unsigned int uid_t;
typedef unsigned int gid_t;
typedef unsigned int useconds_t;
typedef long         off_t;
typedef unsigned int mode_t;
typedef int          clockid_t;
typedef long         suseconds_t;
typedef unsigned int dev_t;

/* ssize_t — guard against redefinition from lwIP's arch.h.
 * lwIP checks SSIZE_MAX; if set it skips its own typedef. */
#ifndef _SSIZE_T_DEFINED
#define _SSIZE_T_DEFINED
typedef int ssize_t;
#endif
#define SSIZE_MAX INT_MAX

#endif /* __SYS_TYPES_H__ */
