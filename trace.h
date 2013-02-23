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

#ifndef PINK_TRACE_H
#define PINK_TRACE_H

/**
 * @file pinktrace/trace.h
 * @brief Pink's low level wrappers around ptrace(2) internals
 *
 * Do not include this file directly. Use pinktrace/pink.h instead.
 *
 * @defgroup pink_trace Pink's low level wrappers around ptrace(2) internals
 * @ingroup pinktrace
 * @{
 **/

#include <pinktrace/regs.h>

#include <stdbool.h>
#include <sys/types.h>
#include <signal.h>

/**
 * This define represents the trace option SYSGOOD.
 * If this flag is set in the options argument of pink_trace_setup(), when
 * delivering syscall traps, bit 7 is set in signal number (i.e., deliver
 * (SIGTRAP | 0x80) This makes it easy for the tracer to tell the difference
 * between normal traps and those caused by a syscall. This option may not work
 * on all architectures.
 *
 * @see #PINK_HAVE_OPTION_SYSGOOD
 **/
#define PINK_TRACE_OPTION_SYSGOOD   (1 << 0)
/**
 * This define represents the trace option FORK.
 * If this flag is set in the options argument of pink_trace_setup(), stop the
 * child at the next fork(2) call with (SIGTRAP | PTRACE_EVENT_FORK << 8) and
 * automatically start tracing the newly forked process, which will start with
 * a SIGSTOP. The PID for the new process can be retrieved with
 * pink_trace_geteventmsg().
 *
 * @see #PINK_HAVE_OPTION_FORK
 **/
#define PINK_TRACE_OPTION_FORK      (1 << 1)
/**
 * This define represents the trace option VFORK.
 * If this flag is set in the options argument of pink_trace_setup(), stop the
 * child at the next vfork(2) call with (SIGTRAP | PTRACE_EVENT_VFORK << 8) and
 * automatically start tracing the newly vforked process, which will start with
 * a SIGSTOP. The PID for the new process can be retrieved with
 * pink_trace_geteventmsg().
 *
 * @see #PINK_HAVE_OPTION_VFORK
 **/
#define PINK_TRACE_OPTION_VFORK     (1 << 2)
/**
 * This define represents the trace option CLONE.
 * If this flag is set in the options argument of pink_trace_setup(), stop the
 * child at the next clone(2) call with (SIGTRAP | PTRACE_EVENT_CLONE << 8) and
 * automatically start tracing the newly cloned process, which will start with
 * a SIGSTOP. The PID for the new process can be retrieved with
 * pink_trace_geteventmsg().
 *
 * @see #PINK_HAVE_OPTION_CLONE
 **/
#define PINK_TRACE_OPTION_CLONE     (1 << 3)
/**
 * This define represents the trace option EXEC.
 * If this flag is set in the options argument of pink_trace_setup(), stop the
 * child at the next execve(2) call with (SIGTRAP | PTRACE_EVENT_EXEC << 8)
 *
 * @see #PINK_HAVE_OPTION_EXEC
 **/
#define PINK_TRACE_OPTION_EXEC      (1 << 4)
/**
 * This define represents the trace option VFORKDONE.
 * If this flag is set in the options argument of pink_trace_setup(), stop the
 * child at the completion of the next vfork(2) call with
 * (SIGTRAP | PTRACE_EVENT_VFORK_DONE << 8)
 *
 * @see #PINK_HAVE_OPTION_VFORKDONE
 **/
#define PINK_TRACE_OPTION_VFORKDONE (1 << 5)
/**
 * This define represents the trace option EXIT.
 * If this flag is set in the options argument of pink_trace_setup(), stop the
 * child at exit with (SIGTRAP | PTRACE_EVENT_EXIT << 8). This child's exit
 * status can be retrieved with pink_trace_geteventmsg(). This stop will be
 * done early during process exit when registers are still available, allowing
 * the tracer to see where the exit occured, whereas the normal exit
 * notification is done after the process is finished exiting. Even though
 * context is available, the tracer cannot prevent the exit from happening at
 * this point.
 *
 * @see #PINK_HAVE_OPTION_EXIT
 **/
#define PINK_TRACE_OPTION_EXIT      (1 << 6)

/**
 * This define represents the trace option SECCOMP.
 * If this flag is set in the options argument of pink_trace_setup(), notify
 * the tracer with (SIGTRAP | PTRACE_EVENT_SECCOMP << 8) on seccomp filtering
 * events. SECCOMP_RET_DATA portion of the BPF program return value will be
 * available to the tracer via pink_trace_geteventmsg()
 *
 * @see #PINK_HAVE_OPTION_SECCOMP
 **/
#define PINK_TRACE_OPTION_SECCOMP   (1 << 7)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Small wrapper around @e ptrace(2) addressing oddities
 *
 * @param req Ptrace request
 * @param tid Thread ID
 * @param addr Address, see "man 2 ptrace"
 * @param data Data, see "man 2 ptrace"
 * @return Same as @e ptrace(2)
 **/
long pink_ptrace(int req, pid_t tid, void *addr, void *data);

/**
 * Indicates that this process is to be traced by its parent. Any signal
 * (except SIGKILL) delivered to this process will cause it to stop and its
 * parent to be notified via wait(2). Also, all subsequent calls to execve(2)
 * by this process will cause a SIGTRAP to be sent to it, giving the parent a
 * chance to gain control before the new program begins execution.
 *
 * @note This function is used only by the child process; the rest are used
 *       only by the parent.
 *
 * @return 0 on success, negated errno on failure
 **/
int pink_trace_me(void);

/**
 * Restarts the stopped child process
 *
 * @param tid Thread ID
 * @param sig If this is non-zero and not SIGSTOP, it is interpreted as the
 *            signal to be delivered to the child; otherwise, no signal is
 *            delivered. Thus, for example, the parent can control whether a
 *            signal sent to the child is delivered or not.
 * @return 0 on success, negated errno on failure
 **/
int pink_trace_resume(pid_t tid, int sig);

/**
 * Send signal to the tracee
 *
 * @note
 *   - If @e tgkill(2) system call is available: tgkill(tid, tgid, sig);
 *   - Otherwise if @e tkill(2) system call is available: tkill(tid, sig);
 *   - And otherwise: kill(tid, sig);
 *   is called. For #tgid <= 0 @e tgkill(2) is skipped.
 *
 * @see #PINK_HAVE_TKILL
 * @see #PINK_HAVE_TGKILL
 *
 * @param tid Thread ID
 * @param tgid Thread group ID
 * @param sig Signal
 * @return 0 on success, negated errno on failure
 **/
int pink_trace_kill(pid_t tid, pid_t tgid, int sig);

/**
 * Restarts the stopped child process and arranges it to be stopped after
 * execution of a single instruction.
 *
 * @param tid Thread ID
 * @param sig Treated the same as the signal argument of pink_trace_cont()
 * @return 0 on success, negated errno on failure
 **/
int pink_trace_singlestep(pid_t tid, int sig);

/**
 * Restarts the stopped child process and arranges it to be stopped after
 * the entry or exit of the next system call.
 *
 * @param tid Thread ID
 * @param sig Treated the same was as the signal argument of pink_trace_cont()
 * @return 0 on success, negated errno on failure
 **/
int pink_trace_syscall(pid_t tid, int sig);

/**
 * Retrieve a message (as an unsigned long) about the trace event that just
 * happened, placing it in the location given by the second argument. For
 * EXIT event this is the child's exit status. For FORK, VFORK, CLONE and
 * VFORK_DONE events this is the process ID of the new process. For SECCOMP
 * event, this is the SECCOMP_RET_DATA portion of the BPF program return value.
 *
 * @see PINK_HAVE_GETEVENTMSG
 *
 * @param tid Thread ID
 * @param data Pointer to store the message
 * @return 0 on success, negated errno on failure
 **/
int pink_trace_geteventmsg(pid_t tid, unsigned long *data);

/**
 * Copy the child's general purpose registers to the given location
 *
 * @see PINK_HAVE_REGS_T
 *
 * @param tid Thread ID
 * @param regs Pointer to the structure of registers.
 * @return 0 on success, negated errno on failure
 **/
int pink_trace_get_regs(pid_t tid, pink_regs_t *regs);

/**
 * Retrieve information about the signal that caused the stop.
 * Copy a siginfo_t structure (see sigaction(2)) from the tracee to the address
 * data in the tracer.
 *
 * @see PINK_HAVE_GETSIGINFO
 *
 * @param tid Thread ID
 * @param info Signal information
 * @return 0 on success, negated errno on failure
 **/
int pink_trace_get_siginfo(pid_t tid, siginfo_t *info);

/**
 * Set the child's general purpose registers
 *
 * @see PINK_HAVE_REGS_T
 *
 * @param tid Thread ID
 * @param regs Same as pink_trace_get_regs()
 * @return 0 on success, negated errno on failure
 **/
int pink_trace_set_regs(pid_t tid, const pink_regs_t *regs);

/*
 * Set the tracing options
 *
 * @see #PINK_HAVE_SETUP
 *
 * @param tid Thread ID
 * @param options Bitwise OR'ed PINK_TRACE_OPTION_* flags
 * @return 0 on success, negated errno on failure
 **/
int pink_trace_setup(pid_t tid, int options);

/**
 * Restarts the stopped child process and arranges it to be stopped after
 * the entry of the next system call which will *not* be executed.
 *
 * @see #PINK_HAVE_SYSEMU
 *
 * @param tid Thread ID
 * @param sig Treated same as the signal argument of pink_trace_cont()
 * @return 0 on success, negated errno on failure
 **/
int pink_trace_sysemu(pid_t tid, int sig);

/**
 * Restarts the stopped child process like pink_trace_sysemu() but also
 * singlesteps if not a system call.
 *
 * @see #PINK_HAVE_SYSEMU_SINGLESTEP
 *
 * @param tid Thread ID of the child to be restarted
 * @param sig Treated same as the signal argument of pink_trace_cont()
 * @return 0 on success, negated errno on failure
 **/
int pink_trace_sysemu_singlestep(pid_t tid, int sig);

/**
 * Attaches to the process specified in tid, making it a traced "child" of the
 * calling process; the behaviour of the child is as if it had done a
 * PTRACE_TRACEME. The child is sent a SIGSTOP, but will not necessarily have
 * stopped by the completion of this call; use wait(2) to wait for the child to
 * stop.
 *
 * @param tid Thread ID
 * @return 0 on success, negated errno on failure
 **/
int pink_trace_attach(pid_t tid);

/**
 * Restarts the stopped child as for pink_trace_cont(), but first detaches from
 * the process, undoing the reparenting effect of pink_trace_attach().
 *
 * @param tid Thread ID
 * @param sig Treated same as the signal argument of pink_trace_cont()
 * @return 0 on success, negated errno on failure
 **/
int pink_trace_detach(pid_t tid, int sig);

/**
 * Attach to the process specified in tid, without trapping it or affecting its
 * signal and job control states.
 *
 * @see #PINK_HAVE_SEIZE
 *
 * @param tid Thread ID
 * @param options Bitwise OR'ed PINK_TRACE_OPTION_* flags
 * @return 0 on success, negated errno on failure
 **/
int pink_trace_seize(pid_t tid, int options);

/**
 * Trap the process without any signal or job control related side effects.
 *
 * @see #PINK_HAVE_INTERRUPT
 *
 * @param tid Thread ID
 * @return 0 on success, negated errno on failure
 **/
int pink_trace_interrupt(pid_t tid);

/**
 * Listen for ptrace events asynchronously after pink_trace_interrupt().
 *
 * @see #PINK_HAVE_LISTEN
 * @see pink_trace_interrupt()
 *
 * @param tid Thread ID
 * @return 0 on success, negated errno on failure
 **/
int pink_trace_listen(pid_t tid);

#ifdef __cplusplus
}
#endif
/** @} */
#endif
