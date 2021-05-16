/*
 * Copyright (c) 2013, 2021 Ali Polatel <alip@exherbo.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef PINK_REGSET_H
#define PINK_REGSET_H

/**
 * @file pinktrace/regset.h
 * @brief Pink's process registry set
 *
 * Do not include this file directly. Use pinktrace/pink.h instead.
 *
 * @defgroup pink_regset Pink's process registry set
 * @ingroup pinktrace
 * @{
 **/

#include <sys/types.h>

/** This opaque structure represents a registry set of a traced process */
struct pink_regset;

/**
 * Allocate a registry set
 *
 * @param regptr Pointer to store the dynamically allocated registry set,
 *		 Use pink_regset_free() to free after use.
 * @return 0 on success, negated errno on failure
 **/
int pink_regset_alloc(struct pink_regset **regptr)
	PINK_GCC_ATTR((nonnull(1)));

/**
 * Free the memory allocated for the registry set
 *
 * @param regset Registry set
 **/
void pink_regset_free(struct pink_regset *regset);

/**
 * Fill the given regset structure with the registry information of the given
 * process ID
 *
 * @param pid Process ID
 * @param regset Registry set
 * @return 0 on success, negated errno on failure
 **/
int pink_regset_fill(pid_t pid, struct pink_regset *regset)
	PINK_GCC_ATTR((nonnull(2)));

/** @} */
#endif
