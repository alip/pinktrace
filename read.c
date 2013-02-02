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

int pink_read_word_user(pid_t tid, long off, long *res)
{
	long val;

	val = pink_ptrace(PTRACE_PEEKUSER, tid, (void *)off, NULL);
	if (val < 0)
		return -errno;
	if (res != NULL)
		*res = val;
	return 0;
}

int pink_read_word_data(pid_t tid, long off, long *res)
{
	long val;

	val = pink_ptrace(PTRACE_PEEKDATA, tid, (void *)off, NULL);
	if (val < 0)
		return -errno;
	if (res)
		*res = val;
	return 0;
}

int pink_read_abi(pid_t tid, const pink_regs_t *regs, enum pink_abi *abi)
{
	enum pink_abi abival;
#if PINK_ABIS_SUPPORTED == 1
	abival = 0;
#elif PINK_ARCH_X86_64 || PINK_ARCH_X32
	/* Check CS register value. On x86-64 linux it is:
	 *	0x33	for long mode (64 bit)
	 *	0x23	for compatibility mode (32 bit)
	 * Check DS register value. On x86-64 linux it is:
	 *	0x2b	for x32 mode (x86-64 in 32 bit)
	 */
	switch (regs->cs) {
	case 0x23:
		abival = 1;
		break;
	case 0x33:
		if (regs->ds == 0x2b) {
			abival = PINK_ABI_X32;
			break;
		}
		else {
#if PINK_ARCH_X86_64
			abival = 0;
			break;
#else /* PINK_ARCH_X32 */
			/* fall through */;
#endif
		}
	default:
		return -ENOTSUP;
	}
#elif PINK_ARCH_IA64
	/*
	 * 0 : ia64
	 * 1 : ia32
	 */
#	define IA64_PSR_IS	((long)1 << 34)
	int r;
	long psr;

	r = pink_read_word_user(pid, PT_CR_IPSR, &psr);
	if (r < 0)
		return r;
	abival = (psr & IA64_PSR_IS) ? 1 : 0;
#elif PINK_ARCH_ARM
	abival = (regs->ARM.cpsr & 0x20) ? 0 : 1;
#elif PINK_ARCH_POWERPC64
	/* SF is bit 0 of MSR (Machine State Register) */
	abival = (regs->msr & 0) ? 0 : 1;
#else
#error unsupported architecture
#endif
	*abi = abival;
	return 0;
}

static ssize_t _pink_process_vm_readv(pid_t tid,
				      const struct iovec *local_iov,
				      unsigned long liovcnt,
				      const struct iovec *remote_iov,
				      unsigned long riovcnt,
				      unsigned long flags)
{
	ssize_t r;
#ifdef HAVE_PROCESS_VM_READV
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

static ssize_t _pink_read_vm_data_ptrace(pid_t tid, long addr,
					 char *dest, size_t len)
{
	bool started;
	int n, m, r;
	union {
		long val;
		char x[sizeof(long)];
	} u;
	ssize_t count_read;

	started = false;
	count_read = 0;
	if (addr & (sizeof(long) - 1)) {
		/* addr not a multiple of sizeof(long) */
		n = addr - (addr & -sizeof(long)); /* residue */
		addr &= -sizeof(long); /* residue */
		r = pink_read_word_data(tid, addr, &u.val);
		if (r < 0) {
			/* Not started yet, thus we had a bogus address. */
			errno = -r;
			return -1;
		}
		started = true;
		m = MIN(sizeof(long) - n, len);
		memcpy(dest, &u.x[n], m);
		addr += sizeof(long), dest += m, len -= m, count_read += m;
	}
	while (len > 0) {
		r = pink_read_word_data(tid, addr, &u.val);
		if (r < 0) {
			errno = -r;
			return started ? count_read : -1;
		}
		started = true;
		m = MIN(sizeof(long), len);
		memcpy(dest, u.x, m);
		addr += sizeof(long), dest += m, len -= m, count_read += m;
	}

	return count_read;
}

#if PINK_HAVE_PROCESS_VM_READV
static bool _pink_process_vm_readv_not_supported = false;
#define process_vm_readv _pink_process_vm_readv
#else
static bool _pink_process_vm_readv_not_supported = true;
#define process_vm_readv(...) (errno = ENOSYS, -1)
#endif

PINK_GCC_ATTR((nonnull(4)))
ssize_t pink_read_vm_data(pid_t tid, enum pink_abi abi, long addr,
			  char *dest, size_t len)
{
#if PINK_ABIS_SUPPORTED > 1
	int errcond;
	size_t wsize;

	errcond = pink_abi_wordsize(abi, &wsize);
	if (errcond < 0) {
		errno = -errcond;
		return -1;
	}

	if (wsize < sizeof(addr))
		addr &= (1ul << 8 * wsize) - 1;
#endif

	if (!_pink_process_vm_readv_not_supported) {
		int r;
		struct iovec local[1], remote[1];

		local[0].iov_base = dest;
		remote[0].iov_base = (void *)addr;
		local[0].iov_len = remote[0].iov_len = len;

		r = process_vm_readv(tid,
				     local, 1,
				     remote, 1,
				     /*flags:*/0);
		if (r < 0 && errno == ENOSYS) {
			_pink_process_vm_readv_not_supported = true;
			goto vm_readv_didnt_work;
		}
		return r;
	}
vm_readv_didnt_work:
	return _pink_read_vm_data_ptrace(tid, addr, dest, len);
}

static ssize_t _pink_read_vm_data_nul_ptrace(pid_t tid, long addr,
					     char *dest, size_t len)
{
	unsigned i;
	int n, m, r;
	union {
		long val;
		char x[sizeof(long)];
	} u;
	ssize_t count_read;

	count_read = 0;
	if (addr & (sizeof(long) - 1)) {
		/* addr not a multiple of sizeof(long) */
		n = addr - (addr & -sizeof(long)); /* residue */
		addr &= -sizeof(long); /* residue */
		r = pink_read_word_data(tid, addr, &u.val);
		if (r < 0) {
			/* Not started yet, thus we had a bogus address. */
			errno = -r;
			return -1;
		}
		m = MIN(sizeof(long) - n, len);
		memcpy(dest, &u.x[n], m);
		while (n & (sizeof(long) - 1))
			if (u.x[n++] == '\0')
				return m;
		addr += sizeof(long), dest += m, len -= m;
		count_read += m;
	}
	while (len > 0) {
		r = pink_read_word_data(tid, addr, &u.val);
		if (r < 0) {
			errno = -r;
			return count_read;
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

PINK_GCC_ATTR((nonnull(4)))
ssize_t pink_read_vm_data_nul(pid_t tid, enum pink_abi abi, long addr,
			      char *dest, size_t len)
{
#if PINK_ABIS_SUPPORTED > 1
	int errcond;
	size_t wsize;

	errcond = pink_abi_wordsize(abi, &wsize);
	if (errcond < 0) {
		errno = -errcond;
		return -1;
	}

	if (wsize < sizeof(addr))
		addr &= (1ul << 8 * wsize) - 1;
#endif

	if (!_pink_process_vm_readv_not_supported) {
		ssize_t count_read;
		struct iovec local[1], remote[1];

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
			r = _pink_process_vm_readv(tid,
					local, 1,
					remote, 1,
					/*flags:*/ 0
			);
			if (r < 0) {
				if (errno == ENOSYS)
					_pink_process_vm_readv_not_supported = true;
				else if (errno == EINVAL || errno == ESRCH)
					return -1; /* is the process gone? */
				else
					goto vm_readv_didnt_work;
			}
			count_read += r;

			p = memchr(local[0].iov_base, '\0', r);
			if (p != NULL)
				return count_read + (p - (char *)local[0].iov_base);
			local[0].iov_base = (char *)local[0].iov_base + r;
			remote[0].iov_base = (char *)local[0].iov_base + r;
			len -= r;
		}
		return count_read;
	}
vm_readv_didnt_work:
	return _pink_read_vm_data_nul_ptrace(tid, addr, dest, len);
}

PINK_GCC_ATTR((nonnull(3)))
int pink_read_syscall(pid_t tid, enum pink_abi abi,
		      const pink_regs_t *regs, long *sysnum)
{
#if PINK_ARCH_ARM
	int r;
	long sysval;

	/*
	 * Note: we only deal with only 32-bit CPUs here.
	 */
	if (regs->ARM_cpsr & 0x20) {
		/*
		 * Get the Thumb-mode system call number
		 */
		sysval = regs->ARM_r7;
	} else {
		/*
		 * Get the ARM-mode system call number
		 */
		if ((r = pink_read_word_data(tid, regs->ARM_pc - 4,
					     &sysval)) < 0)
			return r;

		/* Handle the EABI syscall convention.  We do not
		   bother converting structures between the two
		   ABIs, but basic functionality should work even
		   if the tracer and the tracee have different
		   ABIs. */
		if (sysval == 0xef000000) {
			sysval = regs->ARM_r7;
		} else {
			if ((sysval & 0x0ff00000) != 0x0f900000) {
				/* unknown syscall trap: 0x%08lx (sysval) */
				return -EFAULT;
			}

			/*
			 * Fixup the syscall number
			 */
			sysval &= 0x000fffff;
		}
	}
	if (sysval & 0x0f0000) {
		/*
		 * Handle ARM specific syscall
		 */
		sysval &= 0x0000ffff;
	}

	*sysnum = sysval;
	return 0;
#elif PINK_ARCH_IA64
	int r;
	long reg;
	long sysval;

	reg = (abi == 1 /* ia32 */) ? PT_R1 : PT_R15;

	if ((r = pink_read_word_user(tid, reg, &sysval)) < 0)
		return r;

	*sysnum = sysval;
	return 0;
#elif PINK_ARCH_POWERPC
	*sysnum = regs->gpr[0];
	return 0;
#elif PINK_ARCH_I386
	*sysnum = regs->orig_eax;
	return 0;
#elif PINK_ARCH_X86_64 || PINK_ARCH_X32
# ifndef __X32_SYSCALL_BIT
#  define __X32_SYSCALL_BIT	0x40000000
# endif
# ifndef __X32_SYSCALL_MASK
#  define __X32_SYSCALL_MASK	__X32_SYSCALL_BIT
# endif
	*sysnum = regs->orig_rax;
	if (abi == PINK_ABI_X32)
		*sysnum &= ~__X32_SYSCALL_MASK;

	return 0;
#else
#error unsupported architecture
#endif /* arch */
}

/*
 * Check the syscall return value register value for whether it is
 * a negated errno code indicating an error, or a success return value.
 */
static inline int is_negated_errno(unsigned long int val,
				   size_t current_wordsize)
{
	int nerrnos = 530; /* XXX: strace, errnoent.h */
	unsigned long int max = -(long int) nerrnos;
#if PINK_ABIS_SUPPORTED > 1
	if (current_wordsize < sizeof(val)) {
		val = (unsigned int) val;
		max = (unsigned int) max;
	}
#endif
	return val > max;
}

PINK_GCC_ATTR((nonnull(3,4)))
int pink_read_retval(pid_t tid, enum pink_abi abi,
		     const pink_regs_t *regs, long *retval,
		     int *error)
{
	long myrval;
	int myerror = 0, r;
	size_t wsize;

	r = pink_abi_wordsize(abi, &wsize);
	if (r < 0)
		return r;

#if PINK_ARCH_ARM
	if (is_negated_errno(regs->ARM_r0, wsize)) {
		myrval = -1;
		myerror = -regs->ARM_r0;
	} else {
		myrval = regs->ARM_r0;
	}
#elif PINK_ARCH_IA64
	long r8, r10;

	r = pink_read_word_user(tid, PT_R8, &r8);
	if (r < 0)
		return r;
	r = pink_read_word_user(tid, PT_R10, &r10);
	if (r < 0)
		return r;

	if (abi == 1) { /* ia32 */
		int err;

		err = (int)r8;
		if (is_negated_errno(err, wsize)) {
			myrval = -1;
			myerror = -err;
		} else {
			myrval = err;
		}
	} else {
		if (r10) {
			myrval = -1;
			myerror = r8;
		} else {
			myrval = r8;
		}
	}
#elif PINK_ARCH_POWERPC
#define SO_MASK 0x10000000
	long ppc_result;

	ppc_result = regs->gpr[3];
	if (regs->ccr & SO_MASK)
		ppc_result = -ppc_result;

	if (is_negated_errno(ppc_result, wsize)) {
		myrval = -1;
		myerror = -ppc_result;
	} else {
		myrval = ppc_result;
	}
#elif PINK_ARCH_I386
	if (is_negated_errno(regs->eax, wsize)) {
		myrval = -1;
		myerror = -regs->eax;
	} else {
		myrval = regs->eax;
	}
#elif PINK_ARCH_X86_64 || PINK_ARCH_X32
	if (is_negated_errno(regs->rax, wsize)) {
		myrval = -1;
		myerror = -regs->rax;
	} else {
		myrval = regs->rax;
	}
#else
#error unsupported architecture
#endif
	*retval = myrval;
	if (error)
		*error = myerror;
	return 0;
}

PINK_GCC_ATTR((nonnull(5)))
int pink_read_argument(pid_t tid, enum pink_abi abi,
		       const pink_regs_t *regs,
		       unsigned arg_index, long *argval)
{
	if (arg_index >= PINK_MAX_ARGS) {
		errno = EINVAL;
		return false;
	}

#if PINK_ARCH_ARM
	*argval = regs->uregs[arg_index];
	return 0;
#elif PINK_ARCH_IA64
	int r;
	long myval;

	if (abi == 0) { /* !ia32 */
		unsigned long *out0, cfm, sof, sol, skip;
		long rbs_end;
#		ifndef PT_RBS_END
#		  define PT_RBS_END	PT_AR_BSP
#		endif

		if ((r = pink_read_word_user(tid, PT_RBS_END, &rbs_end)) < 0)
			return r;
		if ((r = pink_read_word_user(tid, PT_CFM, (long *) &cfm)) < 0)
			return r;

		sof = (cfm >> 0) & 0x7f;
		sol = (cfm >> 7) & 0x7f;
		out0 = ia64_rse_skip_regs((unsigned long *) rbs_end, -sof + sol);
		skip = (unsigned long) ia64_rse_skip_regs(out0, arg_index);

		if ((r = pink_read_vm_data(tid, skip, sizeof(long),
					   &myval)) < 0)
			return r;
	} else { /* ia32 */
		int argreg;

		switch (arg_index) {
		case 0: argreg = PT_R11; break; /* EBX = out0 */
		case 1: argreg = PT_R9;  break; /* ECX = out1 */
		case 2: argreg = PT_R10; break; /* EDX = out2 */
		case 3: argreg = PT_R14; break; /* ESI = out3 */
		case 4: argreg = PT_R15; break; /* EDI = out4 */
		case 5: argreg = PT_R13; break; /* EBP = out5 */
		default: _pink_assert_not_reached();
		}

		if ((r = pink_read_word_user(pid, argreg, &myval)) < 0)
			return r;
		/* truncate away IVE sign-extension */
		myval &= 0xffffffff;
	}
	*argval = myval;
	return 0;
#elif PINK_ARCH_POWERPC
	if (arg_index == 0)
		*argval = regs->orig_gpr3;
	else
		*argval = regs->gpr[arg_index + 3];
	return 0;
#elif PINK_ARCH_I386
	switch (arg_index) {
	case 0: *argval = regs->ebx; break;
	case 1: *argval = regs->ecx; break;
	case 2: *argval = regs->edx; break;
	case 3: *argval = regs->esi; break;
	case 4: *argval = regs->edi; break;
	case 5: *argval = regs->ebp; break;
	default: _pink_assert_not_reached();
	}
	return 0;
#elif PINK_ARCH_X86_64 || PINK_ARCH_X32
	if (abi != 1) { /* x86-64 or x32 ABI */
		switch (arg_index) {
		case 0: *argval = regs->rdi; break;
		case 1: *argval = regs->rsi; break;
		case 2: *argval = regs->rdx; break;
		case 3: *argval = regs->r10; break;
		case 4: *argval = regs->r8;  break;
		case 5: *argval = regs->r9;  break;
		default: _pink_assert_not_reached();
		}
	} else { /* i386 ABI */
		/* (long)(int) is to sign-extend lower 32 bits */
		switch (arg_index) {
		case 0: *argval = (long)(int)regs->rbx; break;
		case 1: *argval = (long)(int)regs->rcx; break;
		case 2: *argval = (long)(int)regs->rdx; break;
		case 3: *argval = (long)(int)regs->rsi; break;
		case 4: *argval = (long)(int)regs->rdi; break;
		case 5: *argval = (long)(int)regs->rbp; break;
		default: _pink_assert_not_reached();
		}
	}
	return 0;
#else
#error unsupported architecture
#endif
}

ssize_t pink_read_string_array(pid_t tid, enum pink_abi abi,
			       long arg, unsigned arr_index,
			       char *dest, size_t dest_len,
			       bool *nullptr)
{
	int r;
	size_t wsize;
	union {
		unsigned int p32;
		unsigned long p64;
		char data[sizeof(long)];
	} cp;

	r = pink_abi_wordsize(abi, &wsize);
	if (r < 0)
		return -1;
	arg += arr_index * wsize;

	if (pink_read_vm_data(tid, abi, arg, cp.data, wsize) < 0)
		return -1;
	if (wsize == 4)
		cp.p64 = cp.p32;
	if (cp.p64 == 0) {
		/* hit NULL, end of the array */
		if (nullptr)
			*nullptr = true;
		return 0;
	}
	if (nullptr)
		*nullptr = false;
	return pink_read_vm_data_nul(tid, abi, cp.p64, dest, dest_len);
}
