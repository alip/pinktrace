SUBDIRS= linux .

CLEANFILES=
EXTRA_DIST=
MAINTAINERCLEANFILES=

OS	= linux
# ARCH is `arm', `i386', 'x86_64', etc.
ARCH	= @arch@

AM_CFLAGS= @PINKTRACE_CFLAGS@ \
	   -I$(top_builddir) -I$(top_srcdir) \
	   -I$(srcdir)/$(OS)/$(ARCH) -I$(srcdir)/$(OS)

lib_LTLIBRARIES = libpinktrace_@PINKTRACE_PC_SLOT@.la

libpinktrace_@PINKTRACE_PC_SLOT@_la_SOURCES= \
					     trace.c \
					     event.c \
					     abi.c \
					     name.c \
					     pipe.c \
					     regset.c \
					     read.c \
					     write.c \
					     vm.c \
					     socket.c
libpinktrace_@PINKTRACE_PC_SLOT@_la_LDFLAGS= \
					     -version-info @PINK_VERSION_LIB_CURRENT@:@PINK_VERSION_LIB_REVISION@:0 \
					     -export-symbols-regex '^pink_'

pinktrace_includedir= $(includedir)/pinktrace-$(PINKTRACE_PC_SLOT)/pinktrace/
pinktrace_include_HEADERS= \
			   about.h \
			   system.h \
			   compat.h \
			   compiler.h \
			   event.h \
			   trace.h \
			   abi.h \
			   name.h \
			   pipe.h \
			   regset.h \
			   vm.h \
			   read.h \
			   write.h \
			   socket.h \
			   pink.h
noinst_HEADERS= \
		private.h

IF_CHECK_SRCS= \
	       seatest.c \
	       trace-TEST.c \
	       read-TEST.c \
	       write-TEST.c \
	       socket-TEST.c \
	       pinktrace-check.c

noinst_HEADERS+= seatest.h pinktrace-check.h
EXTRA_DIST+= $(IF_CHECK_SRCS)

CHECK_CFLAGS= \
	      -I$(top_builddir) \
	      -I$(top_srcdir) \
	      @PINKTRACE_CFLAGS@
CHECK_LIBS= \
	    -lrt -lm \
	    $(builddir)/libpinktrace_@PINKTRACE_PC_SLOT@.la \
	    -L$(builddir)/.libs \
	    -lpinktrace_@PINKTRACE_PC_SLOT@

check_PROGRAMS= pinktrace-check

pinktrace_check_SOURCES= $(IF_CHECK_SRCS)
pinktrace_check_CFLAGS= $(CHECK_CFLAGS)
pinktrace_check_LDADD= $(CHECK_LIBS)

TESTS= $(check_PROGRAMS)

if ENABLE_INSTALLED_TESTS

bin_PROGRAMS= pinktrace-check

endif # ENABLE_INSTALLED_TESTS

SPARSE=sparse
SPARSE_CPPFLAGS= $(DEFAULT_INCLUDES) \
		 -D__STDC_VERSION__=199901L \
		 -I$(shell $(CC) -print-file-name=include) \
		 -I$(shell $(CC) -print-file-name=include-fixed)
# Fix this flag for your architecture!
SPARSE_CPPFLAGS+= -D__x86_64__=1
#SPARSE_CPPFLAGS+= -D__i386__=1
#SPARSE_CPPFLAGS+= -D__ia64__=1
#SPARSE_CPPFLAGS+= -D__powerpc__=1
#SPARSE_CPPFLAGS+= -D__powerpc64__=1
#SPARSE_CPPFLAGS+= -D__arm__=1

.PHONY: cppcheck
cppcheck:
	cppcheck $(libpinktrace_@PINKTRACE_PC_SLOT@_la_SOURCES) \
		--std=c99 --std=posix \
		--enable=warning,style,performance,portability,information,missingInclude \
		-I$(top_builddir) -I$(top_srcdir) \
		-I$(srcdir)/$(OS)/$(ARCH) -I$(srcdir)/$(OS)

.PHONY: sparse-check
sparse-check:
	for src in $(libpinktrace_@PINKTRACE_PC_SLOT@_la_SOURCES); \
	do \
		$(SPARSE) $(DEFS) $(SPARSE_CPPFLAGS) $(AM_CFLAGS) $$src || exit 1; \
	done
