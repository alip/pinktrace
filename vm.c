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

static inline long setup_addr(struct pink_process *tracee, long addr)
{
#if PINK_ABIS_SUPPORTED > 1 && SIZEOF_LONG > 4
	size_t wsize;

	wsize = pink_abi_wordsize(pink_process_get_abi(tracee));
	if (wsize < sizeof(addr))
		addr &= (1ul << 8 * wsize) - 1;
#endif
	return addr;
}

PINK_GCC_ATTR((nonnull(3)))
ssize_t pink_vm_lread(struct pink_process *tracee, long addr, char *dest, size_t len)
{
	int n, m, r;
	union {
		long val;
		char x[sizeof(long)];
	} u;
	ssize_t count_read;

	addr = setup_addr(tracee, addr);
	count_read = 0;
	if (addr & (sizeof(long) - 1)) {
		/* addr not a multiple of sizeof(long) */
		n = addr - (addr & -sizeof(long)); /* residue */
		addr &= -sizeof(long); /* residue */
		if ((r = pink_read_word_data(tracee->pid, addr, &u.val)) < 0) {
			/* Not started yet, thus we had a bogus address. */
			errno = -r;
			return count_read > 0 ? count_read : -1;
		}
		m = MIN(sizeof(long) - n, len);
		memcpy(dest, &u.x[n], m);
		addr += sizeof(long), dest += m, len -= m, count_read += m;
	}
	while (len > 0) {
		if ((r = pink_read_word_data(tracee->pid, addr, &u.val)) < 0) {
			errno = -r;
			return count_read > 0 ? count_read : -1;
		}
		m = MIN(sizeof(long), len);
		memcpy(dest, u.x, m);
		addr += sizeof(long), dest += m, len -= m, count_read += m;
	}
	return count_read;
}

PINK_GCC_ATTR((nonnull(3)))
ssize_t pink_vm_lread_nul(struct pink_process *tracee, long addr, char *dest, size_t len)
{
	unsigned i;
	int n, m, r;
	union {
		long val;
		char x[sizeof(long)];
	} u;
	ssize_t count_read;

	addr = setup_addr(tracee, addr);
	count_read = 0;
	if (addr & (sizeof(long) - 1)) {
		/* addr not a multiple of sizeof(long) */
		n = addr - (addr & -sizeof(long)); /* residue */
		addr &= -sizeof(long); /* residue */
		if ((r = pink_read_word_data(tracee->pid, addr, &u.val)) < 0) {
			/* Not started yet, thus we had a bogus address. */
			errno = -r;
			return -1;
		}
		m = MIN(sizeof(long) - n, len);
		memcpy(dest, &u.x[n], m);
		while (n & (sizeof(long) - 1))
			if (u.x[n++] == '\0')
				return count_read + m;
		addr += sizeof(long), dest += m, len -= m;
		count_read += m;
	}
	while (len > 0) {
		if ((r = pink_read_word_data(tracee->pid, addr, &u.val)) < 0) {
			errno = -r;
			return count_read > 0 ? count_read : -1;
		}
		m = MIN(sizeof(long), len);
		memcpy(dest, u.x, m);
		for (i = 0; i < sizeof(long); i++)
			if (u.x[i] == '\0')
				return count_read + i;
		addr += sizeof(long), dest += m, len -= m;
		count_read += m;
	}
	return count_read;
}

PINK_GCC_ATTR((nonnull(3)))
ssize_t pink_vm_lwrite(struct pink_process *tracee, long addr, const char *src, size_t len)
{
	int r;
	int n, m;
	union {
		long val;
		char x[sizeof(long)];
	} u;
	ssize_t count_written;

	addr = setup_addr(tracee, addr);
	count_written = 0;
	if (addr & (sizeof(long) - 1)) {
		/* addr not a multiple of sizeof(long) */
		n = addr - (addr & - sizeof(long)); /* residue */
		addr &= -sizeof(long); /* residue */
		m = MIN(sizeof(long) - n, len);
		memcpy(u.x, &src[n], m);
		if ((r = pink_write_word_data(tracee->pid, addr, u.val)) < 0) {
			/* Not started yet, thus we had a bogus address. */
			errno = -r;
			return -1;
		}
		addr += sizeof(long), src += m, len -= m, count_written += m;
	}
	while (len > 0) {
		m = MIN(sizeof(long), len);
		memcpy(u.x, src, m);
		if ((r = pink_write_word_data(tracee->pid, addr, u.val)) < 0) {
			errno = -r;
			return count_written > 0 ? count_written : -1;
		}
		addr += sizeof(long), src += m, len -= m, count_written += m;
	}

	return count_written;
}


static ssize_t _pink_process_vm_readv(pid_t pid,
				      const struct iovec *local_iov,
				      unsigned long liovcnt,
				      const struct iovec *remote_iov,
				      unsigned long riovcnt,
				      unsigned long flags)
{
	ssize_t r;
#if defined(HAVE_PROCESS_VM_READV)
	r = process_vm_readv(pid,
			     local_iov, liovcnt,
			     remote_iov, riovcnt,
			     flags);
#elif defined(__NR_process_vm_readv)
	r = syscall(__NR_process_vm_readv, (long)pid,
		    local_iov, liovcnt,
		    remote_iov, riovcnt, flags);
#else
	errno = ENOSYS;
	return -1;
#endif
	return r;
}

#if PINK_HAVE_PROCESS_VM_READV
# define process_vm_readv _pink_process_vm_readv
#else
# define process_vm_readv(...) (errno = ENOSYS, -1)
#endif

PINK_GCC_ATTR((nonnull(3)))
ssize_t pink_vm_cread(struct pink_process *tracee, long addr, char *dest, size_t len)
{
	struct iovec local[1], remote[1];

	addr = setup_addr(tracee, addr);
	local[0].iov_base = dest;
	remote[0].iov_base = (void *)addr;
	local[0].iov_len = remote[0].iov_len = len;

	return process_vm_readv(tracee->pid, local, 1, remote, 1, /*flags:*/0);
}

PINK_GCC_ATTR((nonnull(3)))
ssize_t pink_vm_cread_nul(struct pink_process *tracee, long addr, char *dest, size_t len)
{
	ssize_t count_read;
	struct iovec local[1], remote[1];

	addr = setup_addr(tracee, addr);
	count_read = 0;
	local[0].iov_base = dest;
	remote[0].iov_base = (void *)addr;

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
		end_in_page = ((addr + chunk_len) & 4095);
		r = chunk_len - end_in_page;
		if (r > 0) /* if chunk_len > end_in_page */
			chunk_len = r; /* chunk_len -= end_in_page */

		local[0].iov_len = remote[0].iov_len = chunk_len;
		r = process_vm_readv(tracee->pid, local, 1, remote, 1, /*flags:*/ 0);
		if (r < 0)
			return count_read > 0 ? count_read : -1;

		p = memchr(local[0].iov_base, '\0', r);
		if (p != NULL)
			return count_read + (p - (char *)local[0].iov_base) + 1;
		local[0].iov_base = (char *)local[0].iov_base + r;
		remote[0].iov_base = (char *)remote[0].iov_base + r;
		len -= r, count_read += r;
	}
	return count_read;
}

static ssize_t _pink_process_vm_writev(pid_t pid,
				       const struct iovec *local_iov,
				       unsigned long liovcnt,
				       const struct iovec *remote_iov,
				       unsigned long riovcnt,
				       unsigned long flags)
{
	ssize_t r;
#if defined(HAVE_PROCESS_VM_WRITEV)
	r = process_vm_writev(pid,
			      local_iov, liovcnt,
			      remote_iov, riovcnt,
			      flags);
#elif defined(__NR_process_vm_writev)
	r = syscall(__NR_process_vm_writev, (long)pid,
		    local_iov, liovcnt,
		    remote_iov, riovcnt,
		    flags);
#else
	errno = ENOSYS;
	return -1;
#endif
	return r;
}

#if PINK_HAVE_PROCESS_VM_WRITEV
# define process_vm_writev _pink_process_vm_writev
#else
# define process_vm_writev(...) (errno = ENOSYS, -1)
#endif

PINK_GCC_ATTR((nonnull(3)))
ssize_t pink_vm_cwrite(struct pink_process *tracee, long addr, const char *src, size_t len)
{
	struct iovec local[1], remote[1];

	addr = setup_addr(tracee, addr);
	local[0].iov_base = (void *)src;
	remote[0].iov_base = (void *)addr;
	local[0].iov_len = remote[0].iov_len = len;

	return process_vm_writev(tracee->pid, local, 1, remote, 1, /*flags:*/ 0);
}
