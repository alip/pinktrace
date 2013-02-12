/*
 * Copyright (c) 2012 Ali Polatel <alip@exherbo.org>
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

static void check_socketcall_equal_or_kill(pid_t pid, enum pink_abi abi,
					   int test_sys, long subcall)
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
	fail_verbose("unexpected socketcall %ld"
			" (name:%s expected:%ld %s)",
			subcall,
			SOCKDECODE ? pink_socket_subcall_name(subcall)
				   : pink_syscall_name(subcall, abi),
			subcall_expected,
			subcall_expected == PINK_SYSCALL_INVALID
				? "PINK_SYSCALL_INVALID"
				: (SOCKDECODE ? pink_socket_subcall_name(subcall_expected)
					      : pink_syscall_name(subcall_expected, abi)));
	abort();
}

/*
 * Test whether reading NULL socket address works.
 * First fork a new child, call syscall(PINK_SYSCALL_INVALID, fd, NULL,...) with
 * a number and socket address and then check whether it's read correctly.
 */
START_TEST(TEST_read_socket_address_af_null)
{
	pid_t pid;
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

	LOOP_WHILE_TRUE() {
		int status;
		pid_t tracee_pid;
		enum pink_abi abi;
		long subcall;
		pink_regs_t regs;

		tracee_pid = wait_verbose(&status);
		if (tracee_pid <= 0 && check_echild_or_kill(pid, tracee_pid))
			break;
		if (check_exit_code_or_fail(status, 0))
			break;
		check_signal_or_fail(status, 0);
		check_stopped_or_kill(tracee_pid, status);
		if (WSTOPSIG(status) == SIGTRAP) {
			trace_get_regs_or_kill(pid, &regs);
			read_abi_or_kill(pid, &regs, &abi);
			read_socket_subcall_or_kill(pid, abi, &regs, SOCKDECODE,
						    &subcall);
			check_socketcall_equal_or_kill(pid, abi, test_sys,
						       subcall);
			read_socket_address_or_kill(pid, abi, &regs, SOCKDECODE,
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
END_TEST

/*
 * Test whether reading AF_UNIX socket address works.
 * First fork a new child, call syscall(PINK_SYSCALL_INVALID, fd, $sun_addr,...)
 * with a number and socket address and then check whether it's read correctly.
 */
START_TEST(TEST_read_socket_address_af_unix)
{
	pid_t pid;
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

	LOOP_WHILE_TRUE() {
		int status;
		pid_t tracee_pid;
		enum pink_abi abi;
		long subcall;
		pink_regs_t regs;

		tracee_pid = wait_verbose(&status);
		if (tracee_pid <= 0 && check_echild_or_kill(pid, tracee_pid))
			break;
		if (check_exit_code_or_fail(status, 0))
			break;
		check_signal_or_fail(status, 0);
		check_stopped_or_kill(tracee_pid, status);
		if (WSTOPSIG(status) == SIGTRAP) {
			trace_get_regs_or_kill(pid, &regs);
			read_abi_or_kill(pid, &regs, &abi);
			read_socket_subcall_or_kill(pid, abi, &regs, SOCKDECODE,
						    &subcall);
			check_socketcall_equal_or_kill(pid, abi, test_sys,
						       subcall);
			read_socket_address_or_kill(pid, abi, &regs, SOCKDECODE,
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
END_TEST

/*
 * Test whether reading AF_UNIX abstract socket address works.
 * First fork a new child, call syscall(PINK_SYSCALL_INVALID, fd, $sun_addr,...)
 * with a number and socket address and then check whether it's read correctly.
 */
START_TEST(TEST_read_socket_address_af_unixabs)
{
	pid_t pid;
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

	LOOP_WHILE_TRUE() {
		int status;
		pid_t tracee_pid;
		enum pink_abi abi;
		long subcall;
		pink_regs_t regs;

		tracee_pid = wait_verbose(&status);
		if (tracee_pid <= 0 && check_echild_or_kill(pid, tracee_pid))
			break;
		if (check_exit_code_or_fail(status, 0))
			break;
		check_signal_or_fail(status, 0);
		check_stopped_or_kill(tracee_pid, status);
		if (WSTOPSIG(status) == SIGTRAP) {
			trace_get_regs_or_kill(pid, &regs);
			read_abi_or_kill(pid, &regs, &abi);
			read_socket_subcall_or_kill(pid, abi, &regs, SOCKDECODE,
						    &subcall);
			check_socketcall_equal_or_kill(pid, abi, test_sys,
						       subcall);
			read_socket_address_or_kill(pid, abi, &regs, SOCKDECODE,
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
		fail_verbose("%s: Test for reading socket address"
			     " for %s() failed",
			     test_name,
			     test_sys_name(test_sys));
}
END_TEST

/*
 * Test whether reading AF_INET abstract socket address works.
 * First fork a new child, call syscall(PINK_SYSCALL_INVALID, fd, $sin_addr,...)
 * with a number and socket address and then check whether it's read correctly.
 */
START_TEST(TEST_read_socket_address_af_inet)
{
	pid_t pid;
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

	LOOP_WHILE_TRUE() {
		int status;
		pid_t tracee_pid;
		enum pink_abi abi;
		long subcall;
		pink_regs_t regs;

		tracee_pid = wait_verbose(&status);
		if (tracee_pid <= 0 && check_echild_or_kill(pid, tracee_pid))
			break;
		if (check_exit_code_or_fail(status, 0))
			break;
		check_signal_or_fail(status, 0);
		check_stopped_or_kill(tracee_pid, status);
		if (WSTOPSIG(status) == SIGTRAP) {
			trace_get_regs_or_kill(pid, &regs);
			read_abi_or_kill(pid, &regs, &abi);
			read_socket_subcall_or_kill(pid, abi, &regs, SOCKDECODE,
						    &subcall);
			check_socketcall_equal_or_kill(pid, abi, test_sys,
						       subcall);
			read_socket_address_or_kill(pid, abi, &regs, SOCKDECODE,
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
		fail_verbose("%s: Test for reading socket address"
			     " for %s() failed",
			     test_name,
			     test_sys_name(test_sys));
}
END_TEST

/*
 * Test whether reading AF_INET6 abstract socket address works.
 * First fork a new child, call syscall(PINK_SYSCALL_INVALID, fd, $sin6_addr,...)
 * with a number and socket address and then check whether it's read correctly.
 */
START_TEST(TEST_read_socket_address_af_inet6)
{
#if !PINK_HAVE_IPV6
	message("PINK_HAVE_IPV6 is 0, skipping test\n");
	return;
#else
	pid_t pid;
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

	LOOP_WHILE_TRUE() {
		int status;
		pid_t tracee_pid;
		enum pink_abi abi;
		long subcall;
		pink_regs_t regs;

		tracee_pid = wait_verbose(&status);
		if (tracee_pid <= 0 && check_echild_or_kill(pid, tracee_pid))
			break;
		if (check_exit_code_or_fail(status, 0))
			break;
		check_signal_or_fail(status, 0);
		check_stopped_or_kill(tracee_pid, status);
		if (WSTOPSIG(status) == SIGTRAP) {
			trace_get_regs_or_kill(pid, &regs);
			read_abi_or_kill(pid, &regs, &abi);
			read_socket_subcall_or_kill(pid, abi, &regs, SOCKDECODE,
						    &subcall);
			check_socketcall_equal_or_kill(pid, abi, test_sys,
						       subcall);
			read_socket_address_or_kill(pid, abi, &regs, SOCKDECODE,
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
		fail_verbose("%s: Test for reading socket address"
			     " for %s() failed",
			     test_name,
			     test_sys_name(test_sys));
#endif
}
END_TEST

/*
 * Test whether reading AF_NETLINK abstract socket address works.
 * First fork a new child, call syscall(PINK_SYSCALL_INVALID, fd, $nl_addr,...)
 * with a number and socket address and then check whether it's read correctly.
 */
START_TEST(TEST_read_socket_address_af_netlink)
{
#if !PINK_HAVE_NETLINK
	message("PINK_HAVE_NETLINK is 0, skipping test\n");
	return;
#else
	pid_t pid;
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

	LOOP_WHILE_TRUE() {
		int status;
		pid_t tracee_pid;
		enum pink_abi abi;
		long subcall;
		pink_regs_t regs;

		tracee_pid = wait_verbose(&status);
		if (tracee_pid <= 0 && check_echild_or_kill(pid, tracee_pid))
			break;
		if (check_exit_code_or_fail(status, 0))
			break;
		check_signal_or_fail(status, 0);
		check_stopped_or_kill(tracee_pid, status);
		if (WSTOPSIG(status) == SIGTRAP) {
			trace_get_regs_or_kill(pid, &regs);
			read_abi_or_kill(pid, &regs, &abi);
			read_socket_subcall_or_kill(pid, abi, &regs, SOCKDECODE,
						    &subcall);
			check_socketcall_equal_or_kill(pid, abi, test_sys,
						       subcall);
			read_socket_address_or_kill(pid, abi, &regs, SOCKDECODE,
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
		fail_verbose("%s: Test for reading socket address"
			     " for %s() failed",
			     test_name,
			     test_sys_name(test_sys));
#endif
}
END_TEST

TCase *create_testcase_socket(void)
{
	TCase *tc = tcase_create("socket");

	tcase_add_loop_test(tc, TEST_read_socket_address_af_null,
			    TEST_SYS_BIND, TEST_SYS_MAX);
	tcase_add_loop_test(tc, TEST_read_socket_address_af_unix,
			    TEST_SYS_BIND, TEST_SYS_MAX);
	tcase_add_loop_test(tc, TEST_read_socket_address_af_unixabs,
			    TEST_SYS_BIND, TEST_SYS_MAX);
	tcase_add_loop_test(tc, TEST_read_socket_address_af_inet,
			    TEST_SYS_BIND, TEST_SYS_MAX);
	tcase_add_loop_test(tc, TEST_read_socket_address_af_inet6,
			    TEST_SYS_BIND, TEST_SYS_MAX);
	tcase_add_loop_test(tc, TEST_read_socket_address_af_netlink,
			    TEST_SYS_BIND, TEST_SYS_MAX);
	return tc;
}
