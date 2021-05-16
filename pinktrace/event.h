/*
 * Copyright (c) 2010, 2011, 2012, 2013, 2021 Ali Polatel <alip@exherbo.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef PINK_EVENT_H
#define PINK_EVENT_H

/**
 * @file pinktrace/event.h
 * @brief Pink's ptrace(2) event handling for Linux
 *
 * Do not include this file directly. Use pinktrace/pink.h instead.
 *
 * @defgroup pink_event Pink's ptrace(2) event handling
 * @ingroup pinktrace
 * @{
 **/

/**
 * @e ptrace(2) event constants
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

/**
 * Calculate the event from the status argument as returned by @e waitpid(2)
 *
 * @param status Status argument as returned by @e waitpid(2)
 * @return One of PINK_EVENT constants
 **/
enum pink_event pink_event_decide(int status)
	PINK_GCC_ATTR((pure));

/** @} */
#endif
