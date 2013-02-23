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
#include <stdio.h>
#include <string.h>

static const char *const sysnames0[] = {
#include "syscallent.h"
};
static int nsys0 = ARRAY_SIZE(sysnames0);

#if PINK_ABIS_SUPPORTED >= 2
static const char *const sysnames1[] = {
#include "syscallent1.h"
};
static int nsys1 = ARRAY_SIZE(sysnames1);
#endif

#if PINK_ABIS_SUPPORTED >= 3
static const char *const sysnames2[] = {
#include "syscallent2.h"
};
static int nsys2 = ARRAY_SIZE(sysnames2);
#endif

PINK_GCC_ATTR((pure))
const char *pink_syscall_name(long scno, short abi)
{
	int nsys;
	const char *const *names;

#if PINK_ABIS_SUPPORTED == 1
	nsys = nsys0;
	names = sysnames0;
#else
	switch (abi) {
	case 0:
		nsys = nsys0;
		names = sysnames0;
		break;
# if PINK_ABIS_SUPPORTED >= 2
	case 1:
		nsys = nsys1;
		names = sysnames1;
		break;
# endif
# if PINK_ABIS_SUPPORTED >= 3
	case 2:
		nsys = nsys2;
		names = sysnames2;
		break;
# endif
	default:
		return NULL;
	}
#endif

#ifdef SYSCALL_OFFSET
	scno -= SYSCALL_OFFSET;
#endif

	if (scno < 0 || scno >= nsys)
		return NULL;
	return names[scno];
}

PINK_GCC_ATTR((pure))
long pink_syscall_lookup(const char *name, short abi)
{
	int nsys;
	const char *const *names;
	long scno;

	if (!name || *name == '\0')
		return -1;

#if PINK_ABIS_SUPPORTED == 1
	nsys = nsys0;
	names = sysnames0;
#else
	switch (abi) {
	case 0:
		nsys = nsys0;
		names = sysnames0;
		break;
# if PINK_ABIS_SUPPORTED >= 2
	case 1:
		nsys = nsys1;
		names = sysnames1;
		break;
# endif
# if PINK_ABIS_SUPPORTED >= 3
	case 2:
		nsys = nsys2;
		names = sysnames2;
		break;
# endif
	default:
		return -1;
	}
#endif

	for (scno = 0; scno < nsys; scno++) {
		if (names[scno] && !strcmp(names[scno], name)) {
#ifdef SYSCALL_OFFSET
			return scno + SYSCALL_OFFSET;
#else
			return scno;
#endif
		}
	}

	return -1;
}
