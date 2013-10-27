/*
 * Copyright (c) 2010, 2011, 2013 Ali Polatel <alip@exherbo.org>
 * All rights reserved.
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
PyInit_event(void);
#else
initevent(void);
#endif /* PY_MAJOR_VERSION > 2 */

static PyObject *EventError;

static char pinkpy_event_decide_doc[] = ""
	"Return the last event made by child.\n"
	"\n"
	"@param status: The status argument, received from os.waitpid() call.\n"
	"@return: One of the C{pinktrace.event.EVENT_*} constants\n"
	"@rtype: int";
static PyObject *
pinkpy_event_decide(PyObject *self, PyObject *args)
{
	int status;
	enum pink_event event;

	if (!PyArg_ParseTuple(args, "i", &status))
		return NULL;

	event = pink_event_decide(status); /* pure */
	return Py_BuildValue("I", event); /* unsigned int w/o overflow checking */
}

static char event_doc[] = "Pink's event handling";
static PyMethodDef event_methods[] = {
	{"decide", pinkpy_event_decide, METH_VARARGS, pinkpy_event_decide_doc},
	{NULL, NULL, 0, NULL}
};

static void
event_init(PyObject *mod)
{
	PyModule_AddIntConstant(mod, "EVENT_NONE", PINK_EVENT_NONE);
	PyModule_AddIntConstant(mod, "EVENT_FORK", PINK_EVENT_FORK);
	PyModule_AddIntConstant(mod, "EVENT_VFORK", PINK_EVENT_VFORK);
	PyModule_AddIntConstant(mod, "EVENT_CLONE", PINK_EVENT_CLONE);
	PyModule_AddIntConstant(mod, "EVENT_EXEC", PINK_EVENT_EXEC);
	PyModule_AddIntConstant(mod, "EVENT_VFORK_DONE", PINK_EVENT_VFORK_DONE);
	PyModule_AddIntConstant(mod, "EVENT_EXIT", PINK_EVENT_EXIT);
	PyModule_AddIntConstant(mod, "EVENT_SECCOMP", PINK_EVENT_SECCOMP);
	PyModule_AddIntConstant(mod, "EVENT_STOP", PINK_EVENT_STOP);
}

#if PY_MAJOR_VERSION > 2
static struct PyModuleDef event_module = {
	PyModuleDef_HEAD_INIT,
	"event",
	event_doc,
	-1,
	event_methods,
	NULL,
	NULL,
	NULL,
	NULL
};

PyMODINIT_FUNC
PyInit_event(void)
{
	PyObject *mod, *dict;

	mod = PyModule_Create(&event_module);
	if (!mod)
		return NULL;

	event_init(mod);

	return mod;
}
#else
PyMODINIT_FUNC
initevent(void)
{
	PyObject *mod, *dict;

	mod = Py_InitModule3("event", event_methods, event_doc);
	if (!mod)
		return;

	event_init(mod);
}
#endif /* PY_MAJOR_VERSION > 2 */
