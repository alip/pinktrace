/*
 * Copyright (c) 2012 Ali Polatel <alip@exherbo.org>
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

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

int pink_easy_errback_child_stderr(enum pink_easy_child_error e)
{
	fprintf(stderr, "pinktrace child error: %s (errno:%d %s)\n",
			pink_easy_child_strerror(e),
			errno, strerror(errno));
	return EXIT_FAILURE;
}

void pink_easy_errback_stderr(const struct pink_easy_context *ctx, ...)
{
	va_list ap;
	const char *errctx;
	pid_t tid;
	struct pink_easy_process *current;

	fprintf(stderr, "pinktrace error: ");

	va_start(ap, ctx);
	switch (ctx->error) {
	case PINK_EASY_ERROR_CALLBACK_ABORT:
	case PINK_EASY_ERROR_WAIT:
		fprintf(stderr, "%s (errno:%d %s)\n",
				pink_easy_strerror(ctx->error),
				errno, strerror(errno));
		break;
	case PINK_EASY_ERROR_ALLOC:
	case PINK_EASY_ERROR_FORK:
		errctx = va_arg(ap, const char *);
		fprintf(stderr, "%s: %s (errno:%d %s)\n",
				pink_easy_strerror(ctx->error),
				errctx, errno, strerror(errno));
		break;
	case PINK_EASY_ERROR_ATTACH:
		tid = va_arg(ap, pid_t);
		fprintf(stderr, "%s (tid:%lu errno:%d %s)\n",
				pink_easy_strerror(ctx->error),
				(unsigned long)tid,
				errno, strerror(errno));
		break;
	case PINK_EASY_ERROR_TRACE:
	case PINK_EASY_ERROR_PROCESS:
		current = va_arg(ap, struct pink_easy_process *);
		errctx = va_arg(ap, const char *);
		fprintf(stderr, "%s (tid:%lu[%d]",
				pink_easy_strerror(ctx->error),
				(unsigned long)current->tid,
				current->abi);
		if (ctx->error == PINK_EASY_ERROR_TRACE) {
			fprintf(stderr, " errno:%d %s",
					errno,
					strerror(errno));
		}
		fprintf(stderr, ")\n");
		break;
	default:
		fprintf(stderr, "unknown:%u\n", ctx->error);
		break;
	}
}
