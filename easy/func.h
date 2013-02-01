/*
 * Copyright (c) 2010, 2012 Ali Polatel <alip@exherbo.org>
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

#ifndef PINK_EASY_FUNC_H
#define PINK_EASY_FUNC_H

/**
 * @file pinktrace/easy/func.h
 * @brief Pink's easy function pointers
 *
 * Do not include this file directly. Use pinktrace/easy/pink.h instead.
 *
 * @defgroup pink_easy_func Pink's easy function pointers
 * @ingroup pinktrace-easy
 * @{
 **/

struct pink_easy_process;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * This type definition represents a generic @e free(3) function.
 *
 * @see pink_easy_context_new
 * @see pink_easy_process_set_data
 **/
typedef void (*pink_easy_free_func_t) (void *data);

/**
 * This type definition represents the process tree walk function.
 * It takes a process entry and userdata as argument. If this function returns
 * false, struct pink_easy_processree_walk() stops iterating through the process
 * tree and returns immediately.
 *
 * @see struct pink_easy_processree_walk
 **/
typedef bool (*pink_easy_walk_func_t) (struct pink_easy_process *proc,
		void *userdata);

/**
 * This type definition represents a function to be executed by the child under
 * tracing. Its return value is passed directly to @e _exit(2).
 *
 * @see pink_easy_call
 **/
typedef int (*pink_easy_child_func_t) (void *userdata);

#ifdef __cplusplus
}
#endif
/** @} */
#endif
