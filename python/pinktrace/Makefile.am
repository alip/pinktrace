SUBDIRS= .
IF_PYTHON_FILES= \
		 __init__.py.in \
		 test.py.in \
		 py-system.c \
		 py-abi.c \
		 py-event.c \
		 py-name.c \
		 py-regset.c \
		 py-trace.c
EXTRA_DIST= $(IF_PYTHON_FILES)

CLEANFILES= *.so *.pyc *.pyo
clean-local: clean-local-pycache
.PHONY: clean-local-pycache
clean-local-pycache:
	-rm -fr __pycache__

AM_CFLAGS= \
	   $(PYTHON_CFLAGS) \
	   -I. -I$(top_builddir) -I$(top_srcdir) \
	   @PINKTRACE_CFLAGS@
AM_LDFLAGS= -module -avoid-version -shared \
	    $(PYTHON_LIBS) \
	    -L$(top_builddir)/pinktrace/.libs \
	    -lpinktrace_@PINKTRACE_PC_SLOT@

# There is no AM_LIBADD!
# Adding it manually for now.
PINKTRACE_LIB_ADD= $(top_builddir)/pinktrace/libpinktrace_@PINKTRACE_PC_SLOT@.la

BUILT_SOURCES=

noinst_HEADERS= \
		py-pink.h

if ENABLE_PYTHON
pkgpyexec_DATA= __init__.py test.py
CLEANFILES+= __init__.py test.py
__init__.py: __init__.py.in
	$(AM_V_GEN)
	$(AM_V_at)$(SED) \
		-e "s,#PACKAGE#,$(PACKAGE)," \
		-e "s,#VERSION#,$(PINK_VERSION_FULL)," \
		-e "s,#VERSION_MAJOR#,$(PINK_VERSION_MAJOR)," \
		-e "s,#VERSION_MINOR#,$(PINK_VERSION_MINOR)," \
		-e "s,#VERSION_MICRO#,$(PINK_VERSION_MICRO)," \
		-e "s,#VERSION_SUFFIX#,$(PINK_VERSION_SUFFIX)," \
		-e "s,#GIT_HEAD#,$(GITHEAD)," \
		-e "s,#PC_VERSION#,$(PINKTRACE_PC_VERSION)," \
		-e "s,#PC_SLOT#,$(PINKTRACE_PC_SLOT)," \
		$< > $@

# FIXME: This is a hack to fix make distcheck.
test.py: test.py.in
	$(AM_V_GEN)
	$(AM_V_at)cat $< > $@

pkgpyexec_LTLIBRARIES= \
		       system.la \
		       abi.la \
		       event.la \
		       name.la \
		       regset.la \
		       trace.la

system_la_SOURCES= py-system.c
system_la_LIBADD= $(PINKTRACE_LIB_ADD)

abi_la_SOURCES= py-abi.c
abi_la_LIBADD= $(PINKTRACE_LIB_ADD)

event_la_SOURCES= py-event.c
event_la_LIBADD= $(PINKTRACE_LIB_ADD)

name_la_SOURCES= py-name.c
name_la_LIBADD= $(PINKTRACE_LIB_ADD)

regset_la_SOURCES= py-regset.c
regset_la_LIBADD= $(PINKTRACE_LIB_ADD)

trace_la_SOURCES= py-trace.c
trace_la_LIBADD= $(PINKTRACE_LIB_ADD)

# Hack to make epydoc & tests work.
%.so: %.la
	$(AM_V_GEN)
	$(AM_V_at)test -h $@ || $(LN_S) $(addprefix .libs/, $(addsuffix .so,$(basename $<))) $@
noinst_DATA= \
	     system.so \
	     abi.so \
	     event.so \
	     name.so \
	     regset.so \
	     trace.so

# Remove the .la files - *.la is never linked against (pinktrace is a module)
# and Python doesn't use libltdl. Note that the library gets installed by
# install-data, so that's where we need to hook.
install-data-hook:
	$(RM) -f $(DESTDIR)$(pkgpyexecdir)/*.la

# For some reason make uninstall does not clean up *.so files under Python
# library directory. Let's do this manually.
uninstall-hook:
	$(RM) -f $(DESTDIR)$(pkgpyexecdir)/*.so

endif # ENABLE_PYTHON
