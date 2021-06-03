/* vim: set cino= fo=croql sw=8 ts=8 sts=0 noet cin fdm=syntax : */

/**
 * \file
 *
 * Example \ref pink-about.c "pink-about.c" .
 **/

/**
 * \example pink-about.c
 *
 * A simple example demonstrating how to use Pink's version macros.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pinktrace/pink.h>

int
main(void)
{
	printf("Built using %s %d.%d.%d%s",
		PINKTRACE_PACKAGE,
		PINKTRACE_VERSION_MAJOR,
		PINKTRACE_VERSION_MINOR,
		PINKTRACE_VERSION_MICRO,
		PINKTRACE_VERSION_SUFFIX);

	if (strncmp(PINKTRACE_GIT_HEAD, "", 1))
		printf(" %s", PINKTRACE_GIT_HEAD);

	fputc('\n', stdout);

	return EXIT_SUCCESS;
}
