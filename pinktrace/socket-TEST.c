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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <sys/syscall.h>
#ifdef SYS_socketcall
#define SOCKDECODE	true
#define SYS_bind	PINK_SOCKET_SUBCALL_BIND
#define SYS_connect	PINK_SOCKET_SUBCALL_CONNECT
#define SYS_sendto	PINK_SOCKET_SUBCALL_SENDTO
#else
#define SOCKDECODE	false
#define SYS_socketcall	PINK_SYSCALL_INVALID
#endif

static const unsigned int test_options = PINK_TRACE_OPTION_SYSGOOD;

enum {
	TEST_SYS_BIND,
	TEST_SYS_CONNECT,
	TEST_SYS_SENDTO,
	TEST_SYS_MAX
};

static int test_sys_index(int test_sys)
{
	switch (test_sys) {
	case TEST_SYS_BIND:
	case TEST_SYS_CONNECT:
		return 1;
	case TEST_SYS_SENDTO:
		return 4;
	default:
		abort();
	}
}

static const char *test_sys_name(int test_sys)
{
	switch (test_sys) {
	case TEST_SYS_BIND:
		return "bind";
	case TEST_SYS_CONNECT:
		return "connect";
	case TEST_SYS_SENDTO:
		return "sendto";
	default:
		return "wtf?";
	}
}

static void check_socketcall_equal_or_kill(pid_t pid, int test_sys, long subcall)
{
	long subcall_expected;

	switch (test_sys) {
	case TEST_SYS_BIND:
		subcall_expected = SYS_bind;
		break;
	case TEST_SYS_CONNECT:
		subcall_expected = SYS_connect;
		break;
	case TEST_SYS_SENDTO:
		subcall_expected = SYS_sendto;
		break;
	default:
		abort();
	}

	if (subcall == subcall_expected)
		return;
	kill(pid, SIGKILL);
	fail_verbose("unexpected socketcall %ld (name:%s expected:%ld %s)",
			subcall,
			SOCKDECODE ? pink_name_socket_subcall(subcall)
				   : pink_name_syscall(subcall, PINK_ABI_DEFAULT),
			subcall_expected,
			subcall_expected == PINK_SYSCALL_INVALID
				? "PINK_SYSCALL_INVALID"
				: (SOCKDECODE ? pink_name_socket_subcall(subcall_expected)
					      : pink_name_syscall(subcall_expected, PINK_ABI_DEFAULT)));
	abort();
}

/*
 * Test whether reading NULL socket address works.
 * First fork a new child, call syscall(PINK_SYSCALL_INVALID, fd, NULL,...) with
 * a number and socket address and then check whether it's read correctly.
 */
static void test_read_socket_address_af_null(void)
{
	pid_t pid;
	struct pink_regset *regset;
	bool it_worked = false;
	int test_sys = _i;
	const char *test_name;
	int expfd = 23;
	int newfd;
	struct pink_sockaddr expaddr;
	struct pink_sockaddr newaddr;

	test_name = "test_af_null";
	expaddr.family = -1;
	info("Test: %s\n", test_name);

	pid = fork_assert();
	if (pid == 0) {
		pid = getpid();
		trace_me_and_stop();
		switch (test_sys) {
		case TEST_SYS_BIND:
			bind(expfd, NULL, 0);
			break;
		case TEST_SYS_CONNECT:
			connect(expfd, NULL, 0);
			break;
		case TEST_SYS_SENDTO:
			sendto(expfd, (void *)0xbad, 0xbad, 0xbad, NULL, 0);
			break;
		default:
			_exit(1);
		}
		_exit(0);
	}
	regset_alloc_or_kill(pid, &regset);

	LOOP_WHILE_TRUE() {
		int status;
		pid_t tracee_pid;
		long subcall;

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
			read_socket_subcall_or_kill(pid, regset, SOCKDECODE, &subcall);
			check_socketcall_equal_or_kill(pid, test_sys, subcall);
			read_socket_address_or_kill(pid, regset, SOCKDECODE,
						    test_sys_index(test_sys),
						    &newfd, &newaddr);
			if (newfd != expfd) {
				kill(pid, SIGKILL);
				fail_verbose("File descriptors not equal"
					     " (expected:%d got:%d)",
					     expfd, newfd);
			}
			if (newaddr.family != expaddr.family) {
				kill(pid, SIGKILL);
				fail_verbose("Address families not equal"
					     " (expected:%d got:%d)",
					     expaddr.family,
					     newaddr.family);
			}
			it_worked = true;
			kill(pid, SIGKILL);
			break;
		}
		trace_syscall_or_kill(pid, 0);
	}

	if (!it_worked)
		fail_verbose("%s: Test for reading socket address"
			     " for %s() failed",
			     test_name,
			     test_sys_name(test_sys));
}

/*
 * Test whether reading AF_UNIX socket address works.
 * First fork a new child, call syscall(PINK_SYSCALL_INVALID, fd, $sun_addr,...)
 * with a number and socket address and then check whether it's read correctly.
 */
static void test_read_socket_address_af_unix(void)
{
	pid_t pid;
	struct pink_regset *regset;
	bool it_worked = false;
	int test_sys = _i;
	const char *test_name;
	int expfd = 23;
	int newfd;
	struct pink_sockaddr expaddr;
	struct pink_sockaddr newaddr;
	socklen_t socklen = sizeof(struct sockaddr);

	test_name = "test_af_unix";
	expaddr.family = expaddr.u.sa_un.sun_family = AF_UNIX;
	strcpy(expaddr.u.sa_un.sun_path, "pinktrace");
	info("Test: %s\n", test_name);

	pid = fork_assert();
	if (pid == 0) {
		pid = getpid();
		trace_me_and_stop();
		switch (test_sys) {
		case TEST_SYS_BIND:
			bind(expfd, (struct sockaddr *)&expaddr.u.sa_un,
			     socklen);
			break;
		case TEST_SYS_CONNECT:
			connect(expfd, (struct sockaddr *)&expaddr.u.sa_un,
			     socklen);
			break;
		case TEST_SYS_SENDTO:
			sendto(expfd, (void *)0xbad, 0xbad, 0xbad,
				 (struct sockaddr *)&expaddr.u.sa_un,
				 socklen);
			break;
		default:
			_exit(1);
		}
		_exit(0);
	}
	regset_alloc_or_kill(pid, &regset);

	LOOP_WHILE_TRUE() {
		int status;
		pid_t tracee_pid;
		long subcall;

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
			read_socket_subcall_or_kill(pid, regset, SOCKDECODE, &subcall);
			check_socketcall_equal_or_kill(pid, test_sys, subcall);
			read_socket_address_or_kill(pid, regset, SOCKDECODE,
						    test_sys_index(test_sys),
						    &newfd, &newaddr);
			if (newfd != expfd) {
				kill(pid, SIGKILL);
				fail_verbose("File descriptors not equal"
					     " (expected:%d got:%d)",
					     expfd, newfd);
			}
			if (newaddr.family != expaddr.family) {
				kill(pid, SIGKILL);
				fail_verbose("Address families not equal"
					     " (expected:%d got:%d)",
					     expaddr.family,
					     newaddr.family);
			}
			if (strcmp(newaddr.u.sa_un.sun_path,
				   expaddr.u.sa_un.sun_path)) {
				kill(pid, SIGKILL);
				fail_verbose("AF_UNIX paths not identical"
						" (expected:`%s' got:`%s')",
						expaddr.u.sa_un.sun_path,
						newaddr.u.sa_un.sun_path);
			}
			it_worked = true;
			kill(pid, SIGKILL);
			break;
		}
		trace_syscall_or_kill(pid, 0);
	}

	if (!it_worked)
		fail_verbose("%s: Test for reading socket address"
			     " for %s() failed",
			     test_name,
			     test_sys_name(test_sys));
}

/*
 * Test whether reading AF_UNIX abstract socket address works.
 * First fork a new child, call syscall(PINK_SYSCALL_INVALID, fd, $sun_addr,...)
 * with a number and socket address and then check whether it's read correctly.
 */
static void test_read_socket_address_af_unixabs(void)
{
	pid_t pid;
	struct pink_regset *regset;
	bool it_worked = false;
	int test_sys = _i;
	const char *test_name;
	int expfd = 23;
	int newfd;
	struct pink_sockaddr expaddr;
	struct pink_sockaddr newaddr;
	socklen_t socklen = sizeof(struct sockaddr);

	test_name = "test_af_unixabs";
	expaddr.family = expaddr.u.sa_un.sun_family = AF_UNIX;
	strcpy(expaddr.u.sa_un.sun_path, "xpinktrace");
	expaddr.u.sa_un.sun_path[0] = '\0';
	info("Test: %s\n", test_name);

	pid = fork_assert();
	if (pid == 0) {
		pid = getpid();
		trace_me_and_stop();
		switch (test_sys) {
		case TEST_SYS_BIND:
			bind(expfd, (struct sockaddr *)&expaddr.u.sa_un,
			     socklen);
			break;
		case TEST_SYS_CONNECT:
			connect(expfd, (struct sockaddr *)&expaddr.u.sa_un,
				socklen);
			break;
		case TEST_SYS_SENDTO:
			sendto(expfd, (void *)0xbad, 0xbad, 0xbad,
				 (struct sockaddr *)&expaddr.u.sa_un,
				 socklen);
			break;
		default:
			_exit(1);
		}
		_exit(0);
	}
	regset_alloc_or_kill(pid, &regset);

	LOOP_WHILE_TRUE() {
		int status;
		pid_t tracee_pid;
		long subcall;

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
			read_socket_subcall_or_kill(pid, regset, SOCKDECODE, &subcall);
			check_socketcall_equal_or_kill(pid, test_sys, subcall);
			read_socket_address_or_kill(pid, regset, SOCKDECODE,
						    test_sys_index(test_sys),
						    &newfd, &newaddr);
			if (newfd != expfd) {
				kill(pid, SIGKILL);
				fail_verbose("File descriptors not equal"
					     " (expected:%d got:%d)",
					     expfd, newfd);
			}
			if (newaddr.family != expaddr.family) {
				kill(pid, SIGKILL);
				fail_verbose("Address families not equal"
					     " (expected:%d got:%d)",
					     expaddr.family,
					     newaddr.family);
			}
			if (newaddr.u.sa_un.sun_path[0] != '\0') {
				kill(pid, SIGKILL);
				fail_verbose("AF_UNIX path not abstract"
					     " (expected:`\\0' got:`%c')",
					     newaddr.u.sa_un.sun_path[0]);
			}
			if (strcmp(newaddr.u.sa_un.sun_path + 1,
				   expaddr.u.sa_un.sun_path + 1)) {
				kill(pid, SIGKILL);
				fail_verbose("AF_UNIX paths not identical"
						" (expected:`%s' got:`%s')",
						expaddr.u.sa_un.sun_path + 1,
						newaddr.u.sa_un.sun_path + 1);
			}
			it_worked = true;
			kill(pid, SIGKILL);
			break;
		}
		trace_syscall_or_kill(pid, 0);
	}

	if (!it_worked)
		fail_verbose("%s: Test for reading socket address for %s() failed",
			     test_name, test_sys_name(test_sys));
}

/*
 * Test whether reading AF_INET abstract socket address works.
 * First fork a new child, call syscall(PINK_SYSCALL_INVALID, fd, $sin_addr,...)
 * with a number and socket address and then check whether it's read correctly.
 */
static void test_read_socket_address_af_inet(void)
{
	pid_t pid;
	struct pink_regset *regset;
	bool it_worked = false;
	int test_sys = _i;
	const char *test_name;
	int expfd = 23;
	int newfd;
	struct pink_sockaddr expaddr;
	struct pink_sockaddr newaddr;
	socklen_t socklen = sizeof(struct sockaddr);

	test_name = "test_af_inet";
	expaddr.family = expaddr.u.sa_in.sin_family = AF_INET;
	expaddr.u.sa_in.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	expaddr.u.sa_in.sin_port = htons(1969); /* woodstock */
	info("Test: %s\n", test_name);

	pid = fork_assert();
	if (pid == 0) {
		pid = getpid();
		trace_me_and_stop();
		switch (test_sys) {
		case TEST_SYS_BIND:
			bind(expfd, (struct sockaddr *)&expaddr.u.sa_in,
			     socklen);
			break;
		case TEST_SYS_CONNECT:
			connect(expfd, (struct sockaddr *)&expaddr.u.sa_in,
				socklen);
			break;
		case TEST_SYS_SENDTO:
			sendto(expfd, (void *)0xbad, 0xbad, 0xbad,
				 (struct sockaddr *)&expaddr.u.sa_in,
				 socklen);
			break;
		default:
			_exit(1);
		}
		_exit(0);
	}
	regset_alloc_or_kill(pid, &regset);

	LOOP_WHILE_TRUE() {
		int status;
		pid_t tracee_pid;
		long subcall;

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
			read_socket_subcall_or_kill(pid, regset, SOCKDECODE, &subcall);
			check_socketcall_equal_or_kill(pid, test_sys, subcall);
			read_socket_address_or_kill(pid, regset, SOCKDECODE,
						    test_sys_index(test_sys),
						    &newfd, &newaddr);
			if (newfd != expfd) {
				kill(pid, SIGKILL);
				fail_verbose("File descriptors not equal"
					     " (expected:%d got:%d)",
					     expfd, newfd);
			}
			if (newaddr.family != expaddr.family) {
				kill(pid, SIGKILL);
				fail_verbose("Address families not equal"
					     " (expected:%d got:%d)",
					     expaddr.family,
					     newaddr.family);
			}
			if (expaddr.u.sa_in.sin_port != newaddr.u.sa_in.sin_port) {
				kill(pid, SIGKILL);
				fail_verbose("AF_INET ports not equal"
					     " (expected:%d got:%d)",
					     ntohs(expaddr.u.sa_in.sin_port),
					     ntohs(newaddr.u.sa_in.sin_port));
			}
			check_addr_loopback_or_kill(pid, newaddr.u.sa_in.sin_addr.s_addr);
			it_worked = true;
			kill(pid, SIGKILL);
			break;
		}
		trace_syscall_or_kill(pid, 0);
	}

	if (!it_worked)
		fail_verbose("%s: Test for reading socket address for %s() failed",
			     test_name, test_sys_name(test_sys));
}

/*
 * Test whether reading AF_INET6 abstract socket address works.
 * First fork a new child, call syscall(PINK_SYSCALL_INVALID, fd, $sin6_addr,...)
 * with a number and socket address and then check whether it's read correctly.
 */
static void test_read_socket_address_af_inet6(void)
{
#if !PINK_HAVE_IPV6
	message("PINK_HAVE_IPV6 is 0, skipping test\n");
	return;
#else
	pid_t pid;
	struct pink_regset *regset;
	bool it_worked = false;
	int test_sys = _i;
	const char *test_name;
	int expfd = 23;
	int newfd;
	struct pink_sockaddr expaddr;
	struct pink_sockaddr newaddr;
	socklen_t socklen = sizeof(struct sockaddr_in6);

	test_name = "test_af_inet6";
	expaddr.family = expaddr.u.sa6.sin6_family = AF_INET6;
	expaddr.u.sa6.sin6_addr = in6addr_loopback;
	expaddr.u.sa6.sin6_port = htons(1969);
	info("Test: %s\n", test_name);

	pid = fork_assert();
	if (pid == 0) {
		pid = getpid();
		trace_me_and_stop();
		switch (test_sys) {
		case TEST_SYS_BIND:
			bind(expfd, (struct sockaddr *)&expaddr.u.sa6,
			     socklen);
			break;
		case TEST_SYS_CONNECT:
			connect(expfd, (struct sockaddr *)&expaddr.u.sa6,
				socklen);
			break;
		case TEST_SYS_SENDTO:
			sendto(expfd, (void *)0xbad, 0xbad, 0xbad,
				 (struct sockaddr *)&expaddr.u.sa6,
				 socklen);
			break;
		default:
			_exit(1);
		}
		_exit(0);
	}
	regset_alloc_or_kill(pid, &regset);

	LOOP_WHILE_TRUE() {
		int status;
		pid_t tracee_pid;
		long subcall;

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
			read_socket_subcall_or_kill(pid, regset, SOCKDECODE, &subcall);
			check_socketcall_equal_or_kill(pid, test_sys, subcall);
			read_socket_address_or_kill(pid, regset, SOCKDECODE,
						    test_sys_index(test_sys),
						    &newfd, &newaddr);
			if (newfd != expfd) {
				kill(pid, SIGKILL);
				fail_verbose("File descriptors not equal"
					     " (expected:%d got:%d)",
					     expfd, newfd);
			}
			if (newaddr.family != expaddr.family) {
				kill(pid, SIGKILL);
				fail_verbose("Address families not equal"
					     " (expected:%d got:%d)",
					     expaddr.family,
					     newaddr.family);
			}
			if (expaddr.u.sa6.sin6_port != newaddr.u.sa6.sin6_port) {
				kill(pid, SIGKILL);
				fail_verbose("AF_INET6 ports not equal"
						" (expected:%d got:%d)",
						ntohs(expaddr.u.sa6.sin6_port),
						ntohs(newaddr.u.sa6.sin6_port));
			}
			check_addr6_loopback_or_kill(pid, &newaddr.u.sa6.sin6_addr);
			it_worked = true;
			kill(pid, SIGKILL);
			break;
		}
		trace_syscall_or_kill(pid, 0);
	}

	if (!it_worked)
		fail_verbose("%s: Test for reading socket address for %s() failed",
			     test_name, test_sys_name(test_sys));
#endif
}

/*
 * Test whether reading AF_NETLINK abstract socket address works.
 * First fork a new child, call syscall(PINK_SYSCALL_INVALID, fd, $nl_addr,...)
 * with a number and socket address and then check whether it's read correctly.
 */
static void test_read_socket_address_af_netlink(void)
{
#if !PINK_HAVE_NETLINK
	message("PINK_HAVE_NETLINK is 0, skipping test\n");
	return;
#else
	pid_t pid;
	struct pink_regset *regset;
	bool it_worked = false;
	int test_sys = _i;
	const char *test_name;
	int expfd = 23;
	int newfd;
	struct pink_sockaddr expaddr;
	struct pink_sockaddr newaddr;
	socklen_t socklen = sizeof(struct sockaddr_nl);

	test_name = "test_af_netlink";
	expaddr.family = expaddr.u.nl.nl_family = AF_NETLINK;
	expaddr.u.nl.nl_pid = 3;
	expaddr.u.nl.nl_groups = 3;
	info("Test: %s\n", test_name);

	pid = fork_assert();
	if (pid == 0) {
		pid = getpid();
		trace_me_and_stop();
		switch (test_sys) {
		case TEST_SYS_BIND:
			bind(expfd, (struct sockaddr *)&expaddr.u.nl,
			     socklen);
			break;
		case TEST_SYS_CONNECT:
			connect(expfd, (struct sockaddr *)&expaddr.u.nl,
				socklen);
			break;
		case TEST_SYS_SENDTO:
			sendto(expfd, (void *)0xbad, 0xbad, 0xbad,
				 (struct sockaddr *)&expaddr.u.nl,
				 socklen);
			break;
		default:
			_exit(1);
		}
		_exit(0);
	}
	regset_alloc_or_kill(pid, &regset);

	LOOP_WHILE_TRUE() {
		int status;
		pid_t tracee_pid;
		long subcall;

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
			read_socket_subcall_or_kill(pid, regset, SOCKDECODE, &subcall);
			check_socketcall_equal_or_kill(pid, test_sys, subcall);
			read_socket_address_or_kill(pid, regset, SOCKDECODE,
						    test_sys_index(test_sys),
						    &newfd, &newaddr);
			if (newfd != expfd) {
				kill(pid, SIGKILL);
				fail_verbose("File descriptors not equal"
					     " (expected:%d got:%d)",
					     expfd, newfd);
			}
			if (newaddr.family != expaddr.family) {
				kill(pid, SIGKILL);
				fail_verbose("Address families not equal"
					     " (expected:%d got:%d)",
					     expaddr.family,
					     newaddr.family);
			}
			if (expaddr.u.nl.nl_pid != newaddr.u.nl.nl_pid) {
				kill(pid, SIGKILL);
				fail_verbose("AF_NETLINK pids not equal"
					     " (expected:%u got:%u)",
					     expaddr.u.nl.nl_pid,
					     newaddr.u.nl.nl_pid);
			}
			if (expaddr.u.nl.nl_groups != newaddr.u.nl.nl_groups) {
				kill(pid, SIGKILL);
				fail_verbose("AF_NETLINK groups not equal"
					     " (expected:%u got:%u)",
					     expaddr.u.nl.nl_groups,
					     newaddr.u.nl.nl_groups);
			}
			it_worked = true;
			kill(pid, SIGKILL);
			break;
		}
		trace_syscall_or_kill(pid, 0);
	}

	if (!it_worked)
		fail_verbose("%s: Test for reading socket address for %s() failed",
			     test_name, test_sys_name(test_sys));
#endif
}

static void test_fixture_socket(void) {
	test_fixture_start();

	for (_i = TEST_SYS_BIND; _i < TEST_SYS_MAX; _i++)
		run_test(test_read_socket_address_af_null);
	for (_i = TEST_SYS_BIND; _i < TEST_SYS_MAX; _i++)
		run_test(test_read_socket_address_af_unix);
	for (_i = TEST_SYS_BIND; _i < TEST_SYS_MAX; _i++)
		run_test(test_read_socket_address_af_unixabs);
	for (_i = TEST_SYS_BIND; _i < TEST_SYS_MAX; _i++)
		run_test(test_read_socket_address_af_inet);
	for (_i = TEST_SYS_BIND; _i < TEST_SYS_MAX; _i++)
		run_test(test_read_socket_address_af_inet6);
	for (_i = TEST_SYS_BIND; _i < TEST_SYS_MAX; _i++)
		run_test(test_read_socket_address_af_netlink);

	test_fixture_end();
}

void test_suite_socket(void) {
	test_fixture_socket();
}
