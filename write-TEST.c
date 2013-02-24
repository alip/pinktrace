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

#include "pinktrace-check.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

/*
 * Test whether writing syscall works.
 * 0: Change getpid() to PINK_SYSCALL_INVALID and expect -ENOSYS.
 * 1: Change lseek(0, 0, 0) to open(0, ...); and expect -EFAULT.
 */
START_TEST(TEST_write_syscall)
{
	pid_t pid;
	struct pink_process *current;
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
		change_call = pink_lookup_syscall("open", PINK_ABI_DEFAULT);
		if (change_call == -1)
			fail_verbose("don't know the syscall number of open()");
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
	process_alloc_or_kill(pid, &current);

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
		if (WSTOPSIG(status) == SIGTRAP) {
			if (!insyscall) {
				process_update_regset_or_kill(current);
				read_syscall_or_kill(current, &sysnum);
				check_syscall_equal_or_kill(pid, sysnum, test_call);
				write_syscall_or_kill(current, change_call);
				insyscall = true;
			} else {
				process_update_regset_or_kill(current);
				read_retval_or_kill(current, &rval, &error);
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
END_TEST

/*
 * Test whether writing return value works
 * 0: Change getpid() return value to 0xdead and check exit status
 * 1: Change getpid() return to -EPERM and check exit status
 */
START_TEST(TEST_write_retval)
{
	pid_t pid;
	struct pink_process *current;
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
	process_alloc_or_kill(pid, &current);

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
		if (!write_done && WSTOPSIG(status) == SIGTRAP) {
			if (!insyscall) {
				insyscall = true;
			} else {
				process_update_regset_or_kill(current);
				write_retval_or_kill(current, change_retval, change_error);
				write_done = true;
			}
		}
		trace_syscall_or_kill(pid, 0);
	}

	if (!it_worked)
		fail_verbose("Test for reading return value of `%s' failed", test_name);
}
END_TEST

/*
 * Test whether writing syscall arguments works.
 * First fork a new child, call syscall(PINK_SYSCALL_INVALID, ...) with known
 * arguments. From parent write the argument on system call entry and then read
 * it on system call exit.
 */
START_TEST(TEST_write_argument)
{
	pid_t pid;
	struct pink_process *current;
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
	process_alloc_or_kill(pid, &current);

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
		if (WSTOPSIG(status) == SIGTRAP) {
			if (!insyscall) {
				process_update_regset_or_kill(current);
				read_syscall_or_kill(current, &sysnum);
				check_syscall_equal_or_kill(pid, sysnum, PINK_SYSCALL_INVALID);
				write_argument_or_kill(current, arg_index, newval);
				insyscall = true;
			} else {
				process_update_regset_or_kill(current);
				read_argument_or_kill(current, arg_index, &argval);
				check_argument_equal_or_kill(pid, argval, newval);
				it_worked = true;
				kill(pid, SIGKILL);
				break;
			}
		}
		trace_syscall_or_kill(pid, 0);
	}

	if (!it_worked)
		fail_verbose("Test for writing syscall argument %d failed", arg_index);
}
END_TEST

/*
 * Test whether writing syscall VM data works.
 * First fork a new child, call syscall(PINK_SYSCALL_INVALID, ...) with known
 * arguments. From parent write VM data on system call entry and then read
 * it on system call exit.
 */
START_TEST(TEST_write_vm_data)
{
	pid_t pid;
	struct pink_process *current;
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
	process_alloc_or_kill(pid, &current);

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
		if (WSTOPSIG(status) == SIGTRAP) {
			if (!insyscall) {
				process_update_regset_or_kill(current);
				read_syscall_or_kill(current, &sysnum);
				check_syscall_equal_or_kill(pid, sysnum, PINK_SYSCALL_INVALID);
				read_argument_or_kill(current, arg_index, &argval);
				write_vm_data_or_kill(current, argval, newstr, sizeof(newstr));
				insyscall = true;
			} else {
				process_update_regset_or_kill(current);
				read_argument_or_kill(current, arg_index, &argval);
				read_vm_data_or_kill(current, argval, getstr, sizeof(getstr));
				if (strcmp(newstr, getstr) != 0) {
					kill(pid, SIGKILL);
					fail_verbose("VM data not identical"
						     " (expected:`%s' got:`%s')",
						     newstr, getstr);
				}
				it_worked = true;
				kill(pid, SIGKILL);
				break;
			}
		}
		trace_syscall_or_kill(pid, 0);
	}

	if (!it_worked)
		fail_verbose("Test for writing VM data to argument %d failed", arg_index);
}
END_TEST

TCase *create_testcase_write(void)
{
	TCase *tc = tcase_create("write");

	tcase_add_loop_test(tc, TEST_write_syscall, 0, TEST_WRITE_SYSCALL_MAX);
	tcase_add_loop_test(tc, TEST_write_retval, 0, TEST_WRITE_RETVAL_MAX);
	tcase_add_loop_test(tc, TEST_write_argument, 0, PINK_MAX_ARGS);
	tcase_add_loop_test(tc, TEST_write_vm_data, 0, PINK_MAX_ARGS);

	return tc;
}
