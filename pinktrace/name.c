/*
 * Copyright (c) 2010, 2011, 2012, 2013, 2021 Ali Polatel <alip@exherbo.org>
 * Based in part upon strace which is:
 *   Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 *   Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 *   Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 *   Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 *   Copyright (c) 1999 IBM Deutschland Entwicklung GmbH, IBM Corporation
 *                       Linux for s390 port by D.J. Barrow
 *                      <barrow_dj@mail.yahoo.com,djbarrow@de.ibm.com>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <pinktrace/private.h>
#include <pinktrace/pink.h>

const char *const errnoent0[] = {
#include "errnoent.h"
};
const char *const signalent0[] = {
#include "signalent.h"
};
const char *const sysent0[] = {
#include "syscallent.h"
};

#if PINK_ABIS_SUPPORTED > 1
const char *const errnoent1[] = {
# include "errnoent1.h"
};
const char *const signalent1[] = {
# include "signalent1.h"
};
const char *const sysent1[] = {
# include "syscallent1.h"
};
#endif

#if PINK_ABIS_SUPPORTED > 2
const char *const errnoent2[] = {
# include "errnoent2.h"
};
const char *const signalent2[] = {
# include "signalent2.h"
};
const char *const sysent2[] = {
#include "syscallent2.h"
};
#endif

const size_t nerrnos0 = ARRAY_SIZE(errnoent0);
const size_t nsignals0 = ARRAY_SIZE(signalent0);
const size_t nsyscalls0 = ARRAY_SIZE(sysent0);
#if PINK_ABIS_SUPPORTED > 1
const size_t nerrnos1 = ARRAY_SIZE(errnoent1);
const size_t nsignals1 = ARRAY_SIZE(signalent1);
const size_t nsyscalls1 = ARRAY_SIZE(sysent1);
# if PINK_ABIS_SUPPORTED > 2
const size_t nerrnos2 = ARRAY_SIZE(errnoent2);
const size_t nsignals2 = ARRAY_SIZE(signalent2);
const size_t nsyscalls2 = ARRAY_SIZE(sysent2);
# endif
#endif

static const char *const *sysent_vec[PINK_ABIS_SUPPORTED] = {
	sysent0,
#if PINK_ABIS_SUPPORTED > 1
	sysent1,
#endif
#if PINK_ABIS_SUPPORTED > 2
	sysent2,
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
	const char *str;
	int val;
};

static const struct xlat events[] = {
	{"FORK",	PINK_EVENT_FORK},
	{"VFORK",	PINK_EVENT_VFORK},
	{"CLONE",	PINK_EVENT_CLONE},
	{"EXEC",	PINK_EVENT_EXEC},
	{"VFORK_DONE",	PINK_EVENT_VFORK_DONE},
	{"EXIT",	PINK_EVENT_EXIT},
	{"SECCOMP",	PINK_EVENT_SECCOMP},
	{"STOP",	PINK_EVENT_STOP},
	{NULL,		0},
};

static const struct xlat socket_subcalls[] = {
	{"bind",		PINK_SOCKET_SUBCALL_BIND},
	{"connect",		PINK_SOCKET_SUBCALL_CONNECT},
	{"listen",		PINK_SOCKET_SUBCALL_LISTEN},
	{"accept",		PINK_SOCKET_SUBCALL_ACCEPT},
	{"getsockname",		PINK_SOCKET_SUBCALL_GETSOCKNAME},
	{"getpeername",		PINK_SOCKET_SUBCALL_GETPEERNAME},
	{"socketpair",		PINK_SOCKET_SUBCALL_SOCKETPAIR},
	{"send",		PINK_SOCKET_SUBCALL_SEND},
	{"recv",		PINK_SOCKET_SUBCALL_RECV},
	{"sendto",		PINK_SOCKET_SUBCALL_SENDTO},
	{"recvfrom",		PINK_SOCKET_SUBCALL_RECVFROM},
	{"shutdown",		PINK_SOCKET_SUBCALL_SHUTDOWN},
	{"setsockopt",		PINK_SOCKET_SUBCALL_SETSOCKOPT},
	{"getsockopt",		PINK_SOCKET_SUBCALL_GETSOCKOPT},
	{"sendmsg",		PINK_SOCKET_SUBCALL_SENDMSG},
	{"recvmsg",		PINK_SOCKET_SUBCALL_RECVMSG},
	{"accept4",		PINK_SOCKET_SUBCALL_ACCEPT4},
	{NULL,			0},
};

static const struct xlat addrfams[] = {
#ifdef AF_ALG
	{ "AF_ALG",		AF_ALG},
#endif
#ifdef AF_APPLETALK
	{ "AF_APPLETALK",	AF_APPLETALK},
#endif
#ifdef AF_ASH
	{ "AF_ASH",	AF_ASH},
#endif
#ifdef AF_ATMPVC
	{ "AF_ATMPVC",	AF_ATMPVC},
#endif
#ifdef AF_ATMSVC
	{ "AF_ATMSVC",	AF_ATMSVC},
#endif
#ifdef AF_AX25
	{ "AF_AX25",	AF_AX25},
#endif
#ifdef AF_BLUETOOTH
	{ "AF_BLUETOOTH",	AF_BLUETOOTH},
#endif
#ifdef AF_BRIDGE
	{ "AF_BRIDGE",	AF_BRIDGE},
#endif
#ifdef AF_CAIF
	{ "AF_CAIF",	AF_CAIF},
#endif
#ifdef AF_CAN
	{ "AF_CAN",	AF_CAN},
#endif
#ifdef AF_DECnet
	{ "AF_DECnet",	AF_DECnet},
#endif
#ifdef AF_ECONET
	{ "AF_ECONET",	AF_ECONET},
#endif
#ifdef AF_FILE
	{ "AF_FILE",	AF_FILE},
#endif
#ifdef AF_IB
	{ "AF_IB",	AF_IB},
#endif
#ifdef AF_IEEE802154
	{ "AF_IEEE802154",	AF_IEEE802154},
#endif
#ifdef AF_IMPLINK
	{ "AF_IMPLINK",	AF_IMPLINK},
#endif
#ifdef AF_INET
	{ "AF_INET",	AF_INET},
#endif
#ifdef AF_INET6
	{ "AF_INET6",	AF_INET6},
#endif
#ifdef AF_IPX
	{ "AF_IPX",	AF_IPX},
#endif
#ifdef AF_IRDA
	{ "AF_IRDA",	AF_IRDA},
#endif
#ifdef AF_ISDN
	{ "AF_ISDN",	AF_ISDN},
#endif
#ifdef AF_ISO
	{ "AF_ISO",	AF_ISO},
#endif
#ifdef AF_IUCV
	{ "AF_IUCV",	AF_IUCV},
#endif
#ifdef AF_KCM
	{ "AF_KCM",	AF_KCM},
#endif
#ifdef AF_KEY
	{ "AF_KEY",	AF_KEY},
#endif
#ifdef AF_LLC
	{ "AF_LLC",	AF_LLC},
#endif
#ifdef AF_LOCAL
	{ "AF_LOCAL",	AF_LOCAL},
#endif
#ifdef AF_MAX
	{ "AF_MAX",	AF_MAX},
#endif
#ifdef AF_MPLS
	{ "AF_MPLS",	AF_MPLS},
#endif
#ifdef AF_NETBEUI
	{ "AF_NETBEUI",	AF_NETBEUI},
#endif
#ifdef AF_NETLINK
	{ "AF_NETLINK",	AF_NETLINK},
#endif
#ifdef AF_NETROM
	{ "AF_NETROM",	AF_NETROM},
#endif
#ifdef AF_NFC
	{ "AF_NFC",	AF_NFC},
#endif
#ifdef AF_PACKET
	{ "AF_PACKET",	AF_PACKET},
#endif
#ifdef AF_PHONET
	{ "AF_PHONET",	AF_PHONET},
#endif
#ifdef AF_PPPOX
	{ "AF_PPPOX",	AF_PPPOX},
#endif
#ifdef AF_QIPCRTR
	{ "AF_QIPCRTR",	AF_QIPCRTR},
#endif
#ifdef AF_RDS
	{ "AF_RDS",	AF_RDS},
#endif
#ifdef AF_ROSE
	{ "AF_ROSE",	AF_ROSE},
#endif
#ifdef AF_ROUTE
	{ "AF_ROUTE",	AF_ROUTE},
#endif
#ifdef AF_RXRPC
	{ "AF_RXRPC",	AF_RXRPC},
#endif
#ifdef AF_SECURITY
	{ "AF_SECURITY",	AF_SECURITY},
#endif
#ifdef AF_SNA
	{ "AF_SNA",	AF_SNA},
#endif
#ifdef AF_SMC
	{ "AF_SMC",	AF_SMC},
#endif
#ifdef AF_TIPC
	{ "AF_TIPC",	AF_TIPC},
#endif
#ifdef AF_UNIX
	{ "AF_UNIX",	AF_UNIX},
#endif
#ifdef AF_UNSPEC
	{ "AF_UNSPEC",	AF_UNSPEC},
#endif
#ifdef AF_VSOCK
	{ "AF_VSOCK",	AF_VSOCK},
#endif
#ifdef AF_WANPIPE
	{ "AF_WANPIPE",	AF_WANPIPE},
#endif
#ifdef AF_X25
	{ "AF_X25",	AF_X25},
#endif
#ifdef AF_XDP
	{ "AF_XDP",	AF_XDP},
#endif
	{ NULL,		0},
};

/*
 * Shuffle syscall numbers so that we don't have huge gaps in syscall table.
 * The shuffling should be an involution: shuffle_scno(shuffle_scno(n)) == n.
 */
#if defined(ARM) || defined(AARCH64) /* So far only 32-bit ARM needs this */
static long
shuffle_scno(unsigned long scno)
{
	if (scno < ARM_FIRST_SHUFFLED_SYSCALL)
		return scno;

	/* __ARM_NR_cmpxchg? Swap with LAST_ORDINARY+1 */
	if (scno == ARM_FIRST_SHUFFLED_SYSCALL)
		return 0x000ffff0;
	if (scno == 0x000ffff0)
		return ARM_FIRST_SHUFFLED_SYSCALL;

#define ARM_SECOND_SHUFFLED_SYSCALL (ARM_FIRST_SHUFFLED_SYSCALL + 1)
	/*
	 * Is it ARM specific syscall?
	 * Swap [0x000f0000, 0x000f0000 + LAST_SPECIAL] range
	 * with [SECOND_SHUFFLED, SECOND_SHUFFLED + LAST_SPECIAL] range.
	 */
	if (scno >= 0x000f0000 &&
	    scno <= 0x000f0000 + ARM_LAST_SPECIAL_SYSCALL) {
		return scno - 0x000f0000 + ARM_SECOND_SHUFFLED_SYSCALL;
	}
	if (scno <= ARM_SECOND_SHUFFLED_SYSCALL + ARM_LAST_SPECIAL_SYSCALL) {
		return scno + 0x000f0000 - ARM_SECOND_SHUFFLED_SYSCALL;
	}

	return scno;
}
#else
# define shuffle_scno(scno) ((long)(scno))
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
	const size_t nsyscall_vec[PINK_ABIS_SUPPORTED] = {
		nsyscalls0,
#if PINK_ABIS_SUPPORTED > 1
		nsyscalls1,
#endif
#if PINK_ABIS_SUPPORTED > 2
		nsyscalls2,
#endif
	};

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
	const size_t nsyscall_vec[PINK_ABIS_SUPPORTED] = {
		nsyscalls0,
#if PINK_ABIS_SUPPORTED > 1
		nsyscalls1,
#endif
#if PINK_ABIS_SUPPORTED > 2
		nsyscalls2,
#endif
	};

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
	const size_t nerrno_vec[PINK_ABIS_SUPPORTED] = {
		nerrnos0,
#if PINK_ABIS_SUPPORTED > 1
		nerrnos1,
#endif
#if PINK_ABIS_SUPPORTED > 2
		nerrnos2,
#endif
	};

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
	const size_t nerrno_vec[PINK_ABIS_SUPPORTED] = {
		nerrnos0,
#if PINK_ABIS_SUPPORTED > 1
		nerrnos1,
#endif
#if PINK_ABIS_SUPPORTED > 2
		nerrnos2,
#endif
	};

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
	const size_t nsignal_vec[PINK_ABIS_SUPPORTED] = {
		nsignals0,
#if PINK_ABIS_SUPPORTED > 1
		nsignals1,
#endif
#if PINK_ABIS_SUPPORTED > 2
		nsignals2,
#endif
	};

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
	const size_t nsignal_vec[PINK_ABIS_SUPPORTED] = {
		nsignals0,
#if PINK_ABIS_SUPPORTED > 1
		nsignals1,
#endif
#if PINK_ABIS_SUPPORTED > 2
		nsignals2,
#endif
	};

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
