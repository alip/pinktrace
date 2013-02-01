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

#ifndef PINK_EASY_CALL_H
#define PINK_EASY_CALL_H

/**
 * @file pinktrace/easy/call.h
 * @brief Pink's easy tracing function calls
 *
 * Do not include this file directly. Use pinktrace/easy/pink.h instead.
 *
 * @defgroup pink_easy_call Pink's easy tracing function calls
 * @ingroup pinktrace-easy
 * @{
 **/

#include <pinktrace/compiler.h>
#include <pinktrace/easy/func.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Call a simple function which will be traced.
 *
 * @note This function uses fork() to spawn the initial child.
 *
 * @param ctx Tracing context
 * @param func Function which will be executed under the tracing environment
 * @param userdata User data to be passed to the child function
 * @return true on success, false on failure and sets errno accordingly
 **/
bool pink_easy_call(struct pink_easy_context *ctx, pink_easy_child_func_t func, void *userdata)
	PINK_GCC_ATTR((nonnull(1,2)));

#ifdef __cplusplus
}
#endif
/** @} */
#endif
