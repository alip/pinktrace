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

static const char *const sysent0[] = {
#include "syscallent.h"
};
static const char *const errnoent0[] = {
#include "errnoent.h"
};
static const char *const signalent0[] = {
#include "signalent.h"
};
enum { nsyscalls0 = ARRAY_SIZE(sysent0) };
enum { nerrnos0 = ARRAY_SIZE(errnoent0) };
enum { nsignals0 = ARRAY_SIZE(signalent0) };

#if PINK_ABIS_SUPPORTED > 1
static const char *const sysent1[] = {
# include "syscallent1.h"
};
static const char *const errnoent1[] = {
# include "errnoent1.h"
};
static const char *const signalent1[] = {
# include "signalent1.h"
};
enum { nsyscalls1 = ARRAY_SIZE(sysent1) };
enum { nerrnos1 = ARRAY_SIZE(errnoent1) };
enum { nsignals1 = ARRAY_SIZE(signalent1) };
#endif

#if PINK_ABIS_SUPPORTED > 2
static const char *const sysent2[] = {
#include "syscallent2.h"
};
static const char *const errnoent2[] = {
# include "errnoent2.h"
};
static const char *const signalent2[] = {
# include "signalent2.h"
};
enum { nsyscalls2 = ARRAY_SIZE(sysent2) };
enum { nerrnos2 = ARRAY_SIZE(errnoent2) };
enum { nsignals2 = ARRAY_SIZE(signalent2) };
#endif

static const unsigned nsyscall_vec[PINK_ABIS_SUPPORTED] = {
	nsyscalls0,
#if PINK_ABIS_SUPPORTED > 1
	nsyscalls1,
#endif
#if PINK_ABIS_SUPPORTED > 2
	nsyscalls2,
#endif
};
static const char *const *sysent_vec[PINK_ABIS_SUPPORTED] = {
	sysent0,
#if PINK_ABIS_SUPPORTED > 1
	sysent1,
#endif
#if PINK_ABIS_SUPPORTED > 2
	sysent2,
#endif
};
static const unsigned nerrno_vec[PINK_ABIS_SUPPORTED] = {
	nerrnos0,
#if PINK_ABIS_SUPPORTED > 1
	nerrnos1,
#endif
#if PINK_ABIS_SUPPORTED > 2
	nerrnos2,
#endif
};
static const char *const *errnoent_vec[PINK_ABIS_SUPPORTED] = {
	errnoent0,
#if PINK_ABIS_SUPPORTED > 1
	errnoent1,
#endif
#if PINK_ABIS_SUPPORTED > 2
	errnoent2,
#endif
};
static const unsigned nsignal_vec[PINK_ABIS_SUPPORTED] = {
	nsignals0,
#if PINK_ABIS_SUPPORTED > 1
	nsignals1,
#endif
#if PINK_ABIS_SUPPORTED > 2
	nsignals2,
#endif
};
static const char *const *signalent_vec[PINK_ABIS_SUPPORTED] = {
	signalent0,
#if PINK_ABIS_SUPPORTED > 1
	signalent1,
#endif
#if PINK_ABIS_SUPPORTED > 2
	signalent2,
#endif
};

struct xlat {
	int val;
	const char *str;
};

static const struct xlat events[] = {
	{PINK_EVENT_FORK,	"FORK"},
	{PINK_EVENT_VFORK,	"VFORK"},
	{PINK_EVENT_CLONE,	"CLONE"},
	{PINK_EVENT_EXEC,	"EXEC"},
	{PINK_EVENT_VFORK_DONE,	"VFORK_DONE"},
	{PINK_EVENT_EXIT,	"EXIT"},
	{PINK_EVENT_SECCOMP,	"SECCOMP"},
	{PINK_EVENT_STOP,	"STOP"},
	{0,			NULL},
};

static const struct xlat socket_subcalls[] = {
	{PINK_SOCKET_SUBCALL_BIND,		"bind"},
	{PINK_SOCKET_SUBCALL_CONNECT,		"connect"},
	{PINK_SOCKET_SUBCALL_LISTEN,		"listen"},
	{PINK_SOCKET_SUBCALL_ACCEPT,		"accept"},
	{PINK_SOCKET_SUBCALL_GETSOCKNAME,	"getsockname"},
	{PINK_SOCKET_SUBCALL_GETPEERNAME,	"getpeername"},
	{PINK_SOCKET_SUBCALL_SOCKETPAIR,	"socketpair"},
	{PINK_SOCKET_SUBCALL_SEND,		"send"},
	{PINK_SOCKET_SUBCALL_RECV,		"recv"},
	{PINK_SOCKET_SUBCALL_SENDTO,		"sendto"},
	{PINK_SOCKET_SUBCALL_RECVFROM,		"recvfrom"},
	{PINK_SOCKET_SUBCALL_SHUTDOWN,		"shutdown"},
	{PINK_SOCKET_SUBCALL_SETSOCKOPT,	"setsockopt"},
	{PINK_SOCKET_SUBCALL_GETSOCKOPT,	"getsockopt"},
	{PINK_SOCKET_SUBCALL_SENDMSG,		"sendmsg"},
	{PINK_SOCKET_SUBCALL_RECVMSG,		"recvmsg"},
	{PINK_SOCKET_SUBCALL_ACCEPT4,		"accept4"},
	{0,					NULL},
};

static const struct xlat addrfams[] = {
#ifdef AF_APPLETALK
	{ AF_APPLETALK,	"AF_APPLETALK"	},
#endif
#ifdef AF_ASH
	{ AF_ASH,	"AF_ASH"	},
#endif
#ifdef AF_ATMPVC
	{ AF_ATMPVC,	"AF_ATMPVC"	},
#endif
#ifdef AF_ATMSVC
	{ AF_ATMSVC,	"AF_ATMSVC"	},
#endif
#ifdef AF_AX25
	{ AF_AX25,	"AF_AX25"	},
#endif
#ifdef AF_BLUETOOTH
	{ AF_BLUETOOTH,	"AF_BLUETOOTH"	},
#endif
#ifdef AF_BRIDGE
	{ AF_BRIDGE,	"AF_BRIDGE"	},
#endif
#ifdef AF_DECnet
	{ AF_DECnet,	"AF_DECnet"	},
#endif
#ifdef AF_ECONET
	{ AF_ECONET,	"AF_ECONET"	},
#endif
#ifdef AF_FILE
	{ AF_FILE,	"AF_FILE"	},
#endif
#ifdef AF_IMPLINK
	{ AF_IMPLINK,	"AF_IMPLINK"	},
#endif
#ifdef AF_INET
	{ AF_INET,	"AF_INET"	},
#endif
#ifdef AF_INET6
	{ AF_INET6,	"AF_INET6"	},
#endif
#ifdef AF_IPX
	{ AF_IPX,	"AF_IPX"	},
#endif
#ifdef AF_IRDA
	{ AF_IRDA,	"AF_IRDA"	},
#endif
#ifdef AF_ISO
	{ AF_ISO,	"AF_ISO"	},
#endif
#ifdef AF_KEY
	{ AF_KEY,	"AF_KEY"	},
#endif
#ifdef AF_UNIX
	{ AF_UNIX,	"AF_UNIX"	},
#endif
#ifdef AF_LOCAL
	{ AF_LOCAL,	"AF_LOCAL"	},
#endif
#ifdef AF_NETBEUI
	{ AF_NETBEUI,	"AF_NETBEUI"	},
#endif
#ifdef AF_NETLINK
	{ AF_NETLINK,	"AF_NETLINK"	},
#endif
#ifdef AF_NETROM
	{ AF_NETROM,	"AF_NETROM"	},
#endif
#ifdef AF_PACKET
	{ AF_PACKET,	"AF_PACKET"	},
#endif
#ifdef AF_PPPOX
	{ AF_PPPOX,	"AF_PPPOX"	},
#endif
#ifdef AF_ROSE
	{ AF_ROSE,	"AF_ROSE"	},
#endif
#ifdef AF_ROUTE
	{ AF_ROUTE,	"AF_ROUTE"	},
#endif
#ifdef AF_SECURITY
	{ AF_SECURITY,	"AF_SECURITY"	},
#endif
#ifdef AF_SNA
	{ AF_SNA,	"AF_SNA"	},
#endif
#ifdef AF_UNSPEC
	{ AF_UNSPEC,	"AF_UNSPEC"	},
#endif
#ifdef AF_WANPIPE
	{ AF_WANPIPE,	"AF_WANPIPE"	},
#endif
#ifdef AF_X25
	{ AF_X25,	"AF_X25"	},
#endif
	{ 0,		NULL		},
};

/* Shuffle syscall numbers so that we don't have huge gaps in syscall table.
 * The shuffling should be reversible: shuffle_scno(shuffle_scno(n)) == n.
 */
#if PINK_ARCH_ARM /* So far only ARM needs this */
static long shuffle_scno(unsigned long scno)
{
	if (scno <= ARM_LAST_ORDINARY_SYSCALL)
		return scno;

	/* __ARM_NR_cmpxchg? Swap with LAST_ORDINARY+1 */
	if (scno == 0x000ffff0)
		return ARM_LAST_ORDINARY_SYSCALL+1;
	if (scno == ARM_LAST_ORDINARY_SYSCALL+1)
		return 0x000ffff0;

	/* Is it ARM specific syscall?
	 * Swap with [LAST_ORDINARY+2, LAST_ORDINARY+2 + LAST_SPECIAL] range.
	 */
	if (scno >= 0x000f0000
	 && scno <= 0x000f0000 + ARM_LAST_SPECIAL_SYSCALL
	) {
		return scno - 0x000f0000 + (ARM_LAST_ORDINARY_SYSCALL+2);
	}
	if (/* scno >= ARM_LAST_ORDINARY_SYSCALL+2 - always true */ 1
	 && scno <= (ARM_LAST_ORDINARY_SYSCALL+2) + ARM_LAST_SPECIAL_SYSCALL
	) {
		return scno + 0x000f0000 - (ARM_LAST_ORDINARY_SYSCALL+2);
	}

	return scno;
}
#else
# define shuffle_scno(scno) (long)(scno)
#endif

PINK_GCC_ATTR((pure))
static const char *xname(const struct xlat *xlat, int val)
{
	for (; xlat->str != NULL; xlat++)
		if (xlat->val == val)
			return xlat->str;
	return NULL;
}

PINK_GCC_ATTR((pure))
static int xlookup(const struct xlat *xlat, const char *str)
{
	if (!str || *str == '\0')
		return -1;

	for (; xlat->str != NULL; xlat++)
		if (!strcmp(str, xlat->str))
			return xlat->val;
	return -1;
}

PINK_GCC_ATTR((pure))
const char *pink_name_event(enum pink_event event)
{
	return xname(events, event);
}

PINK_GCC_ATTR((pure))
int pink_lookup_event(const char *name)
{
	return xlookup(events, name);
}

PINK_GCC_ATTR((pure))
const char *pink_name_socket_family(int family)
{
	return xname(addrfams, family);
}

PINK_GCC_ATTR((pure))
int pink_lookup_socket_family(const char *name)
{
	return xlookup(addrfams, name);
}

PINK_GCC_ATTR((pure))
const char *pink_name_socket_subcall(enum pink_socket_subcall subcall)
{
	return xname(socket_subcalls, subcall);
}

PINK_GCC_ATTR((pure))
int pink_lookup_socket_subcall(const char *name)
{
	return xlookup(socket_subcalls, name);
}

PINK_GCC_ATTR((pure))
const char *pink_name_syscall(long scno, short abi)
{
	int nsyscalls;
	const char *const *sysent;

	if (abi < 0 || abi >= PINK_ABIS_SUPPORTED)
		return NULL;

	nsyscalls = nsyscall_vec[abi];
	sysent = sysent_vec[abi];
#ifdef SYSCALL_OFFSET
	scno -= SYSCALL_OFFSET;
#endif

	scno = shuffle_scno(scno);
	if (scno < 0 || scno >= nsyscalls)
		return NULL;
	return sysent[scno];
}

PINK_GCC_ATTR((pure))
long pink_lookup_syscall(const char *name, short abi)
{
	int nsyscalls;
	const char *const *sysent;
	long scno;

	if (!name || *name == '\0')
		return -1;
	if (abi < 0 || abi >= PINK_ABIS_SUPPORTED)
		return -1;

	nsyscalls = nsyscall_vec[abi];
	sysent = sysent_vec[abi];

	for (scno = 0; scno < nsyscalls; scno++) {
		if (sysent[scno] && !strcmp(sysent[scno], name)) {
#ifdef SYSCALL_OFFSET
			return scno + SYSCALL_OFFSET;
#else
			return shuffle_scno(scno);
#endif
		}
	}

	return -1;
}

PINK_GCC_ATTR((pure))
const char *pink_name_errno(int err_no, short abi)
{
	int nerrnos;
	const char *const *errnoent;

	if (abi < 0 || abi >= PINK_ABIS_SUPPORTED)
		return NULL;

	nerrnos = nerrno_vec[abi];
	errnoent = errnoent_vec[abi];

	if (err_no < 0 || err_no >= nerrnos)
		return NULL;
	return errnoent[err_no];
}

PINK_GCC_ATTR((pure))
int pink_lookup_errno(const char *name, short abi)
{
	int nerrnos;
	const char *const *errnoent;
	int err_no;

	if (!name || *name == '\0')
		return -1;
	if (abi < 0 || abi >= PINK_ABIS_SUPPORTED)
		return -1;

	nerrnos = nerrno_vec[abi];
	errnoent = errnoent_vec[abi];

	for (err_no = 0; err_no < nerrnos; err_no++) {
		if (errnoent[err_no] && !strcmp(errnoent[err_no], name))
			return err_no;
	}

	return -1;
}

PINK_GCC_ATTR((pure))
const char *pink_name_signal(int sig, short abi)
{
	int nsignals;
	const char *const *signalent;

	if (abi < 0 || abi >= PINK_ABIS_SUPPORTED)
		return NULL;

	nsignals = nsignal_vec[abi];
	signalent = signalent_vec[abi];

	if (sig < 0 || sig >= nsignals)
		return NULL;
	return signalent[sig];
}

PINK_GCC_ATTR((pure))
int pink_lookup_signal(const char *name, short abi)
{
	int nsignals;
	const char *const *signalent;
	int sig;

	if (!name || *name == '\0')
		return -1;
	if (abi < 0 || abi >= PINK_ABIS_SUPPORTED)
		return -1;

	nsignals = nsignal_vec[abi];
	signalent = signalent_vec[abi];

	for (sig = 0; sig < nsignals; sig++) {
		if (signalent[sig] && !strcmp(signalent[sig], name))
			return sig;
	}

	return -1;
}
