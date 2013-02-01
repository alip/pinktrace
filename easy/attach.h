/*
 * Copyright (c) 2010, 2011, 2012 Ali Polatel <alip@exherbo.org>
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

#ifndef PINK_EASY_ATTACH_H
#define PINK_EASY_ATTACH_H

/**
 * @file pinktrace/easy/attach.h
 * @brief Pink's easy process attaching
 *
 * Do not include this file directly. Use pinktrace/easy/pink.h instead.
 *
 * @defgroup pink_easy_attach Pink's easy process attaching
 * @ingroup pinktrace-easy
 * @{
 **/

#include <pinktrace/compiler.h>

#include <stdbool.h>
#include <sys/types.h>

struct pink_easy_context;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Attach to a process for tracing.
 * Call this multiple times before pink_easy_loop() to attach to multiple
 * processes.
 *
 * @param ctx Tracing context
 * @param tid Thread ID
 * @param tgid Thread group ID. Use this to specify the thread group in case
 *             the process is a clone. This is useful when attaching to all
 *             threads of a process and lets pinktrace track whether the
 *             process is a clone. Specify -1 for non-clones.
 * @return true on success, false on failure and sets errno accordingly
 **/
bool pink_easy_attach(struct pink_easy_context *ctx, pid_t tid, pid_t tgid)
	PINK_GCC_ATTR((nonnull(1)));

#ifdef __cplusplus
}
#endif
/** @} */
#endif
