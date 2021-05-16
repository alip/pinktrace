/*
 * Copyright (c) 2010, 2011, 2012, 2013, 2014, 2021 Ali Polatel <alip@exherbo.org>
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
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <pinktrace/private.h>
#include <pinktrace/pink.h>

PINK_GCC_ATTR((nonnull(2)))
static inline long setup_addr(pid_t pid, const struct pink_regset *regset,
			      long addr)
{
#if PINK_ABIS_SUPPORTED > 1 && SIZEOF_LONG > 4
	size_t wsize;

	wsize = pink_abi_wordsize(regset->abi);
	if (wsize < sizeof(addr))
		addr &= (1ul << 8 * wsize) - 1;
#endif
	return addr;
}

PINK_GCC_ATTR((nonnull(2,4)))
ssize_t pink_vm_lread(pid_t pid, const struct pink_regset *regset,
		      long addr, char *dest, size_t len)
{
	int r;
	unsigned int count_read = 0;
	unsigned int residue = addr & (sizeof(long) - 1);

	while (len) {
		addr &= -sizeof(long); /* aligned address */

		union {
			long val;
			char x[sizeof(long)];
		} u;
		if ((r = pink_read_word_data(pid, addr, &u.val)) < 0) {
			/* Not started yet: process is gone or address space is
			 * inacessible. */
			errno = -r;
			return -1;
		}

		unsigned int m = MIN(sizeof(long) - residue, len);
		memcpy(dest, &u.x[residue], m);
		residue = 0;
		addr += sizeof(long);
		dest += m;
		count_read += m;
		len -= m;
	}
	return count_read;
}

PINK_GCC_ATTR((nonnull(2,4)))
ssize_t pink_vm_lread_nul(pid_t pid, const struct pink_regset *regset,
			  long addr, char *dest, size_t len)
{
	int r;
	unsigned int count_read = 0;
	unsigned int residue = addr & (sizeof(long) - 1);
	const char *orig_dest = dest;

	while (len) {
		addr &= -sizeof(long); /* aligned address */

		union {
			long val;
			char x[sizeof(long)];
		} u;
		if ((r = pink_read_word_data(pid, addr, &u.val)) < 0) {
			/* Not started yet: process is gone or address space is
			 * inacessible. */
			errno = -r;
			return -1;
		}

		unsigned int m = MIN(sizeof(long) - residue, len);
		memcpy(dest, &u.x[residue], m);
		while (residue < sizeof(long))
			if (u.x[residue++] == '\0')
				return (dest - orig_dest) + residue - 1;
		residue = 0;
		addr += sizeof(long);
		dest += m;
		count_read += m;
		len -= m;
	}
	return count_read;
}

PINK_GCC_ATTR((nonnull(2,4)))
ssize_t pink_vm_lwrite(pid_t pid, const struct pink_regset *regset,
		       long addr, const char *src, size_t len)
{
	int r;
	unsigned int count_written = 0;
	unsigned int residue = addr & (sizeof(long) - 1);

	while (len) {
		addr &= -sizeof(long); /* aligned address */

		union {
			long val;
			char x[sizeof(long)];
		} u;
		unsigned int m = MIN(sizeof(long) - residue, len);
		memcpy(u.x, &src[residue], m);
		residue = 0;
		if ((r = pink_write_word_data(pid, addr, u.val)) < 0) {
			errno = -r;
			return count_written > 0 ? count_written : -1;
		}
		addr += sizeof(long);
		src += m;
		count_written += m;
		len -= m;
	}

	return count_written;
}

#if PINK_HAVE_PROCESS_VM_READV
static ssize_t _pink_process_vm_readv(pid_t pid,
				      const struct iovec *local_iov,
				      unsigned long liovcnt,
				      const struct iovec *remote_iov,
				      unsigned long riovcnt,
				      unsigned long flags)
{
	ssize_t r;
# if defined(HAVE_PROCESS_VM_READV)
	r = process_vm_readv(pid,
			     local_iov, liovcnt,
			     remote_iov, riovcnt,
			     flags);
# elif defined(__NR_process_vm_readv)
	r = syscall(__NR_process_vm_readv, (long)pid,
		    local_iov, liovcnt,
		    remote_iov, riovcnt, flags);
# else
	errno = ENOSYS;
	return -1;
# endif
	return r;
}

# define process_vm_readv _pink_process_vm_readv
#else
# define process_vm_readv(...) (errno = ENOSYS, -1)
#endif

PINK_GCC_ATTR((nonnull(2,4)))
ssize_t pink_vm_cread(pid_t pid, const struct pink_regset *regset,
		      long addr, char *dest, size_t len)
{
#if PINK_HAVE_PROCESS_VM_READV
	struct iovec local[1], remote[1];

	addr = setup_addr(pid, regset, addr);
	local[0].iov_base = dest;
	remote[0].iov_base = (void *)addr;
	local[0].iov_len = remote[0].iov_len = len;
#endif

	return process_vm_readv(pid, local, 1, remote, 1, /*flags:*/0);
}

PINK_GCC_ATTR((nonnull(2,4)))
ssize_t pink_vm_cread_nul(pid_t pid, const struct pink_regset *regset,
			  long addr, char *dest, size_t len)
{
	ssize_t count_read;
	struct iovec local[1], remote[1];

	addr = setup_addr(pid, regset, addr);
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
		r = process_vm_readv(pid, local, 1, remote, 1, /*flags:*/ 0);
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

#if PINK_HAVE_PROCESS_VM_WRITEV
static ssize_t _pink_process_vm_writev(pid_t pid,
				       const struct iovec *local_iov,
				       unsigned long liovcnt,
				       const struct iovec *remote_iov,
				       unsigned long riovcnt,
				       unsigned long flags)
{
	ssize_t r;
# if defined(HAVE_PROCESS_VM_WRITEV)
	r = process_vm_writev(pid,
			      local_iov, liovcnt,
			      remote_iov, riovcnt,
			      flags);
# elif defined(__NR_process_vm_writev)
	r = syscall(__NR_process_vm_writev, (long)pid,
		    local_iov, liovcnt,
		    remote_iov, riovcnt,
		    flags);
# else
	errno = ENOSYS;
	return -1;
# endif
	return r;
}

# define process_vm_writev _pink_process_vm_writev
#else
# define process_vm_writev(...) (errno = ENOSYS, -1)
#endif

PINK_GCC_ATTR((nonnull(2,4)))
ssize_t pink_vm_cwrite(pid_t pid, const struct pink_regset *regset,
		       long addr, const char *src, size_t len)
{
#if PINK_HAVE_PROCESS_VM_WRITEV
	struct iovec local[1], remote[1];

	addr = setup_addr(pid, regset, addr);
	local[0].iov_base = (void *)src;
	remote[0].iov_base = (void *)addr;
	local[0].iov_len = remote[0].iov_len = len;
#endif

	return process_vm_writev(pid, local, 1, remote, 1, /*flags:*/ 0);
}
