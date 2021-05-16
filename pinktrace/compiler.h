/*
 * Copyright (c) 2010, 2011, 2012, 2021 Ali Polatel <alip@exherbo.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef PINK_COMPILER_H
#define PINK_COMPILER_H

/**
 * @file pinktrace/compiler.h
 * @brief Pink's compiler specific definitions
 *
 * Do not include this file directly. Use pinktrace/pink.h instead.
 *
 * @defgroup pink_compiler Pink's compiler specific definitions
 * @ingroup pinktrace
 * @{
 **/

#if !defined(SPARSE) && defined(__GNUC__) && __GNUC__ >= 3
#define PINK_GCC_ATTR(x)     __attribute__(x)
#define PINK_GCC_LIKELY(x)   __builtin_expect(!!(x), 1)
#define PINK_GCC_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
/** Macro for GCC attributes */
#define PINK_GCC_ATTR(x) /* empty */
/** GCC builtin_expect macro */
#define PINK_GCC_LIKELY(x)   (x)
/** GCC builtin_expect macro */
#define PINK_GCC_UNLIKELY(x) (x)
#endif

/** @} */
#endif
