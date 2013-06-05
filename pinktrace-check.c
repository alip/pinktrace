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

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define ANSI_NORMAL		"[00;00m"
#define ANSI_DARK_MAGENTA	"[01;35m"
#define ANSI_MAGENTA		"[00;35m"
#define ANSI_GREEN		"[00;32m"
#define ANSI_YELLOW		"[00;33m"
#define ANSI_CYAN		"[00;36m"

unsigned os_release;

PINK_GCC_ATTR((format (printf, 2, 0)))
int pprintf_va(int pretty, const char *format, va_list ap)
{
	int r, tty;
	const char *pre;

	tty = isatty(STDOUT_FILENO);

	if (!tty || getenv("PINKTRACE_CHECK_NOCOLOUR"))
		return vprintf(format, ap);

	switch (pretty) {
	case DEBUG:
		pre = ANSI_CYAN;
		break;
	case INFO:
		pre = ANSI_YELLOW;
		break;
	case MESSAGE:
		pre = ANSI_GREEN;
		break;
	case WARNING:
		pre = ANSI_DARK_MAGENTA;
		break;
	default:
		pre = "";
		break;
	}

	printf("%s", pre);
	r = vprintf(format, ap);
	printf("%s", ANSI_NORMAL);
	if (pretty == WARNING)
		fputc('\n', stdout);

	return r;
}

PINK_GCC_ATTR((format (printf, 2, 3)))
int pprintf(int pretty, const char *format, ...)
{
	int r;
	va_list ap;

	va_start(ap, format);
	r = pprintf_va(pretty, format, ap);
	va_end(ap);

	return r;
}

static void dump_basic_hex(const void *addr, size_t len)
{
#define BYTES_IN_LINE 16
	unsigned off;
	const unsigned char *caddr = addr;
	unsigned char buf[BYTES_IN_LINE+1];

	debug("\t --8< HEXDUMP %p >8--\n", addr);

	for (off = 0; off < len; off++) {
		if ((off % BYTES_IN_LINE) == 0) {
			if (off != 0)
				debug("\t\t%s\n", buf);
			debug("\t\t%04x ", off);
		}

		debug(" %02x", caddr[off]);
		if (caddr[off] < 0x20 || caddr[off] > 0x7E)
			buf[off % BYTES_IN_LINE] = '.';
		else
			buf[off % BYTES_IN_LINE] = caddr[off];
		buf[(off % 16) + 1] = '\0';
	}

	while (off % BYTES_IN_LINE != 0) {
		debug("  ");
		off++;
	}
	debug("  %s\n", buf);

	debug("\t --8< DUMPEND %p >8--\n", addr);
}

static void dump_regset(const struct pink_regset *regset)
{
	debug("\t --8< REGSET DUMP %p >8--\n", (void *)regset);
#if PINK_ABIS_SUPPORTED > 1
	debug("\t\tregset->abi = %d\n", regset->abi);
#endif
#if PINK_ARCH_ARM
	struct pt_regs regs = regset->arm_regs;
	debug("\t\tregs.ARM_cpsr = %#lx\n", regs.ARM_cpsr);
	debug("\t\tregs.ARM_pc = %#lx\n", regs.ARM_pc);
	debug("\t\tregs.ARM_lr = %#lx\n", regs.ARM_lr);
	debug("\t\tregs.ARM_sp = %#lx\n", regs.ARM_sp);
	debug("\t\tregs.ARM_ip = %#lx\n", regs.ARM_ip);
	debug("\t\tregs.ARM_fp = %#lx\n", regs.ARM_fp);
	debug("\t\tregs.ARM_r10 = %#lx\n", regs.ARM_r10);
	debug("\t\tregs.ARM_r9 = %#lx\n", regs.ARM_r9);
	debug("\t\tregs.ARM_r8 = %#lx\n", regs.ARM_r8);
	debug("\t\tregs.ARM_r7 = %#lx\n", regs.ARM_r7);
	debug("\t\tregs.ARM_r6 = %#lx\n", regs.ARM_r6);
	debug("\t\tregs.ARM_r5 = %#lx\n", regs.ARM_r5);
	debug("\t\tregs.ARM_r4 = %#lx\n", regs.ARM_r4);
	debug("\t\tregs.ARM_r3 = %#lx\n", regs.ARM_r3);
	debug("\t\tregs.ARM_r2 = %#lx\n", regs.ARM_r2);
	debug("\t\tregs.ARM_r1 = %#lx\n", regs.ARM_r1);
	debug("\t\tregs.ARM_r0 = %#lx\n", regs.ARM_r0);
	debug("\t\tregs.ARM_ORIG_r0 = %#lx\n", regs.ARM_ORIG_r0);
#elif PINK_ARCH_IA64
	debug("\t\tregset->ia32 = %d\n", regset->ia32);
#elif PINK_ARCH_POWERPC
	debug("\t\tregs = TODO\n");
#elif PINK_ARCH_I386
	struct user_regs_struct regs = regset->i386_regs;
	debug("\t\tregs.ebx = %#lx\n", regs.ebx);
	debug("\t\tregs.ecx = %#lx\n", regs.ecx);
	debug("\t\tregs.edx = %#lx\n", regs.edx);
	debug("\t\tregs.esi = %#lx\n", regs.esi);
	debug("\t\tregs.edi = %#lx\n", regs.edi);
	debug("\t\tregs.ebp = %#lx\n", regs.ebp);
	debug("\t\tregs.eax = %#lx\n", regs.eax);
	debug("\t\tregs.xds = %#x\n", regs.xds);
	debug("\t\tregs.xes = %#x\n", regs.xes);
	debug("\t\tregs.xfs = %#x\n", regs.xfs);
	debug("\t\tregs.xgs = %#x\n", regs.xgs);
	debug("\t\tregs.orig_eax = %#lx\n", regs.orig_eax);
	debug("\t\tregs.eip = %#lx\n", regs.eip);
	debug("\t\tregs.xcs = %#x\n", regs.xcs);
	debug("\t\tregs.eflags = %#lx\n", regs.eflags);
	debug("\t\tregs.esp = %#lx\n", regs.esp);
	debug("\t\tregs.xss = %#lx\n", regs.xss);
#elif PINK_ARCH_X86_64 || PINK_ARCH_X32
	struct user_regs_struct regs = regset->x86_regs_union.x86_64_r;
	debug("\t\tregs.r15 = %llx\n", regs.r15);
	debug("\t\tregs.r14 = %llx\n", regs.r14);
	debug("\t\tregs.r13 = %llx\n", regs.r13);
	debug("\t\tregs.r12 = %llx\n", regs.r12);
	debug("\t\tregs.rbp = %llx\n", regs.rbp);
	debug("\t\tregs.rbx = %llx\n", regs.rbx);
	debug("\t\tregs.r11 = %llx\n", regs.r11);
	debug("\t\tregs.r10 = %llx\n", regs.r10);
	debug("\t\tregs.r9 = %llx\n", regs.r9);
	debug("\t\tregs.r8 = %llx\n", regs.r8);
	debug("\t\tregs.rax = %llx\n", regs.rax);
	debug("\t\tregs.rcx = %llx\n", regs.rcx);
	debug("\t\tregs.rdx = %llx\n", regs.rdx);
	debug("\t\tregs.rsi = %llx\n", regs.rsi);
	debug("\t\tregs.rdi = %llx\n", regs.rdi);
	debug("\t\tregs.orig_rax = %llx\n", regs.orig_rax);
	debug("\t\tregs.rip = %llx\n", regs.rip);
	debug("\t\tregs.cs = %llx\n", regs.cs);
	debug("\t\tregs.eflags = %llx\n", regs.eflags);
	debug("\t\tregs.rsp = %llx\n", regs.rsp);
	debug("\t\tregs.ss = %llx\n", regs.ss);
	debug("\t\tregs.fs_base = %llx\n", regs.fs_base);
	debug("\t\tregs.gs_base = %llx\n", regs.gs_base);
	debug("\t\tregs.ds = %llx\n", regs.ds);
	debug("\t\tregs.es = %llx\n", regs.es);
	debug("\t\tregs.fs = %llx\n", regs.fs);
	debug("\t\tregs.gs = %llx\n", regs.gs);
#else
#error unsupported architecture
#endif
	debug("\t --8< REGSET DUMP END %p >8--\n", (void *)regset);
}

static void dump_socket_address(struct pink_sockaddr *sockaddr)
{
	char ip[64];

	debug("\t --8< SOCKADDRDUMP %p >8--\n", (void *)sockaddr);

	debug("\t\tfamily:%d\n", sockaddr->family);

	switch (sockaddr->family) {
	case AF_UNIX:
		debug("\t\t%s: `%s'\n",
			sockaddr->u.sa_un.sun_path[0] != '\0'
				? "unix"
				: "unix-abstract",
			sockaddr->u.sa_un.sun_path[0] != '\0'
				? sockaddr->u.sa_un.sun_path
				: sockaddr->u.sa_un.sun_path + 1);
		break;
	case AF_INET:
		inet_ntop(AF_INET, &sockaddr->u.sa_in.sin_addr, ip, sizeof(ip));
		debug("\t\tinet: %s@%d\n", ip, ntohs(sockaddr->u.sa_in.sin_port));
		break;
#if PINK_HAVE_IPV6
	case AF_INET6:
		inet_ntop(AF_INET6, &sockaddr->u.sa6.sin6_addr, ip, sizeof(ip));
		debug("\t\tinet6: %s@%d\n", ip, ntohs(sockaddr->u.sa6.sin6_port));
		break;
#endif
#if PINK_HAVE_NETLINK
	case AF_NETLINK:
		debug("\t\tnetlink: nl_pid=%u nl_groups=%u\n",
				sockaddr->u.nl.nl_pid,
				sockaddr->u.nl.nl_groups);
		break;
#endif
	}

	debug("\t --8< SOCKADDRDUMP END %p >8--\n", (void *)sockaddr);
}

pid_t fork_assert(void)
{
	pid_t pid;

	pid = fork();
	fail_if_verbose(pid == -1, "fork (errno:%d %s)",
			errno, strerror(errno));

	return pid;
}

void kill_save_errno(pid_t pid, int sig)
{
	int r;
	int saved_errno = errno;

	r = kill(pid, sig);
	warning("\tkill(%u, %d) = %d (errno:%d %s)\n",
			pid, sig,
			r, errno, strerror(errno));
	errno = saved_errno;
}

pid_t waitpid_no_intr(pid_t pid, int *status, int options)
{
	while (1) {
		pid_t r;

		errno = 0;
		r = waitpid(pid, status, options);
		if (r <= 0 && errno == EINTR)
			continue;
		return r;
	}
}

pid_t waitpid_no_intr_debug(unsigned loopcnt,
			    const char *file, const char *func, int linecnt,
			    pid_t pid, int *status, int options)
{
	int saved_errno;
	pid_t tracee_pid;

	tracee_pid = waitpid_no_intr(pid, status, options);
	saved_errno = errno;
	message("%s:%s@%d[%u] wait(pid:%d status:%p opts:%d) = %d ",
			file, func, linecnt, loopcnt,
			pid, (void *)status, options,
			tracee_pid);
	if (tracee_pid > 0) {
		int s = *status;

		debug("(status:%#x", (unsigned)*status);
		if (WIFSTOPPED(s))
			debug("{stop:%d %s}",
					WSTOPSIG(s),
					strsignal(WSTOPSIG(s)));
		else if (WIFEXITED(s))
			debug("{exit:%d}", WEXITSTATUS(s));
		else if (WIFSIGNALED(s))
			debug("{term:%d %s%s}",
					WTERMSIG(s), strsignal(WTERMSIG(s)),
					WCOREDUMP(s) ? " (core dumped)" : "");
#ifdef WIFCONTINUED
		else if (WIFCONTINUED(s))
			debug("{cont:%d %s}",
					SIGCONT,
					strsignal(SIGCONT));
#endif
		debug(")\n");
	} else {
		warning("(errno:%d %s)\n", saved_errno,
				strerror(saved_errno));
	}
	errno = saved_errno;

	return tracee_pid;
}

pid_t wait_no_intr(int *status)
{
	return waitpid_no_intr(-1, status, 0);
}

pid_t wait_no_intr_debug(unsigned loopcnt,
			 const char *file, const char *func, int linecnt,
			 int *status)
{
	return waitpid_no_intr_debug(loopcnt,
			file, func, linecnt,
			-1, status, 0);
}

bool check_echild_or_kill(pid_t pid, pid_t retval)
{
	if (errno == ECHILD)
		return true;
	kill_save_errno(pid, SIGKILL);
	fail_verbose("unexpected wait result %d (errno:%d %s)",
			retval, errno, strerror(errno));
	abort();
}

bool check_exit_code_or_fail(int status, int code)
{
	if (!WIFEXITED(status))
		return false;
	if (WEXITSTATUS(status) == code)
		return true;
	fail_verbose("unexpected exit status %u", WEXITSTATUS(status));
	abort();
}

bool check_signal_or_fail(int status, int sig)
{
	if (!WIFSIGNALED(status))
		return false;
	if (WTERMSIG(status) == sig)
		return true;
	fail_verbose("unexpected signal (signal:%u %s)",
			WTERMSIG(status),
			strsignal(WTERMSIG(status)));
	abort();
}

bool check_stopped_or_kill(pid_t pid, int status)
{
	if (WIFSTOPPED(status))
		return false;
	kill(pid, SIGKILL);
	fail_verbose("unexpected wait status %#x", status);
	abort();
}

void check_syscall_equal_or_kill(pid_t pid, long sysnum, long sysnum_expected)
{
	if (sysnum == sysnum_expected)
		return;
	kill(pid, SIGKILL);
	fail_verbose("unexpected syscall %ld"
			" (name:%s expected:%ld %s)",
			sysnum,
			pink_name_syscall(sysnum, PINK_ABI_DEFAULT),
			sysnum_expected,
			sysnum_expected == PINK_SYSCALL_INVALID
				? "PINK_SYSCALL_INVALID"
				: pink_name_syscall(sysnum_expected,
						    PINK_ABI_DEFAULT));
	abort();
}

void check_retval_equal_or_kill(pid_t pid,
				long retval, long retval_expected,
				int error, int error_expected)
{
	if (retval == retval_expected && error == error_expected)
		return;
	kill(pid, SIGKILL);
	fail_verbose("unexpected retval %ld (errno:%d %s)"
			", expected %ld (errno:%d %s)",
			retval,
			error, strerror(error),
			retval_expected,
			error_expected, strerror(error_expected));
	abort();
}

void check_argument_equal_or_kill(pid_t pid,
				  long arg, long arg_expected)
{
	if (arg == arg_expected)
		return;
	kill(pid, SIGKILL);
	fail_verbose("unexpected argument %ld expected %ld",
			arg, arg_expected);
	abort();
}

void check_memory_equal_or_kill(pid_t pid,
				const void *val,
				const void *val_expected,
				size_t n)
{
	if (memcmp(val, val_expected, n) == 0)
		return;
	kill(pid, SIGKILL);
	warning("Memory area %p not identical with the expected %p",
			val, val_expected);
	dump_basic_hex(val, n);
	dump_basic_hex(val_expected, n);
	fail_verbose("Memory area %p not identical with the expected %p",
			val, val_expected);
	abort();
}

void check_string_equal_or_kill(pid_t pid,
				const char *str,
				const char *str_expected,
				size_t len)
{
	if (strncmp(str, str_expected, len) == 0)
		return;
	kill(pid, SIGKILL);
	warning("String %p:`%s' not identical with the expected %p:`%s'",
			str, str,
			str_expected, str_expected);
	dump_basic_hex(str, len);
	dump_basic_hex(str_expected, len);
	fail_verbose("String %p:`%s' not identical with the expected %p:`%s'",
			str, str,
			str_expected, str_expected);
	abort();
}

void check_string_endswith_or_kill(pid_t pid, const char *str,
				   const char *suffix_expected)
{
	size_t slen, elen;

	slen = strlen(str);
	elen = strlen(suffix_expected);

	if (elen == 0)
		return;
	if (slen < elen)
		goto fail;
	if (memcmp(str + (slen - elen), suffix_expected, elen) == 0)
		return;
fail:
	kill(pid, SIGKILL);
	warning("String %p:`%s' doesn't end with the expected %p:`%s'",
			str, str,
			suffix_expected, suffix_expected);
	dump_basic_hex(str, slen);
	dump_basic_hex(suffix_expected, elen);
	fail_verbose("String %p:`%s' doesn't end with the expected %p:`%s'",
			str, str,
			suffix_expected, suffix_expected);
	abort();
}

void check_addr_loopback_or_kill(pid_t pid, in_addr_t addr)
{
	char ip[64];

	if (htonl(addr) == INADDR_LOOPBACK)
		return;

	inet_ntop(AF_INET, &addr, ip, sizeof(ip));
	warning("in_addr %#x (ip: `%s') not identical with INADDR_LOOPBACK:%#x",
		(u_int32_t)addr, ip,
		(u_int32_t)INADDR_LOOPBACK);
	fail_verbose("in_addr %#x (ip: `%s') not identical with INADDR_LOOPBACK:%#x",
		     (u_int32_t)addr, ip,
		     (u_int32_t)INADDR_LOOPBACK);
	abort();
}

#if PINK_HAVE_IPV6
void check_addr6_loopback_or_kill(pid_t pid, struct in6_addr *addr6)
{
	char ip[64];

	if (IN6_IS_ADDR_LOOPBACK(addr6))
		return;

	inet_ntop(AF_INET6, addr6, ip, sizeof(ip));
	warning("in6_addr: `%s' not identical to in6addr_loopback: `::1'", ip);

	fail_verbose("in6_addr: `%s' not identical to in6addr_loopback: `::1'",
		     ip);
	abort();
}
#endif

void trace_me_and_stop(void)
{
	int r;
	pid_t pid;

	pid = getpid();
	r = pink_trace_me();
	if (r < 0) {
		warning("pink_trace_me (errno:%d %s)", -r, strerror(-r));
		_exit(127);
	}
	kill(pid, SIGSTOP);
}

void trace_syscall_or_kill(pid_t pid, int sig)
{
	int r;

	r = pink_trace_syscall(pid, sig);
	info("\ttrace_syscall(%u, %d) = %d (errno:%d %s)\n",
	     pid, sig, r, errno, strerror(errno));

	if (r < 0) {
		kill_save_errno(pid, SIGKILL);
		fail_verbose("PTRACE_SYSCALL (pid:%u sig:%d errno:%d %s)",
			     pid, sig, -r, strerror(-r));
	}
}

void trace_setup_or_kill(pid_t pid, int options)
{
	int r;

	r = pink_trace_setup(pid, options);
	info("\ttrace_setup(%u, %#x) = %d (errno:%d %s)\n",
			pid, (unsigned)options, r,
			errno, strerror(errno));

	if (r < 0) {
		kill_save_errno(pid, SIGKILL);
		fail_verbose("PTRACE_SETOPTIONS (pid:%u opts:%#x errno:%d %s)",
			     pid, (unsigned)options, -r, strerror(-r));
	}
}

void trace_geteventmsg_or_kill(pid_t pid, unsigned long *data)
{
	int r;

	r = pink_trace_geteventmsg(pid, data);
	info("\ttrace_geteventmsg(%u, %#lx) = %d (errno:%d %s)\n",
	     pid, (r < 0) ? 0xbad : *data, r, errno, strerror(errno));

	if (r < 0) {
		kill_save_errno(pid, SIGKILL);
		fail_verbose("PTRACE_GETEVENTMSG (pid:%u errno:%d %s)",
			     pid, -r, strerror(-r));
	}
}

enum pink_event event_decide_and_print(int status)
{
	enum pink_event e;

	e = pink_event_decide(status);
	info("\tevent_decide(%#x) = %u %s\n", (unsigned)status, e, pink_name_event(e));
	return e;
}

void regset_alloc_or_kill(pid_t pid, struct pink_regset **regptr)
{
	int r;

	r = pink_regset_alloc(regptr);
	if (r < 0) {
		kill_save_errno(pid, SIGKILL);
		fail_verbose("pink_regset_alloc (errno:%d %s)", -r, strerror(-r));
	}
}

void regset_fill_or_kill(pid_t pid, struct pink_regset *regset)
{
	int r;

	r = pink_regset_fill(pid, regset);
	if (r < 0) {
		kill_save_errno(pid, SIGKILL);
		fail_verbose("pink_regset_fill (pid:%u errno:%d %s)",
			     pid, -r, strerror(-r));
	}
	dump_regset(regset);
}

void read_syscall_or_kill(pid_t pid, struct pink_regset *regset, long *sysnum)
{
	int r;

	r = pink_read_syscall(pid, regset, sysnum);
	if (r == 0) {
		info("\tread_syscall (pid:%u) = %ld\n", pid, *sysnum);
	} else if (r < 0) {
		kill_save_errno(pid, SIGKILL);
		fail_verbose("pink_read_syscall (pid:%u, errno:%d %s)",
			     pid, -r, strerror(-r));
	}
}

void read_retval_or_kill(pid_t pid, struct pink_regset *regset, long *retval, int *error)
{
	int r;

	r = pink_read_retval(pid, regset, retval, error);
	if (r == 0) {
		info("\tread_retval (pid:%u) = %ld,%d\n", pid,
		     *retval, *error);
	} else if (r < 0) {
		kill_save_errno(pid, SIGKILL);
		fail_verbose("pink_read_retval (pid:%u, errno:%d %s)",
			     pid, -r, strerror(-r));
	}
}

void read_argument_or_kill(pid_t pid, struct pink_regset *regset, unsigned arg_index, long *argval)
{
	int r;

	r = pink_read_argument(pid, regset, arg_index, argval);
	if (r == 0) {
		info("\tread_argument (pid:%u, index:%u) = %ld\n", pid,
		     arg_index, *argval);
	} else if (r < 0) {
		kill_save_errno(pid, SIGKILL);
		fail_verbose("pink_read_argument (pid:%u, index:%u, errno:%d %s)",
			     pid, arg_index, -r, strerror(-r));
	}
}

void read_vm_data_or_kill(pid_t pid, struct pink_regset *regset, long addr, char *dest, size_t len)
{
	ssize_t r;

	errno = 0;
	r = pink_read_vm_data(pid, regset, addr, dest, len);
	if (r < 0) {
		kill_save_errno(pid, SIGKILL);
		fail_verbose("pink_read_vm_data (pid:%u, addr:%ld, len:%zd errno:%d %s)",
			     pid, addr, len, errno, strerror(errno));
	} else if ((size_t)r < len) {
		message("\tpink_read_vm_data partial read, expected:%zu got:%zd\n",
			len, r);
	}
	info("\tread_vm_data (pid:%u, addr:%ld len:%zd) = %zd", pid, addr, len, r);
	dump_basic_hex(dest, r);
}

ssize_t read_vm_data_nul_or_kill(pid_t pid, struct pink_regset *regset, long addr, char *dest, size_t len)
{
	ssize_t r;

	errno = 0;
	r = pink_read_vm_data_nul(pid, regset, addr, dest, len);
	if (r < 0) {
		kill_save_errno(pid, SIGKILL);
		fail_verbose("pink_read_vm_data_nul (pid:%u, addr:%ld, len:%zd errno:%d %s)",
			     pid, addr, len, errno, strerror(errno));
	} else if ((size_t)r < len) {
		message("\tpink_read_vm_data_nul partial read, expected:%zu got:%zd\n",
			len, r);
	}
	info("\tread_vm_data_nul (pid:%u, addr:%ld len:%zd) = %zd\n", pid, addr, len, r);
	dump_basic_hex(dest, r);

	return r;
}

void read_string_array_or_kill(pid_t pid, struct pink_regset *regset,
			       long arg, unsigned arr_index,
			       char *dest, size_t dest_len,
			       bool *nullptr)
{
	ssize_t r;

	r = pink_read_string_array(pid, regset, arg, arr_index, dest, dest_len, nullptr);
	if (r < 0) {
		kill_save_errno(pid, SIGKILL);
		fail_verbose("pink_read_string_array (pid:%u, arg:%ld, arr_index:%u dest_len:%zu errno:%d %s)",
			     pid, arg, arr_index, dest_len, errno, strerror(errno));
	} else if ((size_t)r < dest_len) {
		message("\tpink_read_string_array partial read,"
				" expected:%zu got:%zd\n",
				dest_len, r);
	}
	info("read_string_array (pid:%u arg:%ld arr_index:%u, dest_len:%zd) = %zd",
	     pid, arg, arr_index, dest_len, r);
	dump_basic_hex(dest, r);
}

void read_socket_subcall_or_kill(pid_t pid, struct pink_regset *regset,
				 bool decode_socketcall,
				 long *subcall)
{
	int r;

	r = pink_read_socket_subcall(pid, regset, decode_socketcall, subcall);
	if (r < 0) {
		kill_save_errno(pid, SIGKILL);
		fail_verbose("pink_read_socket_subcall (pid:%u decode:%d errno:%d %s)",
			     pid, decode_socketcall, -r, strerror(-r));
	}
	info("\tread_socket_subcall (pid:%u decode:%d) = %ld",
	     pid, decode_socketcall, *subcall);
}

void read_socket_argument_or_kill(pid_t pid, struct pink_regset *regset, bool decode_socketcall,
				  unsigned arg_index, unsigned long *argval)
{
	int r;

	r = pink_read_socket_argument(pid, regset, decode_socketcall, arg_index, argval);
	if (r < 0) {
		kill_save_errno(pid, SIGKILL);
		fail_verbose("pink_read_socket_argument (pid:%u decode:%d arg_index:%u errno:%d %s",
			     pid, decode_socketcall, arg_index, -r, strerror(-r));
	}
	info("\tread_socket_argument (pid:%u decode:%d arg_index:%u) = %ld",
	     pid, decode_socketcall, arg_index, *argval);
}

void read_socket_address_or_kill(pid_t pid, struct pink_regset *regset, bool decode_socketcall,
				 unsigned arg_index, int *fd,
				 struct pink_sockaddr *sockaddr)
{
	int r;

	r = pink_read_socket_address(pid, regset, decode_socketcall, arg_index, fd, sockaddr);
	if (r < 0) {
		kill_save_errno(pid, SIGKILL);
		fail_verbose("pink_read_socket_address (pid:%u decode:%d arg_index:%u errno:%d %s)",
			     pid, decode_socketcall, arg_index, -r, strerror(-r));
	}

	info("\tread_socket_address (pid:%u decode:%d arg_index:%u) = %d,%p",
	     pid, decode_socketcall, arg_index, fd ? *fd : -1, (void *)sockaddr);
	dump_socket_address(sockaddr);
}

void write_syscall_or_kill(pid_t pid, struct pink_regset *regset, long sysnum)
{
	int r;

	r = pink_write_syscall(pid, regset, sysnum);
	if (r < 0) {
		kill_save_errno(pid, SIGKILL);
		fail_verbose("pink_write_syscall (pid:%u sysnum:%ld errno:%d %s)",
			     pid, sysnum, -r, strerror(-r));
	}
	info("\twrite_syscall (pid:%u sysnum:%ld) = 0",
	     pid, sysnum);
}

void write_retval_or_kill(pid_t pid, struct pink_regset *regset, long retval, int error)
{
	int r;

	r = pink_write_retval(pid, regset, retval, error);
	if (r < 0) {
		kill_save_errno(pid, SIGKILL);
		fail_verbose("pink_write_retval (pid:%u retval:%ld error:%d errno:%d %s)",
			     pid, retval, error, -r, strerror(-r));
	}
	info("\twrite_syscall (pid:%u retval:%ld error:%d) = 0",
	     pid, retval, error);
}

void write_argument_or_kill(pid_t pid, struct pink_regset *regset, unsigned arg_index, long argval)
{
	int r;

	r = pink_write_argument(pid, regset, arg_index, argval);
	if (r < 0) {
		kill_save_errno(pid, SIGKILL);
		fail_verbose("pink_write_argument (pid:%u arg_index:%u argval:%ld errno:%d %s)",
			     pid, arg_index, argval, -r, strerror(-r));
	}
	info("\twrite_argument (pid:%u arg_index:%u argval:%ld) = 0",
	     pid, arg_index, argval);
}

void write_vm_data_or_kill(pid_t pid, struct pink_regset *regset, long addr, const char *src, size_t len)
{
	ssize_t r;

	errno = 0;
	r = pink_write_vm_data(pid, regset, addr, src, len);
	if (r < 0) {
		kill_save_errno(pid, SIGKILL);
		fail_verbose("pink_write_vm_data (pid:%u addr:%ld src:%p len:%zd errno:%d %s)",
			     pid, addr, (void *)src, len, errno, strerror(errno));
	} else if ((size_t)r < len) {
		message("\twrite_vm_data partial write, expected:%zd got:%zd\n",
			len, r);
	}
	info("\twrite_vm_data (pid:%u addr:%ld src:%p len:%zd) = %zu",
	     pid, addr, (void *)src, len, r);
}

static unsigned get_os_release(void)
{
	unsigned rel;
	const char *p;
	struct utsname u;

	if (uname(&u) < 0) {
		fprintf(stderr, "uname failed (errno:%d %s)\n",
			errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
	/* u.release has this form: "3.2.9[-some-garbage]" */
	rel = 0;
	p = u.release;
	for (;;) {
		if (!(*p >= '0' && *p <= '9')) {
			fprintf(stderr, "Bad OS release string: '%s'\n",
				u.release);
			exit(EXIT_FAILURE);
		}
		/* Note: this open-codes KERNEL_VERSION(): */
		rel = (rel << 8) | atoi(p);
		if (rel >= KERNEL_VERSION(1,0,0))
			break;
		while (*p >= '0' && *p <= '9')
			p++;
		if (*p != '.') {
			if (rel >= KERNEL_VERSION(0,1,0)) {
				/* "X.Y-something" means "X.Y.0" */
				rel <<= 8;
				break;
			}
			fprintf(stderr, "Bad OS release string: '%s'\n",
				u.release);
			exit(EXIT_FAILURE);
		}
		p++;
	}

	return rel;
}

int main(void)
{
	int number_failed;
	SRunner *sr;
	Suite *s;

	os_release = get_os_release();

	s = suite_create("pink-core");
	if (getenv("PINK_CHECK_SKIP_TRACE") == NULL)
		suite_add_tcase(s, create_testcase_trace());
	if (getenv("PINK_CHECK_SKIP_READ") == NULL)
		suite_add_tcase(s, create_testcase_read());
	if (getenv("PINK_CHECK_SKIP_WRITE") == NULL)
		suite_add_tcase(s, create_testcase_write());
	if (getenv("PINK_CHECK_SKIP_SOCKET") == NULL)
		suite_add_tcase(s, create_testcase_socket());

	sr = srunner_create(s);

	srunner_run_all(sr, CK_VERBOSE);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	warning("Failed test cases: %d", number_failed);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
