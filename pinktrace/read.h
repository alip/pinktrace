/*
 * Copyright (c) 2010, 2011, 2012, 2013, 2021 Ali Polatel <alip@exherbo.org>
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
 * SPDX-License-Identifier: LGPL-2.1-or-later
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
 * Read the system call ABI
 *
 * @param pid Process ID
 * @param regset Registry set
 * @param abi Pointer to store the system call ABI, must @b not be @e NULL
 * @return 0 on success, negated errno on failure
 **/
int pink_read_abi(pid_t pid, const struct pink_regset *regset, short *abi)
	PINK_GCC_ATTR((nonnull(2,3)));

/**
 * Read the system call number
 *
 * @param pid Process ID
 * @param regset Registry set
 * @param sysnum Pointer to store the system call, must @b not be @e NULL
 * @return 0 on success, negated errno on failure
 **/
int pink_read_syscall(pid_t pid, const struct pink_regset *regset, long *sysnum)
	PINK_GCC_ATTR((nonnull(2,3)));

/**
 * Read the return value
 *
 * @param pid Process ID
 * @param regset Registry set
 * @param retval Pointer to store the return value, must @b not be @e NULL
 * @param error Pointer to store the error condition
 * @return 0 on success, negated errno on failure
 **/
int pink_read_retval(pid_t pid, const struct pink_regset *regset,
		     long *retval, int *error)
	PINK_GCC_ATTR((nonnull(2,3)));

/**
 * Read the specified system call argument
 *
 * @param pid Process ID
 * @param regset Registry set
 * @param arg_index Index of the argument, first argument is 0
 * @param argval Pointer to store the value of the argument, must @b not be @e NULL
 * @return 0 on success, negated errno on failure
 **/
int pink_read_argument(pid_t pid, const struct pink_regset *regset,
		       unsigned arg_index, long *argval)
	PINK_GCC_ATTR((nonnull(2,4)));

/**
 * Read len bytes of data of tracee at address @b addr, to our address
 * space @b dest
 *
 * @note This function uses either one of the functions:
 *       - pink_vm_cread()
 *       - pink_vm_lread()
 * depending on availability.
 * @see pink_vm_cread()
 * @see pink_vm_lread()
 * @see PINK_HAVE_PROCESS_VM_READV
 *
 * @param pid Process ID
 * @param regset Registry set
 * @param addr Address in tracee's address space
 * @param dest Pointer to store the data, must @b not be @e NULL
 * @param len Number of bytes of data to read
 * @return On success, this function returns the number of bytes read.
 *         On error, -1 is returned and errno is set appropriately.
 *         Check the return value for partial reads.
 **/
ssize_t pink_read_vm_data(pid_t pid, const struct pink_regset *regset,
			  long addr, char *dest, size_t len)
	PINK_GCC_ATTR((nonnull(2,4)));

/**
 * Like pink_read_vm_data() but instead of setting errno, this function returns
 * negated errno on failure and -EFAULT on partial reads.
 *
 * @see pink_read_vm_data()
 *
 * @param pid Process ID
 * @param regset Registry set
 * @param addr Address in tracee's address space
 * @param dest Pointer to store the data, must @b not be @e NULL
 * @param len Number of bytes of data to read
 * @return 0 on success, negated errno on failure
 **/
int pink_read_vm_data_full(pid_t pid, const struct pink_regset *regset,
			   long addr, char *dest, size_t len)
	PINK_GCC_ATTR((nonnull(2,4)));

/**
 * Convenience macro to read an object
 *
 * @see pink_read_vm_data()
 **/
#define pink_read_vm_object(pid, regset, addr, objp) \
		pink_read_vm_data((pid), (regset), (addr), \
				  (char *)(objp), sizeof(*(objp)))

/**
 * Convenience macro to read an object fully
 *
 * @see pink_read_vm_data_full()
 **/
#define pink_read_vm_object_full(pid, regset, addr, objp) \
		pink_read_vm_data_full((pid), (regset), (addr), \
				       (char *)(objp), sizeof(*(objp)))

/**
 * Like pink_read_vm_data() but make the additional effort of looking for a
 * terminating zero-byte
 *
 * @note This function uses either one of the functions:
 *       - pink_vm_cread_nul()
 *       - pink_vm_lread_nul()
 * depending on availability.
 * @see pink_vm_cread_nul()
 * @see pink_vm_lread_nul()
 * @see PINK_HAVE_PROCESS_VM_READV
 *
 * @param pid Process ID
 * @param regset Registry set
 * @param addr Address in tracee's address space
 * @param dest Pointer to store the data, must @b not be @e NULL
 * @param len Number of bytes of data to read
 * @return On success, this function returns the number of bytes read @b including the zero-byte.
 *         On error, -1 is returned and errno is set appropriately.
 *         Check the return value for partial reads.
 **/
ssize_t pink_read_vm_data_nul(pid_t pid, const struct pink_regset *regset,
			      long addr, char *dest, size_t len)
	PINK_GCC_ATTR((nonnull(2,4)));

/**
 * Synonym for pink_read_vm_data_nul()
 *
 * @see pink_read_vm_data_nul()
 **/
#define pink_read_string(pid, regset, addr, dest, len) \
		pink_read_vm_data_nul((pid), (regset), (addr), (dest), (len))

/**
 * Read the requested member of a NULL-terminated string array
 *
 * @see pink_read_string()
 * @see pink_read_vm_data_nul()
 *
 * @param pid Process ID
 * @param regset Registry set
 * @param arg Address of the argument, see pink_read_argument()
 * @param arr_index Array index
 * @param dest Pointer to store the result, must @b not be @e NULL
 * @param dest_len Length of the destination
 * @param nullptr If non-NULL, specifies the address of a boolean which can be
 *                used to determine whether the member at the given index is
 *                @e NULL, in which case the dest argument is left unmodified.
 * @return Same as pink_read_vm_data_nul()
 **/
ssize_t pink_read_string_array(pid_t pid, const struct pink_regset *regset,
			       long arg, unsigned arr_index,
			       char *dest, size_t dest_len,
			       bool *nullptr)
	PINK_GCC_ATTR((nonnull(2,5)));

/** @} */
#endif
