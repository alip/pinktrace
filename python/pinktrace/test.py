#!/usr/bin/env python
# coding: utf-8
# Copyright 2010, 2013 Ali Polatel <alip@exherbo.org>
# Distributed under the same terms as Python itself.

from __future__ import with_statement

"""\
pinktrace Python bindings (unit tests)
"""

from pinktrace import *

import os, sys
import ctypes, errno, signal
from errno import *

import traceback
import unittest

INT_MAX = 2147483647

def strerror(err_no):
    if err_no == 0:
        return "ESUCCESS"
    return errno.errorcode.get(err_no, "E???")

def formatString(string, role = "P", pid = None, isError = False):
    """
    Format a string for printing.
    Role is 'P' for parent, 'C' for child.
    """

    if pid is None:
        pid = os.getpid()

    formatted_string = "\n"
    for line in string.splitlines():
        info = "%s[pid=%d]" % (role, pid)

        if role == "P":
            suff = ">>> "
        if role == "C":
            suff = "<<< "

        if isError:
            suff = suff[0] + "!" + suff[1:]

        formatted_string += info + suff + line + "\n"

    return formatted_string

def writeFormatString(dest_file, string, role = "P", pid = None, isError = None):
    formatted_string = formatString(string, role, pid, isError)
    dest_file.write(formatted_string)
    dest_file.flush()

def printError(string, role = "P", pid = None):
    writeFormatString(sys.stderr, string, role, pid, isError = True)

def printMessage(string, role = "P", pid = None):
    writeFormatString(sys.stderr, string, role, pid, isError = False)

class Libc:
    def __init__(self, libc = None):
        if libc is None:
            libc = ctypes.CDLL("libc.so.6", use_errno = True)
        self.libc = libc

    def kill(self, pid, sig):
        """
        libc kill() function

        python's os.kill() raises ValueError when signal number is 0
        which makes it impossible to check the existence of a PID with it.
        """
        ctypes.set_errno(0)

        r = self.libc.kill(pid, sig)

        e = ctypes.get_errno()
        if r == -1 and e != 0:
            return -e
        return r

LIBC = Libc()

class PinkTestCase(unittest.TestCase):
    longMessage = True

    def assertErrno(self, err_no, err_no_wanted, msg = None):
        if isinstance(err_no, str):
            err_no = getattr(errno, err_no, None)
            if err_no is None:
                msg = self._formatMessage(msg, "errno '%s' is invalid" % err_no)
                raise self.failureException(msg)
        if not isinstance(err_no, int):
            msg = self._formatMessage(msg, "errno %r is invalid" % err_no)
            raise self.failureException(msg)

        if isinstance(err_no_wanted, str):
            err_no = getattr(errno, err_no_wanted, None)
            if err_no_wanted is None:
                msg = self._formatMessage(msg, "errno '%s' is invalid" % err_no_wanted)
                raise self.failureException(msg)
        if not isinstance(err_no_wanted, int):
            msg = self._formatMessage(msg, "errno %r is invalid" % err_no_wanted)
            raise self.failureException(msg)

        if err_no == err_no_wanted:
            return True

        msg = self._formatMessage(msg, "errno is %d %s (expected %d %s)" % (err_no,
                                                                            strerror(err_no),
                                                                            err_no_wanted,
                                                                            strerror(err_no_wanted)))
        raise self.failureException(msg)

    def assertRaisesOSErrorWithErrno(self, err_no, callableObj, *args):
        with self.assertRaises(OSError) as context:
            callableObj(*args)
        self.assertErrno(context.exception.errno, err_no)

    def assertProcessIsAlive(self, pid):
        global LIBC

        negated_errno = LIBC.kill(pid, 0)
        self.assertErrno(-negated_errno, 0, "process '%d' is dead (expected alive)" % pid)

    def assertProcessIsDead(self, pid):
        global LIBC

        negated_errno = LIBC.kill(pid, 0)
        self.assertErrno(-negated_errno, ESRCH, "process '%d' kill failed" % pid)

    def forkProcess(self, callableObj, *args, **kwargs):
        try:
            pid = os.fork()
        except OSError as e:
            self.assertErrno(e.errno, 0, "fork failed")

        if pid == 0:
            r = callableObj(*args, **kwargs)
            if r is None:
                r = 0
            os._exit(r)
        else:
            self.assertProcessIsAlive(pid)
            return pid

    def killProcess(self, pid, check_alive = True, check_killed = True):
        if check_alive:
            self.assertProcessIsAlive(pid)

        negated_errno = LIBC.kill(pid, signal.SIGKILL)
        self.assertErrno(-negated_errno, 0, "process '%d' was not killed" % pid)

        # Wait the process to exit!
        wait_pid, wait_status = self.waitProcess(pid, 0)

        if check_killed:
            self.assertProcessTerminatedWith(signal.SIGKILL, wait_status, "process '%d' was not terminated" % pid)

    def waitProcess(self, pid, options):
        return os.waitpid(pid, options)

    def assertProcessExitedWith(self, exit_code, wait_status, msg = None):
        if os.WIFEXITED(wait_status) and os.WEXITSTATUS(wait_status) == exit_code:
            return True

        if not os.WIFEXITED(wait_status):
            msg = self._formatMessage(msg, "status '%#x' does not have exit bit set" % wait_status)
            raise self.failureException(msg)
        else: # if os.WEXITSTATUS(wait_status) != status:
            msg = self._formatMessage(msg, "unexpected exit code '%d' (expected: '%d', status = '%#x')" %
                    (os.WEXITSTATUS(wait_status), exit_code, wait_status))
            raise self.failureException(msg)

    def assertProcessTerminatedWith(self, sig, wait_status, msg = None):
        if os.WIFSIGNALED(wait_status) and os.WTERMSIG(wait_status) == sig:
            return True

        if not os.WIFSIGNALED(wait_status):
            msg = self._formatMessage(msg, "status '%#x' does not have signal bit set" % wait_status)
            raise self.failureException(msg)
        else: # if os.WTERMSIG(wait_status) != sig:
            msg = self._formatMessage(msg, "unexpected termimnation signal '%d' (expected: '%d', status = '%#x')" %
                    (os.WTERMSIG(wait_status), sig, wait_status))
            raise self.failureException(msg)

    def _checkPinkCall(self, retval_wanted, check_errno, callableObj, *args):
        try:
            retval = callableObj(*args)
            if retval == True:
                retval = 0
        except Exception as e:
            if check_errno is not None and getattr(e, "errno") and e.errno == check_errno:
                    return -e.errno
            errMsg = """
checkPinkCall: callable Object raised unexpected Exception {exception.__class__.__name__}
\tcallableObj: {func.__name__}
\targs: {args!r}

\tcheck_errno: {check_errno!r}
\terrno: {errno} {errid}

\tretval: None
\tretval_wanted: {retval_wanted!r}

-- Traceback:
{traceback}
-- End of Traceback
""".format(exception = e, func = callableObj, args = args,
           retval_wanted = retval_wanted,
           check_errno = check_errno,
           errno = getattr(e, "errno", -1),
           errid = strerror(getattr(e, "errno", -1)),
           traceback = traceback.format_exc())

            raise AssertionError(errMsg)

        if check_errno is None and retval == retval_wanted:
            return retval

        errMsg = """
checkPinkCall: callable Object unexpected retval '{retval!r}' (expected: '{retval_wanted!r}')
\tcallableObj: {func.__name__}
\targs: {args!r}

\tcheck_errno: {check_errno!r}
\terrno: {errno} {errid}

\tretval: {retval!r}
\tretval_wanted: {retval_wanted!r}
""".format(retval = retval, retval_wanted = retval_wanted,
           check_errno = check_errno,
           errno = -retval,
           errid = strerror(-retval),
           func = callableObj, args = args)

        raise AssertionError(errMsg)

    def checkPinkCallRetval(self, retval_wanted, callableObj, *args, **kwargs):
        return self._checkPinkCall(retval_wanted, None, callableObj, *args, **kwargs)

    def checkPinkCallErrno(self, errno_wanted, callableObj, *args, **kwargs):
        return self._checkPinkCall(0, errno_wanted, callableObj, *args, **kwargs)

class TestCase_01_Abi(PinkTestCase):
    def test_01_abi_wordsize_raises_type_error_for_invalid_arguments(self):
        self.assertRaises(TypeError, abi.wordsize, 1, 2)
        self.assertRaises(TypeError, abi.wordsize, 'pink')

    def test_02_abi_wordsize_without_argument_returns_for_default_abi(self):
        self.assertEqual(abi.wordsize(), abi.wordsize(abi.ABI_DEFAULT))

class TestCase_02_Event(PinkTestCase):
    def test_01_event_decide_raises_type_error_for_invalid_arguments(self):
        self.assertRaises(TypeError, event.decide)
        self.assertRaises(TypeError, event.decide, 1, 2)
        self.assertRaises(TypeError, event.decide, 'pink')

class TestCase_03_Name(PinkTestCase):
    def test_01_name_event_raises_type_error_for_invalid_arguments(self):
        self.assertRaises(TypeError, name.name_event)
        self.assertRaises(TypeError, name.name_event, 1, 2)
        self.assertRaises(TypeError, name.name_event, 'pink')

    def test_02_name_event_raises_value_error_for_lookup_failure(self):
        self.assertRaises(ValueError, name.name_event, INT_MAX)

    def test_03_lookup_event_raises_type_error_for_invalid_arguments(self):
        self.assertRaises(TypeError, name.lookup_event)
        self.assertRaises(TypeError, name.lookup_event, 1, 2)

    def test_04_lookup_event_raises_value_error_for_lookup_failure(self):
        self.assertRaises(ValueError, name.lookup_event, 'pink')

    def test_05_name_syscall_raises_type_error_for_invalid_arguments(self):
        self.assertRaises(TypeError, name.name_syscall)
        self.assertRaises(TypeError, name.name_syscall, 1, 2, 3)
        self.assertRaises(TypeError, name.name_syscall, 'pink')
        self.assertRaises(TypeError, name.name_syscall, 'pink', 1)
        self.assertRaises(TypeError, name.name_syscall, 1, 'pink')

    def test_06_name_syscall_raises_value_error_for_lookup_failure(self):
        self.assertRaises(ValueError, name.name_syscall, INT_MAX)
        self.assertRaises(ValueError, name.name_syscall, INT_MAX, abi.ABI_DEFAULT)

    def test_07_lookup_syscall_raises_type_error_for_invalid_arguments(self):
        self.assertRaises(TypeError, name.lookup_syscall)
        self.assertRaises(TypeError, name.lookup_syscall, 1, 2)

    def test_08_lookup_syscall_raises_value_error_for_lookup_failure(self):
        self.assertRaises(ValueError, name.lookup_syscall, 'pink')
        self.assertRaises(ValueError, name.lookup_syscall, 'pink', abi.ABI_DEFAULT)

    def test_10_name_socket_family_raises_type_error_for_invalid_arguments(self):
        self.assertRaises(TypeError, name.name_socket_family)
        self.assertRaises(TypeError, name.name_socket_family, 1, 2)
        self.assertRaises(TypeError, name.name_socket_family, 'pink')

    def test_11_name_socket_family_raises_value_error_for_lookup_failure(self):
        self.assertRaises(ValueError, name.name_socket_family, INT_MAX)

    def test_12_lookup_socket_family_raises_type_error_for_invalid_arguments(self):
        self.assertRaises(TypeError, name.lookup_socket_family)
        self.assertRaises(TypeError, name.lookup_socket_family, 1, 2)

    def test_13_lookup_socket_family_raises_value_error_for_lookup_failure(self):
        self.assertRaises(ValueError, name.lookup_socket_family, 'pink')

    def test_14_name_socket_subcall_raises_type_error_for_invalid_arguments(self):
        self.assertRaises(TypeError, name.name_socket_subcall)
        self.assertRaises(TypeError, name.name_socket_subcall, 1, 2)
        self.assertRaises(TypeError, name.name_socket_subcall, 'pink')

    def test_15_name_socket_subcall_raises_value_error_for_lookup_failure(self):
        self.assertRaises(ValueError, name.name_socket_subcall, INT_MAX)

    def test_16_lookup_socket_subcall_raises_type_error_for_invalid_arguments(self):
        self.assertRaises(TypeError, name.lookup_socket_subcall)
        self.assertRaises(TypeError, name.lookup_socket_subcall, 1, 2)

    def test_17_lookup_socket_subcall_raises_value_error_for_lookup_failure(self):
        self.assertRaises(ValueError, name.lookup_socket_subcall, 'pink')

    def test_18_name_errno_raises_type_error_for_invalid_arguments(self):
        self.assertRaises(TypeError, name.name_errno)
        self.assertRaises(TypeError, name.name_errno, 1, 2, 3)
        self.assertRaises(TypeError, name.name_errno, 'pink')
        self.assertRaises(TypeError, name.name_errno, 'pink', 1)
        self.assertRaises(TypeError, name.name_errno, 1, 'pink')

    def test_19_name_errno_raises_value_error_for_lookup_failure(self):
        self.assertRaises(ValueError, name.name_errno, INT_MAX)
        self.assertRaises(ValueError, name.name_errno, INT_MAX, abi.ABI_DEFAULT)

    def test_20_lookup_errno_raises_type_error_for_invalid_arguments(self):
        self.assertRaises(TypeError, name.lookup_errno)
        self.assertRaises(TypeError, name.lookup_errno, 1, 2)

    def test_21_lookup_errno_raises_value_error_for_lookup_failure(self):
        self.assertRaises(ValueError, name.lookup_errno, 'pink')
        self.assertRaises(ValueError, name.lookup_errno, 'pink', abi.ABI_DEFAULT)

    def test_22_name_signal_raises_type_error_for_invalid_arguments(self):
        self.assertRaises(TypeError, name.name_signal)
        self.assertRaises(TypeError, name.name_signal, 1, 2, 3)
        self.assertRaises(TypeError, name.name_signal, 'pink')
        self.assertRaises(TypeError, name.name_signal, 'pink', 1)
        self.assertRaises(TypeError, name.name_signal, 1, 'pink')

    def test_23_name_signal_raises_value_error_for_lookup_failure(self):
        self.assertRaises(ValueError, name.name_signal, INT_MAX)
        self.assertRaises(ValueError, name.name_signal, INT_MAX, abi.ABI_DEFAULT)

    def test_24_lookup_signal_raises_type_error_for_invalid_arguments(self):
        self.assertRaises(TypeError, name.lookup_signal)
        self.assertRaises(TypeError, name.lookup_signal, 1, 2)

    def test_25_lookup_signal_raises_value_error_for_lookup_failure(self):
        self.assertRaises(ValueError, name.lookup_signal, 'pink')
        self.assertRaises(ValueError, name.lookup_signal, 'pink', abi.ABI_DEFAULT)

class TestCase_04_Trace(PinkTestCase):
    @classmethod
    def setUpClass(cls):
        cls.killPID = None

    def setUp(self):
        def born_to_die_child_func():
            signal.pause()
            os._exit(1)

        self.killPID = self.forkProcess(born_to_die_child_func)

    def tearDown(self):
        if self.killPID is not None:
            self.killProcess(self.killPID)
            self.killPID = None

    def test_01_trace_me_raises_type_error_for_invalid_arguments(self):
        self.assertRaises(TypeError, trace.me, 1)

    def test_02_trace_me_raises_os_error_with_eperm_in_traced_process(self):
        def child_func():
            try:
                r = self.checkPinkCallRetval(0, trace.me)
            except AssertionError as e:
                printError(e.message)
                return 1

            try:
                r = self.checkPinkCallErrno(EPERM, trace.me)
            except AssertionError as e:
                printError(e.message)
                return 2

        pid = self.forkProcess(child_func)
        _, wait_status = self.waitProcess(pid, 0)
        self.assertProcessExitedWith(0, wait_status)

    def test_03_trace_resume_raises_type_error_for_invalid_arguments(self):
        self.assertRaises(TypeError, trace.resume)
        self.assertRaises(TypeError, trace.resume, 1, 2, 3)
        self.assertRaises(TypeError, trace.resume, 'pink')
        self.assertRaises(TypeError, trace.resume, 0, 'pink')

    def test_04_trace_resume_raises_os_error_with_esrch_for_invalid_process(self):
        self.assertRaisesOSErrorWithErrno(ESRCH, trace.resume, 0)

    def test_05_trace_resume_raises_value_error_for_invalid_signal(self):
        self.assertRaises(ValueError, trace.resume, self.killPID, -1)
        self.assertRaises(ValueError, trace.resume, self.killPID, INT_MAX)

    def test_06_trace_kill_raises_type_error_for_invalid_arguments(self):
        self.assertRaises(TypeError, trace.kill)
        self.assertRaises(TypeError, trace.kill, 'pink')
        self.assertRaises(TypeError, trace.kill, 0, 'pink')
        self.assertRaises(TypeError, trace.kill, 'pink', 0)
        self.assertRaises(TypeError, trace.kill, 'pink', 'pink')
        self.assertRaises(TypeError, trace.kill, 'pink', 'pink', 'pink')
        self.assertRaises(TypeError, trace.kill, 0, 'pink', 'pink')
        self.assertRaises(TypeError, trace.kill, 'pink', 0, 'pink')
        self.assertRaises(TypeError, trace.kill, 'pink', 'pink', 0)
        self.assertRaises(TypeError, trace.kill, 0, 0, 'pink')
        self.assertRaises(TypeError, trace.kill, 0, 'pink', 0)
        self.assertRaises(TypeError, trace.kill, 'pink', 0, 0)

    def test_07_trace_kill_raises_value_error_for_invalid_signal(self):
        self.assertRaises(ValueError, trace.kill, self.killPID, -1, -1)
        self.assertRaises(ValueError, trace.kill, self.killPID, -1, INT_MAX)

    def test_08_trace_singlestep_raises_type_error_for_invalid_arguments(self):
        self.assertRaises(TypeError, trace.singlestep)
        self.assertRaises(TypeError, trace.singlestep, 1, 2, 3)
        self.assertRaises(TypeError, trace.singlestep, 'pink')
        self.assertRaises(TypeError, trace.singlestep, 0, 'pink')

    def test_09_trace_singlestep_raises_os_error_with_esrch_for_invalid_process(self):
        self.assertRaisesOSErrorWithErrno(ESRCH, trace.singlestep, 0)

    def test_10_trace_singlestep_raises_value_error_for_invalid_signal(self):
        self.assertRaises(ValueError, trace.singlestep, self.killPID, -1)
        self.assertRaises(ValueError, trace.singlestep, self.killPID, INT_MAX)

    def test_11_trace_syscall_raises_type_error_for_invalid_arguments(self):
        self.assertRaises(TypeError, trace.syscall)
        self.assertRaises(TypeError, trace.syscall, 1, 2, 3)
        self.assertRaises(TypeError, trace.syscall, 'pink')
        self.assertRaises(TypeError, trace.syscall, 0, 'pink')

    def test_12_trace_syscall_raises_os_error_with_esrch_for_invalid_process(self):
        self.assertRaisesOSErrorWithErrno(ESRCH, trace.syscall, 0)

    def test_13_trace_syscall_raises_value_error_for_invalid_signal(self):
        self.assertRaises(ValueError, trace.syscall, self.killPID, -1)
        self.assertRaises(ValueError, trace.syscall, self.killPID, INT_MAX)

    def test_14_trace_geteventmsg_raises_type_error_for_invalid_arguments(self):
        self.assertRaises(TypeError, trace.geteventmsg)
        self.assertRaises(TypeError, trace.geteventmsg, 'pink')

    def test_15_trace_geteventmsg_raises_os_error_with_esrch_for_invalid_process(self):
        self.assertRaisesOSErrorWithErrno(ESRCH, trace.geteventmsg, 0)

    def test_16_trace_setup_raises_type_error_for_invalid_arguments(self):
        self.assertRaises(TypeError, trace.setup)
        self.assertRaises(TypeError, trace.setup, 'pink')
        self.assertRaises(TypeError, trace.setup, 0, 'pink')

    def test_17_trace_setup_raises_os_error_with_esrch_for_invalid_process(self):
        self.assertRaisesOSErrorWithErrno(ESRCH, trace.setup, 0, trace.OPTION_SYSGOOD)

    def test_18_trace_attach_raises_type_error_for_invalid_arguments(self):
        self.assertRaises(TypeError, trace.attach)
        self.assertRaises(TypeError, trace.attach, 1, 2)
        self.assertRaises(TypeError, trace.attach, 'pink')

    def test_19_trace_attach_raises_os_error_with_esrch_for_invalid_process(self):
        self.assertRaisesOSErrorWithErrno(ESRCH, trace.attach, 0)

    def test_20_trace_detach_raises_type_error_for_invalid_arguments(self):
        self.assertRaises(TypeError, trace.detach)
        self.assertRaises(TypeError, trace.detach, 1, 2, 3)
        self.assertRaises(TypeError, trace.detach, 'pink')
        self.assertRaises(TypeError, trace.detach, 0, 'pink')

    def test_21_trace_detach_raises_os_error_with_esrch_for_invalid_process(self):
        self.assertRaisesOSErrorWithErrno(ESRCH, trace.detach, 0)

    def test_22_trace_detach_raises_value_error_for_invalid_signal(self):
        self.assertRaises(ValueError, trace.detach, self.killPID, -1)
        self.assertRaises(ValueError, trace.detach, self.killPID, INT_MAX)

    def test_23_trace_seize_raises_type_error_for_invalid_arguments(self):
        self.assertRaises(TypeError, trace.seize)
        self.assertRaises(TypeError, trace.seize, 'pink')
        self.assertRaises(TypeError, trace.seize, 0, 'pink')

    def test_24_trace_seize_raises_os_error_with_einval_for_invalid_options(self):
        self.assertRaises(ValueError, trace.seize, 0, -1)
        self.assertRaises(ValueError, trace.seize, 0, INT_MAX)

    def test_25_trace_seize_raises_os_error_with_esrch_for_invalid_process(self):
        self.assertRaisesOSErrorWithErrno(ESRCH, trace.seize, 0, trace.OPTION_SYSGOOD)

    def test_26_trace_interrupt_raises_type_error_for_invalid_arguments(self):
        self.assertRaises(TypeError, trace.interrupt)
        self.assertRaises(TypeError, trace.interrupt, 1, 2)
        self.assertRaises(TypeError, trace.interrupt, 'pink')

    def test_27_trace_interrupt_raises_os_error_with_esrch_for_invalid_process(self):
        self.assertRaisesOSErrorWithErrno(ESRCH, trace.interrupt, 0)

    def test_28_trace_listen_raises_type_error_for_invalid_arguments(self):
        self.assertRaises(TypeError, trace.listen)
        self.assertRaises(TypeError, trace.listen, 1, 2)
        self.assertRaises(TypeError, trace.listen, 'pink')

    def test_29_trace_listen_raises_os_error_with_esrch_for_invalid_process(self):
        self.assertRaisesOSErrorWithErrno(ESRCH, trace.listen, 0)

def main():
    import sys
    sys.exit(unittest.main(verbosity = 2))

if __name__ == '__main__':
    main()
