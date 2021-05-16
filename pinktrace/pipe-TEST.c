/*
 * Copyright (c) 2021 Ali Polatel <alip@exherbo.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
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
