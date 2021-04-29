/*
 * Copyright (c) 2021 Ali Polatel <alip@exherbo.org>
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

static void test_read_write_int(void)
{
	int r;
	int rint = 0, wint = 42;
	int pipefd[2];

	if ((r = pink_pipe_init(pipefd)))
		fail_verbose("pink_pipe_init failed: %d(%s)", -r, strerror(-r));
	if ((r = pink_pipe_write_int(pipefd[1], wint)))
		fail_verbose("pink_pipe_write_int failed: %d(%s)", -r,
			     strerror(-r));
	if ((r = pink_pipe_read_int(pipefd[0], &rint)))
		fail_verbose("pink_pipe_read_int failed: %d(%s)", -r,
			     strerror(-r));
	if (rint != wint)
		fail_verbose("pipe read: %d != pipe write: %d", rint, wint);
	if ((r = pink_pipe_close(pipefd[0])))
		fail_verbose("pink_pipe_close(0) failed: %d(%s)", -r,
			     strerror(-r));
	if ((r = pink_pipe_close(pipefd[1])))
		fail_verbose("pink_pipe_close(1) failed: %d(%s)", -r,
			     strerror(-r));
}

static void test_fixture_pipe(void) {
	test_fixture_start();

	run_test(test_read_write_int);

	test_fixture_end();
}

void test_suite_pipe(void) {
	test_fixture_pipe();
}
