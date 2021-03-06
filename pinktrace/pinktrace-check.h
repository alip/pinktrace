/*
 * Copyright (c) 2012, 2013, 2021 Ali Polatel <alip@exherbo.org>
 * Based in part upon strace which is:
 *   Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 *   Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 *   Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 *   Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * SPDX-License-Identifier: LGPL-2.1-or-later
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

#include <stdarg.h>
#include <sys/types.h>
#include <setjmp.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>

#include "seatest.h"

extern unsigned _i;
extern pid_t _pgid;

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

#define PINKTRACE_FAIL_MSG_BUFSIZE 512
extern char _pinktrace_fail_msg[PINKTRACE_FAIL_MSG_BUFSIZE];
#define fail_verbose(...) \
	do { \
		pprintf(WARNING, __VA_ARGS__); \
		memset(_pinktrace_fail_msg, 0, PINKTRACE_FAIL_MSG_BUFSIZE * sizeof(char)); \
		snprintf(_pinktrace_fail_msg, PINKTRACE_FAIL_MSG_BUFSIZE, __VA_ARGS__); \
		seatest_simple_test_result(0, _pinktrace_fail_msg, __func__, __LINE__); \
	} while (0)

#define assert_true_verbose(x, fmt, ...) \
	do { \
		if (!(x)) { \
			fail_verbose((fmt), __VA_ARGS__); \
		} \
	} \
	while (0)

#define assert_false_verbose(x, fmt, ...) \
	do { \
		if ((x)) { \
			fail_verbose((fmt), __VA_ARGS__); \
		} \
	} \
	while (0)

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

void vm_lread_or_kill(pid_t pid, struct pink_regset *regset, long addr, char *dest, size_t len);
ssize_t vm_lread_nul_or_kill(pid_t pid, struct pink_regset *regset, long addr, char *dest, size_t len);

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

void test_suite_trace(void);
void test_suite_vm(void);
void test_suite_read(void);
void test_suite_write(void);
void test_suite_socket(void);
void test_suite_pipe(void);

#endif
