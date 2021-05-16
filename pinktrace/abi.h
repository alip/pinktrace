/*
 * Copyright (c) 2012, 2013 Ali Polatel <alip@exherbo.org>
 * Based in part upon strace which is:
 *   Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 *   Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 *   Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef PINK_ABI_H
#define PINK_ABI_H

/**
 * @file pinktrace/abi.h
 * @brief Pink's supported system call ABIs
 *
 * Do not include this header directly. Use pinktrace/pink.h instead.
 *
 * @defgroup pink_abi Pink's supported system call ABIs
 * @ingroup pinktrace
 * @{
 **/

#ifndef DOXYGEN

# if PINK_ARCH_X86_64
#  define PINK_ABIS_SUPPORTED 3
#  define PINK_ABI_X86_64 0
#  define PINK_ABI_I386 1
#  define PINK_ABI_X32 2
#endif

# if PINK_ARCH_X32
#  define PINK_ABIS_SUPPORTED 2
#  define PINK_ABI_X32 0
#  define PINK_ABI_I386 1
# endif

# if PINK_ARCH_POWERPC64
#  define PINK_ABIS_SUPPORTED 2
#  define PINK_ABI_PPC64 0
#  define PINK_ABI_PPC32 1
# endif

# ifndef PINK_ABIS_SUPPORTED
#  define PINK_ABIS_SUPPORTED 1
# endif
# define PINK_ABI_DEFAULT 0

#else
/**
 * Default ABI of the host architecture
 *
 * Consult the definitions under <pinktrace/abi.h> for more information.
 **/
# define PINK_ABI_DEFAULT	0
/**
 * Total number of supported ABIs
 *
 * Consult the definitions under <pinktrace/abi.h> for more information.
 */
# define PINK_ABIS_SUPPORTED	-1
#endif

/**
 * Return the word size of the system call ABI
 *
 * @param abi System call ABI
 * @return Word size
 **/
size_t pink_abi_wordsize(short abi)
	PINK_GCC_ATTR((pure));

/** @} */
#endif
