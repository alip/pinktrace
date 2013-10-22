#!/bin/sh -ex

rm -fr autom4te.cache build-aux
rm -f config.cache
test -d build-aux || mkdir build-aux

libtoolize --copy --force
aclocal -I m4
autoheader
autoconf
automake --add-missing --copy
