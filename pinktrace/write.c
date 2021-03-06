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
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <pinktrace/private.h>
#include <pinktrace/pink.h>

#ifndef NT_ARM_SYSTEM_CALL
# define NT_ARM_SYSTEM_CALL 0x404
#endif
/*
 * NT_ARM_SYSTEM_CALL regset is supported by linux kernel
 * starting with commit v3.19-rc1~59^2~16.
 */

int pink_write_word_user(pid_t pid, long off, long val)
{
	return pink_ptrace(PTRACE_POKEUSER, pid, (void *)off, (void *)val, NULL);
}

int pink_write_word_data(pid_t pid, long off, long val)
{
	return pink_ptrace(PTRACE_POKEDATA, pid, (void *)off, (void *)val, NULL);
}

PINK_GCC_ATTR((nonnull(2)))
int pink_write_syscall(pid_t pid, struct pink_regset *regset, long sysnum)
{
	int r;
#if PINK_ARCH_AARCH64
	unsigned int n = (uint16_t) sysnum;
	const struct iovec io = {
		.iov_base = &n,
		.iov_len = sizeof(n)
	};
	r = pink_trace_set_regset(pid, &io, NT_ARM_SYSTEM_CALL);
#elif PINK_ARCH_ARM
# ifndef PTRACE_SET_SYSCALL
#  define PTRACE_SET_SYSCALL 23
# endif
	r = pink_ptrace(PTRACE_SET_SYSCALL, pid, NULL,
			(void *)(long)(sysnum & 0xffff), NULL);
#elif PINK_ARCH_IA64
	if (regset->ia32)
		r = pink_write_word_user(pid, PT_R1, sysnum);
	else
		r = pink_write_word_user(pid, PT_R15, sysnum);
#elif PINK_ARCH_POWERPC
	r = pink_write_word_user(pid, sizeof(unsigned long)*PT_R0, sysnum);
#elif PINK_ARCH_I386
	r = pink_write_word_user(pid, 4 * ORIG_EAX, sysnum);
#elif PINK_ARCH_X86_64 || PINK_ARCH_X32
	r = pink_write_word_user(pid, 8 * ORIG_RAX, sysnum);
#else
#error unsupported architecture
#endif
	return r;
}

PINK_GCC_ATTR((nonnull(2)))
int pink_write_retval(pid_t pid, struct pink_regset *regset, long retval, int error)
{
#if PINK_ARCH_AARCH64 || PINK_ARCH_ARM
	if (regset->abi == PINK_ABI_ARM) {
		if (error)
			retval = (long)-error;
		return pink_write_word_user(pid, 0, retval);
	} else {
		regset->arm_regs_union.aarch64_r.regs[0] = error? -error : retval;
		return pink_trace_set_regset(pid, &regset->aarch64_io, NT_PRSTATUS);
	}
#elif PINK_ARCH_IA64
	int r;
	long r8, r10;

	if (error) {
		r8 = -error;
		r10 = -1;
	} else {
		r8 = retval;
		r10 = 0;
	}

	if ((r = pink_write_word_user(pid, PT_R8, r8)) < 0)
		return r;
	return pink_write_word_user(pid, PT_R10, r10);
#elif PINK_ARCH_POWERPC
# define SO_MASK 0x10000000
	int r;
	long flags;

	if ((r = pink_read_word_user(pid, sizeof(unsigned long) * PT_CCR, &flags)) < 0)
		return r;

	if (error) {
		retval = error;
		flags |= SO_MASK;
	} else {
		flags &= ~SO_MASK;
	}

	if ((r = pink_write_word_user(pid, sizeof(unsigned long) * PT_R3, retval)) < 0)
		return r;
	return pink_write_word_user(pid, sizeof(unsigned long) * PT_CCR, flags);
#elif PINK_ARCH_I386
	if (error)
		retval = (long)-error;
	return pink_write_word_user(pid, 4 * EAX, retval);
#elif PINK_ARCH_X86_64 || PINK_ARCH_X32
	if (error)
		retval = (long)-error;
	return pink_write_word_user(pid, 8 * RAX, retval);
#else
#error unsupported architecture
#endif
}

PINK_GCC_ATTR((nonnull(2)))
int pink_write_argument(pid_t pid, struct pink_regset *regset,
			unsigned arg_index, long argval)
{
	if (arg_index >= PINK_MAX_ARGS)
		return -EINVAL;
#if PINK_ARCH_AARCH64
	if (regset->abi == PINK_ABI_AARCH64)
		regset->arm_regs_union.aarch64_r.regs[arg_index] = argval;
	else
		regset->arm_regs_union.arm_r.uregs[arg_index] = argval;
	return pink_trace_set_regset(pid, &regset->aarch64_io, NT_PRSTATUS);
#elif PINK_ARCH_ARM
	if (arg_index == 0)
		return pink_write_word_user(pid, sizeof(long) * 17, argval); /* ARM_ORIG_r0 */
	else
		return pink_write_word_user(pid, sizeof(long) * arg_index, argval);
#elif PINK_ARCH_IA64
	if (regset->ia32) {
		static const int argreg[PINK_MAX_ARGS] = { PT_R11 /* EBX = out0 */,
						           PT_R9  /* ECX = out1 */,
						           PT_R10 /* EDX = out2 */,
						           PT_R14 /* ESI = out3 */,
						           PT_R15 /* EDI = out4 */,
						           PT_R13 /* EBP = out5 */};
		return pink_write_word_user(pid, argreg[arg_index], argval);
	} else {
		unsigned long cfm, sof, sol;
		long bsp;
		unsigned long arg_state;

		if ((r = pink_read_word_user(pid, PT_AR_BSP, &bsp)) < 0)
			return r;
		if ((r = pink_read_word_user(pid, PT_CFM, (long *)&cfm)) < 0)
			return r;

		sof = (cfm >> 0) & 0x7f;
		sol = (cfm >> 7) & 0x7f;
		bsp = (long) ia64_rse_skip_regs((unsigned long *) bsp, -sof + sol);
		state = (unsigned long)bsp;

		return pink_write_vm_data(pid,
					  (unsigned long)ia64_rse_skip_regs(state, arg_index),
					  (const char *) &argval, sizeof(long));
	}
#elif PINK_ARCH_POWERPC
	return pink_write_word_user(pid,
				    (arg_index == 0) ? (sizeof(unsigned long) * PT_ORIG_R3)
						     : ((arg_index + PT_R3) * sizeof(unsigned long)),
				    argval);
#elif PINK_ARCH_I386
	switch (arg_index) {
	case 0: return pink_write_word_user(pid, 4 * EBX, argval);
	case 1: return pink_write_word_user(pid, 4 * ECX, argval);
	case 2: return pink_write_word_user(pid, 4 * EDX, argval);
	case 3: return pink_write_word_user(pid, 4 * ESI, argval);
	case 4: return pink_write_word_user(pid, 4 * EDI, argval);
	case 5: return pink_write_word_user(pid, 4 * EBP, argval);
	default: _pink_assert_not_reached();
	}
#elif PINK_ARCH_X86_64 || PINK_ARCH_X32
	switch (regset->abi) {
	case PINK_ABI_I386:
		switch (arg_index) {
		case 0: return pink_write_word_user(pid, 8 * RBX, argval);
		case 1: return pink_write_word_user(pid, 8 * RCX, argval);
		case 2: return pink_write_word_user(pid, 8 * RDX, argval);
		case 3: return pink_write_word_user(pid, 8 * RSI, argval);
		case 4: return pink_write_word_user(pid, 8 * RDI, argval);
		case 5: return pink_write_word_user(pid, 8 * RBP, argval);
		default: _pink_assert_not_reached();
		}
		break;
	case PINK_ABI_X32:
#if PINK_ARCH_X86_64
	case PINK_ABI_X86_64:
#endif
		switch (arg_index) {
		case 0: return pink_write_word_user(pid, 8 * RDI, argval);
		case 1: return pink_write_word_user(pid, 8 * RSI, argval);
		case 2: return pink_write_word_user(pid, 8 * RDX, argval);
		case 3: return pink_write_word_user(pid, 8 * R10, argval);
		case 4: return pink_write_word_user(pid, 8 * R8, argval);
		case 5: return pink_write_word_user(pid, 8 * R9, argval);
		default: _pink_assert_not_reached();
		}
		break;
	default:
		return -EINVAL;
	}
#else
#error unsupported architecture
#endif
}

PINK_GCC_ATTR((nonnull(2,4)))
ssize_t pink_write_vm_data(pid_t pid, const struct pink_regset *regset,
			   long addr, const char *src, size_t len)
{
	ssize_t r;

	errno = 0;
	r = pink_vm_cwrite(pid, regset, addr, src, len);
	if (errno == ENOSYS || errno == EPERM)
		return pink_vm_lwrite(pid, regset, addr, src, len);
	return r;
}
