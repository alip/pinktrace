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

int pink_read_word_user(pid_t pid, long off, long *res)
{
	long val;

	val = pink_ptrace(PTRACE_PEEKUSER, pid, (void *)off, NULL);
	if (val < 0)
		return -errno;
	if (res != NULL)
		*res = val;
	return 0;
}

int pink_read_word_data(pid_t pid, long off, long *res)
{
	long val;

	val = pink_ptrace(PTRACE_PEEKDATA, pid, (void *)off, NULL);
	if (val < 0)
		return -errno;
	if (res)
		*res = val;
	return 0;
}

PINK_GCC_ATTR((nonnull(2)))
int pink_read_syscall(struct pink_process *tracee, long *sysnum)
{
#if PINK_ARCH_ARM
	int r;
	long sysval;
	struct pt_regs regs = tracee->regset.arm_regs;

	/*
	 * Note: we only deal with only 32-bit CPUs here.
	 */
	if (regs.ARM_cpsr & 0x20) {
		/*
		 * Get the Thumb-mode system call number
		 */
		sysval = regs.ARM_r7;
	} else {
		/*
		 * Get the ARM-mode system call number
		 */
		if ((r = pink_read_word_data(tracee->pid, regs.ARM_pc - 4, &sysval)) < 0)
			return r;

		/* EABI syscall convention? */
		if (sysval == 0xef000000) {
			sysval = regs.ARM_r7; /* yes */
		} else {
			if ((sysval & 0x0ff00000) != 0x0f900000) {
				/* unknown syscall trap: 0x%08lx (sysval) */
				return -EFAULT;
			}
			/* Fixup the syscall number */
			sysval &= 0x000fffff;
		}
	}
	sysval = _pink_syscall_shuffle(sysval);
	*sysnum = sysval;
	return 0;
#elif PINK_ARCH_IA64
	int r;
	long reg;
	long sysval;

	reg = tracee->regset.ia32 ? PT_R1 : PT_R15;
	if ((r = pink_read_word_user(tracee->pid, reg, &sysval)) < 0)
		return r;

	*sysnum = sysval;
	return 0;
#elif PINK_ARCH_POWERPC
	*sysnum = tracee->regset.ppc_regs.gpr[0];
	return 0;
#elif PINK_ARCH_I386
	*sysnum = tracee->regset.i386_regs.orig_eax;
	return 0;
#elif PINK_ARCH_X86_64 || PINK_ARCH_X32
	if (tracee->regset.abi == PINK_ABI_I386) {
		*sysnum = tracee->regset.x86_regs_union.i386_r.orig_eax;
	} else {
		*sysnum = tracee->regset.x86_regs_union.x86_64_r.orig_rax;
		if (tracee->regset.abi == PINK_ABI_X32)
			*sysnum -= __X32_SYSCALL_BIT;
	}
	return 0;
#else
#error unsupported architecture
#endif /* arch */
}

/*
 * Check the syscall return value register value for whether it is
 * a negated errno code indicating an error, or a success return value.
 */
static inline int is_negated_errno(unsigned long int val, size_t current_wordsize)
{
	int nerrnos = 530; /* XXX: strace, errnoent.h */
	unsigned long int max = -(long int) nerrnos;
#if SUPPORTED_ABIS > 1
	if (current_wordsize < sizeof(val)) {
		val = (unsigned int) val;
		max = (unsigned int) max;
	}
#endif
	return val > max;
}

PINK_GCC_ATTR((nonnull(2,3)))
int pink_read_retval(struct pink_process *tracee, long *retval, int *error)
{
	long myrval;
	int myerror = 0;
	size_t wsize = pink_abi_wordsize(pink_process_get_abi(tracee));

#if PINK_ARCH_ARM
	struct pt_regs regs = tracee->regset.arm_regs;

	if (is_negated_errno(regs.ARM_r0, wsize)) {
		myrval = -1;
		myerror = -regs.ARM_r0;
	} else {
		myrval = regs.ARM_r0;
	}
#elif PINK_ARCH_IA64
	int r;
	long r8, r10;

	r = pink_read_word_user(tracee->pid, PT_R8, &r8);
	if (r < 0)
		return r;
	r = pink_read_word_user(tracee->pid, PT_R10, &r10);
	if (r < 0)
		return r;

	if (tracee->regset.abi == 1) { /* ia32 */
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
# define SO_MASK 0x10000000
	long ppc_result;
	struct pt_regs regs = tracee->regset.ppc_regs;

	ppc_result = regs.gpr[3];
	if (regs.ccr & SO_MASK)
		ppc_result = -ppc_result;

	if (is_negated_errno(ppc_result, wsize)) {
		myrval = -1;
		myerror = -ppc_result;
	} else {
		myrval = ppc_result;
	}
#elif PINK_ARCH_I386
	struct user_regs_struct regs = tracee->regset.i386_regs;

	if (is_negated_errno(regs.eax, wsize)) {
		myrval = -1;
		myerror = -regs.eax;
	} else {
		myrval = regs.eax;
	}
#elif PINK_ARCH_X86_64 || PINK_ARCH_X32
	long rax;

	if (tracee->regset.abi == PINK_ABI_I386) {
		/* Sign extend from 32 bits */
		rax = (int32_t)tracee->regset.x86_regs_union.i386_r.eax;
	} else {
		/* Note: in X32 build, this truncates 64 to 32 bits */
		rax = tracee->regset.x86_regs_union.x86_64_r.rax;
	}
	if (is_negated_errno(rax, wsize)) {
		myrval = -1;
		myerror = -rax;
	} else {
		myrval = rax;
	}
#else
#error unsupported architecture
#endif
	*retval = myrval;
	if (error)
		*error = myerror;
	return 0;
}

PINK_GCC_ATTR((nonnull(3)))
int pink_read_argument(struct pink_process *tracee, unsigned arg_index, long *argval)
{
	if (arg_index >= PINK_MAX_ARGS)
		return -EINVAL;

#if PINK_ARCH_ARM
	*argval = tracee->regset.arm_regs.uregs[arg_index];
	return 0;
#elif PINK_ARCH_IA64
	int r;
	long myval;

	if (tracee->regset.abi == 0) { /* !ia32 */
		unsigned long *out0, cfm, sof, sol, addr;
		long rbs_end;
#		ifndef PT_RBS_END
#		  define PT_RBS_END	PT_AR_BSP
#		endif

		if ((r = pink_read_word_user(tracee->pid, PT_RBS_END, &rbs_end)) < 0)
			return r;
		if ((r = pink_read_word_user(tracee->id, PT_CFM, (long *) &cfm)) < 0)
			return r;

		sof = (cfm >> 0) & 0x7f;
		sol = (cfm >> 7) & 0x7f;
		out0 = ia64_rse_skip_regs((unsigned long *) rbs_end, -sof + sol);
		addr = (unsigned long) ia64_rse_skip_regs(out0, arg_index);

		if (pink_read_vm_data(tracee, addr, sizeof(long), &myval) < 0)
			return -errno;
	} else { /* ia32 */
		static const int argreg[PINK_MAX_ARGS] = { PT_R11 /* EBX = out0 */,
						           PT_R9  /* ECX = out1 */,
						           PT_R10 /* EDX = out2 */,
						           PT_R14 /* ESI = out3 */,
						           PT_R15 /* EDI = out4 */,
						           PT_R13 /* EBP = out5 */};

		if ((r = pink_read_word_user(tracee->pid, argreg[arg_index], &myval)) < 0)
			return r;
		/* truncate away IVE sign-extension */
		myval &= 0xffffffff;
	}
	*argval = myval;
	return 0;
#elif PINK_ARCH_POWERPC
	*argval = (arg_index == 0) ? tracee->regset.ppc_regs.orig_gpr3
				   : tracee->regset.ppc_regs.gpr[arg_index + 3];
	return 0;
#elif PINK_ARCH_I386
	struct user_regs_struct regs = tracee->regset.i386_regs;

	switch (arg_index) {
	case 0: *argval = regs.ebx; break;
	case 1: *argval = regs.ecx; break;
	case 2: *argval = regs.edx; break;
	case 3: *argval = regs.esi; break;
	case 4: *argval = regs.edi; break;
	case 5: *argval = regs.ebp; break;
	default: _pink_assert_not_reached();
	}
	return 0;
#elif PINK_ARCH_X86_64 || PINK_ARCH_X32
	if (tracee->regset.abi != PINK_ABI_I386) { /* x86-64 or x32 ABI */
		struct user_regs_struct regs = tracee->regset.x86_regs_union.x86_64_r;
		switch (arg_index) {
		case 0: *argval = regs.rdi; break;
		case 1: *argval = regs.rsi; break;
		case 2: *argval = regs.rdx; break;
		case 3: *argval = regs.r10; break;
		case 4: *argval = regs.r8;  break;
		case 5: *argval = regs.r9;  break;
		default: _pink_assert_not_reached();
		}
	} else { /* i386 ABI */
		struct i386_user_regs_struct regs = tracee->regset.x86_regs_union.i386_r;
		/* (long)(int) is to sign-extend lower 32 bits */
		switch (arg_index) {
		case 0: *argval = (long)(int)regs.ebx; break;
		case 1: *argval = (long)(int)regs.ecx; break;
		case 2: *argval = (long)(int)regs.edx; break;
		case 3: *argval = (long)(int)regs.esi; break;
		case 4: *argval = (long)(int)regs.edi; break;
		case 5: *argval = (long)(int)regs.ebp; break;
		default: _pink_assert_not_reached();
		}
	}
	return 0;
#else
#error unsupported architecture
#endif
}

PINK_GCC_ATTR((nonnull(3)))
ssize_t pink_read_vm_data(struct pink_process *tracee, long addr, char *dest, size_t len)
{
	ssize_t r;

	errno = 0;
	r = pink_vm_cread(tracee, addr, dest, len);
	if (errno == ENOSYS)
		return pink_vm_lread(tracee, addr, dest, len);
	return r;
}

PINK_GCC_ATTR((nonnull(3)))
ssize_t pink_read_vm_data_nul(struct pink_process *tracee, long addr, char *dest, size_t len)
{
	ssize_t r;

	errno = 0;
	r = pink_vm_cread_nul(tracee, addr, dest, len);
	if (errno == ENOSYS)
		return pink_vm_lread_nul(tracee, addr, dest, len);
	return r;
}

PINK_GCC_ATTR((nonnull(4)))
ssize_t pink_read_string_array(struct pink_process *tracee,
			       long arg, unsigned arr_index,
			       char *dest, size_t dest_len,
			       bool *nullptr)
{
	size_t wsize;
	union {
		unsigned int p32;
		unsigned long p64;
		char data[sizeof(long)];
	} cp;

	wsize = pink_abi_wordsize(pink_process_get_abi(tracee));
	arg += arr_index * wsize;

	errno = 0;
	pink_read_vm_data(tracee, arg, cp.data, wsize);
	if (errno)
		return 0;
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
	return pink_read_vm_data_nul(tracee, cp.p64, dest, dest_len);
}
