/*
 * Copyright (c) 2013, 2021 Ali Polatel <alip@exherbo.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <pinktrace/private.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <pinktrace/pink.h>

#ifndef HAVE_PIPE2
static int set_cloexec_flag(int fd)
{
	int flags, newflags;

	flags = fcntl(fd, F_GETFD);
	if (flags < 0)
		return -errno;

	newflags = flags | FD_CLOEXEC;
	if (flags == newflags)
		return 0;

	if (fcntl(fd, F_SETFD, newflags))
		return -errno;

	return 0;
}
#endif

/*
 * Reading or writing pipe data is atomic if the size of data written is not
 * greater than PIPE_BUF.
 */

static ssize_t atomic_read(int fd, void *buf, size_t count)
{
	ssize_t total = 0;

	while (count > 0) {
		ssize_t retval;

		retval = read(fd, buf, count);
		if (retval < 0)
			return (total > 0) ? total : -1;
		else if (retval == 0)
			return total;

		total += retval;
		buf = (char *)buf + retval;
		count -= retval;
	}

	return total;
}

static ssize_t atomic_write(int fd, const void *buf, size_t count)
{
	ssize_t total = 0;

	while (count > 0) {
		ssize_t retval;

		retval = write(fd, buf, count);
		if (retval < 0)
			return (total > 0) ? total : -1;
		else if (retval == 0)
			return total;

		total += retval;
		buf = (const char *)buf + retval;
		count -= retval;
	}

	return total;
}

int pink_pipe_init(int pipefd[2])
{
	int retval;

#ifdef HAVE_PIPE2
	retval = pipe2(pipefd, O_CLOEXEC);
#else
	retval = pipe(pipefd);
#endif
	if (retval < 0)
		return -errno;

#ifndef HAVE_PIPE2
	if (set_cloexec_flag(pipefd[0]) < 0 ||
	    set_cloexec_flag(pipefd[1]) < 0)
		return -errno;
#endif
	return 0;
}

int pink_pipe_close(int fd)
{
	return close(fd) < 0 ? -errno : 0;
}

PINK_GCC_ATTR((nonnull(2)))
int pink_pipe_read_int(int fd, int *i)
{
	ssize_t count;

	errno = 0;
	count = atomic_read(fd, i, sizeof(int));
	if (count != sizeof(int))
		return errno ? -errno : -EINVAL;
	return 0;
}

int pink_pipe_write_int(int fd, int i)
{
	ssize_t count;

	errno = 0;
	count = atomic_write(fd, &i, sizeof(int));
	if (count != sizeof(int))
		return errno ? -errno : -EINVAL;
	return 0;
}
