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

#ifndef PINK_PROCESS_H
#define PINK_PROCESS_H

/**
 * @file pinktrace/process.h
 * @brief Pink's process abstraction
 *
 * Do not include this file directly. Use pinktrace/pink.h instead.
 *
 * @defgroup pink_processess Pink's process abstraction
 * @ingroup pinktrace
 * @{
 **/

#include <sys/types.h>

/** This opaque structure represents a traced process. */
struct pink_process;

/**
 * Allocate and initialise traced process data
 *
 * @param pid Process ID
 * @param procptr Pointer to store the allocated structure
 * @return 0 on success, negated errno on failure
 **/
int pink_process_alloc(pid_t pid, struct pink_process **procptr);

/**
 * Free traced process data
 *
 * @param proc Traced process
 **/
void pink_process_free(struct pink_process *proc);

/**
 * Return process ID of the traced process
 *
 * @param proc Traced process
 * @return Process ID
 **/
pid_t pink_process_get_pid(const struct pink_process *proc);
/**
 * Set the process ID of the traced process
 *
 * @param proc Traced process
 * @param pid New process ID
 **/
void pink_process_set_pid(struct pink_process *proc, pid_t pid);

/**
 * Return the system call ABI of the traced process
 *
 * @param proc Traced process
 * @return System call ABI
 **/
short pink_process_get_abi(const struct pink_process *proc);

/**
 * Update registry set of the traced process
 *
 * @param proc Traced process
 * @return 0 on success, negated errno on failure
 **/
int pink_process_update_regset(struct pink_process *proc);

/** @} */
#endif
