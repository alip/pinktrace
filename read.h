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

#include <pinktrace/compiler.h>
#include <pinktrace/abi.h>
#include <pinktrace/regs.h>

#include <stdbool.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Read a word at the given offset in tracee's USER area and place it in res,
 * aka @c PTRACE_PEEKUSER.
 *
 * @param tid Thread ID
 * @param off Offset
 * @param res Result (may be NULL, e.g. to test if the given offset is readable)
 * @return 0 on success, negated errno on failure
 **/
int pink_read_word_user(pid_t tid, long off, long *res);

/**
 * Read a word at the given offset in the tracee's memory, and place it in
 * res, aka @c PTRACE_PEEKDATA or @c PTRACE_PEEKTEXT.
 *
 * @param tid Thread ID
 * @param off Offset
 * @param res Result (may be NULL, e.g. to test if the given offset is readable)
 * @return 0 on success, negated errno on failure
 **/
int pink_read_word_data(pid_t tid, long off, long *res);

/**
 * Read system call ABI
 *
 * @param tid Thread ID
 * @param regs Pointer to the structure of registers; see pink_trace_get_regs()
 * @param abi Pointer to store the result, must @b not be @e NULL
 * @return 0 on success, negated errno on failure
 **/
int pink_read_abi(pid_t tid, const pink_regs_t *regs, enum pink_abi *abi);

/**
 * Read len bytes of data of process @b pid, at address @b addr, to our address
 * space @b dest
 *
 * @note This function uses @c process_vm_readv() if available and falls back to
 *       the old @e ptrace(2) based method in case this system call fails with
 *       @e ENOSYS.
 * @see #PINK_HAVE_PROCESS_VM_READV
 *
 * @param tid Thread ID
 * @param abi System call ABI; see pink_read_abi()
 * @param addr Address in tracee's address space
 * @param dest Pointer to store the data, must @b not be @e NULL
 * @param len Number of bytes of data to read
 * @return On success, this function returns the number of bytes read.
 *         On error, -1 is returned and errno is set appropriately.
 *         Check the return value for partial reads.
 **/
ssize_t pink_read_vm_data(pid_t tid, enum pink_abi abi, long addr,
			  char *dest, size_t len)
	PINK_GCC_ATTR((nonnull(4)));

/**
 * Convenience macro to read an object
 *
 * @see pink_read_vm_data
 **/
#define pink_read_vm_object(pid, abi, addr, objp) \
		pink_read_vm_data((pid), (abi), (addr), \
				(char *)(objp), \
				sizeof(*(objp)))

/**
 * Read the system call number
 *
 * @param tid Thread ID
 * @param abi System call ABI; see pink_read_abi()
 * @param regs Pointer to the structure of registers; see pink_trace_get_regs()
 * @param sysnum Pointer to store the system call, must @b not be @e NULL
 * @return 0 on success, negated errno on failure
 **/
int pink_read_syscall(pid_t tid, enum pink_abi abi,
		      const pink_regs_t *regs, long *sysnum)
	PINK_GCC_ATTR((nonnull(3)));

/**
 * Read the return value
 *
 * @param tid Thread ID
 * @param abi System call ABI; see pink_read_abi()
 * @param regs Pointer to the structure of registers; see pink_trace_get_regs()
 * @param retval Pointer to store the return value, must @b not be @e NULL
 * @param error Pointer to store the error condition, must @b not be @e NULL
 * @return 0 on success, negated errno on failure
 **/
int pink_read_retval(pid_t tid, enum pink_abi abi,
		     const pink_regs_t *regs, long *retval,
		     int *error)
	PINK_GCC_ATTR((nonnull(3,4)));

/**
 * Read the specified system call argument
 *
 * @param tid Thread ID
 * @param abi System call ABI; see pink_read_abi()
 * @param regs Pointer to the structure of registers; see pink_trace_get_regs()
 * @param arg_index Index of the argument, first argument is 0
 * @param argval Pointer to store the value of the argument, must @b not be @e NULL
 * @return 0 on success, negated errno on failure
 **/
int pink_read_argument(pid_t tid, enum pink_abi abi,
		       const pink_regs_t *regs,
		       unsigned arg_index, long *argval)
	PINK_GCC_ATTR((nonnull(5)));

/**
 * Like pink_read_vm_data() but make the additional effort of looking for a
 * terminating zero-byte
 **/
ssize_t pink_read_vm_data_nul(pid_t tid, enum pink_abi abi, long addr,
			      char *dest, size_t len)
	PINK_GCC_ATTR((nonnull(4)));

/**
 * Synonym for pink_read_vm_data_nul()
 **/
#define pink_read_string(tid, abi, addr, dest, len) \
		pink_read_vm_data_nul((tid), (abi), (addr), \
				      (dest), (len))

/**
 * Read the requested member of a NULL-terminated string array
 *
 * @see pink_read_string()
 * @see pink_read_vm_data_nul()
 *
 * @param tid Thread ID
 * @param abi System call ABI; see pink_read_abi()
 * @param arg Address of the argument, see pink_read_argument()
 * @param arr_index Array index
 * @param dest Pointer to store the result, must @b not be @e NULL
 * @param dest_len Length of the destination
 * @param nullptr If non-NULL, specifies the address of a boolean which can be
 *                used to determine whether the member at the given index is
 *                @e NULL, in which case the dest argument is left unmodified.
 * @return Same as pink_read_vm_data_nul()
 **/
ssize_t pink_read_string_array(pid_t tid, enum pink_abi abi,
			       long arg, unsigned arr_index,
			       char *dest, size_t dest_len,
			       bool *nullptr)
	PINK_GCC_ATTR((nonnull(5)));

#ifdef __cplusplus
}
#endif
/** @} */
#endif
