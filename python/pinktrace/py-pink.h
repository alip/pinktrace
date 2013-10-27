/*
 * Copyright (c) 2010, 2011, 2012, 2013 Ali Polatel <alip@exherbo.org>
 * Based in part upon Python-2.6.4's Modules/posixmodule.c which is:
 *   Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009
 *   Python Software Foundation
 *
 * This module is free software, and you may redistribute it and/or modify
 * it under the same terms as Python itself, so long as this copyright message
 * and disclaimer are retained in their original form.
 *
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
 * SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF
 * THIS CODE, EVEN IF THE AUTHOR HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * THE AUTHOR SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE.  THE CODE PROVIDED HEREUNDER IS ON AN "AS IS" BASIS,
 * AND THERE IS NO OBLIGATION WHATSOEVER TO PROVIDE MAINTENANCE,
 * SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef _PINK_PY_H
#define _PINK_PY_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#define PY_SSIZE_T_CLEAN 1
#include <Python.h>
#include <signal.h>
#include <pinktrace/pink.h>

#if PY_MAJOR_VERSION < 3 /* python2 */
# if !defined(SIZEOF_PID_T) || SIZEOF_PID_T == SIZEOF_INT
#  define PARSE_PID "i"
#  define PyLong_FromPid PyInt_FromLong
#  define PyLong_AsPid PyInt_AsLong
# elif SIZEOF_PID_T == SIZEOF_LONG
#  define PARSE_PID "l"
#  define PyLong_FromPid PyInt_FromLong
#  define PyLong_AsPid PyInt_AsLong
# elif defined(SIZEOF_LONG_LONG) && SIZEOF_PID_T == SIZEOF_LONG_LONG
#  define PARSE_PID "L"
#  define PyLong_FromPid PyLong_FromLongLong
#  define PyLong_AsPid PyInt_AsLongLong
# else
#  error sizeof(pid_t) is neither sizeof(int), sizeof(long) or sizeof(long long)
# endif
#else /* python3 */
# if !defined(SIZEOF_PID_T) || SIZEOF_PID_T == SIZEOF_INT
#  define PARSE_PID "i"
#  define PyLong_FromPid PyLong_FromLong
#  define PyLong_AsPid PyLong_AsLong
# elif SIZEOF_PID_T == SIZEOF_LONG
#  define PARSE_PID "l"
#  define PyLong_FromPid PyLong_FromLong
#  define PyLong_AsPid PyLong_AsLong
# elif defined(SIZEOF_LONG_LONG) && SIZEOF_PID_T == SIZEOF_LONG_LONG
#  define PARSE_PID "L"
#  define PyLong_FromPid PyLong_FromLongLong
#  define PyLong_AsPid PyLong_AsLongLong
# else
#  error sizeof(pid_t) is neither sizeof(int), sizeof(long) or sizeof(long long)
# endif
#endif

/* PyErr_SetFromErrno and PyErr_Format always returns NULL. */
#define PINK_CALL(call) \
	do { \
		int r; \
		r = (call); \
		if (r == -ENOSYS) { \
			PyErr_Format(PyExc_SystemError, "Not supported by host system"); \
		} else if (r < 0) { \
			errno = -r; \
			return PyErr_SetFromErrno(PyExc_OSError); \
		} \
	} while (0)

#define PINK_CALL_TRUE(call) \
	do { \
		PINK_CALL((call)); \
		Py_RETURN_TRUE; \
	} while (0)

#define PINK_CHECK_SIGNAL(signum) \
	do { \
		if ((signum) < 0 || (signum) >= _NSIG) { \
			return PyErr_Format(PyExc_ValueError, "Invalid signal '%d'", (signum)); \
		} \
	} while (0)

#endif
