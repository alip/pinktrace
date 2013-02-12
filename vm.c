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
 *   Copyright (c) 2000 PocketPenguins Inc.  Linux for Hitachi SuperH
 *                      port by Greg Banks <gbanks@pocketpenguins.com>
 * Based in part upon truss which is:
 *   Copyright (c) 1997 Sean Eric Fagan
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

PINK_GCC_ATTR((nonnull(3)))
static int _pink_vm_setup_addr(long addr, enum pink_abi abi, long *res)
{
#if PINK_ABIS_SUPPORTED > 1
	int r;
	size_t wsize;

	if ((r = pink_abi_wordsize(abi, &wsize)) < 0)
		return r;

	if (wsize < sizeof(addr))
		addr &= (1ul << 8 * wsize) - 1;
#endif
	*res = addr;
	return 0;
}

PINK_GCC_ATTR((nonnull(4)))
ssize_t pink_vm_lread(pid_t tid, enum pink_abi abi, long addr,
		      char *dest, size_t len)
{
	int n, m, r;
	long myaddr;
	bool started;
	union {
		long val;
		char x[sizeof(long)];
	} u;
	ssize_t count_read;

	if ((r = _pink_vm_setup_addr(addr, abi, &myaddr)) < 0)
		return r;

	started = false;
	count_read = 0;
	if (myaddr & (sizeof(long) - 1)) {
		/* myaddr not a multiple of sizeof(long) */
		n = myaddr - (myaddr & -sizeof(long)); /* residue */
		myaddr &= -sizeof(long); /* residue */
		if ((r = pink_read_word_data(tid, myaddr, &u.val)) < 0) {
			/* Not started yet, thus we had a bogus address. */
			errno = -r;
			return -1;
		}
		started = true;
		m = MIN(sizeof(long) - n, len);
		memcpy(dest, &u.x[n], m);
		myaddr += sizeof(long), dest += m, len -= m, count_read += m;
	}
	while (len > 0) {
		if ((r = pink_read_word_data(tid, myaddr, &u.val)) < 0) {
			errno = -r;
			return started ? count_read : -1;
		}
		started = true;
		m = MIN(sizeof(long), len);
		memcpy(dest, u.x, m);
		myaddr += sizeof(long), dest += m, len -= m, count_read += m;
	}
	return count_read;
}

PINK_GCC_ATTR((nonnull(4)))
ssize_t pink_vm_lread_nul(pid_t tid, enum pink_abi abi, long addr,
			  char *dest, size_t len)
{
	unsigned i;
	int n, m, r;
	long myaddr;
	bool started;
	union {
		long val;
		char x[sizeof(long)];
	} u;
	ssize_t count_read;

	if ((r = _pink_vm_setup_addr(addr, abi, &myaddr)) < 0)
		return r;

	started = false;
	count_read = 0;
	if (myaddr & (sizeof(long) - 1)) {
		/* myaddr not a multiple of sizeof(long) */
		n = myaddr - (myaddr & -sizeof(long)); /* residue */
		myaddr &= -sizeof(long); /* residue */
		if ((r = pink_read_word_data(tid, myaddr, &u.val)) < 0) {
			/* Not started yet, thus we had a bogus address. */
			errno = -r;
			return -1;
		}
		started = true;
		m = MIN(sizeof(long) - n, len);
		memcpy(dest, &u.x[n], m);
		while (n & (sizeof(long) - 1))
			if (u.x[n++] == '\0')
				return m;
		myaddr += sizeof(long), dest += m, len -= m;
		count_read += m;
	}
	while (len > 0) {
		if ((r = pink_read_word_data(tid, myaddr, &u.val)) < 0) {
			errno = -r;
			return started ? count_read : -1;
		}
		started = true;
		m = MIN(sizeof(long), len);
		memcpy(dest, u.x, m);
		for (i = 0; i < sizeof(long); i++)
			if (u.x[i] == '\0')
				return count_read + i;
		myaddr += sizeof(long), dest += m, len -= m;
		count_read += m;
	}
	return count_read;
}

PINK_GCC_ATTR((nonnull(4)))
ssize_t pink_vm_lwrite(pid_t tid, enum pink_abi abi, long addr,
		       const char *src, size_t len)
{
	int r;
	long myaddr;
	bool started;
	int n, m;
	union {
		long val;
		char x[sizeof(long)];
	} u;
	ssize_t count_written;

	if ((r = _pink_vm_setup_addr(addr, abi, &myaddr)) < 0)
		return r;

	started = false;
	count_written = 0;
	if (myaddr & (sizeof(long) - 1)) {
		/* myaddr not a multiple of sizeof(long) */
		n = myaddr - (myaddr & - sizeof(long)); /* residue */
		myaddr &= -sizeof(long); /* residue */
		m = MIN(sizeof(long) - n, len);
		memcpy(u.x, &src[n], m);
		if ((r = pink_write_word_data(tid, myaddr, u.val)) < 0) {
			/* Not started yet, thus we had a bogus address. */
			errno = -r;
			return -1;
		}
		started = true;
		myaddr += sizeof(long), src += m, len -= m, count_written += m;
	}
	while (len > 0) {
		m = MIN(sizeof(long), len);
		memcpy(u.x, src, m);
		if ((r = pink_write_word_data(tid, myaddr, u.val)) < 0) {
			errno = -r;
			return started ? count_written : -1;
		}
		started = true;
		myaddr += sizeof(long), src += m, len -= m, count_written += m;
	}

	return count_written;
}


static ssize_t _pink_process_vm_readv(pid_t tid,
				      const struct iovec *local_iov,
				      unsigned long liovcnt,
				      const struct iovec *remote_iov,
				      unsigned long riovcnt,
				      unsigned long flags)
{
	ssize_t r;
#if defined(HAVE_PROCESS_VM_READV)
	r = process_vm_readv(tid,
			     local_iov, liovcnt,
			     remote_iov, riovcnt,
			     flags);
#elif defined(__NR_process_vm_readv)
	r = syscall(__NR_process_vm_readv, (long)tid,
		    local_iov, liovcnt,
		    remote_iov, riovcnt, flags);
#else
	errno = ENOSYS;
	return -1;
#endif
	return r;
}

#if defined(PINK_HAVE_PROCESS_VM_READV)
# define process_vm_readv _pink_process_vm_readv
#else
# define process_vm_readv(...) (errno = ENOSYS, -1)
#endif

PINK_GCC_ATTR((nonnull(4)))
ssize_t pink_vm_cread(pid_t tid, enum pink_abi abi, long addr,
		      char *dest, size_t len)
{
	int r;
	long myaddr;
	struct iovec local[1], remote[1];

	if ((r = _pink_vm_setup_addr(addr, abi, &myaddr)) < 0)
		return r;

	local[0].iov_base = dest;
	remote[0].iov_base = (void *)myaddr;
	local[0].iov_len = remote[0].iov_len = len;

	return process_vm_readv(tid, local, 1, remote, 1, /*flags:*/0);
}

PINK_GCC_ATTR((nonnull(4)))
ssize_t pink_vm_cread_nul(pid_t tid, enum pink_abi abi, long addr,
			  char *dest, size_t len)
{
	int r;
	long myaddr;
	bool started;
	ssize_t count_read;
	struct iovec local[1], remote[1];

	if ((r = _pink_vm_setup_addr(addr, abi, &myaddr)) < 0)
		return r;

	started = false;
	count_read = 0;
	local[0].iov_base = dest;
	remote[0].iov_base = (void *)myaddr;

	while (len > 0) {
		int end_in_page;
		int r;
		int chunk_len;
		char *p;

		/* Don't read kilobytes: most strings are short */
		chunk_len = len;
		if (chunk_len > 256)
			chunk_len = 256;
		/* Don't cross pages. I guess otherwise we can get EFAULT
		 * and fail to notice that terminating NUL lies
		 * in the existing (first) page.
		 * (I hope there aren't arches with pages < 4K)
		 */
		end_in_page = ((myaddr + chunk_len) & 4095);
		r = chunk_len - end_in_page;
		if (r > 0) /* if chunk_len > end_in_page */
			chunk_len = r; /* chunk_len -= end_in_page */

		local[0].iov_len = remote[0].iov_len = chunk_len;
		if ((r = process_vm_readv(tid, local, 1, remote, 1, /*flags:*/ 0)) < 0)
			return started ? count_read : -1;
		started = true;
		count_read += r;

		p = memchr(local[0].iov_base, '\0', r);
		if (p != NULL)
			return count_read + (p - (char *)local[0].iov_base);
		local[0].iov_base = (char *)local[0].iov_base + r;
		remote[0].iov_base = (char *)remote[0].iov_base + r;
		len -= r;
	}
	return count_read;
}

static ssize_t _pink_process_vm_writev(pid_t tid,
				       const struct iovec *local_iov,
				       unsigned long liovcnt,
				       const struct iovec *remote_iov,
				       unsigned long riovcnt,
				       unsigned long flags)
{
	ssize_t r;
#ifdef HAVE_PROCESS_VM_WRITEV
	r = process_vm_writev(tid,
			      local_iov, liovcnt,
			      remote_iov, riovcnt,
			      flags);
#elif defined(__NR_process_vm_writev)
	r = syscall(__NR_process_vm_writev, (long)tid,
		    local_iov, liovcnt,
		    remote_iov, riovcnt,
		    flags);
#else
	errno = ENOSYS;
	return -1;
#endif
	return r;
}

#if defined(PINK_HAVE_PROCESS_VM_WRITEV)
# define process_vm_writev _pink_process_vm_writev
#else
# define process_vm_writev(...) (errno = ENOSYS, -1)
#endif


PINK_GCC_ATTR((nonnull(4)))
ssize_t pink_vm_cwrite(pid_t tid, enum pink_abi abi, long addr,
		       const char *src, size_t len)
{
	int r;
	long myaddr;
	struct iovec local[1], remote[1];

	if ((r = _pink_vm_setup_addr(addr, abi, &myaddr)) < 0)
		return r;

	local[0].iov_base = (void *)src;
	remote[0].iov_base = (void *)myaddr;
	local[0].iov_len = remote[0].iov_len = len;

	return process_vm_writev(tid, local, 1, remote, 1, /*flags:*/ 0);
}
