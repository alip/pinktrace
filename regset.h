/*
 * Copyright (c) 2013 Ali Polatel <alip@exherbo.org>
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

#ifndef PINK_REGSET_H
#define PINK_REGSET_H

/**
 * @file pinktrace/regset.h
 * @brief Pink's process registry set
 *
 * Do not include this file directly. Use pinktrace/pink.h instead.
 *
 * @defgroup pink_regset Pink's process registry set
 * @ingroup pinktrace
 * @{
 **/

#include <sys/types.h>

/** This opaque structure represents a registry set of a traced process */
struct pink_regset;

/**
 * Allocate a registry set
 *
 * @param regptr Pointer to store the dynamically allocated registry set,
 *		 Use pink_regset_free() to free after use.
 * @return 0 on success, negated errno on failure
 **/
int pink_regset_alloc(struct pink_regset **regptr)
	PINK_GCC_ATTR((nonnull(1)));

/**
 * Free the memory allocated for the registry set
 *
 * @param regset Registry set
 **/
void pink_regset_free(struct pink_regset *regset);

/**
 * Fill the given regset structure with the registry information of the given
 * process ID
 *
 * @param pid Process ID
 * @param regset Registry set
 * @return 0 on success, negated errno on failure
 **/
int pink_regset_fill(pid_t pid, struct pink_regset *regset)
	PINK_GCC_ATTR((nonnull(2)));

/** @} */
#endif
