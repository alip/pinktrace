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

#include <pinktrace/pink.h>
#include <pinktrace/easy/pink.h>

const char *pink_easy_child_strerror(enum pink_easy_child_error e)
{
	switch (e) {
	case PINK_EASY_CHILD_ERROR_SUCCESS:
		return "Success";
	case PINK_EASY_CHILD_ERROR_SETUP:
		return "Failed to set up trace";
	case PINK_EASY_CHILD_ERROR_EXEC:
		return "execve() failed";
	case PINK_EASY_CHILD_ERROR_MAX:
	default:
		return "Unknown error";
	}
}

const char *pink_easy_strerror(enum pink_easy_error e)
{
	switch (e) {
	case PINK_EASY_ERROR_SUCCESS:
		return "Success";
	case PINK_EASY_ERROR_CALLBACK_ABORT:
		return "Operation aborted by callback";
	case PINK_EASY_ERROR_ATTACH:
		return "Failed to attach";
	case PINK_EASY_ERROR_ALLOC:
		return "Failed to allocate memory";
	case PINK_EASY_ERROR_FORK:
		return "Failed to spawn new process";
	case PINK_EASY_ERROR_WAIT:
		return "waitpid() failed";
	case PINK_EASY_ERROR_TRACE:
		return "ptrace() failed";
	case PINK_EASY_ERROR_PROCESS:
		return "Process misbehave";
	case PINK_EASY_ERROR_MAX:
	default:
		return "Unknown error";
	}
}
