/*
 * Copyright (c) 2010, 2011, 2012, 2013, 2021 Ali Polatel <alip@exherbo.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef PINK_NAME_H
#define PINK_NAME_H

/**
 * @file pinktrace/name.h
 * @brief Pink's naming functions
 *
 * Do not include this file directly. Use pinktrace/pink.h directly.
 *
 * @defgroup pink_name Pink's naming functions
 * @ingroup pinktrace
 * @{
 **/

/**
 * Return the name of the given ptrace event
 *
 * @param event Event
 * @return The name of the event, NULL if event name is unknown
 **/
const char *pink_name_event(enum pink_event event)
	PINK_GCC_ATTR((pure));

/**
 * Look up the number of the ptrace event name.
 *
 * @param name Name of the event
 * @return Ptrace event number on successful lookup, -1 otherwise
 **/
int pink_lookup_event(const char *name)
	PINK_GCC_ATTR((pure));

/**
 * Return the name of the given system call.
 *
 * @param scno System call number
 * @param abi System call ABI
 * @return The name of the system call, NULL if system call name is unknown
 **/
const char *pink_name_syscall(long scno, short abi)
	PINK_GCC_ATTR((pure));

/**
 * Look up the number of the given system call name.
 *
 * @param name Name of the system call
 * @param abi System call ABI
 * @return System call number on successful lookup, -1 otherwise
 **/
long pink_lookup_syscall(const char *name, short abi)
	PINK_GCC_ATTR((pure));

/**
 * Return the name of the given socket address family.
 *
 * @param family Socket address family
 * @return The name of the socket address family, NULL if family is unknown
 **/
const char *pink_name_socket_family(int family)
	PINK_GCC_ATTR((pure));

/**
 * Look up the number of the given socket address family
 *
 * @param name Name of the socket address family
 * @return Socket address family number on successful lookup, -1 otherwise
 **/
int pink_lookup_socket_family(const char *name)
	PINK_GCC_ATTR((pure));

/**
 * Return the name of the given socket subcall.
 *
 * @param subcall Socket subcall
 * @return The name of the subcall, NULL if socket subcall name is unknown
 **/
const char *pink_name_socket_subcall(enum pink_socket_subcall subcall)
	PINK_GCC_ATTR((pure));
/**
 * Look up the number of the given socket subcall name
 *
 * @param name Name of the socket subcall
 * @return Socket subcall number on successful lookup, -1 otherwise
 **/
int pink_lookup_socket_subcall(const char *name)
	PINK_GCC_ATTR((pure));

/**
 * Return the name of the given errno.
 *
 * @param err_no errno
 * @param abi System call ABI
 * @return The name of the errno, NULL if errno name is unknown
 **/
const char *pink_name_errno(int err_no, short abi)
	PINK_GCC_ATTR((pure));

/**
 * Look up the number of the given errno.
 *
 * @param name Name of the errno
 * @param abi System call ABI
 * @return Number of errno on successful lookup, -1 otherwise
 **/
int pink_lookup_errno(const char *name, short abi)
	PINK_GCC_ATTR((pure));

/**
 * Return the name of the given signal.
 *
 * @param sig Signal
 * @param abi System call ABI
 * @return The name of the signal, NULL if errno name is unknown
 **/
const char *pink_name_signal(int sig, short abi)
	PINK_GCC_ATTR((pure));

/**
 * Look up the number of the given signal.
 *
 * @param name Name of the signal
 * @param abi System call ABI
 * @return Number of signal on successful lookup, -1 otherwise
 **/
int pink_lookup_signal(const char *name, short abi)
	PINK_GCC_ATTR((pure));

/** @} */
#endif
