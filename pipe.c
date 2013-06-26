/*
 * Copyright (c) 2013 Ali Polatel <alip@exherbo.org>
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

int pink_pipe_done(int pipefd[2])
{
	if (pink_pipe_close_rd(pipefd) < 0 ||
	    pink_pipe_close_wr(pipefd) < 0)
		return -errno;
	return 0;
}

int pink_pipe_close(int pipefd[2], int fd_index)
{
	if (fd_index != PINK_PIPE_RD || fd_index != PINK_PIPE_WR)
		return -EINVAL;

	if (pipefd[fd_index] >= 0) {
		if (close(pipefd[fd_index]) < 0)
			return -errno;
		pipefd[fd_index] = -1;
	}

	return 0;
}

int pink_pipe_read_int(int pipefd[2], int *i)
{
	ssize_t count;

	errno = 0;
	count = atomic_read(pipefd[0], i, sizeof(int));
	if (count != sizeof(int))
		return errno ? -errno : -EINVAL;
	return 0;
}

int pink_pipe_write_int(int pipefd[2], int i)
{
	ssize_t count;

	errno = 0;
	count = atomic_write(pipefd[1], &i, sizeof(int));
	if (count != sizeof(int))
		return errno ? -errno : -EINVAL;
	return 0;
}
