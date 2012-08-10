/*
 * Copyright (c) 2010, 2011, 2012 Ali Polatel <alip@exherbo.org>
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

#include <pinktrace/internal.h> /* FIXME: _pink_assert_not_reached() */
#include <pinktrace/easy/internal.h>
#include <pinktrace/pink.h>
#include <pinktrace/easy/pink.h>

#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <sys/wait.h>
#include <sys/utsname.h>

static void handle_ptrace_error(struct pink_easy_context *ctx,
		struct pink_easy_process *current,
		const char *errctx)
{
	if (errno == ESRCH) {
		if (ctx->callback_table.teardown)
			ctx->callback_table.teardown(ctx, current);
	} else {
		ctx->callback_table.error(ctx, PINK_EASY_ERROR_TRACE, current, errctx);
	}
	pink_easy_process_free(ctx, current);
}

static bool handle_startup(struct pink_easy_context *ctx, struct pink_easy_process *current)
{
	/* Set up tracing options */
	if (!pink_trace_setup(current->tid, ctx->ptrace_options)) {
		handle_ptrace_error(ctx, current, "setup");
		return false;
	}

	/* Set up flags */
	if (ctx->ptrace_options & PINK_TRACE_OPTION_FORK
			|| ctx->ptrace_options & PINK_TRACE_OPTION_VFORK
			|| ctx->ptrace_options & PINK_TRACE_OPTION_CLONE)
		current->flags |= PINK_EASY_PROCESS_FOLLOWFORK;

	/* Happy birthday! */
	if (ctx->callback_table.startup) {
		struct pink_easy_process *parent = NULL;
		if (current->tgid != -1)
			parent = pink_easy_process_list_lookup(&(ctx->process_list), current->tgid);
		ctx->callback_table.startup(ctx, current, parent);
	}

	current->flags &= ~PINK_EASY_PROCESS_STARTUP;
	return true;
}

static void do_step(struct pink_easy_context *ctx,
		struct pink_easy_process *current,
		int sig)
{
	int r;
	enum pink_easy_step step;

	step = current->ptrace_step == PINK_EASY_STEP_NIL
		? ctx->ptrace_default_step
		: current->ptrace_step;

	switch (step) {
	case PINK_EASY_STEP_SINGLESTEP:
		r = pink_trace_singlestep(current->tid, sig);
		break;
	case PINK_EASY_STEP_SYSCALL:
		r = pink_trace_syscall(current->tid, sig);
		break;
	case PINK_EASY_STEP_RESUME:
		r = pink_trace_resume(current->tid, sig);
		break;
	default:
		_pink_assert_not_reached();
	}
	if (!r)
		handle_ptrace_error(ctx, current, "step");
}

int pink_easy_loop(struct pink_easy_context *ctx)
{
	/* Enter the event loop */
	while (ctx->nprocs != 0) {
		pid_t tid;
		int r, status, sig;
		unsigned event;
		pink_regs_t regs;
		struct pink_easy_process *current;

		tid = waitpid(-1, &status, __WALL);
		if (tid < 0) {
			switch (errno) {
			case EINTR:
				continue;
			case ECHILD:
				goto cleanup;
			default:
				ctx->fatal = true;
				ctx->error = PINK_EASY_ERROR_WAIT;
				ctx->callback_table.error(ctx);
				goto cleanup;
			}
		}

		current = pink_easy_process_list_lookup(&(ctx->process_list), tid);
		event = pink_event_decide(status);

		/* Under Linux, execve changes tid to thread leader's tid,
		 * and we see this changed tid on EVENT_EXEC and later,
		 * execve sysexit. Leader "disappears" without exit
		 * notification. Let user know that, drop leader's tcb,
		 * and fix up tid in execve thread's tcb.
		 * Effectively, execve thread's tcb replaces leader's tcb.
		 *
		 * BTW, leader is 'stuck undead' (doesn't report WIFEXITED
		 * on exit syscall) in multithreaded programs exactly
		 * in order to handle this case.
		 *
		 * PTRACE_GETEVENTMSG returns old tid starting from Linux 3.0.
		 * On 2.6 and earlier, it can return garbage.
		 */
		if (event == PINK_EVENT_EXEC) {
			enum pink_abi old_abi = current->abi;
			struct pink_easy_process *execve_thread = current;
			long old_tid = 0;

			if (pink_easy_os_release < KERNEL_VERSION(3,0,0))
				goto dont_switch_procs;
			if (!pink_trace_geteventmsg(tid, (unsigned long *)&old_tid))
				goto dont_switch_procs;
			if (old_tid <= 0 || old_tid == tid)
				goto dont_switch_procs;
			execve_thread = pink_easy_process_list_lookup(&(ctx->process_list), old_tid);
			if (!execve_thread)
				goto dont_switch_procs;

			/* Drop leader, switch to the thread, reusing leader's tid */
			pink_easy_process_free(ctx, current);
			current = execve_thread;
			current->tid = tid;
dont_switch_procs:
			/* Update abi */
#if PINK_HAVE_REGS_T
			if (!pink_trace_get_regs(current->tid, &regs)) {
				handle_ptrace_error(ctx, current, "getregs");
				continue;
			}
#else
			regs = 0;
#endif

			if (!pink_read_abi(current->tid, &regs, &current->abi)) {
				handle_ptrace_error(ctx, current, "abi");
				continue;
			}
			if (ctx->callback_table.exec) {
				r = ctx->callback_table.exec(ctx, current, &regs, old_abi);
				if (r & PINK_EASY_CFLAG_ABORT) {
					ctx->error = PINK_EASY_ERROR_CALLBACK_ABORT;
					goto cleanup;
				}
				if (r & PINK_EASY_CFLAG_DROP) {
					pink_easy_process_free(ctx, current);
					continue;
				}
			}
		}

		if (current == NULL) {
			/* We might see the child's initial trap before we see the parent
			 * return from the clone syscall. Leave the child suspended until
			 * the parent returns from its system call. Only then we will have
			 * the association between parent and child.
			 */
			current = pink_easy_process_new(ctx, tid, -1,
					PINK_EASY_STEP_NIL,
					PINK_EASY_PROCESS_SUSPENDED);
			continue;
		}

		if (WIFSIGNALED(status) || WIFEXITED(status)) {
			pink_easy_process_free(ctx, current);
			if (ctx->callback_table.exit) {
				r = ctx->callback_table.exit(ctx, tid, status);
				if (r & PINK_EASY_CFLAG_ABORT) {
					ctx->error = PINK_EASY_ERROR_CALLBACK_ABORT;
					goto cleanup;
				}
			}
			continue;
		}
		if (!WIFSTOPPED(status)) {
			ctx->callback_table.error(ctx, PINK_EASY_ERROR_PROCESS, current, "WIFSTOPPED");
			pink_easy_process_free(ctx, current);
			continue;
		}

		/* Is this the very first time we see this tracee stopped? */
		if (current->flags & PINK_EASY_PROCESS_STARTUP && !handle_startup(ctx, current))
				continue;

		if (event == PINK_EVENT_FORK || event == PINK_EVENT_VFORK || event == PINK_EVENT_CLONE) {
			struct pink_easy_process *new_thread;
			long new_tid;
			if (!pink_trace_geteventmsg(current->tid, (unsigned long *)&new_tid)) {
				handle_ptrace_error(ctx, current, "geteventmsg");
				continue;
			}
			new_thread = pink_easy_process_list_lookup(&(ctx->process_list), new_tid);
			if (new_thread == NULL) {
				/* Not attached to the thread yet, nor is it alive... */
				new_thread = pink_easy_process_new(ctx, new_tid, current->tid,
						PINK_EASY_STEP_NIL,
						PINK_EASY_PROCESS_IGNORE_ONE_SIGSTOP);
			} else {
				/* Thread is waiting for Pink to let her go on... */
				new_thread->tgid = current->tid;
				new_thread->abi = current->abi;
				new_thread->flags &= ~PINK_EASY_PROCESS_SUSPENDED;
				handle_startup(ctx, new_thread);
				do_step(ctx, new_thread, 0);
			}
		} else if (event == PINK_EVENT_EXIT && ctx->callback_table.pre_exit) {
			unsigned long status;
			if (!pink_trace_geteventmsg(current->tid, &status)) {
				handle_ptrace_error(ctx, current, "geteventmsg");
				continue;
			}
			r = ctx->callback_table.pre_exit(ctx, current, (int)status);
			if (r & PINK_EASY_CFLAG_ABORT) {
				ctx->error = PINK_EASY_ERROR_CALLBACK_ABORT;
				goto cleanup;
			}
			if (r & PINK_EASY_CFLAG_DROP) {
				pink_easy_process_free(ctx, current);
				continue;
			}
		} else if (event == PINK_EVENT_SECCOMP && ctx->callback_table.seccomp) {
			unsigned long ret_data;
			if (!pink_trace_geteventmsg(current->tid, &ret_data)) {
				handle_ptrace_error(ctx, current, "geteventmsg");
				continue;
			}
			r = ctx->callback_table.seccomp(ctx, current, (long)ret_data);
			if (r & PINK_EASY_CFLAG_ABORT) {
				ctx->error = PINK_EASY_ERROR_CALLBACK_ABORT;
				goto cleanup;
			}
			if (r & PINK_EASY_CFLAG_DROP) {
				pink_easy_process_free(ctx, current);
				continue;
			}
		}

		sig = WSTOPSIG(status);

		if (event != 0) /* Ptrace event */
			goto restart_tracee_with_sig_0;

		/* Is this post-attach SIGSTOP? */
		if (sig == SIGSTOP && (current->flags & PINK_EASY_PROCESS_IGNORE_ONE_SIGSTOP)) {
			current->flags &= ~PINK_EASY_PROCESS_IGNORE_ONE_SIGSTOP;
			goto restart_tracee_with_sig_0;
		}
		if (sig != (SIGTRAP|0x80)) {
			if (ctx->callback_table.signal) {
				r = ctx->callback_table.signal(ctx, current, status);
				if (r & PINK_EASY_CFLAG_ABORT) {
					ctx->error = PINK_EASY_ERROR_CALLBACK_ABORT;
					goto cleanup;
				}
				if (r & PINK_EASY_CFLAG_DROP) {
					pink_easy_process_free(ctx, current);
					continue;
				}
				if (r & PINK_EASY_CFLAG_SIGIGN)
					goto restart_tracee_with_sig_0;
			}
			goto restart_tracee;
		}

		/* System call trap! */
		current->flags ^= PINK_EASY_PROCESS_INSYSCALL;
		if (ctx->callback_table.syscall) {
			bool entering = current->flags & PINK_EASY_PROCESS_INSYSCALL;
#if PINK_HAVE_REGS_T
			if (!pink_trace_get_regs(current->tid, &regs)) {
				handle_ptrace_error(ctx, current, "getregs");
				continue;
			}
#else
			regs = 0;
#endif
			r = ctx->callback_table.syscall(ctx, current, &regs, entering);
			if (r & PINK_EASY_CFLAG_ABORT) {
				ctx->error = PINK_EASY_ERROR_CALLBACK_ABORT;
				goto cleanup;
			}
			if (r & PINK_EASY_CFLAG_DROP) {
				pink_easy_process_free(ctx, current);
				continue;
			}
		}

restart_tracee_with_sig_0:
		sig = 0;
restart_tracee:
		do_step(ctx, current, sig);
	}

cleanup:
	return ctx->callback_table.cleanup
		? ctx->callback_table.cleanup(ctx)
		: (ctx->error ? EXIT_FAILURE : EXIT_SUCCESS);
}
