/*
 * Copyright (c) 2010, 2011, 2012, 2013 Ali Polatel <alip@exherbo.org>
 * Based in part upon strace which is:
 *   Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 *   Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 *   Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 *   Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 *   Copyright (c) 1999 IBM Deutschland Entwicklung GmbH, IBM Corporation
 *                       Linux for s390 port by D.J. Barrow
 *                      <barrow_dj@mail.yahoo.com,djbarrow@de.ibm.com>
 *   Copyright (c) 2000 PocketPenguins Inc.  Linux for Hitachi SuperH
 *                      port by Greg Banks <gbanks@pocketpenguins.com>
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

#ifndef PINK_READ_H
#define PINK_READ_H

/**
 * @file pinktrace/read.h
 * @brief Pink's system call readers
 *
 * Do not include this file directly. Use pinktrace/pink.h instead.
 *
 * @defgroup pink_read Pink's system call readers
 * @ingroup pinktrace
 * @{
 **/

#include <stdbool.h>
#include <sys/types.h>

/**
 * Read a word at the given offset in tracee's USER area and place it in res,
 * aka @c PTRACE_PEEKUSER.
 *
 * @param pid Process ID
 * @param off Offset
 * @param res Result (may be NULL, e.g. to test if the given offset is readable)
 * @return 0 on success, negated errno on failure
 **/
int pink_read_word_user(pid_t pid, long off, long *res);

/**
 * Read a word at the given offset in the tracee's memory, and place it in
 * res, aka @c PTRACE_PEEKDATA or @c PTRACE_PEEKTEXT.
 *
 * @param pid Process ID
 * @param off Offset
 * @param res Result (may be NULL, e.g. to test if the given offset is readable)
 * @return 0 on success, negated errno on failure
 **/
int pink_read_word_data(pid_t pid, long off, long *res);

/**
 * Read the system call number
 *
 * @param tracee Traced process
 * @param sysnum Pointer to store the system call, must @b not be @e NULL
 * @return 0 on success, negated errno on failure
 **/
int pink_read_syscall(struct pink_process *tracee, long *sysnum)
	PINK_GCC_ATTR((nonnull(2)));

/**
 * Read the return value
 *
 * @param tracee Traced process
 * @param retval Pointer to store the return value, must @b not be @e NULL
 * @param error Pointer to store the error condition
 * @return 0 on success, negated errno on failure
 **/
int pink_read_retval(struct pink_process *tracee, long *retval, int *error)
	PINK_GCC_ATTR((nonnull(2)));

/**
 * Read the specified system call argument
 *
 * @param tracee Traced process
 * @param arg_index Index of the argument, first argument is 0
 * @param argval Pointer to store the value of the argument, must @b not be @e NULL
 * @return 0 on success, negated errno on failure
 **/
int pink_read_argument(struct pink_process *tracee, unsigned arg_index, long *argval)
	PINK_GCC_ATTR((nonnull(3)));

/**
 * Read len bytes of data of tracee at address @b addr, to our address
 * space @b dest
 *
 * @note This function calls the functions:
 *       - pink_vm_cread()
 *       - pink_vm_lread()
 * depending on availability.
 * @see pink_vm_cread()
 * @see pink_vm_lread()
 * @see PINK_HAVE_PROCESS_VM_READV
 *
 * @param tracee Traced process
 * @param addr Address in tracee's address space
 * @param dest Pointer to store the data, must @b not be @e NULL
 * @param len Number of bytes of data to read
 * @return On success, this function returns the number of bytes read.
 *         On error, -1 is returned and errno is set appropriately.
 *         Check the return value for partial reads.
 **/
ssize_t pink_read_vm_data(struct pink_process *tracee, long addr, char *dest, size_t len)
	PINK_GCC_ATTR((nonnull(3)));

/**
 * Like pink_read_vm_data() but instead of setting errno, this function returns
 * negated errno on failure and -EFAULT on partial reads.
 *
 * @see pink_read_vm_data()
 *
 * @param tracee Traced process
 * @param addr Address in tracee's address space
 * @param dest Pointer to store the data, must @b not be @e NULL
 * @param len Number of bytes of data to read
 * @return 0 on success, negated errno on failure
 **/
int pink_read_vm_data_full(struct pink_process *tracee, long addr, char *dest, size_t len)
	PINK_GCC_ATTR((nonnull(3)));

/**
 * Convenience macro to read an object
 *
 * @see pink_read_vm_data()
 **/
#define pink_read_vm_object(tracee, addr, objp) \
		pink_read_vm_data((tracee), (addr), \
				  (char *)(objp), sizeof(*(objp)))

/**
 * Convenience macro to read an object fully
 *
 * @see pink_read_vm_data_full()
 **/
#define pink_read_vm_object_full(tracee, addr, objp) \
		pink_read_vm_data_full((tracee), (addr), \
				  (char *)(objp), sizeof(*(objp)))

/**
 * Like pink_read_vm_data() but make the additional effort of looking for a
 * terminating zero-byte
 *
 * @see pink_read_vm_data()
 **/
ssize_t pink_read_vm_data_nul(struct pink_process *tracee, long addr, char *dest, size_t len)
	PINK_GCC_ATTR((nonnull(3)));

/**
 * Synonym for pink_read_vm_data_nul()
 *
 * @see pink_read_vm_data_nul()
 **/
#define pink_read_string(tracee, addr, dest, len) \
		pink_read_vm_data_nul((tracee), (addr), (dest), (len))

/**
 * Read the requested member of a NULL-terminated string array
 *
 * @see pink_read_string()
 * @see pink_read_vm_data_nul()
 *
 * @param tracee Traced process
 * @param arg Address of the argument, see pink_read_argument()
 * @param arr_index Array index
 * @param dest Pointer to store the result, must @b not be @e NULL
 * @param dest_len Length of the destination
 * @param nullptr If non-NULL, specifies the address of a boolean which can be
 *                used to determine whether the member at the given index is
 *                @e NULL, in which case the dest argument is left unmodified.
 * @return Same as pink_read_vm_data_nul()
 **/
ssize_t pink_read_string_array(struct pink_process *tracee,
			       long arg, unsigned arr_index,
			       char *dest, size_t dest_len,
			       bool *nullptr)
	PINK_GCC_ATTR((nonnull(4)));

/** @} */
#endif
