/*
 * Copyright (c) 2013, 2021 Ali Polatel <alip@exherbo.org>
 * Based in part upon strace which is:
 *   Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 *   Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 *   Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 *   Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 *   Copyright (c) 1999 IBM Deutschland Entwicklung GmbH, IBM Corporation
 *                       Linux for s390 port by D.J. Barrow
 *                      <barrow_dj@mail.yahoo.com,djbarrow@de.ibm.com>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef PINK_VM_H
#define PINK_VM_H

/**
 * @file pinktrace/vm.h
 * @brief Pink's data transfer between address spaces
 *
 * Do not include this file directly. Use pinktrace/pink.h instead.
 *
 * @defgroup pink_vm Pink's data transfer between address spaces
 * @ingroup pinktrace
 * @{
 **/

/**
 * Read len bytes of data of pid, regset, at address @b addr, to our address
 * space @b dest (ptrace way, one long at a time)
 *
 * @see pink_vm_read()
 **/
ssize_t pink_vm_lread(pid_t pid, const struct pink_regset *regset,
		      long addr, char *dest, size_t len)
	PINK_GCC_ATTR((nonnull(2,4)));

/**
 * Convenience macro to read an object (ptrace way, one long at a time)
 *
 * @see pink_vm_lread()
 **/
#define pink_vm_lread_object(pid, regset, addr, objp) \
		pink_vm_lread((pid), (regset), (addr), (char *)(objp), sizeof(*(objp)))

/**
 * Like pink_vm_lread() but make the additional effort of looking for a
 * terminating zero-byte
 **/
ssize_t pink_vm_lread_nul(pid_t pid, const struct pink_regset *regset,
			  long addr, char *dest, size_t len)
	PINK_GCC_ATTR((nonnull(2,4)));

/**
 * Synonym for pink_vm_lread_nul()
 **/
#define pink_vm_lread_string(pid, regset, addr, dest, len) \
		pink_vm_lread_nul((pid), (regset), (addr), (dest), (len))

/**
 * Write the given data argument @b src to address @b addr (ptrace way one long
 * at a time)
 *
 * @see pink_vm_write()
 **/
ssize_t pink_vm_lwrite(pid_t pid, const struct pink_regset *regset,
		       long addr, const char *src, size_t len)
	PINK_GCC_ATTR((nonnull(2,4)));

/**
 * Convenience macro to write an object (ptrace way one long at a time)
 *
 * @see pink_vm_lwrite()
 **/
#define pink_vm_lwrite_object(pid, regset, addr, objp) \
		pink_vm_lwrite((pid), (regset), (addr), (char *)(objp), sizeof(*(objp)))

/**
 * Read len bytes of data of pid, regset, at address @b addr, to our address
 * space @b dest using cross memory attach
 *
 * @attention If #PINK_HAVE_PROCESS_VM_READV is defined to 0, this function
 *            always returns -1 and sets errno to ENOSYS.
 *
 * @see PINK_HAVE_PROCESS_VM_READV
 * @see pink_vm_read()
 * @see pink_vm_lread()
 **/
ssize_t pink_vm_cread(pid_t pid, const struct pink_regset *regset,
		      long addr, char *dest, size_t len)
	PINK_GCC_ATTR((nonnull(2,4)));

/**
 * Convenience macro to read an object using cross memory attach
 *
 * @see pink_vm_cread
 **/
#define pink_vm_cread_object(pid, regset, addr, objp) \
		pink_vm_cread((pid), (regset), (addr), (char *)(objp), sizeof(*(objp)))

/**
 * Like pink_vm_cread() but make the additional effort of looking for a
 * terminating zero-byte
 **/
ssize_t pink_vm_cread_nul(pid_t pid, const struct pink_regset *regset,
			  long addr, char *dest, size_t len)
	PINK_GCC_ATTR((nonnull(2,4)));

/**
 * Synonym for pink_vm_cread_nul()
 **/
#define pink_vm_cread_string(pid, regset, addr, dest, len) \
		pink_vm_cread_string((pid), (regset), (addr), (dest), (len))

/**
 * Write the given data argument @b src to address @b addr using cross memory
 * attach
 *
 * @attention If #PINK_HAVE_PROCESS_VM_WRITEV is defined to 0, this function
 *            always returns -1 and sets errno to ENOSYS.
 *
 * @see PINK_HAVE_PROCESS_VM_WRITEV
 * @see pink_vm_lwrite()
 * @see pink_vm_write()
 **/
ssize_t pink_vm_cwrite(pid_t pid, const struct pink_regset *regset,
		       long addr, const char *src, size_t len)
	PINK_GCC_ATTR((nonnull(2,4)));

/**
 * Convenience macro to write an object using cross memory attach
 *
 * @see pink_vm_cwrite()
 **/
#define pink_vm_cwrite_object(pid, regset, addr, objp) \
		pink_vm_cwrite((pid), (regset), (addr), (char *)(objp), sizeof(*(objp)))

/** @} */
#endif
