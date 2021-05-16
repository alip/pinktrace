/*
 * Copyright (c) 2012, 2013, 2021 Ali Polatel <alip@exherbo.org>
 * Based in part upon strace which is:
 *   Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 *   Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 *   Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 *   Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "pinktrace-check.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

static const unsigned int test_options = PINK_TRACE_OPTION_SYSGOOD;

/*
 * Test whether writing syscall works.
 * 0: Change getpid() to PINK_SYSCALL_INVALID and expect -ENOSYS.
 * 1: Change lseek(0, 0, 0) to open(0, ...); and expect -EFAULT.
 */
static void test_write_syscall(void)
{
	pid_t pid;
	struct pink_regset *regset;
	bool it_worked = false;
	bool insyscall = false;

#define TEST_GETPID 0
#define TEST_LSEEK 1
#define TEST_WRITE_SYSCALL_MAX 2
	int test = _i;
	const char *test_name = NULL;
	int errno_expected;
	long test_call, change_call;

	if (test == TEST_GETPID) {
		test_name = "getpid";
		errno_expected = ENOSYS;
		change_call = PINK_SYSCALL_INVALID;
		test_call = pink_lookup_syscall("getpid", PINK_ABI_DEFAULT);
		if (test_call == -1)
			fail_verbose("don't know the syscall number of getpid()");
	} else if (test == TEST_LSEEK) {
		test_name = "lseek";
		errno_expected = EFAULT;
		change_call = pink_lookup_syscall("openat", PINK_ABI_DEFAULT);
		if (change_call == -1)
			fail_verbose("don't know the syscall number of openat()");
		test_call = pink_lookup_syscall("lseek", PINK_ABI_DEFAULT);
		if (test_call == -1)
			fail_verbose("don't know the syscall number of lseek()\n");
	} else {
		fail_verbose("invalid loop number");
		abort();
	}

	message("test_syscall_%s: call:%ld expected errno:%d %s\n",
		test_name, test_call,
		errno_expected, strerror(errno_expected));

	pid = fork_assert();
	if (pid == 0) {
		pid = getpid();
		trace_me_and_stop();
		if (test == TEST_GETPID)
			syscall(test_call);
		else if (test == TEST_LSEEK)
			syscall(test_call, 0, 0, 0);
		_exit(0);
	}
#undef TEST_GETPID
#undef TEST_LSEEK
	regset_alloc_or_kill(pid, &regset);

	LOOP_WHILE_TRUE() {
		int status;
		pid_t tracee_pid;
		int error = 0;
		long rval, sysnum;

		tracee_pid = wait_verbose(&status);
		if (tracee_pid <= 0 && check_echild_or_kill(pid, tracee_pid))
			break;
		if (check_exit_code_or_fail(status, 0))
			break;
		check_signal_or_fail(status, 0);
		check_stopped_or_kill(tracee_pid, status);
		if (WSTOPSIG(status) == SIGSTOP) {
			trace_setup_or_kill(pid, test_options);
		} else if (WSTOPSIG(status) == (SIGTRAP|0x80)) {
			if (!insyscall) {
				regset_fill_or_kill(pid, regset);
				read_syscall_or_kill(pid, regset, &sysnum);
				check_syscall_equal_or_kill(pid, sysnum, test_call);
				write_syscall_or_kill(pid, regset, change_call);
				insyscall = true;
			} else {
				regset_fill_or_kill(pid, regset);
				read_retval_or_kill(pid, regset, &rval, &error);
				check_retval_equal_or_kill(pid, rval, -1, error, errno_expected);
				it_worked = true;
				kill(pid, SIGKILL);
				break;
			}
		}
		trace_syscall_or_kill(pid, 0);
	}

	if (!it_worked)
		fail_verbose("Test for writing system call `%s' failed", test_name);
}

/*
 * Test whether writing return value works
 * 0: Change getpid() return value to 0xdead and check exit status
 * 1: Change getpid() return to -EPERM and check exit status
 */
static void test_write_retval(void)
{
	pid_t pid;
	struct pink_regset *regset;
	bool it_worked = false;
	bool insyscall = false;
	bool write_done = false;
	long sys_getpid;

#define TEST_GOOD 0
#define TEST_FAIL 1
#define TEST_WRITE_RETVAL_MAX 2
	int test = _i;
	const char *test_name = NULL;
	int change_error;
	long change_retval;

	sys_getpid = pink_lookup_syscall("getpid", PINK_ABI_DEFAULT);
	if (sys_getpid == -1)
		fail_verbose("don't know the syscall number of getpid()");

	if (test == TEST_GOOD) {
		test_name = "good";
		change_error = 0;
		change_retval = 0xdead;
	} else {
		test_name = "fail";
		change_error = EPERM;
		change_retval = -1;
	}
	message("test_retval_%s: changing retval:%ld errno:%d %s\n",
		test_name, change_retval, change_error, strerror(change_error));
#undef TEST_GOOD
#undef TEST_FAIL

	pid = fork_assert();
	if (pid == 0) {
		int retval;
		trace_me_and_stop();
		retval = syscall(sys_getpid); /* glibc may cache getpid() */
		if (retval != change_retval || errno != change_error) {
			warning("\nchild: unexpected return %d (errno:%d %s)"
					", expected %ld (errno:%d %s)",
					retval,
					errno, strerror(errno),
					change_retval,
					change_error, strerror(change_error));
			_exit(EXIT_FAILURE);
		}
		_exit(EXIT_SUCCESS);
	}
	regset_alloc_or_kill(pid, &regset);

	LOOP_WHILE_TRUE() {
		int status;
		pid_t tracee_pid;

		tracee_pid = wait_verbose(&status);
		if (tracee_pid <= 0 && check_echild_or_kill(pid, tracee_pid))
			break;
		if (check_exit_code_or_fail(status, 0)) {
			it_worked = true;
			break;
		}
		check_signal_or_fail(status, 0);
		check_stopped_or_kill(tracee_pid, status);
		if (WSTOPSIG(status) == SIGSTOP) {
			trace_setup_or_kill(pid, test_options);
		} else if (!write_done && WSTOPSIG(status) == (SIGTRAP|0x80)) {
			if (!insyscall) {
				insyscall = true;
			} else {
				regset_fill_or_kill(pid, regset);
				write_retval_or_kill(pid, regset, change_retval, change_error);
				write_done = true;
			}
		}
		trace_syscall_or_kill(pid, 0);
	}

	if (!it_worked)
		fail_verbose("Test for reading return value of `%s' failed", test_name);
}

/*
 * Test whether writing syscall arguments works.
 * First fork a new child, call syscall(PINK_SYSCALL_INVALID, ...) with known
 * arguments. From parent write the argument on system call entry and then read
 * it on system call exit.
 */
static void test_write_argument(void)
{
	pid_t pid;
	struct pink_regset *regset;
	bool it_worked = false;
	bool insyscall = false;
	int arg_index = _i;
	long origval = 0xaaa;
	long newval = 0xbad;

	pid = fork_assert();
	if (pid == 0) {
		pid = getpid();
		trace_me_and_stop();
		switch (arg_index) {
		case 0: syscall(PINK_SYSCALL_INVALID, origval, 0, 0, 0, -1, 0); break;
		case 1: syscall(PINK_SYSCALL_INVALID, 0, origval, 0, 0, -1, 0); break;
		case 2: syscall(PINK_SYSCALL_INVALID, 0, 0, origval, 0, -1, 0); break;
		case 3: syscall(PINK_SYSCALL_INVALID, 0, 0, 0, origval, -1, 0); break;
		case 4: syscall(PINK_SYSCALL_INVALID, 0, 0, 0, 0, origval, 0);  break;
		case 5: syscall(PINK_SYSCALL_INVALID, 0, 0, 0, 0, -1, origval); break;
		default: _exit(1);
		}
		_exit(0);
	}
	regset_alloc_or_kill(pid, &regset);

	LOOP_WHILE_TRUE() {
		int status;
		pid_t tracee_pid;
		long argval, sysnum;

		tracee_pid = wait_verbose(&status);
		if (tracee_pid <= 0 && check_echild_or_kill(pid, tracee_pid))
			break;
		if (check_exit_code_or_fail(status, 0))
			break;
		check_signal_or_fail(status, 0);
		check_stopped_or_kill(tracee_pid, status);
		if (WSTOPSIG(status) == SIGSTOP) {
			trace_setup_or_kill(pid, test_options);
		} else if (WSTOPSIG(status) == (SIGTRAP|0x80)) {
			if (!insyscall) {
				regset_fill_or_kill(pid, regset);
				read_syscall_or_kill(pid, regset, &sysnum);
				check_syscall_equal_or_kill(pid, sysnum, PINK_SYSCALL_INVALID);
				write_argument_or_kill(pid, regset, arg_index, newval);
				regset_fill_or_kill(pid, regset);
				read_argument_or_kill(pid, regset, arg_index, &argval);
				check_argument_equal_or_kill(pid, argval, newval);
#if PINK_ARCH_AARCH64
				it_worked = true;
#endif
				insyscall = true;
			} else {
#if !PINK_ARCH_AARCH64
/* FIXME: Why does this not work on AArch64? */
				regset_fill_or_kill(pid, regset);
				read_argument_or_kill(pid, regset, arg_index, &argval);
				check_argument_equal_or_kill(pid, argval, newval);
				it_worked = true;
#endif
				kill(pid, SIGKILL);
				break;
			}
		}
		trace_syscall_or_kill(pid, 0);
	}

	if (!it_worked)
		fail_verbose("Test for writing syscall argument %d failed", arg_index);
}

/*
 * Test whether writing syscall VM data works.
 * First fork a new child, call syscall(PINK_SYSCALL_INVALID, ...) with known
 * arguments. From parent write VM data on system call entry and then read
 * it on system call exit.
 */
static void test_write_vm_data(void)
{
	pid_t pid;
	struct pink_regset *regset;
	bool it_worked = false;
	bool insyscall = false;
	int arg_index = _i;
	char origstr[] = "pinktrace";
	char newstr[] = "tracepink";
	char getstr[sizeof(newstr)];

	pid = fork_assert();
	if (pid == 0) {
		pid = getpid();
		trace_me_and_stop();
		switch (arg_index) {
		case 0: syscall(PINK_SYSCALL_INVALID, origstr, 0, 0, 0, -1, 0); break;
		case 1: syscall(PINK_SYSCALL_INVALID, 0, origstr, 0, 0, -1, 0); break;
		case 2: syscall(PINK_SYSCALL_INVALID, 0, 0, origstr, 0, -1, 0); break;
		case 3: syscall(PINK_SYSCALL_INVALID, 0, 0, 0, origstr, -1, 0); break;
		case 4: syscall(PINK_SYSCALL_INVALID, 0, 0, 0, 0, origstr, 0);  break;
		case 5: syscall(PINK_SYSCALL_INVALID, 0, 0, 0, 0, -1, origstr); break;
		default: _exit(1);
		}
		_exit(0);
	}
	regset_alloc_or_kill(pid, &regset);

	LOOP_WHILE_TRUE() {
		int status;
		pid_t tracee_pid;
		long argval, sysnum;

		tracee_pid = wait_verbose(&status);
		if (tracee_pid <= 0 && check_echild_or_kill(pid, tracee_pid))
			break;
		if (check_exit_code_or_fail(status, 0))
			break;
		check_signal_or_fail(status, 0);
		check_stopped_or_kill(tracee_pid, status);
		if (WSTOPSIG(status) == SIGSTOP) {
			trace_setup_or_kill(pid, test_options);
		} else if (WSTOPSIG(status) == (SIGTRAP|0x80)) {
			if (!insyscall) {
				regset_fill_or_kill(pid, regset);
				read_syscall_or_kill(pid, regset, &sysnum);
				check_syscall_equal_or_kill(pid, sysnum, PINK_SYSCALL_INVALID);
				read_argument_or_kill(pid, regset, arg_index, &argval);
				write_vm_data_or_kill(pid, regset, argval, newstr, sizeof(newstr));
#if PINK_ARCH_AARCH64
				read_vm_data_or_kill(pid, regset, argval, getstr, sizeof(getstr));
				if (strcmp(newstr, getstr) != 0) {
					kill(pid, SIGKILL);
					fail_verbose("VM data not identical"
						     " (expected:`%s' got:`%s')",
						     newstr, getstr);
				}
				it_worked = true;
#endif
				insyscall = true;
			} else {
#if !PINK_ARCH_AARCH64
/* FIXME: Why does this not work on AArch64? */
				regset_fill_or_kill(pid, regset);
				read_argument_or_kill(pid, regset, arg_index, &argval);
				read_vm_data_or_kill(pid, regset, argval, getstr, sizeof(getstr));
				if (strcmp(newstr, getstr) != 0) {
					kill(pid, SIGKILL);
					fail_verbose("VM data not identical"
						     " (expected:`%s' got:`%s')",
						     newstr, getstr);
				}
				it_worked = true;
#endif
				kill(pid, SIGKILL);
				break;
			}
		}
		trace_syscall_or_kill(pid, 0);
	}

	if (!it_worked)
		fail_verbose("Test for writing VM data to argument %d failed", arg_index);
}

static void test_fixture_write(void) {
	test_fixture_start();

	for (_i = 0; _i < TEST_WRITE_SYSCALL_MAX; _i++)
		run_test(test_write_syscall);
	for (_i = 0; _i < TEST_WRITE_RETVAL_MAX; _i++)
		run_test(test_write_retval);
	for (_i = 0; _i < PINK_MAX_ARGS; _i++)
		run_test(test_write_argument);
	for (_i = 0; _i < PINK_MAX_ARGS; _i++)
		run_test(test_write_vm_data);

	test_fixture_end();
}

void test_suite_write(void) {
	test_fixture_write();
}
