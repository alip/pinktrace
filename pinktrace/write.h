/*
 * Copyright (c) 2010, 2011, 2012, 2013, 2021 Ali Polatel <alip@exherbo.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
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
int pink_write_syscall(pid_t pid, struct pink_regset *regset, long sysnum)
	PINK_GCC_ATTR((nonnull(2)));

/**
 * Set the system call return value
 *
 * @param pid Process ID
 * @param regset Registry set
 * @param retval Return value
 * @param error Error condition (errno)
 * @return 0 on success, negated errno on failure
 **/
int pink_write_retval(pid_t pid, struct pink_regset *regset,
		      long retval, int error)
	PINK_GCC_ATTR((nonnull(2)));

/**
 * Write the specified value to the specified system call argument
 *
 * @param pid Process ID
 * @param regset Registry set
 * @param arg_index Index of the argument, first argument is 0
 * @param argval Value of the argument
 * @return 0 on success, negated errno on failure
 **/
int pink_write_argument(pid_t pid, struct pink_regset *regset,
			unsigned arg_index, long argval)
	PINK_GCC_ATTR((nonnull((2))));

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
ssize_t pink_write_vm_data(pid_t pid, const struct pink_regset *regset, long addr, const char *src, size_t len)
	PINK_GCC_ATTR((nonnull(2,4)));

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
