/*
 * Copyright (c) 2026 beric-xiong
 * SPDX-License-Identifier: MIT
 *
 * Thin wrapper - forwards to the master rtconfig.h at project root.
 *
 * The master copy lives at:  ../../rtconfig.h
 * Edit THAT file, not this one.
 *
 * This wrapper exists for backward compatibility with the CMake build,
 * which has port/rtthread/ on its include path.
 *
 * IMPORTANT: No include guard here! The root rtconfig.h provides it.
 * If we guard here, the root's content is skipped via double-inclusion.
 */

#include "../../rtconfig.h"
