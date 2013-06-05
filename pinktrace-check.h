/*
 * Copyright (c) 2012, 2013 Ali Polatel <alip@exherbo.org>
 * Based in part upon strace which is:
 *   Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 *   Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 *   Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 *   Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
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

#ifndef _PINKTRACE_CHECK_H
#define _PINKTRACE_CHECK_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifndef _ATFILE_SOURCE
#define _ATFILE_SOURCE 1
#endif

#include <pinktrace/private.h>
#include <pinktrace/pink.h>
#include <check.h>

#include <stdarg.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>

#ifdef KERNEL_VERSION
# undef KERNEL_VERSION
#endif
#define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
extern unsigned os_release;

#define DEBUG	0
#define INFO	1
#define MESSAGE	2
#define WARNING	3
int pprintf_va(int pretty, const char *format, va_list ap)
	PINK_GCC_ATTR((format (printf, 2, 0)));
int pprintf(int pretty, const char *format, ...)
	PINK_GCC_ATTR((format (printf, 2, 3)));
#define debug(...)	pprintf(DEBUG, __VA_ARGS__)
#define info(...)	pprintf(INFO, __VA_ARGS__)
#define message(...)	pprintf(MESSAGE, __VA_ARGS__)
#define warning(...)	pprintf(WARNING, __VA_ARGS__)

#define fail_verbose(...)			\
	do {					\
		pprintf(WARNING, __VA_ARGS__);	\
		fail(__VA_ARGS__);		\
	} while (0)

#define fail_if_verbose(x, fmt, ...)				\
	do {							\
		if ((x)) {					\
			fail_verbose((fmt), __VA_ARGS__);	\
		}						\
	} while (0)

#define fail_unless_verbose(x, fmt, ...)			\
	do {							\
		if (!(x)) {					\
			fail_verbose((fmt), __VA_ARGS__);	\
		}						\
	} while (0)

pid_t fork_assert(void);
void kill_save_errno(pid_t pid, int sig);

pid_t waitpid_no_intr(pid_t pid, int *status, int options);
pid_t waitpid_no_intr_debug(unsigned loopcnt,
		const char *file, const char *func, int linecnt,
		pid_t pid, int *status, int options);
pid_t wait_no_intr(int *status);
pid_t wait_no_intr_debug(unsigned loopcnt,
		const char *file, const char *func, int linecnt,
		int *status);
#define LOOP_WHILE_TRUE()	for (unsigned _pink_loopcnt = 0;;_pink_loopcnt++)
#define wait_verbose(status)	wait_no_intr_debug(_pink_loopcnt, __FILE__, __func__, __LINE__, (status))

bool check_echild_or_kill(pid_t pid, pid_t retval);
bool check_exit_code_or_fail(int status, int code);
bool check_signal_or_fail(int status, int sig);
bool check_stopped_or_kill(pid_t pid, int status);

void check_syscall_equal_or_kill(pid_t pid,
				 long sysnum, long sysnum_expected);
void check_retval_equal_or_kill(pid_t pid,
				long retval, long retval_expected,
				int error, int error_expected);
void check_argument_equal_or_kill(pid_t pid,
				  long arg, long arg_expected);
void check_memory_equal_or_kill(pid_t pid,
				const void *val,
				const void *val_expected,
				size_t n);
void check_string_equal_or_kill(pid_t pid,
				const char *str,
				const char *str_expected,
				size_t len);
void check_string_endswith_or_kill(pid_t pid, const char *str,
				   const char *suffix_expected);
void check_addr_loopback_or_kill(pid_t pid, in_addr_t addr);
#if PINK_HAVE_IPV6
void check_addr6_loopback_or_kill(pid_t pid, struct in6_addr *addr6);
#endif

void trace_me_and_stop(void);
void trace_syscall_or_kill(pid_t pid, int sig);
void trace_setup_or_kill(pid_t pid, int options);
void trace_geteventmsg_or_kill(pid_t pid, unsigned long *data);

enum pink_event event_decide_and_print(int status);

void regset_alloc_or_kill(pid_t pid, struct pink_regset **regptr);
void regset_fill_or_kill(pid_t pid, struct pink_regset *regset);

void read_syscall_or_kill(pid_t pid, struct pink_regset *regset, long *sysnum);
void read_retval_or_kill(pid_t pid, struct pink_regset *regset, long *retval, int *error);
void read_argument_or_kill(pid_t pid, struct pink_regset *regset, unsigned arg_index, long *argval);
void read_vm_data_or_kill(pid_t pid, struct pink_regset *regset, long addr, char *dest, size_t len);
ssize_t read_vm_data_nul_or_kill(pid_t pid, struct pink_regset *regset, long addr, char *dest, size_t len);
void read_string_array_or_kill(pid_t pid, struct pink_regset *regset,
			       long arg, unsigned arr_index,
			       char *dest, size_t dest_len,
			       bool *nullptr);
void read_socket_subcall_or_kill(pid_t pid, struct pink_regset *regset,
				 bool decode_socketcall,
				 long *subcall);
void read_socket_argument_or_kill(pid_t pid, struct pink_regset *regset, bool decode_socketcall,
				  unsigned arg_index, unsigned long *argval);
void read_socket_address_or_kill(pid_t pid, struct pink_regset *regset, bool decode_socketcall,
				 unsigned arg_index, int *fd,
				 struct pink_sockaddr *sockaddr);

void write_syscall_or_kill(pid_t pid, struct pink_regset *regset, long sysnum);
void write_retval_or_kill(pid_t pid, struct pink_regset *regset, long retval, int error);
void write_argument_or_kill(pid_t pid, struct pink_regset *regset, unsigned arg_index, long argval);
void write_vm_data_or_kill(pid_t pid, struct pink_regset *regset, long addr, const char *src, size_t len);

TCase *create_testcase_trace(void);
TCase *create_testcase_read(void);
TCase *create_testcase_write(void);
TCase *create_testcase_socket(void);

#endif
