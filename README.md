# PinkTrace: Pink's Tracing Library

[C99](https://en.wikipedia.org/wiki/C99) library which is a lightweight wrapper
for [ptrace](https://en.wikipedia.org/wiki/Ptrace) hiding away architectural
details.

Please read the file [INSTALL-git](INSTALL-git.md) for installation instructions.

## Download

Release tarballs are located at https://dev.exherbo.org/distfiles/pinktrace/

The main [GIT](https://en.wikipedia.org/wiki/Git) is located at
[exherbo.org](https://exherbo.org/). To clone, use one of:

- https://git.exherbo.org/git/pinktrace-1.git
- [git://git.exherbo.org/pinktrace-1.git](git://git.exherbo.org/pinktrace-1.git)

There's a [GitHub](https://en.wikipedia.org/wiki/GitHub) mirror at
https://github.com/sydbox/pinktrace. To clone, use one of:

- https://github.com/sydbox/pinktrace.git
- [git@github.com:sydbox/pinktrace.git](ssh://git@github.com:sydbox/pinktrace.git)

## Description

Using **PinkTrace**, it is easily possible to read/change system calls or
system call arguments. This means you can deny a specific system call from
executing altogether or only if a system call argument matches a certain value.
You can also change the system call to execute a different system call or
change the arguments of the system call so it takes a different integer or a
different string as argument.

## Reference

An extensive API reference is available at https://dev.exherbo.org/~alip/pinktrace/api/c/

## Background

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

## Operating Systems

**PinkTrace** runs on [Linux](https://en.wikipedia.org/wiki/Linux) only. Version
5.0.0 or later is recommended. Enabling the `CONFIG_CROSS_MEMORY_ATTACH` kernel
option allows **PinkTrace** to make use of the
[process_vm_readv](http://man7.org/linux/man-pages/man2/process_vm_readv.2.html), and
[process_vm_writev](http://man7.org/linux/man-pages/man2/process_vm_readv.2.html) system
calls to transfer data to and from tracee's address space which is much faster and
more reliable than using [ptrace](http://man7.org/linux/man-pages/man2/ptrace.2.html) to do the same.

**Note**: Make sure you run **PinkTrace** on a system with `ptrace()` enabled.
One of the most common ways to restrict `ptrace()` usage is
[Yama](https://www.kernel.org/doc/Documentation/security/Yama.txt). Use the
command `sysctl kernel.yama.ptrace_scope` to check if `ptrace()` usage is
restricted. The sysctl settings (writable only with CAP\_SYS\_PTRACE) are:

```
0 - classic ptrace permissions: a process can PTRACE_ATTACH to any other
    process running under the same uid, as long as it is dumpable (i.e.
    did not transition uids, start privileged, or have called
    prctl(PR_SET_DUMPABLE...) already). Similarly, PTRACE_TRACEME is
    unchanged.

1 - restricted ptrace: a process must have a predefined relationship
    with the inferior it wants to call PTRACE_ATTACH on. By default,
    this relationship is that of only its descendants when the above
    classic criteria is also met. To change the relationship, an
    inferior can call prctl(PR_SET_PTRACER, debugger, ...) to declare
    an allowed debugger PID to call PTRACE_ATTACH on the inferior.
    Using PTRACE_TRACEME is unchanged.

2 - admin-only attach: only processes with CAP_SYS_PTRACE may use ptrace
    with PTRACE_ATTACH, or through children calling PTRACE_TRACEME.

3 - no attach: no processes may use ptrace with PTRACE_ATTACH nor via
    PTRACE_TRACEME. Once set, this sysctl value cannot be changed.
```

## Architectures

**PinkTrace** is supported on the following architectures:

- [X86](https://en.wikipedia.org/wiki/X86)
- [X86_64](https://en.wikipedia.org/wiki/X86-64)
- [AArch64](https://en.wikipedia.org/wiki/AArch64) or [Arm64](https://en.wikipedia.org/wiki/AArch64)
- [ARM](https://en.wikipedia.org/wiki/ARM_architecture)
- [PowerPC](https://en.wikipedia.org/wiki/PowerPC)
- [Ppc64](https://en.wikipedia.org/wiki/Ppc64)
- [IA-64](https://en.wikipedia.org/wiki/IA-64)

## License

**PinkTrace** is released under the terms of [the GNU Lesser General Public
License version 2.1 or later](LGPL-2.1-or-later); see the file
[COPYING](COPYING) for details.
**PinkTrace** Python bindings is released under the terms of [the CNRI Python
Open Source GPL Compatible License
Agreement](https://spdx.org/licenses/CNRI-Python-GPL-Compatible.html); see the
file [python/COPYING](python/COPYING) for details.

## Bugs

```
Hey you, out there beyond the wall,
Breaking bottles in the hall,
Can you help me?
```

You may use [the PinkTrace GitHub page](https://github.com/sydbox/pinktrace) to
submit issues or pull requests.

You may also contact the primary author **Alï Polatel** directly for any questions.
**Mail is preferred. Attaching poems encourages consideration tremendously.**

- **Mail:** [alip@exherbo.org](mailto:alip@exherbo.org)
- **Twitter:** https://twitter.com/hayaliali
- **Mastodon:** [https://mastodon.online/@alip](https://mastodon.online/@alip)
- **IRC:** `alip` at [Libera](https://libera.chat/)

<!-- vim: set tw=80 ft=markdown spell spelllang=en sw=4 sts=4 et : -->
