/*
 * Copyright (c) 2013 Ali Polatel <alip@exherbo.org>
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

#include "py-pink.h"

PyMODINIT_FUNC
#if PY_MAJOR_VERSION > 2
PyInit_name(void);
#else
initname(void);
#endif /* PY_MAJOR_VERSION > 2 */

static char pinkpy_name_event_doc[] = "TODO";
static PyObject *
pinkpy_name_event(PyObject *self, PyObject *args)
{
	enum pink_event event;
	const char *name;

	if (!PyArg_ParseTuple(args, "I", &event))
		return NULL;

	PINK_CHECK_NULL((name = pink_name_event(event)), "Invalid event");
	return Py_BuildValue("s", name);
}

static char pinkpy_lookup_event_doc[] = "TODO";
static PyObject *
pinkpy_lookup_event(PyObject *self, PyObject *args)
{
	enum pink_event event;
	const char *name;

	if (!PyArg_ParseTuple(args, "s", &name))
		return NULL;

	PINK_CHECK_LOOKUP((event = pink_lookup_event(name)), "No matching event");
	return Py_BuildValue("I", event);
}

static char pinkpy_name_syscall_doc[] = "TODO";
static PyObject *
pinkpy_name_syscall(PyObject *self, PyObject *args)
{
	short abi;
	long scno;
	const char *name;

	abi = PINK_ABI_DEFAULT;
	if (!PyArg_ParseTuple(args, "l|h", &scno, &abi))
		return NULL;

	PINK_CHECK_NULL((name = pink_name_syscall(scno, abi)), "Invalid scno");
	return Py_BuildValue("s", name);
}

static char pinkpy_lookup_syscall_doc[] = "TODO";
static PyObject *
pinkpy_lookup_syscall(PyObject *self, PyObject *args)
{
	short abi;
	long scno;
	const char *name;

	abi = PINK_ABI_DEFAULT;
	if (!PyArg_ParseTuple(args, "s|h", &name, &abi))
		return NULL;

	PINK_CHECK_LOOKUP((scno = pink_lookup_syscall(name, abi)), "No matching syscall");
	return Py_BuildValue("l", scno);
}

static char pinkpy_name_socket_family_doc[] = "TODO";
static PyObject *
pinkpy_name_socket_family(PyObject *self, PyObject *args)
{
	int family;
	const char *name;

	if (!PyArg_ParseTuple(args, "i", &family))
		return NULL;

	PINK_CHECK_NULL((name = pink_name_socket_family(family)), "Invalid family");
	return Py_BuildValue("s", name);
}

static char pinkpy_lookup_socket_family_doc[] = "TODO";
static PyObject *
pinkpy_lookup_socket_family(PyObject *self, PyObject *args)
{
	int family;
	const char *name;

	if (!PyArg_ParseTuple(args, "s", &name))
		return NULL;

	PINK_CHECK_LOOKUP((family = pink_lookup_socket_family(name)), "No matching socket_family");
	return Py_BuildValue("i", family);
}

static char pinkpy_name_socket_subcall_doc[] = "TODO";
static PyObject *
pinkpy_name_socket_subcall(PyObject *self, PyObject *args)
{
	enum pink_socket_subcall subcall;
	const char *name;

	if (!PyArg_ParseTuple(args, "I", &subcall))
		return NULL;

	PINK_CHECK_NULL((name = pink_name_socket_subcall(subcall)), "Invalid subcall");
	return Py_BuildValue("s", name);
}

static char pinkpy_lookup_socket_subcall_doc[] = "TODO";
static PyObject *
pinkpy_lookup_socket_subcall(PyObject *self, PyObject *args)
{
	enum pink_socket_subcall subcall;
	const char *name;

	if (!PyArg_ParseTuple(args, "s", &name))
		return NULL;

	PINK_CHECK_LOOKUP((subcall = pink_lookup_socket_subcall(name)), "No matching socket_subcall");
	return Py_BuildValue("I", subcall);
}

static char pinkpy_name_errno_doc[] = "TODO";
static PyObject *
pinkpy_name_errno(PyObject *self, PyObject *args)
{
	short abi;
	int err_no;
	const char *name;

	abi = PINK_ABI_DEFAULT;
	if (!PyArg_ParseTuple(args, "i|h", &err_no, &abi))
		return NULL;

	PINK_CHECK_NULL((name = pink_name_errno(err_no, abi)), "Invalid err_no");
	return Py_BuildValue("s", name);
}

static char pinkpy_lookup_errno_doc[] = "TODO";
static PyObject *
pinkpy_lookup_errno(PyObject *self, PyObject *args)
{
	short abi;
	int err_no;
	const char *name;

	abi = PINK_ABI_DEFAULT;
	if (!PyArg_ParseTuple(args, "s|h", &name, &abi))
		return NULL;

	PINK_CHECK_LOOKUP((err_no = pink_lookup_errno(name, abi)), "No matching errno");
	return Py_BuildValue("i", err_no);
}

static char pinkpy_name_signal_doc[] = "TODO";
static PyObject *
pinkpy_name_signal(PyObject *self, PyObject *args)
{
	short abi;
	int signum;
	const char *name;

	abi = PINK_ABI_DEFAULT;
	if (!PyArg_ParseTuple(args, "i|h", &signum, &abi))
		return NULL;

	PINK_CHECK_NULL((name = pink_name_signal(signum, abi)), "Invalid signum");
	return Py_BuildValue("s", name);
}

static char pinkpy_lookup_signal_doc[] = "TODO";
static PyObject *
pinkpy_lookup_signal(PyObject *self, PyObject *args)
{
	short abi;
	int signum;
	const char *name;

	abi = PINK_ABI_DEFAULT;
	if (!PyArg_ParseTuple(args, "s|h", &name, &abi))
		return NULL;

	PINK_CHECK_LOOKUP((signum = pink_lookup_signal(name, abi)), "No matching signal");
	return Py_BuildValue("i", signum);
}

static char name_doc[] = "Pink's naming functions";
static PyMethodDef name_methods[] = {
	{"name_event", pinkpy_name_event, METH_VARARGS, pinkpy_name_event_doc},
	{"lookup_event", pinkpy_lookup_event, METH_VARARGS, pinkpy_lookup_event_doc},
	{"name_syscall", pinkpy_name_syscall, METH_VARARGS, pinkpy_name_syscall_doc},
	{"lookup_syscall", pinkpy_lookup_syscall, METH_VARARGS, pinkpy_lookup_syscall_doc},
	{"name_socket_family", pinkpy_name_socket_family, METH_VARARGS, pinkpy_name_socket_family_doc},
	{"lookup_socket_family", pinkpy_lookup_socket_family, METH_VARARGS, pinkpy_lookup_socket_family_doc},
	{"name_socket_subcall", pinkpy_name_socket_subcall, METH_VARARGS, pinkpy_name_socket_subcall_doc},
	{"lookup_socket_subcall", pinkpy_lookup_socket_subcall, METH_VARARGS, pinkpy_lookup_socket_subcall_doc},
	{"name_errno", pinkpy_name_errno, METH_VARARGS, pinkpy_name_errno_doc},
	{"lookup_errno", pinkpy_lookup_errno, METH_VARARGS, pinkpy_lookup_errno_doc},
	{"name_signal", pinkpy_name_signal, METH_VARARGS, pinkpy_name_signal_doc},
	{"lookup_signal", pinkpy_lookup_signal, METH_VARARGS, pinkpy_lookup_signal_doc},
	{NULL, NULL, 0, NULL}
};

#if PY_MAJOR_VERSION > 2
static struct PyModuleDef name_module = {
	PyModuleDef_HEAD_INIT,
	"name",
	name_doc,
	-1,
	name_methods,
	NULL,
	NULL,
	NULL,
	NULL
};

PyMODINIT_FUNC
PyInit_name(void)
{
	PyObject *mod, *dict;

	mod = PyModule_Create(&name_module);
	if (!mod)
		return NULL;

	return mod;
}
#else
PyMODINIT_FUNC
initname(void)
{
	PyObject *mod, *dict;

	mod = Py_InitModule3("name", name_methods, name_doc);
	if (!mod)
		return;
}
#endif /* PY_MAJOR_VERSION > 2 */
