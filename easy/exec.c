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

#include <pinktrace/easy/internal.h>
#include <pinktrace/pink.h>
#include <pinktrace/easy/pink.h>

#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <alloca.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

enum {
	PINK_INTERNAL_FUNC_EXECVE,
	PINK_INTERNAL_FUNC_EXECV,
	PINK_INTERNAL_FUNC_EXECVP,
};

static bool pink_easy_exec_helper(struct pink_easy_context *ctx, int type,
		const char *filename, char *const argv[], char *const envp[])
{
	pid_t tid;
	struct pink_easy_process *current;

	tid = fork();
	if (tid < 0) {
		ctx->callback_table.error(ctx, PINK_EASY_ERROR_FORK, "fork");
		return false;
	} else if (tid == 0) { /* child */
		if (!pink_trace_me())
			_exit(ctx->callback_table.cerror(PINK_EASY_CHILD_ERROR_SETUP));
		/* Induce a ptrace stop. Tracer (our parent) will resume us
		 * with PTRACE_SYSCALL and may examine the immediately
		 * following execve syscall.  Note: This can't be done on NOMMU
		 * systems with vfork because the parent would be blocked and
		 * stopping would deadlock.
		 */
		kill(getpid(), SIGSTOP);
		switch (type) {
		case PINK_INTERNAL_FUNC_EXECVE:
			execve(filename, argv, envp);
			break;
		case PINK_INTERNAL_FUNC_EXECV:
			execv(filename, argv);
			break;
		case PINK_INTERNAL_FUNC_EXECVP:
			execvp(filename, argv);
			break;
		default:
			_pink_assert_not_reached();
		}
		/* execve() failed */
		_exit(ctx->callback_table.cerror(PINK_EASY_CHILD_ERROR_EXEC));
	}
	/* parent */
	current = pink_easy_process_new(ctx, tid, -1, PINK_EASY_STEP_NIL, PINK_EASY_PROCESS_IGNORE_ONE_SIGSTOP);
	if (current == NULL) {
		kill(tid, SIGKILL);
		return false;
	}
	return true;
}

bool pink_easy_execve(struct pink_easy_context *ctx, const char *filename,
		char *const argv[], char *const envp[])
{
	return pink_easy_exec_helper(ctx, PINK_INTERNAL_FUNC_EXECVE, filename, argv, envp);
}

bool pink_easy_execl(struct pink_easy_context *ctx, const char *file,
		const char *arg, ...)
{
	unsigned int narg;
	char *foo;
	char **argv;
	va_list ap, orig_ap;

	/* Initialize variable arguments */
	va_start(ap, arg);
	va_copy(orig_ap, ap);

	/* Count the arguments */
	narg = 0;
	while ((foo = va_arg(ap, char *)) != NULL)
		++narg;
	va_end(ap);

	/* Copy the arguments to argv array */
	argv = (char **)alloca(narg * sizeof(char *));
	if (argv) {
		for (unsigned int i = 0; i < narg; i++)
			argv[i] = va_arg(orig_ap, char *);
		va_end(orig_ap);
		return pink_easy_exec_helper(ctx, PINK_INTERNAL_FUNC_EXECVE, file, argv, environ);
	}

	/* OOM */
	va_end(orig_ap);
	errno = ENOMEM;
	return false;
}

bool pink_easy_execlp(struct pink_easy_context *ctx, const char *file,
		const char *arg, ...)
{
	unsigned int narg;
	char *foo;
	char **argv;
	va_list ap, orig_ap;

	/* Initialize variable arguments */
	va_start(ap, arg);
	va_copy(orig_ap, ap);

	/* Count the arguments */
	narg = 0;
	while ((foo = va_arg(ap, char *)) != NULL)
		++narg;
	va_end(ap);

	/* Copy the arguments to argv array */
	argv = (char **)alloca(narg * sizeof(char *));
	if (argv) {
		for (unsigned int i = 0; i < narg; i++)
			argv[i] = va_arg(orig_ap, char *);
		va_end(orig_ap);
		return pink_easy_exec_helper(ctx, PINK_INTERNAL_FUNC_EXECVP, file, argv, NULL);
	}

	/* OOM */
	va_end(orig_ap);
	errno = ENOMEM;
	return false;
}

bool pink_easy_execv(struct pink_easy_context *ctx, const char *path,
		char *const argv[])
{
	return pink_easy_exec_helper(ctx, PINK_INTERNAL_FUNC_EXECV, path, argv, NULL);
}

bool pink_easy_execvp(struct pink_easy_context *ctx, const char *file,
		char *const argv[])
{
	return pink_easy_exec_helper(ctx, PINK_INTERNAL_FUNC_EXECVP, file, argv, NULL);
}
