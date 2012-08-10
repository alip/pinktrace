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

#include <pinktrace/easy/internal.h>
#include <pinktrace/easy/pink.h>

#include <stdbool.h>

volatile sig_atomic_t pink_easy_interrupted;
sigset_t pink_easy_empty_set;
sigset_t pink_easy_blocked_set;

bool pink_easy_interactive = false;

static void pink_easy_interrupt_handler(int sig)
{
	pink_easy_interrupted = sig;
}

void pink_easy_interrupt_init(enum pink_easy_intr intr)
{
	struct sigaction sa;

	sigemptyset(&pink_easy_empty_set);
	sigemptyset(&pink_easy_blocked_set);

	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	sigaction(SIGTTOU, &sa, NULL); /* SIG_IGN */
	sigaction(SIGTTIN, &sa, NULL); /* SIG_IGN */

	if (intr != PINK_EASY_INTR_ANYWHERE) {
		if (intr == PINK_EASY_INTR_BLOCK_TSTP_TOO)
			sigaction(SIGTSTP, &sa, NULL); /* SIG_IGN */

		if (intr == PINK_EASY_INTR_WHILE_WAIT) {
			sigaddset(&pink_easy_blocked_set, SIGHUP);
			sigaddset(&pink_easy_blocked_set, SIGINT);
			sigaddset(&pink_easy_blocked_set, SIGQUIT);
			sigaddset(&pink_easy_blocked_set, SIGPIPE);
			sigaddset(&pink_easy_blocked_set, SIGTERM);
			sa.sa_handler = pink_easy_interrupt_handler;
			pink_easy_interactive = true;
		}
		/* SIG_IGN, or set handler for these */
		sigaction(SIGHUP, &sa, NULL);
		sigaction(SIGINT, &sa, NULL);
		sigaction(SIGQUIT, &sa, NULL);
		sigaction(SIGPIPE, &sa, NULL);
		sigaction(SIGTERM, &sa, NULL);
	}
}
