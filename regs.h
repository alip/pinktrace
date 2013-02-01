/*
 * Copyright (c) 2012 Ali Polatel <alip@exherbo.org>
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

#ifndef PINK_REGS_H
#define PINK_REGS_H

/**
 * @file pinktrace/regs.h
 * @brief Pink's structure of registers
 *
 * Do not include this file directly. Use pinktrace/pink.h instead.
 *
 * @defgroup pink_regs Pink's structure of registers
 * @ingroup pinktrace
 * @{
 **/

/**
 * @def PINK_HAVE_REGS_T
 * Define to 1 if pink_regs_t is supported, 0 otherwise
 *
 * @note This structure is not supported on ia64. On such architectures, the
 * caller may pass @e NULL as the pointer of structures argument for functions
 * which request it (e.g. pink_trace_get_regs() ). In this case, the function
 * always returns false and sets @e errno to @e ENOTSUP.
 **/

/**
 * @typedef pink_regs_t
 * Defined to the structure of registers, or void in case it's unsupported.
 * Below is a list showing how this structure is defined in various
 * architectures:
 *  - arm: struct pt_regs
 *  - ppc: struct pt_regs
 *  - ppc64: struct pt_regs
 *  - x86: struct pt_regs
 *  - x32: struct user_regs_struct
 *  - x86_64: struct user_regs_struct
 *  - ia64: char (not supported)
 *
 * @see PINK_HAVE_REGS_T
 **/

#if PINK_ARCH_ARM || PINK_ARCH_POWERPC || PINK_ARCH_I386
# include <asm/ptrace.h>
typedef struct pt_regs pink_regs_t;
#elif PINK_ARCH_X86_64 || PINK_ARCH_X32
# include <sys/types.h>
# include <sys/user.h>
typedef struct user_regs_struct pink_regs_t;
#elif PINK_ARCH_IA64
# define PINK_HAVE_REGS_T 0
typedef char pink_regs_t;
#else
# error unsupported architecture
#endif

#ifndef PINK_HAVE_REGS_T
# define PINK_HAVE_REGS_T 1
#endif

#if !PINK_HAVE_REGS_T
# warning "pink_regs_t not supported for this architecture!"
#endif

/** @} */
#endif
