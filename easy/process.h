/*
 * Copyright (c) 2010, 2011, 2012 Ali Polatel <alip@exherbo.org>
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LpIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef PINK_EASY_PROCESS_H
#define PINK_EASY_PROCESS_H

/**
 * @file pinktrace/easy/process.h
 * @brief Pink's easy process representation
 *
 * Do not include this file directly. Use pinktrace/easy/pink.h directly.
 *
 * @defgroup pink_easy_process Pink's easy process representation
 * @ingroup pinktrace-easy
 * @{
 **/

#include <pinktrace/compiler.h>
#include <pinktrace/easy/func.h>
#include <pinktrace/easy/step.h>

#include <stdbool.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct pink_easy_context;

/**
 * @struct pink_easy_process
 * @brief Opaque structure which represents a process entry
 * @note These entries are allocated internally by the tracing context.
 **/
struct pink_easy_process;

/**
 * @struct pink_easy_process_list
 * @brief Opaque structure which represents a process list
 * @note This list is maintained internally by the tracing context.
 **/
struct pink_easy_process_list;

/** The process is attached already */
#define PINK_EASY_PROCESS_ATTACHED		00001
/** Next SIGSTOP is to be ignored */
#define PINK_EASY_PROCESS_IGNORE_ONE_SIGSTOP	00002
/** Process should have forks followed **/
#define PINK_EASY_PROCESS_FOLLOWFORK		00004
/** Process is a clone **/
#define PINK_EASY_PROCESS_CLONE_THREAD		00010
/** We have attached to this process, but did not see it stopping yet */
#define PINK_EASY_PROCESS_STARTUP		00020
/** Process is suspended, waiting for its parent */
#define PINK_EASY_PROCESS_SUSPENDED		00040
/** A system call is in progress **/
#define PINK_EASY_PROCESS_INSYSCALL		00100

/**
 * Insert a traced process into the process tree
 *
 * @note By default @e ptrace(2) step is #PINK_EASY_STEP_NOT_SET, thus the
 *       default @e ptrace(2) method of the tracing context is used. Use
 *       pink_easy_process_set_step() to change the stepping method.
 *
 * @param ctx Tracing context
 * @param tid Thread ID
 * @param tgid Thread group ID, specify -1 for non-clones
 * @param ptrace_step @e ptrace(2) stepping
 * @param flags Bitwise OR'ed PINK_EASY_PROCESS flags
 * @return Process structure on success, NULL on failure and sets errno accordingly
 **/
struct pink_easy_process *pink_easy_process_new(struct pink_easy_context *ctx,
		pid_t tid, pid_t tgid, short flags);

/**
 * Free a process
 *
 * @param ctx Tracing context
 * @param proc Process entry
 **/
void pink_easy_process_free(struct pink_easy_context *ctx, struct pink_easy_process *proc);

/**
 * Detach from a process as necessary and resume its execution.
 *
 * @param proc Process entry
 * @return true on success, false on failure and sets errno accordingly
 **/
bool pink_easy_process_detach(const struct pink_easy_process *proc);

/**
 * Kill a process
 *
 * @note This function uses @e tgkill(2) or @e tkill(2) if available.
 *
 * @param proc Process entry
 * @param sig Signal to deliver
 * @return Same as @e kill(2)
 **/
int pink_easy_process_kill(const struct pink_easy_process *proc, int sig);

/**
 * Returns the thread ID of the entry
 *
 * @param proc Process entry
 * @return Thread ID
 **/
pid_t pink_easy_process_get_tid(const struct pink_easy_process *proc)
	PINK_GCC_ATTR((nonnull(1)));

/**
 * Returns the thread group ID of this entry or -1
 *
 * @param proc Process entry
 * @return Thread group ID or -1
 **/
pid_t pink_easy_process_get_tgid(const struct pink_easy_process *proc)
	PINK_GCC_ATTR((nonnull(1)));

/**
 * Returns the execution type of the entry
 *
 * @param proc Process entry
 * @return System call ABI
 **/
int pink_easy_process_get_abi(const struct pink_easy_process *proc)
	PINK_GCC_ATTR((nonnull(1)));

/**
 * Set the @e ptrace(2) stepping method
 *
 * @param proc Process entry
 * @param ptrace_step @e ptrace(2) stepping method
 **/
void pink_easy_process_set_step(struct pink_easy_process *proc, enum pink_easy_step ptrace_step)
	PINK_GCC_ATTR((nonnull(1)));

/**
 * Return the @e ptrace(2) stepping method
 *
 * @param proc Process entry
 * @return @e ptrace(2) stepping method
 **/
enum pink_easy_step pink_easy_process_get_step(const struct pink_easy_process *proc)
	PINK_GCC_ATTR((nonnull(1)));

/**
 * Set process flags
 *
 * @attention Care must be given not to screw pinktrace's internal state!
 *
 * @param proc Process entry
 * @param flags Flags
 **/
void pink_easy_process_set_flags(struct pink_easy_process *proc, short flags);

/**
 * Return process flags
 *
 * @param proc Process entry
 * @return Process flags
 **/
short pink_easy_process_get_flags(const struct pink_easy_process *proc);

/**
 * Set the user data of the process entry.
 *
 * @note This function accepts a destructor function pointer which may be used
 *       to free the user data. You may pass NULL if you want to handle the
 *       destruction yourself or use the standard @e free(3) function from
 *       stdlib.h for basic destruction.
 *
 * @param proc Process entry
 * @param userdata User data
 * @param userdata_destroy The destructor function of the user data
 **/
void pink_easy_process_set_userdata(struct pink_easy_process *proc, void *userdata,
		pink_easy_free_func_t userdata_destroy)
	PINK_GCC_ATTR((nonnull(1)));

/**
 * Get the user data of the process entry, previously set by
 * pink_easy_process_set_data()
 *
 * @param proc Process entry
 * @return User data
 **/
void *pink_easy_process_get_userdata(const struct pink_easy_process *proc)
	PINK_GCC_ATTR((nonnull(1)));

/**
 * Remove a process from the process list.
 *
 * @note pinktrace doesn't export an insertion function because insertions are
 *       handled internally by this library. You may, however, need to remove
 *       an entry due to problems (e.g. -ESRCH) caused by the process.
 *
 * @param list Process list
 * @param proc Process entry
 **/
void pink_easy_process_list_remove(struct pink_easy_process_list *list,
		const struct pink_easy_process *proc)
	PINK_GCC_ATTR((nonnull(1)));

/**
 * Look up the process list for the given process ID.
 *
 * @param list The process list
 * @param tid Thread ID
 * @return The process on successful look up, NULL on failure
 **/
struct pink_easy_process *pink_easy_process_list_lookup(const struct pink_easy_process_list *list,
		pid_t tid)
	PINK_GCC_ATTR((nonnull(1)));

/**
 * Walk the process tree.
 *
 * @param list Process list
 * @param func Walk function
 * @param userdata User data to pass to the walk function
 * @return Total number of visited entries
 **/
unsigned pink_easy_process_list_walk(const struct pink_easy_process_list *list,
		pink_easy_walk_func_t func, void *userdata)
	PINK_GCC_ATTR((nonnull(1,2)));

#ifdef __cplusplus
}
#endif
/** @} */
#endif
