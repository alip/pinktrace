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
#include <sys/wait.h>

/*
 * Test whether reading socket address works.
 * First fork a new child, call syscall(PINK_SYSCALL_INVALID, ...) with a
 * number and socket address and then check whether it's read
 * correctly.
 * 0: AF_NULL
 * 1: AF_UNIX
 * 2: AF_UNIX (abstract)
 * 3: AF_INET
 * 4: AF_INET6   (skip if PINK_HAVE_IPV6 == 0)
 * 5: AF_NETLINK (skip if PINK_HAVE_NETLINK == 0)
 */
START_TEST(TEST_read_socket_address)
{
	pid_t pid;
	bool it_worked = false;
	int test_number = _i;
	const char *test_name;
	long expfd = 23;
	long newfd;
	struct pink_sockaddr expaddr;
	struct pink_sockaddr newaddr;
	char ip[64];

#define TEST_AF_NULL 0
#define TEST_AF_UNIX 1
#define TEST_AF_UNIX_ABSTRACT 2
#define TEST_AF_INET 3
#define TEST_AF_INET6 4
#define TEST_AF_NETLINK 5
#define TEST_READ_SOCKET_ADDRESS_MAX 6
	expaddr.length = sizeof(struct sockaddr);
	switch(test_number) {
	case TEST_AF_NULL:
		test_name = "test_af_null";
		expaddr.family = -1;
		expaddr.length = 0;
		break;
	case TEST_AF_UNIX:
		test_name = "test_af_unix";
		expaddr.family = expaddr.u.sa_un.sun_family = AF_UNIX;
		strcpy(expaddr.u.sa_un.sun_path, "pinktrace");
		break;
	case TEST_AF_UNIX_ABSTRACT:
		test_name = "test_af_unix_abstract";
		expaddr.family = expaddr.u.sa_un.sun_family = AF_UNIX;
		strcpy(expaddr.u.sa_un.sun_path, "xpinktrace");
		expaddr.u.sa_un.sun_path[0] = '\0';
		break;
	case TEST_AF_INET:
		test_name = "test_af_inet";
		expaddr.family = expaddr.u.sa_in.sin_family = AF_INET;
		expaddr.u.sa_in.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		expaddr.u.sa_in.sin_port = htons(1969); /* woodstock */
		break;
	case TEST_AF_INET6:
		test_name = "test_af_inet6";
#if PINK_HAVE_IPV6
		expaddr.family = expaddr.u.sa6.sin6_family = AF_INET6;
		expaddr.u.sa6.sin6_addr = in6addr_loopback;
		expaddr.u.sa6.sin6_port = htons(1969);
#else
		message("PINK_HAVE_IPV6 is 0, skipping test %s\n", test_name);
		return;
#endif
		break;
	case TEST_AF_NETLINK:
		test_name = "test_af_netlink";
#if PINK_HAVE_NETLINK
		expaddr.family = expaddr.u.nl.nl_family = AF_NETLINK;
		expaddr.u.nl.nl_pid = 3;
		expaddr.u.nl.nl_groups = 3;
#else
		message("PINK_HAVE_NETLINK is 0, skipping test %s\n", test_name);
		return;
#endif
		break;
	default:
		fail_verbose("invalid test number %d", test_number);
		abort();
	}
	info("Test: %s\n", test_name);

	pid = fork_assert();
	if (pid == 0) {
		pid = getpid();
		trace_me_and_stop();
		switch (test_number) {
		case TEST_AF_NULL:
			syscall(PINK_SYSCALL_INVALID, expfd, NULL, 0);
			break;
		case TEST_AF_UNIX:
		case TEST_AF_UNIX_ABSTRACT:
			syscall(PINK_SYSCALL_INVALID, expfd,
					(struct sockaddr *)&expaddr.u.sa_un,
					expaddr.length);
			break;
		case TEST_AF_INET:
			syscall(PINK_SYSCALL_INVALID, expfd,
					(struct sockaddr *)&expaddr.u.sa_in,
					expaddr.length);
			break;
#if PINK_HAVE_IPV6
		case TEST_AF_INET6:
			syscall(PINK_SYSCALL_INVALID, expfd,
					(struct sockaddr *)&expaddr.u.sa6,
					expaddr.length);
			break;
#endif
#if PINK_HAVE_NETLINK
		case TEST_AF_NETLINK:
			syscall(PINK_SYSCALL_INVALID, expfd,
					(struct sockaddr *)&expaddr.u.nl,
					expaddr.length);
			break;
#endif
		default:
			_exit(1);
		}
		_exit(0);
	}

	LOOP_WHILE_TRUE() {
		int status;
		pid_t tracee_pid;
		enum pink_abi abi;
		long argval, sysnum;
		char *exp_sun_path;
		char *new_sun_path;
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
			read_syscall_or_kill(pid, abi, &regs, &sysnum);
			check_syscall_equal_or_kill(pid, abi, sysnum, PINK_SYSCALL_INVALID);
			read_socket_address_or_kill(pid, abi, &regs, false, 1, &newfd, &newaddr);
			if (newfd != expfd) {
				kill(pid, SIGKILL);
				fail_verbose("File descriptors not equal"
						" (expected:%ld got:%ld)",
						expfd, newfd);
			}
			if (newaddr.family != expaddr.family) {
				kill(pid, SIGKILL);
				fail_verbose("Address families not equal"
						" (expected:%d got:%d)",
						expaddr.family,
						newaddr.family);
			}
			if (newaddr.length != expaddr.length) {
				kill(pid, SIGKILL);
				fail_verbose("Address lengths not equal"
						" (expected:%u got:%d)",
						expaddr.length,
						newaddr.length);
			}
			switch (newaddr.family) {
			case -1:
				break;
			case AF_UNIX:
				if (test_number == TEST_AF_UNIX_ABSTRACT) {
					if (newaddr.u.sa_un.sun_path[0] != '\0') {
						kill(pid, SIGKILL);
						fail_verbose("AF_UNIX path not abstract"
								" (expected:`\\0' got:`%c')",
								newaddr.u.sa_un.sun_path[0]);
					}
					exp_sun_path = expaddr.u.sa_un.sun_path + 1;
					new_sun_path = newaddr.u.sa_un.sun_path + 1;
				} else {
					exp_sun_path = expaddr.u.sa_un.sun_path;
					new_sun_path = newaddr.u.sa_un.sun_path;
				}
				if (strcmp(new_sun_path, exp_sun_path)) {
					kill(pid, SIGKILL);
					fail_verbose("AF_UNIX paths not identical"
							" (expected:`%s' got:`%s')",
							exp_sun_path, new_sun_path);
				}
				break;
			case AF_INET:
				if (expaddr.u.sa_in.sin_port != newaddr.u.sa_in.sin_port) {
					kill(pid, SIGKILL);
					fail_verbose("AF_INET ports not equal"
							" (expected:%d got:%d)",
							ntohs(expaddr.u.sa_in.sin_port),
							ntohs(newaddr.u.sa_in.sin_port));
				}
				if (IS_LOOPBACK(newaddr.u.sa_in.sin_addr.s_addr)) {
					kill(pid, SIGKILL);
					inet_ntop(AF_INET, &newaddr.u.sa_in.sin_addr.s_addr, ip, sizeof(ip));
					fail_verbose("AF_INET addresses not identical"
							" (expected:INADDR_LOOPBACK got:`%s')",
							ip);
				}
				break;
#if PINK_HAVE_IPV6
			case AF_INET6:
				if (expaddr.u.sa6.sin6_port != newaddr.u.sa6.sin6_port) {
					kill(pid, SIGKILL);
					fail_verbose("AF_INET6 ports not equal"
							" (expected:%d got:%d)",
							ntohs(expaddr.u.sa6.sin6_port),
							ntohs(newaddr.u.sa6.sin6_port));
				}
				if (IS_LOOPBACK6(&newaddr.u.sa6.sin6_addr)) {
					kill(pid, SIGKILL);
					inet_ntop(AF_INET6, &newaddr.u.sa6.sin6_addr, ip, sizeof(ip));
					fail_verbose("AF_INET6 addresses not identical"
							" (expected:inaddr6_loopback got:`%s')",
							ip);
				}
				break;
#endif
#if PINK_HAVE_NETLINK
			case AF_NETLINK:
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
				break;
#endif
			}
			it_worked = true;
			kill(pid, SIGKILL);
			break;
		}
		trace_syscall_or_kill(pid, 0);
	}

	if (!it_worked)
		fail_verbose("%s: Test for reading socket address failed", test_name);
}
END_TEST

TCase *create_testcase_socket(void)
{
	TCase *tc = tcase_create("socket");

	tcase_add_loop_test(tc, TEST_read_socket_address, 0, TEST_READ_SOCKET_ADDRESS_MAX);

	return tc;
}
