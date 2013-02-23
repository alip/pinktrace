/*
 * Copyright (c) 2010, 2011, 2012, 2013 Ali Polatel <alip@exherbo.org>
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

#ifndef PINK_SYSCALL_H
#define PINK_SYSCALL_H

/**
 * @file pinktrace/syscall.h
 * @brief Pink's system call naming
 *
 * Do not include this file directly. Use pinktrace/pink.h directly.
 *
 * @defgroup pink_syscall Pink's system call naming
 * @ingroup pinktrace
 * @{
 **/

/**
 * Return the name of the given system call.
 *
 * @param scno System call number
 * @param abi System call ABI
 * @return The name of the system call, NULL if system call name is unknown
 **/
const char *pink_syscall_name(long scno, short abi)
	PINK_GCC_ATTR((pure));

/**
 * Look up the number of the given system call name.
 *
 * @param name Name of the system call
 * @param abi System call ABI
 * @return System call number on successful lookup, -1 otherwise
 **/
long pink_syscall_lookup(const char *name, short abi)
	PINK_GCC_ATTR((pure));

#ifdef __cplusplus
}
#endif
/** @} */
#endif
