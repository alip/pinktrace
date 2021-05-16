/*
 * Copyright (c) 2010, 2011, 2013, 2021 Ali Polatel <alip@exherbo.org>
 * SPDX-License-Identifier: CNRI-Python-GPL-Compatible
 */

#include "py-pink.h"

PyMODINIT_FUNC
#if PY_MAJOR_VERSION > 2
PyInit_system(void);
#else
initsystem(void);
#endif

static char system_doc[] = "Pink's system specific definitions";
static PyMethodDef system_methods[] = {
	{NULL, NULL, 0, NULL}
};

static void system_init(PyObject *mod)
{
	PyModule_AddIntConstant(mod, "ARCH_I386", PINK_ARCH_I386);
	PyModule_AddIntConstant(mod, "ARCH_X86_64", PINK_ARCH_X86_64);
	PyModule_AddIntConstant(mod, "ARCH_X32", PINK_ARCH_X32);
	PyModule_AddIntConstant(mod, "ARCH_POWERPC", PINK_ARCH_POWERPC);
	PyModule_AddIntConstant(mod, "ARCH_POWERPC64", PINK_ARCH_POWERPC64);
	PyModule_AddIntConstant(mod, "ARCH_IA64", PINK_ARCH_IA64);
	PyModule_AddIntConstant(mod, "ARCH_ARM", PINK_ARCH_ARM);

	PyModule_AddIntConstant(mod, "MAX_ARGS", PINK_MAX_ARGS);
	PyModule_AddIntConstant(mod, "SYSCALL_INVALID", PINK_SYSCALL_INVALID);

	PyModule_AddIntConstant(mod, "HAVE_IPV6", PINK_HAVE_IPV6);
	PyModule_AddIntConstant(mod, "HAVE_NETLINK", PINK_HAVE_NETLINK);

	PyModule_AddIntConstant(mod, "HAVE_OPTION_SYSGOOD", PINK_HAVE_OPTION_SYSGOOD);
	PyModule_AddIntConstant(mod, "HAVE_OPTION_FORK", PINK_HAVE_OPTION_FORK);
	PyModule_AddIntConstant(mod, "HAVE_OPTION_VFORK", PINK_HAVE_OPTION_VFORK);
	PyModule_AddIntConstant(mod, "HAVE_OPTION_CLONE", PINK_HAVE_OPTION_CLONE);
	PyModule_AddIntConstant(mod, "HAVE_OPTION_EXEC", PINK_HAVE_OPTION_EXEC);
	PyModule_AddIntConstant(mod, "HAVE_OPTION_VFORK_DONE", PINK_HAVE_OPTION_VFORK_DONE);
	PyModule_AddIntConstant(mod, "HAVE_OPTION_EXIT", PINK_HAVE_OPTION_EXIT);
	PyModule_AddIntConstant(mod, "HAVE_OPTION_SECCOMP", PINK_HAVE_OPTION_SECCOMP);
	PyModule_AddIntConstant(mod, "HAVE_OPTION_SECCOMP", PINK_HAVE_OPTION_EXITKILL);

	PyModule_AddIntConstant(mod, "HAVE_SETUP", PINK_HAVE_SETUP);
	PyModule_AddIntConstant(mod, "HAVE_GETEVENTMSG", PINK_HAVE_GETEVENTMSG);
	PyModule_AddIntConstant(mod, "HAVE_GETSIGINFO", PINK_HAVE_GETSIGINFO);
	PyModule_AddIntConstant(mod, "HAVE_GETREGSET", PINK_HAVE_GETREGSET);
	PyModule_AddIntConstant(mod, "HAVE_SETREGSET", PINK_HAVE_SETREGSET);
	PyModule_AddIntConstant(mod, "HAVE_SYSEMU", PINK_HAVE_SYSEMU);
	PyModule_AddIntConstant(mod, "HAVE_SYSEMU_SINGLESTEP", PINK_HAVE_SYSEMU_SINGLESTEP);
	PyModule_AddIntConstant(mod, "HAVE_SEIZE", PINK_HAVE_SEIZE);
	PyModule_AddIntConstant(mod, "HAVE_INTERRUPT", PINK_HAVE_INTERRUPT);
	PyModule_AddIntConstant(mod, "HAVE_LISTEN", PINK_HAVE_LISTEN);

	PyModule_AddIntConstant(mod, "HAVE_EVENT_FORK", PINK_HAVE_EVENT_FORK);
	PyModule_AddIntConstant(mod, "HAVE_EVENT_VFORK", PINK_HAVE_EVENT_VFORK);
	PyModule_AddIntConstant(mod, "HAVE_EVENT_CLONE", PINK_HAVE_EVENT_CLONE);
	PyModule_AddIntConstant(mod, "HAVE_EVENT_EXEC", PINK_HAVE_EVENT_EXEC);
	PyModule_AddIntConstant(mod, "HAVE_EVENT_VFORK_DONE", PINK_HAVE_EVENT_VFORK_DONE);
	PyModule_AddIntConstant(mod, "HAVE_EVENT_EXIT", PINK_HAVE_EVENT_EXIT);
	PyModule_AddIntConstant(mod, "HAVE_EVENT_SECCOMP", PINK_HAVE_EVENT_SECCOMP);
	PyModule_AddIntConstant(mod, "HAVE_EVENT_STOP", PINK_HAVE_EVENT_STOP);

	PyModule_AddIntConstant(mod, "HAVE_TKILL", PINK_HAVE_TKILL);
	PyModule_AddIntConstant(mod, "HAVE_TGKILL", PINK_HAVE_TGKILL);

	PyModule_AddIntConstant(mod, "HAVE_PROCESS_VM_READV", PINK_HAVE_PROCESS_VM_READV);
	PyModule_AddIntConstant(mod, "HAVE_PROCESS_VM_WRITEV", PINK_HAVE_PROCESS_VM_WRITEV);
}

#if PY_MAJOR_VERSION > 2
static struct PyModuleDef system_module = {
	PyModuleDef_HEAD_INIT,
	"system",
	system_doc,
	-1,
	system_methods,
	NULL,
	NULL,
	NULL,
	NULL
};

PyMODINIT_FUNC
PyInit_system(void)
{
	PyObject *mod;

	mod = PyModule_Create(&system_module);
	if (!mod)
		return NULL;

	system_init(mod);

	return mod;
}
#else
PyMODINIT_FUNC
initsystem(void)
{
	PyObject *mod;

	mod = Py_InitModule3("system", system_methods, system_doc);
	if (!mod)
		return;

	system_init(mod);
}
#endif /* PY_MAJOR_VERSION > 2 */
