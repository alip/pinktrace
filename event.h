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

#ifndef PINK_EVENT_H
#define PINK_EVENT_H

/**
 * @file pinktrace/event.h
 * @brief Pink's ptrace(2) event handling for Linux
 *
 * Do not include this file directly. Use pinktrace/pink.h instead.
 *
 * @defgroup pink_event Pink's ptrace(2) event handling for Linux
 * @ingroup pinktrace
 * @{
 **/

#include <pinktrace/compiler.h>

/**
 * @e ptrace(2) event constants
 *
 * @note Availability: Linux
 **/
enum pink_event {
	/** No event */
	PINK_EVENT_NONE = 0,
	/**
	 * Child called @e fork(2)
	 *
	 * @see #PINK_HAVE_EVENT_FORK
	 **/
	PINK_EVENT_FORK = 1,
	/**
	 * Child has called @e vfork(2)
	 *
	 * @see #PINK_HAVE_EVENT_VFORK
	 **/
	PINK_EVENT_VFORK = 2,
	/**
	 * Child called @e clone(2)
	 *
	 * @see #PINK_HAVE_EVENT_CLONE
	 **/
	PINK_EVENT_CLONE = 3,
	/**
	 * Child called @e execve(2)
	 *
	 * @see #PINK_HAVE_EVENT_EXEC
	 **/
	PINK_EVENT_EXEC = 4,
	/**
	 * Child returned from @e vfork(2)
	 *
	 * @see #PINK_HAVE_EVENT_VFORK_DONE
	 **/
	PINK_EVENT_VFORK_DONE = 5,
	/**
	 * Child is exiting (ptrace way, stopped before exit)
	 *
	 * @see #PINK_HAVE_EVENT_EXIT
	 **/
	PINK_EVENT_EXIT = 6,
	/**
	 * Seccomp filter notification
	 *
	 * @see #PINK_HAVE_EVENT_SECCOMP
	 **/
	PINK_EVENT_SECCOMP = 7,
	/**
	 * STOP notification from seized child
	 *
	 * @see #PINK_HAVE_EVENT_STOP
	 **/
	PINK_EVENT_STOP = 128,
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Calculate the event from the status argument as returned by @e waitpid(2)
 *
 * @note Available on Linux, on other systems this function always returns
 * #PINK_EVENT_NONE and sets errno to @e ENOSYS
 *
 * @param status Status argument as returned by @e waitpid(2)
 * @return One of PINK_EVENT constants
 **/
enum pink_event pink_event_decide(int status)
	PINK_GCC_ATTR((pure));

/**
 * Return a string representation of the event
 *
 * @param event Event
 * @return String representation of the event
 **/
const char *pink_event_name(enum pink_event event)
	PINK_GCC_ATTR((pure));

#ifdef __cplusplus
}
#endif
/** @} */
#endif
