---
title: "Background"
date: 2021-06-03T17:50:05+02:00
anchor: "background"
---

The `ptrace()` system call provides a means by which one process (the "tracer")
may observe and control the execution of another process (the "tracee"), and
examine and change the tracee's memory and registers. It is primarily used to
implement breakpoint debugging and system call tracing.

- See [ptrace manual page](http://man7.org/linux/man-pages/man2/ptrace.2.html) on more
  information regarding ptrace.
- See [strace](https://strace.io/) which is a diagnostic, debugging and
  instructional userspace utility for Linux which also uses `ptrace()`.
- See [gdb](https://www.gnu.org/s/gdb/) for the GNU Debugger which allows you
  to see what is going on *inside* another program while it executes using `ptrace()`.
