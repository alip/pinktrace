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

int pink_write_word_user(pid_t pid, long off, long val)
{
	return pink_ptrace(PTRACE_POKEUSER, pid, (void *)off, (void *)val, NULL);
}

int pink_write_word_data(pid_t pid, long off, long val)
{
	return pink_ptrace(PTRACE_POKEDATA, pid, (void *)off, (void *)val, NULL);
}

int pink_write_syscall(pid_t pid, struct pink_regset *regset, long sysnum)
{
	int r;
#if PINK_ARCH_ARM
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

int pink_write_retval(pid_t pid, struct pink_regset *regset, long retval, int error)
{
#if PINK_ARCH_ARM
	return pink_write_word_user(pid, 0, retval);
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

int pink_write_argument(pid_t pid, struct pink_regset *regset, unsigned arg_index, long argval)
{
	if (arg_index >= PINK_MAX_ARGS)
		return -EINVAL;

#if PINK_ARCH_ARM
	if (arg_index == 0) {
		/* TODO: do this with pink_write_word_user() */
		struct pt_regs r = regset->arm_regs;
		r.ARM_ORIG_r0 = argval;
		return pink_trace_set_regs(pid, &r);
	} else {
		return pink_write_word_user(pid, sizeof(long) * arg_index, argval);
	}
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

PINK_GCC_ATTR((nonnull(4)))
ssize_t pink_write_vm_data(pid_t pid, struct pink_regset *regset, long addr, const char *src, size_t len)
{
	ssize_t r;

	errno = 0;
	r = pink_vm_cwrite(pid, regset, addr, src, len);
	if (errno == ENOSYS)
		return pink_vm_lwrite(pid, regset, addr, src, len);
	return r;
}
