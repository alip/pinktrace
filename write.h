/*
 * Copyright (c) 2010, 2011, 2012, 2013 Ali Polatel <alip@exherbo.org>
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

#ifndef PINK_WRITE_H
#define PINK_WRITE_H

/**
 * @file pinktrace/write.h
 * @brief Pink's system call writers
 *
 * Do not include this header directly, use pinktrace/pink.h instead.
 *
 * @defgroup pink_write Pink's system call writers
 * @ingroup pinktrace
 * @{
 **/

#include <stdbool.h>
#include <sys/types.h>

/**
 * Copy the word val to the given offset in the tracee's USER area, aka
 * PTRACE_POKEUSER.
 *
 * @param pid Process ID
 * @param off Offset
 * @param val Word
 * @return 0 on success, negated errno on failure
 **/
int pink_write_word_user(pid_t pid, long off, long val);

/**
 * Copy the word val to location addr in the tracee's memory, aka
 * PTRACE_POKEDATA.
 *
 * @param pid Process ID
 * @param off Offset
 * @param val Word
 * @return 0 on success, negated errno on failure
 **/
int pink_write_word_data(pid_t pid, long off, long val);

/**
 * Set the system call to the given value
 *
 * @note On ARM architecture, this only works for EABI system calls.
 *
 * @param pid Process ID
 * @param regset Registry set
 * @param sysnum System call number
 * @return 0 on success, negated errno on failure
 **/
int pink_write_syscall(pid_t pid, struct pink_regset *regset, long sysnum);

/**
 * Set the system call return value
 *
 * @param pid Process ID
 * @param regset Registry set
 * @param retval Return value
 * @param error Error condition (errno)
 * @return 0 on success, negated errno on failure
 **/
int pink_write_retval(pid_t pid, struct pink_regset *regset, long retval, int error);

/**
 * Write the specified value to the specified system call argument
 *
 * @param pid Process ID
 * @param regset Registry set
 * @param arg_index Index of the argument, first argument is 0
 * @param argval Value of the argument
 * @return 0 on success, negated errno on failure
 **/
int pink_write_argument(pid_t pid, struct pink_regset *regset, unsigned arg_index, long argval);

/**
 * Write the given data argument @b src to address @b addr
 *
 * @note This function calls the functions:
 *       - pink_vm_cwrite()
 *       - pink_vm_lwrite()
 * depending on availability.
 * @see pink_vm_cwrite()
 * @see pink_vm_lwrite()
 * @see #PINK_HAVE_PROCESS_VM_WRITEV
 *
 * @param pid Process ID
 * @param regset Registry set
 * @param addr Address in tracee's address space
 * @param src Pointer to the data, must @b not be @e NULL
 * @param len Number of bytes of data to write
 * @return On success, this function returns the number of bytes written.
 *         On error, -1 is returned and errno is set appropriately.
 *         Check the return value for partial writes.
 **/
ssize_t pink_write_vm_data(pid_t pid, struct pink_regset *regset, long addr, const char *src, size_t len)
	PINK_GCC_ATTR((nonnull(4)));

/**
 * Convenience macro to write an object
 *
 * @see pink_write_vm_data()
 **/
#define pink_write_vm_object(tid, abi, addr, objp) \
		pink_write_vm_data((tid), (abi), (addr), \
				   (char *)(objp), sizeof(*(objp)))

/** @} */
#endif
