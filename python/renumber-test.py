#!/usr/bin/env python
# coding: utf-8

from __future__ import print_function, with_statement

import os, re

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
