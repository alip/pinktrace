---
title: "Building"
date: 2021-06-03T19:23:37+02:00
anchor: "build"
weight: 90
---

This package is made with the GNU autotools, you should run `./configure`
inside the distribution directory for configuring the source tree. Some notable
options you may pass to `./configure` are:

* --enable-doxygen Build API documentation using [Doxygen](https://www.doxygen.nl/index.html)
* --enable-python Build [Python](https://www.python.org/) bindings
* --enable-python-doc Build API documentation of [Python](https://www.python.org) bindings using [PyDoctor](https://pydoctor.readthedocs.io/en/latest/)

After that you should run `make` for compilation and `make install` (as root)
for installation of pinktrace. Optionally you may run `make check` to run the
unit tests.

The source of this web site is also included in the distribution. To view them
offline you should generate them using [Hugo](https://gohugo.io/). After
`./configure` change directory to `doc/` and run `make site`. The web site will
be built under the `public` directory. You may then run `make site-check` and
point your browser to http://localhost:1313/pinktrace` and view this web site
offline.
