/*
 * Copyright (c) 2010, 2012, 2013 Ali Polatel <alip@exherbo.org>
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
PyInit_trace(void);
#else
inittrace(void);
#endif /* PY_MAJOR_VERSION */

static char pinkpy_trace_me_doc[] = ""
	"Indicates that this process is to be traced by its parent.\n"
	"Any signal (except B{SIGKILL}) delivered to this process will cause it to stop\n"
	"and its parent to be notified via I{wait(2)}. Also, all subsequent calls to\n"
	"I{execve(2)} by this process will cause a B{SIGTRAP} to be sent to it, giving the\n"
	"parent a chance to gain control before the new program begins execution.\n"
	"\n"
	"@raise OSError: Raised when the underlying I{ptrace(2)} call fails."
	"@return: True";
static PyObject *pinkpy_trace_me(PyObject *self, PyObject *args)
{
	PINK_CALL_TRUE(pink_trace_me());
}

static char pinkpy_trace_resume_doc[] = ""
	"Restarts the stopped child process.\n"
	"\n"
	"@param pid: Process ID\n"
	"@param sig: If this is non-zero and not B{SIGSTOP}, it is interpreted as the\n"
	"signal to be delivered to the child; otherwise, no signal is delivered.\n"
	"Thus, for example, the parent can control whether a signal sent to the child\n"
	"is delivered or not. (Optional, defaults to C{0})\n"
	"@raise ValueError: Raised when the signal number is invalid.\n"
	"@raise OSError: Raised when the underlying I{ptrace(2)} call fails.\n"
	"@return: True";
static PyObject *pinkpy_trace_resume(PyObject *self, PyObject *args)
{
	int sig;
	pid_t pid;

	sig = 0;
	if (!PyArg_ParseTuple(args, PARSE_PID"|i", &pid, &sig))
		return NULL;
	PINK_CHECK_SIGNAL(sig);

	PINK_CALL_TRUE(pink_trace_resume(pid, sig));
}

static char pinkpy_trace_kill_doc[] = ""
	"Send signal to the tracee\n"
	"\n"
	"@note:\n"
	" - If E{tgkill(2)} system call is available: E{tgkill(tgid, tid, sig);}\n"
	" - Otherwise if E{tkill(2)} system call is available: E{tkill(tid, sig);}\n"
	" - And otherwise: E{kill(tid, sig);}\n"
	"is called. For tgid <= 0 E{tgkill(2)} is skipped.\n"
	"@see pinktrace.system.HAVE_TKILL\n"
	"@see pinktrace.system.HAVE_TGKILL\n"
	"@param tid: Thread ID\n"
	"@param tgid: Thread group ID (Optional, defaults to C{-1}\n"
	"@param sig: Signal (Optional, defaults to C{signal.SIGKILL})\n"
	"@raise ValueError: Raised when the signal number is invalid.\n"
	"@raise OSError: Raised when the underlying I{ptrace(2)} call fails.\n"
	"@return: True";
static PyObject *pinkpy_trace_kill(PyObject *self, PyObject *args)
{
	pid_t tid;
	pid_t tgid;
	int sig;

	tgid = -1;
	sig = SIGKILL;
	if (!PyArg_ParseTuple(args, PARSE_PID"|"PARSE_PID"i", &tid, &tgid, &sig))
		return NULL;
	PINK_CHECK_SIGNAL(sig);

	PINK_CALL_TRUE(pink_trace_kill(tid, tgid, sig));
}

static char pinkpy_trace_singlestep_doc[] = ""
	"Restarts the stopped child process and arranges it to be stopped after\n"
	"execution of a single instruction.\n"
	"\n"
	"@param pid: Process ID\n"
	"@param sig: Treated the same as the signal argument of C{pinktrace.trace.resume()}\n"
	"@raise ValueError: Raised when the signal number is invalid.\n"
	"@raise OSError: Raised when the underlying I{ptrace(2)} call fails.\n"
	"@see: pinktrace.trace.resume";
static PyObject *
pinkpy_trace_singlestep(PyObject *self, PyObject *args)
{
	pid_t pid;
	int sig;

	sig = 0;
	if (!PyArg_ParseTuple(args, PARSE_PID"|i", &pid, &sig))
		return NULL;
	PINK_CHECK_SIGNAL(sig);

	PINK_CALL_TRUE(pink_trace_singlestep(pid, sig));
}

static char pinkpy_trace_syscall_doc[] = ""
	"Restarts the stopped child process and arranges it to be stopped after the\n"
	"entry or exit of the next system call.\n"
	"\n"
	"@param pid: Process ID\n"
	"@param sig: Treated the same as the signal argument of C{pinktrace.trace.resume()}\n"
	"@raise ValueError: Raised when the signal number is invalid.\n"
	"@raise OSError: Raised when the underlying I{ptrace(2)} call fails.\n"
	"@see: pinktrace.trace.resume";
static PyObject *
pinkpy_trace_syscall(PyObject *self, PyObject *args)
{
	pid_t pid;
	int sig;

	sig = 0;
	if (!PyArg_ParseTuple(args, PARSE_PID"|i", &pid, &sig))
		return NULL;
	PINK_CHECK_SIGNAL(sig);

	PINK_CALL_TRUE(pink_trace_syscall(pid, sig));
}

static char pinkpy_trace_geteventmsg_doc[] = ""
	"Retrieve a message (as an I{unsigned long}) about the trace event that just\n"
	"happened, placing it in the location given by the second argument. For B{EXIT}\n"
	"event this is the child's exit status. For B{FORK}, B{VFORK}, B{CLONE} and B{VFORK_DONE}\n"
	"events this is the process ID of the new process. For B{SECCOMP} event, this is the\n"
	"B{SECCOMP_RET_DATA} portion of the BPF program return value.\n"
	"\n"
	"@note This function is supported on Linux-2.5.46 and newer.\n"
	"@see pinktrace.system.HAVE_GETEVENTMSG\n"
	"@param pid: Process ID\n"
	"@raise OSError: Raised when the underlying I{ptrace(2)} call fails.\n"
	"@raise SystemError: Raised if the underlying I{ptrace(2)} call is not supported by the host system\n"
	"@return: The event message"
	"@rtype: long\n";
static PyObject *
pinkpy_trace_geteventmsg(PyObject *self, PyObject *args)
{
	pid_t pid;
	unsigned long data;

	if (!PyArg_ParseTuple(args, PARSE_PID, &pid))
		return NULL;

	PINK_CALL(pink_trace_geteventmsg(pid, &data));
	return PyLong_FromUnsignedLong(data);
}

static char pinkpy_trace_setup_doc[] = ""
	"Sets the tracing options.\n"
	"\n"
	"@note This function is supported on Linux-2.4.6 and newer.\n"
	"@see pinktrace.system.HAVE_SETUP\n"
	"@param pid: Process ID\n"
	"@param options: Bitwise OR'ed C{pinktrace.trace.OPTION_*} flags\n"
	"@raise OSError: Raised when the underlying I{ptrace(2)} call fails.\n"
	"@raise SystemError: Raised if the underlying I{ptrace(2)} call is not supported by the host system\n"
	"@return True\n";
static PyObject *
pinkpy_trace_setup(PyObject *self, PyObject *args)
{
	pid_t pid;
	int options;

	if (!PyArg_ParseTuple(args, PARSE_PID"i", &pid, &options))
		return NULL;

	PINK_CALL_TRUE(pink_trace_setup(pid, options));
}

static char pinkpy_trace_sysemu_doc[] = ""
	"Restarts the stopped child process and arranges it to be stopped after\n"
	"the entry of the next system call which will *not* be executed.\n"
	"\n"
	"@note This function is supported on Linux-2.6.14 and newer.\n"
	"@see pinktrace.system.HAVE_SYSEMU\n"
	"@see: pinktrace.trace.resume"
	"@param pid: Process ID\n"
	"@param sig: Treated the same as the signal argument of C{pinktrace.trace.resume()}\n"
	"@raise ValueError: Raised when the signal number is invalid.\n"
	"@raise OSError: Raised when the underlying I{ptrace(2)} call fails.\n"
	"@raise SystemError: Raised if the underlying I{ptrace(2)} call is not supported by the host system\n"
	"@return True\n";
static PyObject *
pinkpy_trace_sysemu(PyObject *self, PyObject *args)
{
	pid_t pid;
	int sig;

	sig = 0;
	if (!PyArg_ParseTuple(args, PARSE_PID"|i", &pid, &sig))
		return NULL;
	PINK_CHECK_SIGNAL(sig);

	PINK_CALL_TRUE(pink_trace_sysemu(pid, sig));
}

static char pinkpy_trace_sysemu_singlestep_doc[] = ""
	"Restarts the stopped child process like C{pinktrace.trace.sysemu()}\n"
	"but also singlesteps if not a system call.\n"
	"\n"
	"@note This function is supported on Linux-2.6.14 and newer.\n"
	"@see pinktrace.system.HAVE_SYSEMU\n"
	"@see: pinktrace.trace.resume"
	"@param pid: Process ID\n"
	"@param sig: Treated the same as the signal argument of C{pinktrace.trace.resume()}\n"
	"@raise ValueError: Raised when the signal number is invalid.\n"
	"@raise OSError: Raised when the underlying I{ptrace(2)} call fails.\n"
	"@raise SystemError: Raised if the underlying I{ptrace(2)} call is not supported by the host system\n"
	"@return True\n";
static PyObject *
pinkpy_trace_sysemu_singlestep(PyObject *self, PyObject *args)
{
	pid_t pid;
	int sig;

	sig = 0;
	if (!PyArg_ParseTuple(args, PARSE_PID"|i", &pid, &sig))
		return NULL;
	PINK_CHECK_SIGNAL(sig);

	PINK_CALL_TRUE(pink_trace_sysemu_singlestep(pid, sig));
}

static char pinkpy_trace_attach_doc[] = ""
	"Attaches to the process specified in pid, making it a traced \"child\" of the\n"
	"calling process; the behaviour of the child is as if it had done a\n"
	"C{pinktrace.trace.me()}. The child is sent a B{SIGSTOP}, but will not necessarily\n"
	"have stopped by the completion of this call; use I{wait(2)} to wait for the\n"
	"child to stop.\n"
	"\n"
	"@param pid: Process ID\n"
	"@raise OSError: Raised when the underlying I{ptrace(2)} call fails.\n";
static PyObject *
pinkpy_trace_attach(PyObject *self, PyObject *args)
{
	pid_t pid;

	if (!PyArg_ParseTuple(args, PARSE_PID, &pid))
		return NULL;

	PINK_CALL_TRUE(pink_trace_attach(pid));
}

static char pinkpy_trace_detach_doc[] = ""
	"Restarts the stopped child as for C{pinktrace.trace.resume()}, but first detaches\n"
	"from the process, undoing the reparenting effect of\n"
	"C{pinktrace.trace.attach()}.\n"
	"\n"
	"@param pid: Process ID\n"
	"@param sig: Treated the same as the signal argument of pinktrace.trace.resume()\n"
	"@raise ValueError: Raised when the signal number is invalid.\n"
	"@raise OSError: Raised when the underlying I{ptrace(2)} call fails.\n"
	"@see: pinktrace.trace.resume";
static PyObject *
pinkpy_trace_detach(PyObject *self, PyObject *args)
{
	pid_t pid;
	int sig;

	sig = 0;
	if (!PyArg_ParseTuple(args, PARSE_PID"|i", &pid, &sig))
		return NULL;
	PINK_CHECK_SIGNAL(sig);

	PINK_CALL_TRUE(pink_trace_detach(pid, sig));
}

static char pinkpy_trace_seize_doc[] = ""
	"Attach to the process specified in pid, without trapping it or affecting its\n"
	"signal and job control states.\n"
	"\n"
	"@note This function is supported on Linux-3.4 and newer.\n"
	"@see pinktrace.system.HAVE_SEIZE\n"
	"@see: pinktrace.trace.setup\n"
	"@param pid: Process ID\n"
	"@param options: Bitwise OR'ed C{pinktrace.trace.OPTION_*} flags\n"
	"@raise ValueError: Raised when the signal number is invalid.\n"
	"@raise OSError: Raised when the underlying I{ptrace(2)} call fails.\n"
	"@raise SystemError: Raised if the underlying I{ptrace(2)} call is not supported by the host system\n"
	"@return True\n";
static PyObject *
pinkpy_trace_seize(PyObject *self, PyObject *args)
{
	pid_t pid;
	int sig;

	sig = 0;
	if (!PyArg_ParseTuple(args, PARSE_PID"|i", &pid, &sig))
		return NULL;
	PINK_CHECK_SIGNAL(sig);

	PINK_CALL_TRUE(pink_trace_seize(pid, sig));
}

static char pinkpy_trace_interrupt_doc[] = ""
	"Trap the process without any signal or job control related side effects.\n"
	"\n"
	"@note This function is supported on Linux-3.4 and newer.\n"
	"@see pinktrace.system.HAVE_INTERRUPT\n"
	"@param pid: Process ID\n"
	"@raise OSError: Raised when the underlying I{ptrace(2)} call fails.\n"
	"@raise SystemError: Raised if the underlying I{ptrace(2)} call is not supported by the host system\n"
	"@return True\n";
static PyObject *
pinkpy_trace_interrupt(PyObject *self, PyObject *args)
{
	pid_t pid;

	if (!PyArg_ParseTuple(args, PARSE_PID, &pid))
		return NULL;

	PINK_CALL_TRUE(pink_trace_interrupt(pid));
}

static char pinkpy_trace_listen_doc[] = ""
	"Listen for ptrace events asynchronously after pink_trace_interrupt()\n"
	"\n"
	"@note This function is supported on Linux-3.4 and newer.\n"
	"@see pinktrace.system.HAVE_LISTEN\n"
	"@param pid: Process ID\n"
	"@raise OSError: Raised when the underlying I{ptrace(2)} call fails.\n"
	"@raise SystemError: Raised if the underlying I{ptrace(2)} call is not supported by the host system\n"
	"@return True\n";
static PyObject *
pinkpy_trace_listen(PyObject *self, PyObject *args)
{
	pid_t pid;

	if (!PyArg_ParseTuple(args, PARSE_PID, &pid))
		return NULL;

	PINK_CALL_TRUE(pink_trace_listen(pid));
}

static void
trace_init(PyObject *mod)
{
	PyModule_AddIntConstant(mod, "OPTION_SYSGOOD", PINK_TRACE_OPTION_SYSGOOD);
	PyModule_AddIntConstant(mod, "OPTION_FORK", PINK_TRACE_OPTION_FORK);
	PyModule_AddIntConstant(mod, "OPTION_VFORK", PINK_TRACE_OPTION_VFORK);
	PyModule_AddIntConstant(mod, "OPTION_CLONE", PINK_TRACE_OPTION_CLONE);
	PyModule_AddIntConstant(mod, "OPTION_EXEC", PINK_TRACE_OPTION_EXEC);
	PyModule_AddIntConstant(mod, "OPTION_VFORK_DONE", PINK_TRACE_OPTION_VFORK_DONE);
	PyModule_AddIntConstant(mod, "OPTION_EXIT", PINK_TRACE_OPTION_EXIT);
	PyModule_AddIntConstant(mod, "OPTION_EXITKILL", PINK_TRACE_OPTION_EXITKILL);
}

static char trace_doc[] = "Pink's low level wrappers around I{ptrace(2)} internals";
static PyMethodDef trace_methods[] = {
	{"me", pinkpy_trace_me, METH_NOARGS, pinkpy_trace_me_doc},
	{"resume", pinkpy_trace_resume, METH_VARARGS, pinkpy_trace_resume_doc},
	{"kill", pinkpy_trace_kill, METH_VARARGS, pinkpy_trace_kill_doc},
	{"singlestep", pinkpy_trace_singlestep, METH_VARARGS, pinkpy_trace_singlestep_doc},
	{"syscall", pinkpy_trace_syscall, METH_VARARGS, pinkpy_trace_syscall_doc},
	{"geteventmsg", pinkpy_trace_geteventmsg, METH_VARARGS, pinkpy_trace_geteventmsg_doc},
	{"setup", pinkpy_trace_setup, METH_VARARGS, pinkpy_trace_setup_doc},
	{"sysemu", pinkpy_trace_sysemu, METH_VARARGS, pinkpy_trace_sysemu_doc},
	{"sysemu_singlestep", pinkpy_trace_sysemu_singlestep, METH_VARARGS, pinkpy_trace_sysemu_singlestep_doc},
	{"attach", pinkpy_trace_attach, METH_VARARGS, pinkpy_trace_attach_doc},
	{"detach", pinkpy_trace_detach, METH_VARARGS, pinkpy_trace_detach_doc},
	{"seize", pinkpy_trace_seize, METH_VARARGS, pinkpy_trace_seize_doc},
	{"interrupt", pinkpy_trace_interrupt, METH_VARARGS, pinkpy_trace_interrupt_doc},
	{"listen", pinkpy_trace_listen, METH_VARARGS, pinkpy_trace_listen_doc},
	{NULL, NULL, 0, NULL},
};

#if PY_MAJOR_VERSION > 2
static struct PyModuleDef trace_module = {
	PyModuleDef_HEAD_INIT,
	"trace",
	trace_doc,
	-1,
	trace_methods,
	NULL,
	NULL,
	NULL,
	NULL
};

PyMODINIT_FUNC
PyInit_trace(void)
{
	PyObject *mod;

	mod = PyModule_Create(&trace_module);
	if (!mod)
		return NULL;

	trace_init(mod);

	return mod;
}
#else
PyMODINIT_FUNC
inittrace(void)
{
	PyObject *mod;

	mod = Py_InitModule3("trace", trace_methods, trace_doc);
	if (!mod)
		return;

	trace_init(mod);
}
#endif /* PY_MAJOR_VERSION > 2 */
