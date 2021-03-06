Build pinktrace from git repository
===================================
If you use a GIT version of PinkTrace source code, there will be some files missing
that are needed to build PinkTrace. Some of these files are generated by tools
from the GNU Autoconf and GNU Automake packages.

Note: rather than running `autoreconf` directly, please invoke `./autogen.sh` script
and follow the instructions given in [INSTALL](README-configure) file for further
building and installation.

Building Python bindings
========================

PinkTrace [Python](https://www.python.org/) bindings require Python-2.7 or newer.
To build them, pass the `--enable-python` option to `./configure`. Use the
environment variable `PYTHON` to pick a Python version to build against, e.g:

```
env PYTHON=python3 ./configure --enable-python
```

Building API Documentation
==========================

The API reference may be built using [Doxygen](https://www.doxygen.nl/index.html).
To build them, pass the `--enable-doxygen` option to `./configure`.
