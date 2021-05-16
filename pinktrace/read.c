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
 *   Copyright (c) 2000 PocketPenguins Inc.  Linux for Hitachi SuperH
 *                      port by Greg Banks <gbanks@pocketpenguins.com>
 * Based in part upon truss which is:
 *   Copyright (c) 1997 Sean Eric Fagan
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <pinktrace/private.h>
#include <pinktrace/pink.h>

int pink_read_word_user(pid_t pid, long off, long *res)
{
	return pink_ptrace(PTRACE_PEEKUSER, pid, (void *)off, NULL, res);
}

int pink_read_word_data(pid_t pid, long off, long *res)
{
	return pink_ptrace(PTRACE_PEEKDATA, pid, (void *)off, NULL, res);
}

PINK_GCC_ATTR((nonnull(2,3)))
int pink_read_abi(pid_t pid, const struct pink_regset *regset, short *abi)
{
	*abi = regset->abi;
	return 0;
}

PINK_GCC_ATTR((nonnull(2,3)))
int pink_read_syscall(pid_t pid, const struct pink_regset *regset, long *sysnum)
{
#if PINK_ARCH_AARCH64
	switch (regset->abi) {
	case PINK_ABI_AARCH64:
		*sysnum = regset->arm_regs_union.aarch64_r.regs[8];
		return 0;
	case PINK_ABI_ARM:
		/* Note: we don't support OABI, unlike 32-bit ARM build */
		*sysnum = regset->arm_regs_union.arm_r.uregs[7];
		return 0;
	default:
		return -EINVAL;
	}
#elif PINK_ARCH_ARM
	int r;
	long sysval;
	struct pt_regs regs = regset->arm_regs;

	if (regs.ARM_ip != 0) {
		/* It is not a syscall entry */
		return -EFAULT;
	}
	/* Note: we support only 32-bit CPUs, not 26-bit */

	if (regs.ARM_cpsr & 0x20) {
		/* Thumb mode */
		sysval = regs.ARM_r7;
	} else {
		/* ARM mode */
		if ((r = pink_read_word_data(pid, (long)(regs.ARM_pc - 4), &sysval)) < 0)
			return r;
		/* EABI syscall convention? */
		if (sysval == 0xef000000) {
			sysval = regs.ARM_r7; /* yes */
		} else {
			if ((sysval & 0x0ff00000) != 0x0f900000) {
				return -EFAULT;
			}
			/* Fixup the syscall number */
			sysval &= 0x000fffff;
		}
	}
	*sysnum = sysval;
	return 0;
#elif PINK_ARCH_IA64
	int r;
	long reg;
	long sysval;

	reg = regset->ia32 ? PT_R1 : PT_R15;
	if ((r = pink_read_word_user(pid, reg, &sysval)) < 0)
		return r;

	*sysnum = sysval;
	return 0;
#elif PINK_ARCH_POWERPC
	*sysnum = regset->ppc_regs.gpr[0];
	return 0;
#elif PINK_ARCH_I386
	*sysnum = regset->i386_regs.orig_eax;
	return 0;
#elif PINK_ARCH_X86_64 || PINK_ARCH_X32
	if (regset->abi == PINK_ABI_I386) {
		*sysnum = regset->x86_regs_union.i386_r.orig_eax;
	} else {
		*sysnum = regset->x86_regs_union.x86_64_r.orig_rax;
		if (regset->abi == PINK_ABI_X32)
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
static inline int is_negated_errno(unsigned long int val, short abi)
{
	size_t nerrnos;

	switch (abi) {
	case 0:
		nerrnos = nerrnos0;
		break;
#if PINK_ABIS_SUPPORTED > 1
	case 1:
		nerrnos = nerrnos1;
		break;
# if PINK_ABIS_SUPPORTED > 2
	case 2:
		nerrnos = nerrnos2;
		break;
# endif
#endif
	default:
		_pink_assert_not_reached();
	}

	unsigned long int max = -(long int) nerrnos;
#if PINK_ABIS_SUPPORTED > 1 && SIZEOF_LONG > 4
	size_t wordsize = pink_abi_wordsize(abi);
	if (wordsize < sizeof(val)) {
		val = (unsigned int) val;
		max = (unsigned int) max;
	}
#endif
	return val > max;
}

#if PINK_ARCH_X32
static inline int is_negated_errno_x32(unsigned long long val, short abi)
{
	size_t nerrnos;

	switch (abi) {
	case 0:
		nerrnos = nerrnos0;
		break;
	case 1:
		nerrnos = nerrnos1;
		break;
	default:
		_pink_assert_not_reached();
	}

	unsigned long long max = -(long long) nerrnos;
	/*
	 * current_wordsize is 4 even in personality 0 (native X32)
	 * but truncation _must not_ be done in it.
	 * can't check current_wordsize here!
	 */
	if (abi != 0) {
		val = (uint32_t) val;
		max = (uint32_t) max;
	}
	return val > max;
}
#endif

PINK_GCC_ATTR((nonnull(2,3)))
int pink_read_retval(pid_t pid, const struct pink_regset *regset,
		     long *retval, int *error)
{
	long myrval;
	int myerror = 0;

#if PINK_ARCH_AARCH64
	if (regset->abi == PINK_ABI_ARM) {
		if (is_negated_errno(regset->arm_regs_union.arm_r.uregs[0], regset->abi)) {
			myrval = -1;
			myerror = -regset->arm_regs_union.arm_r.uregs[0];
		} else {
			myrval = regset->arm_regs_union.arm_r.uregs[0];
		}
	} else if (is_negated_errno(regset->arm_regs_union.aarch64_r.regs[0],
				    regset->abi)) {
		myrval = -1;
		myerror = -regset->arm_regs_union.aarch64_r.regs[0];
	} else {
		myrval = regset->arm_regs_union.aarch64_r.regs[0];
	}
#elif PINK_ARCH_ARM
	struct pt_regs regs = regset->arm_regs;

	if (is_negated_errno(regs.ARM_r0, regset->abi)) {
		myrval = -1;
		myerror = -regs.ARM_r0;
	} else {
		myrval = regs.ARM_r0;
	}
#elif PINK_ARCH_IA64
	int r;
	long r8, r10;

	if ((r = pink_read_word_user(pid, PT_R8, &r8)) < 0)
		return r;
	if ((r = pink_read_word_user(pid, PT_R10, &r10)) < 0)
		return r;

	if (regset->ia32) {
		int err;

		err = (int)r8;
		if (is_negated_errno(err, regset->abi)) {
			myrval = -1;
			myerror = -err;
		} else {
			myrval = err;
		}
	} else { /* !ia32 */
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
	struct pt_regs regs = regset->ppc_regs;

	ppc_result = regs.gpr[3];
	if (regs.ccr & SO_MASK)
		ppc_result = -ppc_result;

	if (is_negated_errno(ppc_result, regset->abi)) {
		myrval = -1;
		myerror = -ppc_result;
	} else {
		myrval = ppc_result;
	}
#elif PINK_ARCH_I386
	struct user_regs_struct regs = regset->i386_regs;

	if (is_negated_errno(regs.eax, regset->abi)) {
		myrval = -1;
		myerror = -regs.eax;
	} else {
		myrval = regs.eax;
	}
#elif PINK_ARCH_X86_64
	long rax;

	if (regset->abi == PINK_ABI_I386) {
		/* Sign extend from 32 bits */
		rax = (int32_t)regset->x86_regs_union.i386_r.eax;
	} else {
		rax = regset->x86_regs_union.x86_64_r.rax;
	}
	if (is_negated_errno(rax, regset->abi)) {
		myrval = -1;
		myerror = -rax;
	} else {
		myrval = rax;
	}
#elif PINK_ARCH_X32
	/* In X32, return value is 64-bit (llseek uses one).
	 * Using merely "long rax" would not work.
	 */
	long long rax;

	if (regset->abi == PINK_ABI_I386) {
		/* Sign extend from 32 bits */
		rax = (int32_t)regset->x86_regs_union.i386_r.eax;
	} else {
		rax = regset->x86_regs_union.x86_64_r.rax;
	}
	/* Careful: is_negated_errno() works only on longs */
	if (is_negated_errno_x32(rax)) {
		myrval = -1;
		myerror = -rax;
	} else {
		myrval = rax; /* truncating */
	}
#else
#error unsupported architecture
#endif
	*retval = myrval;
	if (error)
		*error = myerror;
	return 0;
}

PINK_GCC_ATTR((nonnull(2,4)))
int pink_read_argument(pid_t pid, const struct pink_regset *regset,
		       unsigned arg_index, long *argval)
{
	if (arg_index >= PINK_MAX_ARGS)
		return -EINVAL;

#if PINK_ARCH_AARCH64
	if (regset->abi == PINK_ABI_AARCH64)
		*argval = regset->arm_regs_union.aarch64_r.regs[arg_index];
	else
		*argval = regset->arm_regs_union.arm_r.uregs[arg_index];

	return 0;
#elif PINK_ARCH_ARM
	struct pt_regs r = regset->arm_regs;

	if (arg_index == 0)
		*argval = r.ARM_ORIG_r0;
	else
		*argval = r.uregs[arg_index];
	return 0;
#elif PINK_ARCH_IA64
	int r;
	long myval;

	if (!regset->ia32) {
		unsigned long *out0, cfm, sof, sol, addr;
		long rbs_end;
#		ifndef PT_RBS_END
#		  define PT_RBS_END	PT_AR_BSP
#		endif

		if ((r = pink_read_word_user(pid, PT_RBS_END, &rbs_end)) < 0)
			return r;
		if ((r = pink_read_word_user(pid, PT_CFM, (long *) &cfm)) < 0)
			return r;

		sof = (cfm >> 0) & 0x7f;
		sol = (cfm >> 7) & 0x7f;
		out0 = ia64_rse_skip_regs((unsigned long *) rbs_end, -sof + sol);
		addr = (unsigned long) ia64_rse_skip_regs(out0, arg_index);

		if ((r = pink_read_vm_data_full(pid, regset, addr, sizeof(long), &myval)) < 0)
			return r;
	} else { /* ia32 */
		static const int argreg[PINK_MAX_ARGS] = { PT_R11 /* EBX = out0 */,
						           PT_R9  /* ECX = out1 */,
						           PT_R10 /* EDX = out2 */,
						           PT_R14 /* ESI = out3 */,
						           PT_R15 /* EDI = out4 */,
						           PT_R13 /* EBP = out5 */};

		if ((r = pink_read_word_user(pid, argreg[arg_index], &myval)) < 0)
			return r;
		/* truncate away IVE sign-extension */
		myval &= 0xffffffff;
	}
	*argval = myval;
	return 0;
#elif PINK_ARCH_POWERPC
	*argval = (arg_index == 0) ? regset->ppc_regs.orig_gpr3
				   : regset->ppc_regs.gpr[arg_index + 3];
	return 0;
#elif PINK_ARCH_I386
	struct user_regs_struct regs = regset->i386_regs;

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
	if (regset->abi != PINK_ABI_I386) { /* x86-64 or x32 ABI */
		struct user_regs_struct regs = regset->x86_regs_union.x86_64_r;
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
		struct i386_user_regs_struct regs = regset->x86_regs_union.i386_r;
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

PINK_GCC_ATTR((nonnull(2,4)))
ssize_t pink_read_vm_data(pid_t pid, const struct pink_regset *regset,
			  long addr, char *dest, size_t len)
{
	ssize_t r;

	errno = 0;
	r = pink_vm_cread(pid, regset, addr, dest, len);
	if (errno == ENOSYS || errno == EPERM)
		return pink_vm_lread(pid, regset, addr, dest, len);
	return r;
}

PINK_GCC_ATTR((nonnull(2,4)))
int pink_read_vm_data_full(pid_t pid, const struct pink_regset *regset,
			   long addr, char *dest, size_t len)
{
	ssize_t l;

	errno = 0;
	l = pink_read_vm_data(pid, regset, addr, dest, len);
	if (l < 0)
		return -errno;
	if (len != (size_t)l)
		return -EFAULT;
	return 0;
}

PINK_GCC_ATTR((nonnull(2,4)))
ssize_t pink_read_vm_data_nul(pid_t pid, const struct pink_regset *regset,
			      long addr, char *dest, size_t len)
{
	ssize_t r;

	errno = 0;
	r = pink_vm_cread_nul(pid, regset, addr, dest, len);
	if (errno == ENOSYS || errno == EPERM)
		return pink_vm_lread_nul(pid, regset, addr, dest, len);
	return r;
}

PINK_GCC_ATTR((nonnull(2,5)))
ssize_t pink_read_string_array(pid_t pid, const struct pink_regset *regset,
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

	wsize = pink_abi_wordsize(regset->abi);
	arg += arr_index * wsize;

	errno = 0;
	pink_read_vm_data(pid, regset, arg, cp.data, wsize);
	if (errno)
		return 0;
	if (!(wsize < sizeof(cp.p64) ? cp.p32 : cp.p64)) {
		/* hit NULL, end of the array */
		if (nullptr)
			*nullptr = true;
		return 0;
	}
	if (nullptr)
		*nullptr = false;
	return pink_read_vm_data_nul(pid, regset,
			wsize < sizeof(cp.p64) ? cp.p32 : cp.p64,
			dest, dest_len);
}
