/* vim: set cino= fo=croql sw=8 ts=8 sts=0 noet cin fdm=syntax : */

/**
 * \file
 *
 * Example \ref pink-fork-linux.c "pink-fork-linux.c" .
 **/

/**
 * \example pink-fork-linux.c
 *
 * Example demonstrating how to fork and start tracing a process on Linux.
 **/

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include <pinktrace/pink.h>

int
main(void)
{
	int r, status;
	pid_t pid;
	enum pink_event event;

	/* Fork and start tracing. */
	if ((pid = fork()) < 0) {
		perror("fork");
		return EXIT_FAILURE;
	}
	else if (!pid) {
		/* Set up for tracing. */
		if ((r = pink_trace_me()) < 0) {
			fprintf(stderr, "pink_trace_me: %d %s\n",
				-r, strerror(-r));
			_exit(EXIT_FAILURE);
		}
		/* Stop, so that the parent has the chance to set up options
		 * for tracing. */
		kill(getpid(), SIGSTOP);

		/* Here's the point where you can do whatever the traced child
		 * should do. */
		printf("hello world\n");

		_exit(EXIT_SUCCESS);
	}
	else {
		waitpid(pid, &status, 0);
		event = pink_event_decide(status);
		assert(event == PINK_EVENT_NONE);

		/* Set up the child for tracing */
		if ((r = pink_trace_setup(pid, PINK_TRACE_OPTION_SYSGOOD)) < 0) {
			perror("pink_trace_setup");
			pink_trace_kill(pid, -1, SIGKILL);
			return EXIT_FAILURE;
		}

		/* The child is ready for tracing, nothing interesting in this
		 * example, let the child resume its execution. */
		pink_trace_resume(pid, 0);

		/* Wait for the child to exit. */
		waitpid(pid, &status, 0);

		return EXIT_SUCCESS;
	}
}
