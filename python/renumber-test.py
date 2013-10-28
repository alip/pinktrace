#!/usr/bin/env python
# coding: utf-8
# Copyright 2013 Ali Polatel <alip@exherbo.org>
# Distributed under the same terms as Python itself.

from __future__ import print_function, with_statement

import os, re

# Shamelessly stolen from python-3.3/shlex.py
_find_unsafe = re.compile(r'[^\w@%+=:,./-]', re.ASCII).search
def quote(s):
    """Return a shell-escaped version of the string *s*."""
    if not s:
        return "''"
    if _find_unsafe(s) is None:
        return s

    # use single quotes, and put single quotes into double quotes
    # the string $'b is then quoted as '$'"'"'b'
    return "'" + s.replace("'", "'\"'\"'") + "'"

TESTS = ["pinktrace/test.py"]

tc = re.compile(r"^class TestCase_[0-9]+_")
tr = re.compile(r"^(\s+)def test_[0-9]+_")
for test in TESTS:
    tc_i = 1
    tr_i = 1
    with open(test, 'r') as src:
        with open(test + ".new", 'w') as dest:
            for line in src.readlines():
                if tc.match(line) is not None:
                    dest.write(tc.sub(r"class TestCase_%02d_" % tc_i, line))
                    tr_i = 1
                    tc_i += 1
                elif tr.match(line) is not None:
                    dest.write(tr.sub(r"\1def test_%02d_" % tr_i, line))
                    tr_i += 1
                else:
                    dest.write(line)
    print("%s -> %s" % (test, test + ".new"))
    os.system("mv -i %s %s" % (quote(test + ".new"), quote(test)))
