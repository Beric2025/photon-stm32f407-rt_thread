/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 */

#ifndef __APP_VERSION_H_
#define __APP_VERSION_H_

#ifdef __cplusplus
	extern "C" {
#endif

/* Injected by CMake at build time; fallback for manual builds (Keil, etc.) */
#ifndef APP_VERSION
#define APP_VERSION "v0.0.2-dev"
#endif

#ifdef APP_DIRTY
#pragma message "WARNING: building from dirty working tree — not for release"
#endif

/**
 * @brief:  print firmware version to log output
 *
 * Return: 0 on success
 */
int build_version(void);

/**
 * @brief:  get firmware version string
 * @verbuffer:   buffer to store version string
 * @buffersize:  size of verbuffer
 *
 * Return: length of version string, -1 on failure
 */
int get_version(char *verbuffer, int buffersize);

#ifdef __cplusplus
}
#endif

#endif  /* __APP_VERSION_H_ */
