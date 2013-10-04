/*
 * Copyright (c) 2010, 2012, 2013 Ali Polatel <alip@exherbo.org>
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

#ifndef PINK_PRIVATE_H
#define PINK_PRIVATE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/wait.h>

#include <netinet/in.h>
#include <sys/un.h>

#include <pinktrace/pink.h>

#ifdef HAVE_SYS_REG_H
#include <sys/reg.h>
#endif /*  HAVE_SYS_REG_H */

#ifdef HAVE_SYS_UIO_H
#include <sys/uio.h>
#endif /* HAVE_SYS_UIO_H */

/* We need additional hackery on IA64 to include linux/ptrace.h. */
#ifdef HAVE_LINUX_PTRACE_H
# if PINK_ARCH_IA64
#  ifdef HAVE_STRUCT_IA64_FPREG
#  define ia64_fpreg XXX_ia64_fpreg
#  endif
#  ifdef HAVE_STRUCT_PT_ALL_USER_REGS
#  define pt_all_user_regs XXX_pt_all_user_regs
#  endif
# endif
# ifdef HAVE_STRUCT_PTRACE_PEEKSIGINFO_ARGS
#  define ptrace_peeksiginfo_args XXX_ptrace_peeksiginfo_args
# endif
# include <linux/ptrace.h>
# if PINK_ARCH_IA64
#  undef ia64_fpreg
#  undef pt_all_user_regs
# endif
# ifdef HAVE_STRUCT_PTRACE_PEEKSIGINFO_ARGS
#  undef ptrace_peeksiginfo_args
# endif
#endif /* HAVE_LINUX_PTRACE_H */

#include <elf.h> /* NT_PRSTATUS */

#if PINK_ARCH_ARM || PINK_ARCH_POWERPC
# include <asm/ptrace.h>
#elif PINK_ARCH_I386 || PINK_ARCH_X86_64 || PINK_ARCH_X32
# include <sys/user.h>
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a)	(sizeof(a) / sizeof(a[0]))
#endif

#ifndef MIN
#define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif

#define _pink_assert_not_implemented()					\
	do {								\
		fprintf(stderr, "pinktrace assertion failure "		\
				"in %s() at %s:%u\n"			\
				"not implemented!\n",			\
				__func__, __FILE__, __LINE__);		\
		abort();						\
	} while (0)
#define _pink_assert_not_reached()					\
	do {								\
		fprintf(stderr, "pinktrace assertion failure "		\
				"in %s() at %s:%u\n"			\
				"code must not be reached!\n",		\
				__func__, __FILE__, __LINE__);		\
		abort();						\
	} while (0)

#if PINK_ARCH_X86_64
# define ABI0_WORDSIZE 8
# define ABI1_WORDSIZE 4
# define ABI2_WORDSIZE 4
#endif

#if PINK_ARCH_X32
# define ABI0_WORDSIZE 4
# define ABI1_WORDSIZE 4
#endif

#if PINK_ARCH_POWERPC64
# define ABI0_WORDSIZE 8
# define ABI1_WORDSIZE 4
#endif

#ifndef ABI0_WORDSIZE
# define ABI0_WORDSIZE (int)(sizeof(long))
#endif

#if PINK_ARCH_X86_64 || PINK_ARCH_X32
/*
 * On i386, pt_regs and user_regs_struct are the same,
 * but on 64 bit x86, user_regs_struct has six more fields:
 * fs_base, gs_base, ds, es, fs, gs.
 * PTRACE_GETREGS fills them too, so struct pt_regs would overflow.
 */
struct i386_user_regs_struct {
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
	uint32_t esi;
	uint32_t edi;
	uint32_t ebp;
	uint32_t eax;
	uint32_t xds;
	uint32_t xes;
	uint32_t xfs;
	uint32_t xgs;
	uint32_t orig_eax;
	uint32_t eip;
	uint32_t xcs;
	uint32_t eflags;
	uint32_t esp;
	uint32_t xss;
};
#endif

struct pink_regset {
	short abi;

#if PINK_ARCH_ARM
	struct pt_regs arm_regs;
#elif PINK_ARCH_POWERPC
	struct pt_regs ppc_regs;
#elif PINK_ARCH_I386
	struct user_regs_struct i386_regs;
#elif PINK_ARCH_X86_64 || PINK_ARCH_X32
# ifndef __X32_SYSCALL_BIT
#  define __X32_SYSCALL_BIT	0x40000000
# endif
	struct iovec x86_io;
	union {
		struct user_regs_struct x86_64_r;
		struct i386_user_regs_struct i386_r;
	} x86_regs_union;
#elif PINK_ARCH_IA64
	bool ia32;
#else
#error "unsupported architecture"
#endif
};

#endif
