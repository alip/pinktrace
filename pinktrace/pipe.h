/*
 * Copyright (c) 2013, 2021 Ali Polatel <alip@exherbo.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef PINK_PIPE_H
#define PINK_PIPE_H

/**
 * @file pinktrace/pipe.h
 * @brief Pink's pipe() helpers
 *
 * Do not include this header directly, use pinktrace/pink.h instead.
 *
 * @defgroup pink_fork Pink's pipe() helpers
 * @ingroup pinktrace
 * @{
 **/

/**
 * Create pipe
 *
 * @param pipefd Used to return two file descriptors referring to the ends of
 * the pipe. pipefd[0] refers to the read end of the pipe. pipefd[1] refers to
 * the write end of the pipe.
 * @return 0 on success, negated errno on failure
 **/
int pink_pipe_init(int pipefd[2]);

/**
 * Close pipe file descriptor
 *
 * @param fd Pipe file descriptor array
 * @return 0 on success, negated errno on failure
 **/
int pink_pipe_close(int fd);

/**
 * Read an integer from the read end of the pipe
 *
 * @param fd Pipe file descriptor
 * @param i Pointer to store the integer
 * @return 0 on success, negated errno on failure
 **/
int pink_pipe_read_int(int fd, int *i)
	PINK_GCC_ATTR((nonnull(2)));

/**
 * Write an integer to the write end of the pipe
 *
 * @param pipefd Pipe file descriptor
 * @param i Integer
 * @return 0 on success, negated errno on failure
 **/
int pink_pipe_write_int(int fd, int i);

/** @} */
#endif
