/*
 * Copyright (c) 2010, 2011, 2013 Ali Polatel <alip@exherbo.org>
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
PyInit_abi(void);
#else
initabi(void);
#endif

static void abi_init(PyObject *mod)
{
	PyModule_AddIntConstant(mod, "ABIS_SUPPORTED", PINK_ABIS_SUPPORTED);
	PyModule_AddIntConstant(mod, "ABI_DEFAULT", PINK_ABI_DEFAULT);

#if PINK_ARCH_X86_64
	PyModule_AddIntConstant(mod, "ABI_X86_64", PINK_ABI_X86_64);
	PyModule_AddIntConstant(mod, "ABI_I386", PINK_ABI_I386);
	PyModule_AddIntConstant(mod, "ABI_X32", PINK_ABI_X32);
#endif

#if PINK_ARCH_X32
	PyModule_AddIntConstant(mod, "ABI_X32", PINK_ABI_X32);
	PyModule_AddIntConstant(mod, "ABI_I386", PINK_ABI_I386);
#endif

#if PINK_ARCH_POWERPC64
	PyModule_AddIntConstant(mod, "ABI_PPC64", PINK_ABI_PPC64);
	PyModule_AddIntConstant(mod, "ABI_PPC32", PINK_ABI_PPC32);
#endif
}

static char pinkpy_abi_wordsize_doc[] = "TODO";
static PyObject *pinkpy_abi_wordsize(PyObject *self, PyObject *args)
{
	short abi;
	size_t wsize;

	abi = PINK_ABI_DEFAULT;
	if (!PyArg_ParseTuple(args, "|h", &abi))
		return NULL;

	wsize = pink_abi_wordsize(abi); /* pure */
	return PyLong_FromSize_t(wsize);
}

static char abi_doc[] = "Pink's supported system call ABIs";
static PyMethodDef abi_methods[] = {
	{"wordsize", pinkpy_abi_wordsize, METH_VARARGS, pinkpy_abi_wordsize_doc},
	{NULL, NULL, 0, NULL}
};

#if PY_MAJOR_VERSION > 2
static struct PyModuleDef abi_module = {
	PyModuleDef_HEAD_INIT,
	"abi",
	abi_doc,
	-1,
	abi_methods,
	NULL,
	NULL,
	NULL,
	NULL
};

PyMODINIT_FUNC
PyInit_abi(void)
{
	PyObject *mod;

	mod = PyModule_Create(&abi_module);
	if (!mod)
		return NULL;

	abi_init(mod);

	return mod;
}
#else
PyMODINIT_FUNC
initabi(void)
{
	PyObject *mod;

	mod = Py_InitModule3("abi", abi_methods, abi_doc);
	if (!mod)
		return;

	abi_init(mod);
}
#endif /* PY_MAJOR_VERSION > 2 */
