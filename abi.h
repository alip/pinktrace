/*
 * Copyright (c) 2012, 2013 Ali Polatel <alip@exherbo.org>
 * Based in part upon strace which is:
 *   Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 *   Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 *   Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

#if PINK_ARCH_X86_64
# define PINK_ABIS_SUPPORTED 3
# define PINK_ABI_X86_64 0
# define PINK_ABI_I386 1
# define PINK_ABI_X32 2
#endif

#if PINK_ARCH_X32
# define PINK_ABIS_SUPPORTED 2
# define PINK_ABI_X32 0
# define PINK_ABI_I386 1
#endif

#if PINK_ARCH_POWERPC64
# define PINK_ABIS_SUPPORTED 2
# define PINK_ABI_PPC64 0
# define PINK_ABI_PPC32 1
#endif

#ifndef PINK_ABIS_SUPPORTED
# define PINK_ABIS_SUPPORTED 1
#endif
#define PINK_ABI_DEFAULT 0

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
