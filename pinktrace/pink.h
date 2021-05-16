/*
 * Copyright (c) 2010, 2011, 2012, 2013, 2021 Ali Polatel <alip@exherbo.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef PINK_PINK_H
#define PINK_PINK_H

/**
 * @mainpage pinktrace
 *
 * pinktrace - Pink's Tracing Library
 *
 * @section overview Overview
 *
 * pinktrace is a wrapper around @c ptrace(2) system call.
 * It provides a robust API for tracing processes.
 *
 * @attention This is a work in progress and the API is @b not stable.
 *
 * @author Ali Polatel <alip@exherbo.org>
 **/

/**
 * @file pinktrace/pink.h
 * @brief A header file including all other header files part of pinktrace
 * @defgroup pinktrace Pink's Tracing Library
 * @{
 **/
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <sys/types.h>

#include <pinktrace/about.h>
#include <pinktrace/compat.h>
#include <pinktrace/compiler.h>
#include <pinktrace/system.h>
#include <pinktrace/abi.h>
#include <pinktrace/event.h>
#include <pinktrace/trace.h>
#include <pinktrace/regset.h>
#include <pinktrace/vm.h>
#include <pinktrace/read.h>
#include <pinktrace/write.h>
#include <pinktrace/socket.h>

#include <pinktrace/name.h>
#include <pinktrace/pipe.h>

#ifdef __cplusplus
}
#endif
/** @} */
#endif
