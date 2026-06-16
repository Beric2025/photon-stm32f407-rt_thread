/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * Minimal fcntl.h stub for ARM Compiler (Keil MDK).
 *
 * ARMCC does not provide <fcntl.h> — RT-Thread's DFS layer
 * (dfs.h) needs the file-open flags defined here.
 *
 * GCC+newlib provide their own <fcntl.h>, so this file is
 * placed under keil/stubs/ and only added to the Keil include
 * path — SCons never sees it.
 */

#ifndef __FCNTL_H__
#define __FCNTL_H__

#define O_RDONLY      0
#define O_WRONLY      1
#define O_RDWR        2
#define O_ACCMODE     (O_RDONLY | O_WRONLY | O_RDWR)
#define O_CREAT       0100
#define O_EXCL        0200
#define O_TRUNC       01000
#define O_APPEND      02000
#define O_NONBLOCK    04000
#define O_DIRECTORY   0x200000
#define O_BINARY      0x10000

#endif /* __FCNTL_H__ */
