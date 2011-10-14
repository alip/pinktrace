/*
 * Copyright (c) 2010, 2011 Ali Polatel <alip@exherbo.org>
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

#include <stdio.h> /* NULL */
#include <string.h> /* strcmp() */

#include <pinktrace/pink.h>

#include "freebsd/pink-syscallent.h"

static int nsys = sizeof(sysnames) / sizeof(sysnames[0]);

const char *
pink_name_syscall(long scno, pink_bitness_t bitness)
{
	if (PINK_GCC_UNLIKELY(bitness != PINK_BITNESS_32))
		return NULL;
	if (PINK_GCC_UNLIKELY(scno < 0 || scno >= nsys))
		return NULL;
	return sysnames[scno];
}

long
pink_name_lookup(const char *name, pink_bitness_t bitness)
{
	long scno;

	if (PINK_GCC_UNLIKELY(bitness != PINK_BITNESS_32))
		return -1;
	if (PINK_GCC_UNLIKELY(name == NULL || name[0] == '\0'))
		return -1;

	for (scno = 0; scno < nsys; scno++) {
		if (!strcmp(sysnames[scno], name))
			return scno;
	}

	return -1;
}

long
pink_name_lookup_with_length(const char *name, size_t length, pink_bitness_t bitness)
{
	long scno;

	if (PINK_GCC_UNLIKELY(bitness != PINK_BITNESS_32))
		return -1;
	if (PINK_GCC_UNLIKELY(name == NULL || name[0] == '\0'))
		return -1;

	for (scno = 0; scno < nsys; scno++) {
		if (!strncmp(sysnames[scno], name, length))
			return scno;
	}

	return -1;
}
