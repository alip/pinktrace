AUTOMAKE_OPTIONS= dist-bzip2 no-dist-gzip std-options subdir-objects foreign
AM_MAKEFLAGS= --no-print-directory

CLEANFILES= *.pyc *.pyo
SUBDIRS= pinktrace .

.PHONY:
check-local:
	$(PYTHON) -m pinktrace
