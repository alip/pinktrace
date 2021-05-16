AUTOMAKE_OPTIONS= dist-bzip2 no-dist-gzip std-options subdir-objects foreign
AM_MAKEFLAGS= --no-print-directory

CLEANFILES= *.pyc *.pyo
SUBDIRS= pinktrace .

dist_doc_data= \
	       COPYING

if ENABLE_PYTHON
.PHONY:
check-local:
	env PYTHONPATH=$(builddir) $(PYTHON) -m pinktrace.test
endif
