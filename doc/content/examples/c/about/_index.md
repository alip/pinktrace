---
title: "pink-about.c"
date: 2021-06-03T19:36:32+02:00
anchor: "pinkaboutc"
weight: 40
---

Raw: https://git.exherbo.org/pinktrace-1.git/plain/examples/pink-about.c

{{< highlight c "linenos=table,hl_lines=16-24" >}}
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
{{< / highlight >}}
