#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set sw=4 ts=4 sts=4 et tw=80 :

import os, signal, sys, unittest

sys.path.insert(0, '.')
from pinktrace import event, fork, trace

class TestEvent_01_Invalid(unittest.TestCase):

    def test_01_decide(self):
        self.assertRaises(TypeError, event.decide)
        self.assertRaises(TypeError, event.decide, 'pink')

    def test_02_decide(self):
        self.assertRaises(event.EventError, event.decide, -1)

class TestEvent_02(unittest.TestCase):

    def test_01_event_stop(self):
        pid = fork.fork()
        if (pid == 0): # child
            os.kill(os.getpid(), signal.SIGSTOP)
            os._exit(0)
        else: # parent
            trace.cont(pid)
            pid, status = os.waitpid(pid, 0)
            ev = event.decide(status)
            self.assertEqual(ev, event.EVENT_STOP)

            try: trace.kill(pid)
            except OSError: pass

    def test_02_event_syscall(self):
        pid = fork.fork()
        if (pid == 0): # child
            os.sleep(1)
            os._exit(0)
        else: # parent
            trace.syscall(pid)
            pid, status = os.waitpid(pid, 0)
            ev = event.decide(status)
            self.assertEqual(ev, event.EVENT_SYSCALL)

            try: trace.kill(pid)
            except OSError: pass

    def test_03_event_fork(self):
        pid = fork.fork(trace.OPTION_FORK)
        if (pid == 0): # child
            os.fork()
            os._exit(0)
        else: # parent
            trace.cont(pid)
            pid, status = os.waitpid(pid, 0)
            ev = event.decide(status)
            self.assertEqual(ev, event.EVENT_FORK)

            try: trace.kill(pid)
            except OSError: pass

    def test_04_event_vfork(self):
        pass

    def test_05_event_clone(self):
        pass

    def test_06_event_vfork_done(self):
        pass

    def test_07_event_exec(self):
        pid = fork.fork(trace.OPTION_EXEC)
        if (pid == 0): # child
            os.execvp('true', ())
            os._exit(0)
        else: # parent
            trace.cont(pid)
            pid, status = os.waitpid(pid, 0)
            ev = event.decide(status)
            self.assertEqual(ev, event.EVENT_EXEC)

            try: trace.kill(pid)
            except OSError: pass

    def test_08_event_exit(self):
        pid = fork.fork(trace.OPTION_EXIT)
        if (pid == 0): # child
            os._exit(13)
        else:
            trace.cont(pid)
            pid, status = os.waitpid(pid, 0)
            ev = event.decide(status)
            self.assertEqual(ev, event.EVENT_EXIT)
            msg = trace.geteventmsg(pid)
            self.assertEqual(os.WEXITSTATUS(msg), 13L)

            try: trace.kill(pid)
            except OSError: pass

    def test_09_event_genuine(self):
        pid = fork.fork()
        if (pid == 0): # child
            os.kill(os.getpid(), signal.SIGINT)
            os._exit(0)
        else: # parent
            trace.cont(pid)
            pid, status = os.waitpid(pid, 0)
            ev = event.decide(status)
            self.assertEqual(ev, event.EVENT_GENUINE)

            try: trace.kill(pid)
            except OSError: pass

    def test_10_event_exit_genuine(self):
        pid = fork.fork()
        if (pid == 0): # child
            os._exit(0)
        else: # parent
            trace.cont(pid)
            pid, status = os.waitpid(pid, 0)
            ev = event.decide(status)
            self.assertEqual(ev, event.EVENT_EXIT_GENUINE)

            try: trace.kill(pid)
            except OSError: pass

    def test_11_event_exit_signal(self):
        pid = fork.fork()
        if (pid == 0): # child
            os.kill(os.getpid(), signal.SIGKILL)
        else: # parent
            trace.cont(pid)
            pid, status = os.waitpid(pid, 0)
            ev = event.decide(status)
            self.assertEqual(ev, event.EVENT_EXIT_SIGNAL)

            try: trace.kill(pid)
            except OSError: pass

if __name__ == '__main__':
    unittest.main()