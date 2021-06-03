---
title: "YAMA"
date: 2021-06-03T17:56:18+02:00
anchor: "YAMA"
---

**Note**: Make sure you run **PinkTrace** on a system with `ptrace()` enabled.
One of the most common ways to restrict `ptrace()` usage is
[Yama](https://www.kernel.org/doc/Documentation/security/Yama.txt). Use the
command `sysctl` **kernel.yama.ptrace\_scope** to check if `ptrace()` usage is
restricted. The sysctl settings (writable only with **CAP\_SYS\_PTRACE**) are:

> 0 - classic ptrace permissions: a process can **PTRACE\_ATTACH** to any other
>     process running under the same uid, as long as it is dumpable (i.e.
>     did not transition uids, start privileged, or have called
>     `prctl(PR_SET_DUMPABLE...)` already). Similarly, **PTRACE_TRACEME**
>     is unchanged.
>
> 1 - restricted ptrace: a process must have a predefined relationship
>     with the inferior it wants to call **PTRACE\_ATTACH** on. By default,
>     this relationship is that of only its descendants when the above
>     classic criteria is also met. To change the relationship, an
>     inferior can call `prctl(PR_SET_PTRACER, debugger, ...)` to declare
>     an allowed debugger PID to call **PTRACE_ATTACH** on the inferior.
>     Using **PTRACE\_TRACEME is unchanged.
>
> 2 - admin-only attach: only processes with **CAP\_SYS\_PTRACE** may use ptrace
>     with PTRACE_ATTACH, or through children calling **PTRACE\_TRACEME**.
>
> 3 - no attach: no processes may use ptrace with **PTRACE_ATTACH** nor via
>     **PTRACE_TRACEME**. Once set, this sysctl value cannot be changed.
