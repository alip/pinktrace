/*
 * Copyright (c) 2010, 2011, 2012, 2013 Ali Polatel <alip@exherbo.org>
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
PyInit_regset(void);
#else
initregset(void);
#endif /* PY_MAJOR_VERSION */

static char ProcessRegistrySet_doc[] = "This class represents a registry set of a traced process.";
typedef struct {
	PyObject_HEAD
	struct pink_regset *regset;
} ProcessRegistrySet;

static PyObject *
ProcessRegistrySet_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	int r;
	ProcessRegistrySet *self;

	self = (ProcessRegistrySet *)type->tp_alloc(type, 0);
	if (self == NULL)
		return NULL;

	if ((r = pink_regset_alloc(&self->regset)) < 0) {
		errno = -r;
		return PyErr_NoMemory();
	}

	return (PyObject *)self;
}

static void
ProcessRegistrySet_dealloc(PyObject *obj)
{
	ProcessRegistrySet *self = (ProcessRegistrySet *)obj;

	pink_regset_free(self->regset);
	obj->ob_type->tp_free(obj);
}

static char pinkpy_regset_fill_doc[] = "TODO";
static PyObject *
pinkpy_regset_fill(PyObject *obj, PyObject *args)
{
	int r;
	pid_t pid;
	ProcessRegistrySet *self = (ProcessRegistrySet *)obj;

	if (!PyArg_ParseTuple(args, PARSE_PID, &pid))
		return NULL;

	PINK_CALL_TRUE(pink_regset_fill(pid, self->regset));
}

static PyMethodDef ProcessRegistrySet_methods[] = {
	{"fill", pinkpy_regset_fill, METH_VARARGS, pinkpy_regset_fill_doc},
	{ NULL, NULL, 0, NULL } /* Sentinel */
};

static PyTypeObject ProcessRegistrySet_type = {
#if PY_MAJOR_VERSION > 2
	PyVarObject_HEAD_INIT(NULL, 0)
#else
	PyObject_HEAD_INIT(NULL)
	0,						/* ob_size */
#endif /* PY_MAJOR_VERSION > 2 */
	"regset.ProcessRegistrySet",			/* tp_name */
	sizeof(ProcessRegistrySet),			/* tp_basicsize */
	0,						/* tp_itemsize */
	(destructor)ProcessRegistrySet_dealloc,		/* tp_dealloc */
	0,						/* tp_print */
	0,						/* tp_getattr */
	0,						/* tp_setattr */
	0,						/* tp_compare */
	0,						/* tp_repr */
	0,						/* tp_as_number*/
	0,						/* tp_as_sequence*/
	0,						/* tp_as_mapping*/
	0,						/* tp_hash */
	0,						/* tp_call*/
	0,						/* tp_str*/
	0,						/* tp_getattro*/
	0,						/* tp_setattro*/
	0,						/* tp_as_buffer*/
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/* tp_flags*/
	ProcessRegistrySet_doc,				/* tp_doc */
	0,						/* tp_traverse */
	0,						/* tp_clear */
	0,						/* tp_richcompare */
	0,						/* tp_weaklistoffset */
	0,						/* tp_iter */
	0,						/* tp_iternext */
	ProcessRegistrySet_methods,			/* tp_methods */
	0,						/* tp_members */
	0,						/* tp_getset */
	0,						/* tp_base */
	0,						/* tp_dict */
	0,						/* tp_descr_get */
	0,						/* tp_descr_set */
	0,						/* tp_dictoffset */
	0,						/* tp_init */
	0,						/* tp_alloc */
	ProcessRegistrySet_new,				/* tp_new */
};

static void
regset_init(PyObject *mod)
{
	Py_INCREF(&ProcessRegistrySet_type);
	PyModule_AddObject(mod, "ProcessRegistrySet", (PyObject *)&ProcessRegistrySet_type);
}

static char regset_doc[] = "Pink's process registry set";
static PyMethodDef regset_methods[] = {
	{NULL, NULL, 0, NULL}
};

#if PY_MAJOR_VERSION > 2
static struct PyModuleDef regset_module = {
	PyModuleDef_HEAD_INIT,
	"regset",
	regset_doc,
	-1,
	regset_methods,
	NULL,
	NULL,
	NULL,
	NULL
};

PyMODINIT_FUNC
PyInit_regset(void)
{
	PyObject *mod;

	if (PyType_Ready(&ProcessRegistrySet_type) < 0)
		return NULL;

	mod = PyModule_Create(&regset_module);
	if (!mod)
		return NULL;

	regset_init(mod);

	return mod;
}
#else
PyMODINIT_FUNC
initregset(void)
{
	PyObject *mod;

	if (PyType_Ready(&ProcessRegistrySet_type) < 0)
		return;

	mod = Py_InitModule3("regset", regset_methods, regset_doc);

	regset_init(mod);
}
#endif /* PY_MAJOR_VERSION > 2 */
