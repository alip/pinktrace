/*
 * Copyright (c) 2010, 2011, 2012, 2013 Ali Polatel <alip@exherbo.org>
 * Based in part upon strace which is:
 *   Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 *   Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 *   Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 *   Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 *   Copyright (c) 1999 IBM Deutschland Entwicklung GmbH, IBM Corporation
 *                       Linux for s390 port by D.J. Barrow
 *                      <barrow_dj@mail.yahoo.com,djbarrow@de.ibm.com>
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

#include <pinktrace/private.h>
#include <pinktrace/pink.h>

const char *pink_socket_subcall_name(enum pink_socket_subcall subcall)
{
	switch (subcall) {
	case PINK_SOCKET_SUBCALL_SOCKET:
		return "socket";
	case PINK_SOCKET_SUBCALL_BIND:
		return "bind";
	case PINK_SOCKET_SUBCALL_CONNECT:
		return "connect";
	case PINK_SOCKET_SUBCALL_LISTEN:
		return "listen";
	case PINK_SOCKET_SUBCALL_ACCEPT:
		return "accept";
	case PINK_SOCKET_SUBCALL_GETSOCKNAME:
		return "getsockname";
	case PINK_SOCKET_SUBCALL_GETPEERNAME:
		return "getpeername";
	case PINK_SOCKET_SUBCALL_SOCKETPAIR:
		return "socketpair";
	case PINK_SOCKET_SUBCALL_SEND:
		return "send";
	case PINK_SOCKET_SUBCALL_RECV:
		return "recv";
	case PINK_SOCKET_SUBCALL_SENDTO:
		return "sendto";
	case PINK_SOCKET_SUBCALL_RECVFROM:
		return "recvfrom";
	case PINK_SOCKET_SUBCALL_SHUTDOWN:
		return "shutdown";
	case PINK_SOCKET_SUBCALL_SETSOCKOPT:
		return "setsockopt";
	case PINK_SOCKET_SUBCALL_GETSOCKOPT:
		return "getsockopt";
	case PINK_SOCKET_SUBCALL_SENDMSG:
		return "sendmsg";
	case PINK_SOCKET_SUBCALL_RECVMSG:
		return "recvmsg";
	case PINK_SOCKET_SUBCALL_ACCEPT4:
		return "accept4";
	default:
		return NULL;
	}
}

PINK_GCC_ATTR((nonnull(4)))
int pink_read_socket_argument(struct pink_process *tracee, bool decode_socketcall,
			      unsigned arg_index, unsigned long *argval)
{
	int r;
	size_t wsize;
	long addr;
	unsigned long u_addr;

	if (!argval)
		return -EINVAL;

	if (!decode_socketcall) {
		long arg;
		r = pink_read_argument(tracee, arg_index, &arg);
		if (r < 0)
			return r;
		*argval = arg;
		return 0;
	}

	/*
	 * Decoding the second argument of:
	 * int socketcall(int call, unsigned long *args);
	 */

	r = pink_read_argument(tracee, 1, &addr);
	if (r < 0)
		return r;
	u_addr = addr;
	wsize = pink_abi_wordsize(pink_process_get_abi(tracee));
	u_addr += arg_index * wsize;
	if (wsize == sizeof(int)) {
		unsigned int arg;
		if (pink_read_vm_object(tracee, u_addr, &arg) < 0)
			return -errno;
		*argval = arg;
	} else {
		unsigned long arg;
		if (pink_read_vm_object(tracee, u_addr, &arg) < 0)
			return -errno;
		*argval = arg;
	}

	return true;
}

PINK_GCC_ATTR((nonnull(5)))
int pink_read_socket_address(struct pink_process *tracee, bool decode_socketcall,
			     unsigned arg_index, int *fd,
			     struct pink_sockaddr *sockaddr)
{
	int r;
	unsigned long myfd;
	unsigned long addr, addrlen;

	if (fd) {
		r = pink_read_socket_argument(tracee, decode_socketcall, 0, &myfd);
		if (r < 0)
			return r;
		*fd = (int)myfd;
	}
	if ((r = pink_read_socket_argument(tracee, decode_socketcall, arg_index, &addr)) < 0)
		return r;
	if ((r = pink_read_socket_argument(tracee, decode_socketcall, arg_index + 1, &addrlen)) < 0)
		return r;

	if (addr == 0) {
		sockaddr->family = -1;
		sockaddr->length = 0;
		return 0;
	}
	if (addrlen < 2 || addrlen > sizeof(sockaddr->u))
		addrlen = sizeof(sockaddr->u);

	memset(&sockaddr->u, 0, sizeof(sockaddr->u));
	if (pink_read_vm_data(tracee, addr, sockaddr->u.pad, addrlen) < 0)
		return -errno;
	sockaddr->u.pad[sizeof(sockaddr->u.pad) - 1] = '\0';

	sockaddr->family = sockaddr->u.sa.sa_family;
	sockaddr->length = addrlen;

	return 0;
}
