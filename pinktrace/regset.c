/*
 * Copyright (c) 2013, 2021 Ali Polatel <alip@exherbo.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <pinktrace/private.h>
#include <pinktrace/pink.h>

static void pink_regset_zero(struct pink_regset *regset)
{
	memset(regset, 0, sizeof(struct pink_regset));
}

PINK_GCC_ATTR((nonnull(1)))
int pink_regset_alloc(struct pink_regset **regptr)
{
	struct pink_regset *r;

	r = malloc(sizeof(struct pink_regset));
	if (!r)
		return -errno;
	pink_regset_zero(r);

	*regptr = r;
	return 0;
}

void pink_regset_free(struct pink_regset *regset)
{
	free(regset);
}

int pink_regset_fill(pid_t pid, struct pink_regset *regset)
{
	int r;

#if PINK_ABIS_SUPPORTED == 1
	regset->abi = PINK_ABI_DEFAULT;
#endif

#if PINK_ARCH_AARCH64
# define aarch64_regs	(regset->arm_regs_union.aarch64_r)
# define arm_regs	(regset->arm_regs_union.arm_r)
# if PINK_HAVE_GETREGSET
	regset->aarch64_io.iov_base = &regset->arm_regs_union;
	regset->aarch64_io.iov_len = sizeof(regset->arm_regs_union);
	if ((r = pink_trace_get_regset(pid, &regset->aarch64_io, NT_PRSTATUS)) < 0)
		return r;
	switch (regset->aarch64_io.iov_len) {
	case sizeof(aarch64_regs):
		/* We are in 64-bit mode. */
		regset->abi = PINK_ABI_AARCH64;
		break;
	case sizeof(arm_regs):
		regset->abi = PINK_ABI_ARM;
		break;
	default:
		abort();
	}
# else
	return -ENOSYS;
# endif
#elif PINK_ARCH_ARM
	if ((r = pink_trace_get_regs(pid, &regset->arm_regs)) < 0)
		return r;
#elif PINK_ARCH_POWERPC
	if ((r = pink_trace_get_regs(pid, &regset->ppc_regs)) < 0)
		return r;
# if PINK_ARCH_POWERPC64
	/* SF is bit 0 of MSR (Machine State Register) */
	regset->abi = (regset->ppc_regs.msr & 0) ? 0 : 1;
# endif
#elif PINK_ARCH_I386
	if ((r = pink_trace_get_regs(pid, &regset->i386_regs)) < 0)
		return r;
#elif PINK_ARCH_X86_64 || PINK_ARCH_X32
# define i386_regs	(regset->x86_regs_union.i386_r)
# define x86_64_regs	(regset->x86_regs_union.x86_64_r)
# if PINK_HAVE_GETREGSET
	regset->x86_io.iov_base = &regset->x86_regs_union;
	regset->x86_io.iov_len = sizeof(regset->x86_regs_union);
	if ((r = pink_trace_get_regset(pid, &regset->x86_io, NT_PRSTATUS)) < 0)
		return r;
# else
	/* Use old method, with heuristical detection of 32-bitness */
	regset->x86_io.iov_len = sizeof(x86_64_regs);
	if ((r = pink_trace_get_regs(pid, &x86_64_regs)) < 0)
		return r;
	if (x86_64_regs.cs == 0x23) {
		regset->x86_io.iov_len = sizeof(i386_regs);
		/*
		 * The order is important: i386_regs and x86_64_regs
		 * are overlaid in memory!
		 */
		i386_regs.ebx = x86_64_regs.rbx;
		i386_regs.ecx = x86_64_regs.rcx;
		i386_regs.edx = x86_64_regs.rdx;
		i386_regs.esi = x86_64_regs.rsi;
		i386_regs.edi = x86_64_regs.rdi;
		i386_regs.ebp = x86_64_regs.rbp;
		i386_regs.eax = x86_64_regs.rax;
		/*i386_regs.xds = x86_64_regs.ds; unused by pinktrace */
		/*i386_regs.xes = x86_64_regs.es; ditto... */
		/*i386_regs.xfs = x86_64_regs.fs;*/
		/*i386_regs.xgs = x86_64_regs.gs;*/
		i386_regs.orig_eax = x86_64_regs.orig_rax;
		i386_regs.eip = x86_64_regs.rip;
		/*i386_regs.xcs = x86_64_regs.cs;*/
		/*i386_regs.eflags = x86_64_regs.eflags;*/
		i386_regs.esp = x86_64_regs.rsp;
		/*i386_regs.xss = x86_64_regs.ss;*/
	}
# endif
	/* GETREGSET of NT_PRSTATUS tells us regset size,
	 * which unambiguously detects i386.
	 *
	 * Linux kernel distinguishes x86-64 and x32 processes
	 * solely by looking at __X32_SYSCALL_BIT:
	 * arch/x86/include/asm/compat.h::is_x32_task():
	 * if (task_pt_regs(current)->orig_ax & __X32_SYSCALL_BIT)
	 *         return true;
	 */
	if (regset->x86_io.iov_len == sizeof(i386_regs))
		regset->abi = PINK_ABI_I386;
	else if (x86_64_regs.orig_rax & __X32_SYSCALL_BIT)
		regset->abi = PINK_ABI_X32;
	else
		regset->abi = PINK_ABI_X86_64;
# undef i386_regs
# undef x86_64_regs
#elif PINK_ARCH_IA64
#	define IA64_PSR_IS	((long)1 << 34)
	long psr;

	if ((r = pink_read_word_user(pid, PT_CR_IPSR, &psr)) < 0)
		return r;
	regset->ia32 = !!(psr & IA64_PSR_IS);
#else
# error "unsupported architecture"
#endif
	return 0;
}
