/*
 * Copyright (c) 2012, 2013, 2014, 2021 Ali Polatel <alip@exherbo.org>
 * Based in part upon strace which is:
 *   Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 *   Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 *   Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 *   Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "pinktrace-check.h"

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

static const unsigned int test_options = PINK_TRACE_OPTION_SYSGOOD;

/*
 * Test whether reading tracee's address space works using ptrace(2)
 * First fork a new child, call syscall(PINK_SYSCALL_INVALID, ...) with
 * a filled 'struct stat' and then check whether it's read correctly.
 */
static void test_vm_lread(void)
{
	pid_t pid;
	struct pink_regset *regset;
	bool it_worked = false;
	int arg_index = _i;
	char expstr[] = "pinktrace";
	char newstr[sizeof(expstr)];

	pid = fork_assert();
	if (pid == 0) {
		pid = getpid();
		trace_me_and_stop();
		switch (arg_index) {
		case 0: syscall(PINK_SYSCALL_INVALID, expstr, 0, 0, 0, -1, 0); break;
		case 1: syscall(PINK_SYSCALL_INVALID, 0, expstr, 0, 0, -1, 0); break;
		case 2: syscall(PINK_SYSCALL_INVALID, 0, 0, expstr, 0, -1, 0); break;
		case 3: syscall(PINK_SYSCALL_INVALID, 0, 0, 0, expstr, -1, 0); break;
		case 4: syscall(PINK_SYSCALL_INVALID, 0, 0, 0, 0, expstr, 0);  break;
		case 5: syscall(PINK_SYSCALL_INVALID, 0, 0, 0, 0, -1, expstr); break;
		default: _exit(1);
		}
		_exit(1); /* expect to be killed */
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
			regset_fill_or_kill(pid, regset);
			read_syscall_or_kill(pid, regset, &sysnum);
			check_syscall_equal_or_kill(pid, sysnum, PINK_SYSCALL_INVALID);
			read_argument_or_kill(pid, regset, arg_index, &argval);
			vm_lread_or_kill(pid, regset, argval, newstr, sizeof(expstr));
			check_memory_equal_or_kill(pid, newstr, expstr, sizeof(expstr));
			it_worked = true;
			kill(pid, SIGKILL);
			break;
		}
		trace_syscall_or_kill(pid, 0);
	}

	if (!it_worked)
		fail_verbose("Test for (ptrace) reading VM data at argument %d failed", arg_index);
}

/*
 * Test whether reading tracee's address space works using ptrace(2).
 * First fork a new child, call syscall(PINK_SYSCALL_INVALID, ...) with a
 * string containing '\0' in the middle and then check whether it's read
 * correctly.
 */
static void test_vm_lread_nul(void)
{
	pid_t pid;
	struct pink_regset *regset;
	bool it_worked = false;
	int arg_index = _i;
	char expstr[] = "trace\0pink"; /* Pink hiding behind the wall again... */
	char newstr[sizeof(expstr)];
#define EXPSTR_LEN 6

	pid = fork_assert();
	if (pid == 0) {
		pid = getpid();
		trace_me_and_stop();
		switch (arg_index) {
		case 0: syscall(PINK_SYSCALL_INVALID, expstr, 0, 0, 0, -1, 0); break;
		case 1: syscall(PINK_SYSCALL_INVALID, 0, expstr, 0, 0, -1, 0); break;
		case 2: syscall(PINK_SYSCALL_INVALID, 0, 0, expstr, 0, -1, 0); break;
		case 3: syscall(PINK_SYSCALL_INVALID, 0, 0, 0, expstr, -1, 0); break;
		case 4: syscall(PINK_SYSCALL_INVALID, 0, 0, 0, 0, expstr, 0);  break;
		case 5: syscall(PINK_SYSCALL_INVALID, 0, 0, 0, 0, -1, expstr); break;
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
			regset_fill_or_kill(pid, regset);
			read_syscall_or_kill(pid, regset, &sysnum);
			check_syscall_equal_or_kill(pid, sysnum, PINK_SYSCALL_INVALID);
			read_argument_or_kill(pid, regset, arg_index, &argval);
			vm_lread_nul_or_kill(pid, regset, argval, newstr, sizeof(expstr));
			check_string_equal_or_kill(pid, newstr, expstr, EXPSTR_LEN);
			it_worked = true;
			kill(pid, SIGKILL);
			break;
		}
		trace_syscall_or_kill(pid, 0);
	}

	if (!it_worked)
		fail_verbose("Test for (ptrace) reading nul-terminated VM data at argument %d failed",
			     arg_index);
}

/*
 * Test whether reading tracee's address space works for subsequent reads.
 * First fork a new child, call syscall(PINK_SYSCALL_INVALID, ...) with a string
 * longer than sizeof(long) then check whether it's read correctly.
 */
static void test_vm_lread_nul_long(void)
{
	pid_t pid;
	struct pink_regset *regset;
	bool it_worked = false;
	int arg_index = _i;
	char expstr[PATH_MAX]; /* PATH_MAX should be sufficiently large, see below */
	char newstr[PATH_MAX]; /* ditto */

	/*
	 * IMPORTANT: pink_vm_lread_nul() function reads one long at a time!
	 * To be able to check whether looking for the zero-byte works correctly
	 * we need to test for two subsequent reads.
	 */
	ssize_t r;
	unsigned l;
	unsigned fill = sizeof(long) / sizeof(char);
	for (l = 0; l < fill; l++)
		expstr[l] = 'P'; /* for pi, for pink, for pink floyd! */
	expstr[l++] = 'i';
	expstr[l++] = 'n';
	expstr[l++] = 'k';
	expstr[l++] = ' ';
	expstr[l++] = 'F';
	expstr[l++] = 'l';
	expstr[l++] = 'o';
	expstr[l++] = 'y';
	expstr[l++] = 'd';
	expstr[l] = '\0';

	pid = fork_assert();
	if (pid == 0) {
		pid = getpid();
		trace_me_and_stop();
		switch (arg_index) {
		case 0: syscall(PINK_SYSCALL_INVALID, expstr, 3, 3, 3, 3, 3); break;
		case 1: syscall(PINK_SYSCALL_INVALID, 3, expstr, 3, 3, 3, 3); break;
		case 2: syscall(PINK_SYSCALL_INVALID, 3, 3, expstr, 3, 3, 3); break;
		case 3: syscall(PINK_SYSCALL_INVALID, 3, 3, 3, expstr, 3, 3); break;
		case 4: syscall(PINK_SYSCALL_INVALID, 3, 3, 3, 3, expstr, 3);  break;
		case 5: syscall(PINK_SYSCALL_INVALID, 3, 3, 3, 3, 3, expstr); break;
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
			regset_fill_or_kill(pid, regset);
			read_syscall_or_kill(pid, regset, &sysnum);
			check_syscall_equal_or_kill(pid, sysnum, PINK_SYSCALL_INVALID);
			read_argument_or_kill(pid, regset, arg_index, &argval);
			r = vm_lread_nul_or_kill(pid, regset, argval, newstr, l);
			info("read_vm_data_nul() returned r:%zu for l:%d\n", r, l);
			if ((size_t)r <= l)
				newstr[r] = '\0';
			check_string_endswith_or_kill(pid, newstr, "Pink Floyd");
			it_worked = true;
			kill(pid, SIGKILL);
			break;
		}
		trace_syscall_or_kill(pid, 0);
	}

	if (!it_worked)
		fail_verbose("Test for (ptrace) reading nul-terminated VM data subsequently at argument %d failed",
			     arg_index);
}

static void test_fixture_vm(void) {
	test_fixture_start();

	for (_i = 0; _i < PINK_MAX_ARGS; _i++)
		run_test(test_vm_lread);
	for (_i = 0; _i < PINK_MAX_ARGS; _i++)
		run_test(test_vm_lread_nul);
	for (_i = 0; _i < PINK_MAX_ARGS; _i++)
		run_test(test_vm_lread_nul_long);

	test_fixture_end();
}

void test_suite_vm(void) {
	test_fixture_vm();
}
