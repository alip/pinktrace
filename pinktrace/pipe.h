/*
 * Copyright (c) 2013 Ali Polatel <alip@exherbo.org>
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

#define PINK_PIPE_RD	0 /**< Index of the read end of the pipe */
#define PINK_PIPE_WR	1 /**< Index of the write end of the pipe */

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
 * Close pipe file descriptors
 *
 * @param pipefd Pipe file descriptor array
 * @return 0 on success, negated errno on failure
 **/
int pink_pipe_done(int pipefd[2]);

/**
 * Close pipe file descriptor with the given index
 *
 * @param pipefd Pipe file descriptor array
 * @param fd_index Index of the file descriptor
 * @return 0 on success, negated errno on failure
 **/
int pink_pipe_close(int pipefd[2], int fd_index);
/**
 * Close read end of the pipe
 *
 * @see pink_pipe_close()
 *
 * @param pipefd Pipe file descriptor array
 **/
#define pink_pipe_close_rd(pipefd)	pink_pipe_close((pipefd), PINK_PIPE_RD)
/**
 * Close write end of the pipe
 *
 * @see pink_pipe_close()
 *
 * @param pipefd Pipe file descriptor array
 **/
#define pink_pipe_close_wr(pipefd)	pink_pipe_close((pipefd), PINK_PIPE_WR)

/**
 * Read an integer from the read end of the pipe
 *
 * @param pipefd Pipe file descriptor array
 * @param i Pointer to store the integer
 * @return 0 on success, negated errno on failure
 **/
int pink_pipe_read_int(int pipefd[2], int *i);
/**
 * Write an integer to the write end of the pipe
 *
 * @param pipefd Pipe file descriptor array
 * @param i Integer
 * @return 0 on success, negated errno on failure
 **/
int pink_pipe_write_int(int pipefd[2], int i);

/** @} */
#endif
