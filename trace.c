/*
 * Copyright (c) 2010, 2011, 2012, 2013 Ali Polatel <alip@exherbo.org>
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

#include <pinktrace/private.h>
#include <pinktrace/pink.h>

int pink_ptrace(int req, pid_t pid, void *addr, void *data, long *retval)
{
	long val;

	errno = 0;
	val = ptrace(req, pid, addr, (long)data);
	if (val == -1 && errno) {
		/*
		 * "Unfortunately, under Linux, different variations of this
		 * fault will return EIO or EFAULT more or less arbitrarily."
		 */
		if (errno == EIO)
			errno = EFAULT;
		return -errno;
	}

	if (retval)
		*retval = val;
	return 0;
}

int pink_trace_me(void)
{
	return pink_ptrace(PTRACE_TRACEME, 0, NULL, NULL, NULL);
}

int pink_trace_resume(pid_t pid, int sig)
{
	return pink_ptrace(PTRACE_CONT, pid, NULL, (void *)(long)sig, NULL);
}

int pink_trace_kill(pid_t tid, pid_t tgid, int sig)
{
	if (tid <= 0)
		return -EINVAL;

	if (tgid <= 0) {
#if PINK_HAVE_TKILL
		return syscall(__NR_tkill, tid, sig) < 0 ? -errno : 0;
#else
		return kill(tid, sig) < 0 ? -errno : 0;
#endif
	} else {
#if PINK_HAVE_TGKILL
		return syscall(__NR_tgkill, tgid, tid, sig) ? -errno : 0;
#elif PINK_HAVE_TKILL
		return syscall(__NR_tkill, tid, sig) < 0 ? -errno : 0;
#else
		return kill(tid, sig) < 0 ? -errno : 0;
#endif
	}
}

int pink_trace_singlestep(pid_t pid, int sig)
{
	return pink_ptrace(PTRACE_SINGLESTEP, pid, NULL, (void *)(long)sig, NULL);
}

int pink_trace_syscall(pid_t pid, int sig)
{
	return pink_ptrace(PTRACE_SYSCALL, pid, NULL, (void *)(long)sig, NULL);
}

int pink_trace_geteventmsg(pid_t pid, unsigned long *data)
{
#if PINK_HAVE_GETEVENTMSG
	return pink_ptrace(PTRACE_GETEVENTMSG, pid, NULL, data, NULL);
#else
	return -ENOSYS;
#endif
}

int pink_trace_get_regs(pid_t pid, void *regs)
{
	return pink_ptrace(PTRACE_GETREGS, pid, NULL, regs, NULL);
}

int pink_trace_get_regset(pid_t pid, void *regset, int n_type)
{
#if PINK_HAVE_GETREGSET
	return pink_ptrace(PTRACE_GETREGSET, pid, (void *)(long)n_type, regset, NULL);
#else
	return -ENOSYS;
#endif
}

int pink_trace_set_regs(pid_t pid, const void *regs)
{
	return pink_ptrace(PTRACE_SETREGS, pid, NULL, (void *)regs, NULL);
}

int pink_trace_set_regset(pid_t pid, const void *regset, int n_type)
{
#if PINK_HAVE_SETREGSET
	return pink_ptrace(PTRACE_SETREGSET, pid, (void *)(long)n_type, (void *)regset, NULL);
#else
	return -ENOSYS;
#endif
}

int pink_trace_get_siginfo(pid_t pid, void *info)
{
#if PINK_HAVE_GETSIGINFO
	return pink_ptrace(PTRACE_GETSIGINFO, pid, NULL, info, NULL);
#else
	return -ENOSYS;
#endif
}

int pink_trace_setup(pid_t pid, int options)
{
#if PINK_HAVE_SETUP
	int ptrace_options;

	ptrace_options = 0;
	if (options & PINK_TRACE_OPTION_SYSGOOD) {
#if PINK_HAVE_OPTION_SYSGOOD
		ptrace_options |= PTRACE_O_TRACESYSGOOD;
#else
		return -EINVAL;
#endif
	}

	if (options & PINK_TRACE_OPTION_FORK) {
#if PINK_HAVE_OPTION_FORK
		ptrace_options |= PTRACE_O_TRACEFORK;
#else
		return -EINVAL;
#endif
	}

	if (options & PINK_TRACE_OPTION_VFORK) {
#if PINK_HAVE_OPTION_VFORK
		ptrace_options |= PTRACE_O_TRACEVFORK;
#else
		return -EINVAL;
#endif
	}

	if (options & PINK_TRACE_OPTION_CLONE) {
#if PINK_HAVE_OPTION_CLONE
		ptrace_options |= PTRACE_O_TRACECLONE;
#else
		return -EINVAL;
#endif
	}

	if (options & PINK_TRACE_OPTION_EXEC) {
#if PINK_HAVE_OPTION_EXEC
		ptrace_options |= PTRACE_O_TRACEEXEC;
#else
		return -EINVAL;
#endif
	}

	if (options & PINK_TRACE_OPTION_VFORKDONE) {
#if PINK_HAVE_OPTION_VFORKDONE
		ptrace_options |= PTRACE_O_TRACEVFORKDONE;
#else
		return -EINVAL;
#endif
	}

	if (options & PINK_TRACE_OPTION_EXIT) {
#if PINK_HAVE_OPTION_EXIT
		ptrace_options |= PTRACE_O_TRACEEXIT;
#else
		return -EINVAL;
#endif
	}

	if (options & PINK_TRACE_OPTION_SECCOMP) {
#if PINK_HAVE_OPTION_SECCOMP
		ptrace_options |= PTRACE_O_TRACESECCOMP;
#else
		return -EINVAL;
#endif
	}

	if (options & PINK_TRACE_OPTION_EXITKILL) {
#if PINK_HAVE_OPTION_EXITKILL
		ptrace_options |= PTRACE_O_EXITKILL;
#else
		return -EINVAL;
#endif
	}

	return pink_ptrace(PTRACE_SETOPTIONS, pid, NULL, (void *)(long)ptrace_options, NULL);
#else
	return -ENOSYS;
#endif
}

int pink_trace_sysemu(pid_t pid, int sig)
{
#if PINK_HAVE_SYSEMU
	return pink_ptrace(PTRACE_SYSEMU, pid, NULL, (void *)(long)sig, NULL);
#else
	return -ENOSYS;
#endif
}

int pink_trace_sysemu_singlestep(pid_t pid, int sig)
{
#if PINK_HAVE_SYSEMU
	return pink_ptrace(PTRACE_SYSEMU_SINGLESTEP, pid, NULL, (void *)(long)sig, NULL);
#else
	return -ENOSYS;
#endif
}

int pink_trace_attach(pid_t pid)
{
	return pink_ptrace(PTRACE_ATTACH, pid, NULL, NULL, NULL);
}

int pink_trace_detach(pid_t pid, int sig)
{
	return pink_ptrace(PTRACE_DETACH, pid, NULL, (void *)(long)sig, NULL);
}

int pink_trace_seize(pid_t pid, int options)
{
#if PINK_HAVE_SEIZE
	return pink_ptrace(PTRACE_SEIZE, pid, NULL, (void *)(long)options, NULL);
#else
	return -ENOSYS;
#endif
}

int pink_trace_interrupt(pid_t pid)
{
#if PINK_HAVE_INTERRUPT
	return pink_ptrace(PTRACE_INTERRUPT, pid, NULL, NULL, NULL);
#else
	return -ENOSYS;
#endif
}

int pink_trace_listen(pid_t pid)
{
#if PINK_HAVE_LISTEN
	return pink_ptrace(PTRACE_LISTEN, pid, NULL, NULL, NULL);
#else
	return -ENOSYS;
#endif
}
