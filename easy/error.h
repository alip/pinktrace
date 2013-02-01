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

#ifndef PINK_EASY_ERROR_H
#define PINK_EASY_ERROR_H

/**
 * @file pinktrace/easy/error.h
 * @brief Pink's easy error codes
 *
 * Do not include this file directly. Use pinktrace/easy/pink.h instead.
 *
 * @defgroup pink_easy_error Pink's easy error codes
 * @ingroup pinktrace-easy
 * @{
 **/

#include <pinktrace/compiler.h>

/** Child error codes */
enum pink_easy_child_error {
	/** Success **/
	PINK_EASY_CHILD_ERROR_SUCCESS = 0,
	/** Preparation for tracing failed. (e.g. pink_trace_me()) **/
	PINK_EASY_CHILD_ERROR_SETUP,
	/** @e execve(2) failed. **/
	PINK_EASY_CHILD_ERROR_EXEC,
	/** Maximum error number **/
	PINK_EASY_CHILD_ERROR_MAX,
};

/** Error codes */
enum pink_easy_error {
	/** Successful run **/
	PINK_EASY_ERROR_SUCCESS = 0,

	/** Operation aborted by a callback **/
	PINK_EASY_ERROR_CALLBACK_ABORT,

	/** Failure during memory allocation **/
	PINK_EASY_ERROR_ALLOC,

	/** Failure during process attach **/
	PINK_EASY_ERROR_ATTACH,

	/** Failed to @e fork(2) **/
	PINK_EASY_ERROR_FORK,

	/** @e waitpid(2) failed **/
	PINK_EASY_ERROR_WAIT,

	/** @e ptrace(2) failed **/
	PINK_EASY_ERROR_TRACE,

	/** Process misbehave (most likely an indication of a pinktrace bug) **/
	PINK_EASY_ERROR_PROCESS,

	/** Maximum error number **/
	PINK_EASY_ERROR_MAX,
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Returns a string describing the child error.
 *
 * @param e Error code
 * @return String describing the error
 **/
const char *pink_easy_child_strerror(enum pink_easy_child_error e)
	PINK_GCC_ATTR((pure));

/**
 * Returns a string describing the error.
 *
 * @param e Error code
 * @return String describing the error
 **/
const char *pink_easy_strerror(enum pink_easy_error e)
	PINK_GCC_ATTR((pure));

#ifdef __cplusplus
}
#endif
/** @} */
#endif
