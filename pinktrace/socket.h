/*
 * Copyright (c) 2010, 2011, 2012, 2013, 2021 Ali Polatel <alip@exherbo.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef PINK_SOCKET_H
#define PINK_SOCKET_H

/**
 * @file pinktrace/socket.h
 * @brief Pink's socket related data structures and functions
 *
 * Do not include this file directly. Use pinktrace/pink.h instead.
 *
 * @defgroup pink_socket Pink's socket related data structures and functions
 * @ingroup pinktrace
 * @{
 **/

#include <stdbool.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#if PINK_HAVE_NETLINK || defined(DOXYGEN)
#include <linux/netlink.h>
#endif

/** Structure which represents a socket address. */
struct pink_sockaddr {
	/** Family of the socket address **/
	int family;

	/** Length of the socket address */
	socklen_t length;

	/**
	 * This union contains type-safe pointers to the real socket address.
	 * Check the family before attempting to obtain the real object.
	 **/
	union {
		/** Padding, mostly for internal use */
		char pad[128];

		/** Socket address, mostly for internal use */
		struct sockaddr sa;

		/** Unix socket address, only valid if family is AF_UNIX */
		struct sockaddr_un sa_un;

		/** Inet socket address, only valid if family is AF_INET */
		struct sockaddr_in sa_in;

#if PINK_HAVE_IPV6 || defined(DOXYGEN)
		/**
		 * Inet6 socket address, only valid if family is AF_INET6.
		 * This member is only available if IPV6 support was enabled at
		 * compile time. Check with PINK_HAVE_IPV6.
		 **/
		struct sockaddr_in6 sa6;
#endif

#if PINK_HAVE_NETLINK || defined(DOXYGEN)
		/**
		 * Netlink socket address, only valid if family is AF_NETLINK.
		 * This member is only available if Netlink support was enabled
		 * at compile time. Check with PINK_HAVE_NETLINK.
		 **/
		struct sockaddr_nl nl;
#endif
	} u;
};

/** Decoded socket subcalls */
enum pink_socket_subcall {
	/** socket() subcall **/
	PINK_SOCKET_SUBCALL_SOCKET = 1,
	/** bind() subcall **/
	PINK_SOCKET_SUBCALL_BIND,
	/** connect() subcall **/
	PINK_SOCKET_SUBCALL_CONNECT,
	/** listen() subcall **/
	PINK_SOCKET_SUBCALL_LISTEN,
	/** accept() subcall **/
	PINK_SOCKET_SUBCALL_ACCEPT,
	/** getsockname() subcall **/
	PINK_SOCKET_SUBCALL_GETSOCKNAME,
	/** getpeername() subcall **/
	PINK_SOCKET_SUBCALL_GETPEERNAME,
	/** socketpair() subcall **/
	PINK_SOCKET_SUBCALL_SOCKETPAIR,
	/** send() subcall **/
	PINK_SOCKET_SUBCALL_SEND,
	/** recv() subcall **/
	PINK_SOCKET_SUBCALL_RECV,
	/** sendto() subcall **/
	PINK_SOCKET_SUBCALL_SENDTO,
	/** recvfrom() subcall **/
	PINK_SOCKET_SUBCALL_RECVFROM,
	/** shutdown() subcall **/
	PINK_SOCKET_SUBCALL_SHUTDOWN,
	/** setsockopt() subcall **/
	PINK_SOCKET_SUBCALL_SETSOCKOPT,
	/** getsockopt() subcall **/
	PINK_SOCKET_SUBCALL_GETSOCKOPT,
	/** sendmsg() subcall **/
	PINK_SOCKET_SUBCALL_SENDMSG,
	/** recvmsg() subcall **/
	PINK_SOCKET_SUBCALL_RECVMSG,
	/** accept4() subcall **/
	PINK_SOCKET_SUBCALL_ACCEPT4,
};

/**
 * Convenience macro to read socket subcall
 *
 * @see pink_socket_subcall
 * @see pink_read_argument
 * @see pink_read_syscall
 *
 * @param pid Process ID
 * @param regset Registry set
 * @param decode_socketcall Boolean to specify decoding @e socketcall(2)
 * @param subcall Pointer to store the result, must not be @e NULL
 * @return 0 on success, negated errno on failure
 **/
#define pink_read_socket_subcall(pid, regset, decode_socketcall, subcall) \
		((decode_socketcall) \
			? pink_read_argument((pid), (regset), 0, (subcall)) \
			: pink_read_syscall((pid), (regset), (subcall)))

/**
 * Read the specified socket call argument.
 *
 * @note For decode_socketcall == `false', this function is equivalent to
 *       pink_read_argument(). For decode_socketcall == `true' this function
 *       decodes the second argument of the @e socketcall(2) system call.
 *
 * @param pid Process ID
 * @param regset Registry set
 * @param decode_socketcall Boolean to specify decoding @e socketcall(2)
 * @param arg_index Index of the argument, first argument is 0
 * @param argval Pointer to store the value, must @b not be @e NULL
 * @return 0 on success, negated errno on failure
 **/
int pink_read_socket_argument(pid_t pid, const struct pink_regset *regset,
			      bool decode_socketcall,
			      unsigned arg_index, unsigned long *argval)
	PINK_GCC_ATTR((nonnull(2,5)));

/**
 * Read the specified socket call address
 *
 * @note If the address argument of the system call was NULL, this function
 *       returns true and sets sockaddr->family to -1.
 *
 * @param pid Process ID
 * @param regset Registry set
 * @param decode_socketcall Boolean to specify decoding @e socketcall(2)
 * @param arg_index The index of the argument. One of:
 *  - 1 (for connect, bind etc.)
 *  - 4 (for sendto)
 * @param fd The pointer to store the socket file descriptor that resides in
 *           argument one with index zero. Caller may set this to @e NULL in
 *           case the file descriptor is not requested.
 * @param sockaddr Pointer to store the socket address, must @b not be @e NULL
 * @return 0 on success, negated errno on failure
 **/
int pink_read_socket_address(pid_t pid, const struct pink_regset *regset,
			     bool decode_socketcall,
			     unsigned arg_index, int *fd,
			     struct pink_sockaddr *sockaddr)
	PINK_GCC_ATTR((nonnull(2,6)));

/** @} */
#endif
