/*
 * Copyright (c) 2012, 2013, 2021 Ali Polatel <alip@exherbo.org>
 * Based in part upon strace which is:
 *   Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 *   Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 *   Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <pinktrace/private.h>
#include <pinktrace/pink.h>

PINK_GCC_ATTR((pure))
size_t pink_abi_wordsize(short abi)
{
#if PINK_ABIS_SUPPORTED == 1
	return ABI0_WORDSIZE;
#else
	static const int abi_wordsize[PINK_ABIS_SUPPORTED] = {
		ABI0_WORDSIZE,
		ABI1_WORDSIZE,
# if PINK_ABIS_SUPPORTED > 2
		ABI2_WORDSIZE,
# endif
	};

	return abi_wordsize[abi];
#endif
}
