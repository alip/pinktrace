---
title: "Compiling C Code"
date: 2021-06-03T19:30:52+02:00
anchor: "compile"
weight: 100
---

You will need to specify various compiler flags when compiling C code. The
usual way to do this is via **pkg-config**:

{{< highlight shell "linenos=table" >}}
gcc -c $(pkg-config --cflags pinktrace) -o example.o example.c
gcc $(pkg-config --libs pinktrace) -o example example.o
{{< / highlight >}}
