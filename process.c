/*
 * Copyright (c) 2013 Ali Polatel <alip@exherbo.org>
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

int pink_process_alloc(pid_t pid, struct pink_process **procptr)
{
	struct pink_process *p;

	p = malloc(sizeof(struct pink_process));
	if (!p)
		return -errno;

	p->pid = pid;
	*procptr = p;
	return 0;
}

void pink_process_free(struct pink_process *proc)
{
	free(proc);
}

pid_t pink_process_get_pid(const struct pink_process *proc)
{
	return proc->pid;
}

void pink_process_set_pid(struct pink_process *proc, pid_t pid)
{
	proc->pid = pid;
}

short pink_process_get_abi(const struct pink_process *proc)
{
#if PINK_ABIS_SUPPORTED == 1
	return PINK_ABI_DEFAULT;
#else
	return proc->regset.abi;
#endif
}

int pink_process_update_regset(struct pink_process *proc)
{
	int r;
	pid_t pid = proc->pid;
	struct pink_regset *regset = &proc->regset;

#if PINK_ARCH_ARM
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
