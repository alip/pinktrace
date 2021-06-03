---
title: "Portability"
date: 2021-06-03T17:51:57+02:00
anchor: "portability"
weight: 50
---

**PinkTrace** runs on [Linux](https://en.wikipedia.org/wiki/Linux) only. Version
5.0.0 or later is recommended. Enabling the **CONFIG\_CROSS\_MEMORY\_ATTACH** kernel
option allows **PinkTrace** to make use of the
[process_vm_readv](http://man7.org/linux/man-pages/man2/process_vm_readv.2.html), and
[process_vm_writev](http://man7.org/linux/man-pages/man2/process_vm_readv.2.html) system
calls to transfer data to and from tracee's address space which is much faster and
more reliable than using [ptrace](http://man7.org/linux/man-pages/man2/ptrace.2.html) to do the same.
