/*
 * Copyright (c) 2010, 2011, 2012, 2013 Ali Polatel <alip@exherbo.org>
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

#include <pinktrace/easy/private.h>
#include <pinktrace/pink.h>
#include <pinktrace/easy/pink.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <asm/unistd.h>

struct pink_easy_process *pink_easy_process_new(struct pink_easy_context *ctx,
		pid_t tid, pid_t tgid,
		short flags)
{
	struct pink_easy_process *current;

	current = calloc(1, sizeof(struct pink_easy_process));
	if (current == NULL)
		return NULL;
	SLIST_INSERT_HEAD(&ctx->process_list, current, entries);

	current->tid = tid;
	current->tgid = tgid;
	current->ptrace_step = PINK_EASY_STEP_NOT_SET;

	current->flags = flags;
	current->flags |= PINK_EASY_PROCESS_STARTUP;
	current->flags &= ~PINK_EASY_PROCESS_INSYSCALL;

	ctx->nprocs++;

	return current;
}

void pink_easy_process_free(struct pink_easy_context *ctx, struct pink_easy_process *proc)
{
	SLIST_REMOVE(&ctx->process_list, proc, pink_easy_process, entries);
	if (proc->userdata_destroy && proc->userdata)
		proc->userdata_destroy(proc->userdata);
	free(proc);
	ctx->nprocs--;
}

int pink_easy_process_kill(const struct pink_easy_process *proc, int sig)
{
	if (proc->flags & PINK_EASY_PROCESS_CLONE_THREAD)
		return pink_trace_kill(proc->tid, proc->tgid, sig);
	return pink_trace_kill(proc->tid, proc->tid, sig);
}

bool pink_easy_process_detach(const struct pink_easy_process *proc)
{
	bool retval, sigstop_expected;
	int status;

	/*
	 * Linux wrongly insists the child be stopped
	 * before detaching.  Arghh.  We go through hoops
	 * to make a clean break of things.
	 */

	retval = true;
	sigstop_expected = false;
	if (proc->flags & PINK_EASY_PROCESS_ATTACHED) {
		/*
		 * We attached but possibly didn't see the expected SIGSTOP.
		 * We must catch exactly one as otherwise the detached process
		 * would be left stopped (process state T).
		 */
		sigstop_expected = !!(proc->flags & PINK_EASY_PROCESS_IGNORE_ONE_SIGSTOP);
		retval = pink_trace_detach(proc->tid, 0);
		if (retval) {
			/* On a clear day, you can see forever. */
		} else if (errno != ESRCH) {
			/* Shouldn't happen. */
			return false;
		} else if (pink_trace_kill(proc->tid, proc->tgid, 0) < 0) {
			if (errno != ESRCH) {
				/* detach: checking sanity. */
				return false;
			}
		} else if (!sigstop_expected && pink_trace_kill(proc->tid,
								proc->tgid,
								SIGSTOP) < 0) {
			if (errno != ESRCH) {
				/* detach: stopping child. */
				return false;
			}
		} else {
			sigstop_expected = true;
		}
	} else {
		/* Expect the child is in stopped state */
		return pink_trace_detach(proc->tid, 0);
	}

	if (sigstop_expected) {
		for (;;) {
#ifdef __WALL
			if (waitpid(proc->tid, &status, __WALL) < 0) {
				if (errno == ECHILD) {
					/* Already gone! */
					break;
				}
				if (errno != EINVAL) {
					/* detach: waiting */
					break;
				}
#endif /* __WALL */
				/* No __WALL here */
				if (waitpid(proc->tid, &status, 0) < 0) {
					if (errno != ECHILD) {
						/* detach: waiting */
						break;
					}
#ifdef __WCLONE
					/* If no processes, try clones. */
					if (waitpid(proc->tid, &status, __WCLONE) < 0) {
						if (errno != ECHILD) {
							/* detach: waiting */
							break;
						}
					}
#endif /* __WCLONE */
				}
#ifdef __WALL
			}
#endif
			if (!WIFSTOPPED(status)) {
				/* Au revoir, mon ami. */
				break;
			}
			if (WSTOPSIG(status) == SIGSTOP) {
				pink_trace_detach(proc->tid, 0);
				break;
			}
			retval = pink_trace_resume(proc->tid,
						   WSTOPSIG(status) == (SIGTRAP|0x80) ? 0 : WSTOPSIG(status));
			if (!retval)
				break;
		}
	}

	return retval;
}

pid_t pink_easy_process_get_tid(const struct pink_easy_process *proc)
{
	return proc->tid;
}

pid_t pink_easy_process_get_tgid(const struct pink_easy_process *proc)
{
	return proc->tgid;
}

int pink_easy_process_get_abi(const struct pink_easy_process *proc)
{
	return proc->abi;
}

void pink_easy_process_set_step(struct pink_easy_process *proc, enum pink_easy_step ptrace_step)
{
	proc->ptrace_step = ptrace_step;
}

enum pink_easy_step pink_easy_process_get_step(const struct pink_easy_process *proc)
{
	return proc->ptrace_step;
}

void pink_easy_process_set_flags(struct pink_easy_process *proc, short flags)
{
	proc->flags = flags;
}

short pink_easy_process_get_flags(const struct pink_easy_process *proc)
{
	return proc->flags;
}

void *pink_easy_process_get_userdata(const struct pink_easy_process *proc)
{
	return proc->userdata;
}

void pink_easy_process_set_userdata(struct pink_easy_process *proc, void *userdata, pink_easy_free_func_t userdata_destroy)
{
	proc->userdata = userdata;
	proc->userdata_destroy = userdata_destroy;
}

struct pink_easy_process *pink_easy_process_list_lookup(const struct pink_easy_process_list *list, pid_t tid)
{
	struct pink_easy_process *node;

	SLIST_FOREACH(node, list, entries) {
		if (node->tid == tid)
			return node;
	}

	return NULL;
}

void pink_easy_process_list_remove(struct pink_easy_process_list *list, const struct pink_easy_process *proc)
{
	SLIST_REMOVE(list, proc, pink_easy_process, entries);
}

unsigned pink_easy_process_list_walk(const struct pink_easy_process_list *list,
		pink_easy_walk_func_t func, void *userdata)
{
	unsigned count;
	struct pink_easy_process *node;

	count = 0;
	SLIST_FOREACH(node, list, entries) {
		count++;
		if (!func(node, userdata))
			break;
	}

	return count;
}
