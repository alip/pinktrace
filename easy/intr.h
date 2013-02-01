/*
 * Copyright (c) 2012 Ali Polatel <alip@exherbo.org>
 * Based in part upon strace which is:
 *   Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 *   Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 *   Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 *   Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LpIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef PINK_EASY_INTR_H
#define PINK_EASY_INTR_H

/**
 * @file pinktrace/easy/intr.h
 * @brief Pink's easy interrupt handling
 *
 * Do not include this file directly. Use pinktrace/easy/pink.h directly.
 *
 * @defgroup pink_easy_intr Pink's easy interrupt handling
 * @ingroup pinktrace-easy
 * @{
 **/

#include <signal.h>

/** Interrupt states */
enum pink_easy_intr {
	PINK_EASY_INTR_ANYWHERE       = 1, /**< don't block/ignore any signals */
	PINK_EASY_INTR_WHILE_WAIT     = 2, /**< block fatal signals while decoding syscall. default */
	PINK_EASY_INTR_NEVER          = 3, /**< block fatal signals */
	PINK_EASY_INTR_BLOCK_TSTP_TOO = 4, /**< block fatal signals and SIGTSTP (^Z) */
};

/** Interrupt state tracker */
extern volatile sig_atomic_t pink_easy_interrupted;
/** Empty signal set */
extern sigset_t pink_easy_empty_set;
/** Blocked signal set */
extern sigset_t pink_easy_blocked_set;

/**
 * Set up signal handlers
 *
 * @param intr Interrupt method
 **/
void pink_easy_interrupt_init(enum pink_easy_intr intr);

/** @} */
#endif
