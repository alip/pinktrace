/*
 * Copyright (c) 2010, 2011, 2012 Ali Polatel <alip@exherbo.org>
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

#include <pinktrace/internal.h>
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

PINK_GCC_ATTR((nonnull(6)))
bool pink_read_socket_argument(pid_t tid, enum pink_abi abi,
		const pink_regs_t *regs,
		bool decode_socketcall,
		unsigned arg_index, long *argval)
{
	size_t wsize;
	long args;

	if (!pink_read_argument(tid, abi, regs, arg_index, &args))
		return false;
	if (!decode_socketcall) {
		*argval = args;
		return true;
	}

	if (!pink_abi_wordsize(abi, &wsize))
		return false;
	if (wsize == sizeof(int))
		args += arg_index * sizeof(unsigned int);
	else if (wsize == sizeof(long))
		args += arg_index * sizeof(unsigned long);
	else
		_pink_assert_not_reached();

	return pink_read_vm_object(tid, abi, args, argval);
}

PINK_GCC_ATTR((nonnull(7)))
bool pink_read_socket_address(pid_t tid, enum pink_abi abi,
		const pink_regs_t *regs,
		bool decode_socketcall,
		unsigned arg_index, long *fd,
		struct pink_sockaddr *sockaddr)
{
	long addr, addrlen, args;
	size_t wsize;

	if (!decode_socketcall) {
		if (fd && !pink_read_argument(tid, abi, regs, 0, fd))
			return false;
		if (!pink_read_argument(tid, abi, regs, arg_index, &addr))
			return false;
		if (!pink_read_argument(tid, abi, regs, arg_index + 1, &addrlen))
			return false;
	} else {
		if (!pink_abi_wordsize(abi, &wsize))
			return false;
		if (!pink_read_argument(tid, abi, regs, 1, &args))
			return false;
		if (fd && !pink_read_vm_object(tid, abi, args, fd))
			return false;
		if (wsize == sizeof(int)) {
			unsigned int iaddr, iaddrlen;
			args += arg_index + wsize;
			if (!pink_read_vm_object(tid, abi, args, &iaddr))
				return false;
			args += wsize;
			if (!pink_read_vm_object(tid, abi, args, &iaddrlen))
				return false;
			addr = iaddr;
			addrlen = iaddrlen;
		} else if (wsize == sizeof(long)) {
			unsigned long laddr, laddrlen;
			args += arg_index + wsize;
			if (!pink_read_vm_object(tid, abi, args, &laddr))
				return false;
			args += wsize;
			if (!pink_read_vm_object(tid, abi, args, &laddrlen))
				return false;
			addr = laddr;
			addrlen = laddrlen;
		} else {
			_pink_assert_not_reached();
		}
	}

	if (addr == 0) {
		sockaddr->family = -1;
		sockaddr->length = 0;
		return true;
	}
	if (addrlen < 2 || (unsigned long)addrlen > sizeof(sockaddr->u))
		addrlen = sizeof(sockaddr->u);

	memset(&sockaddr->u, 0, sizeof(sockaddr->u));
	if (!pink_read_vm_data(tid, abi, addr, sockaddr->u.pad, addrlen))
		return false;
	sockaddr->u.pad[sizeof(sockaddr->u.pad) - 1] = '\0';

	sockaddr->family = sockaddr->u.sa.sa_family;
	sockaddr->length = addrlen;

	return true;
}
