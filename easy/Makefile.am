SUBDIRS= .

CLEANFILES=
AM_CFLAGS= \
	   -I$(top_builddir)/include \
	   -I$(top_srcdir)/include \
	   @PINKTRACE_CFLAGS@

easy_SRCS= \
	   attach.c \
	   call.c \
	   callback.c \
	   context.c \
	   exec.c \
	   error.c \
	   init.c \
	   loop.c \
	   process.c
EXTRA_DIST= $(easy_SRCS)

#if WANT_EASY
#lib_LTLIBRARIES= libpinktrace_easy_@PINKTRACE_PC_SLOT@.la
noinst_LTLIBRARIES= libpinktrace_easy_@PINKTRACE_PC_SLOT@.la
libpinktrace_easy_@PINKTRACE_PC_SLOT@_la_SOURCES= $(easy_SRCS)
#libpinktrace_easy_@PINKTRACE_PC_SLOT@_la_LDFLAGS= \
#						  -version-info @PINK_VERSION_LIB_CURRENT@:@PINK_VERSION_LIB_REVISION@:0 \
#						  -export-symbols-regex '^pink_'
libpinktrace_easy_@PINKTRACE_PC_SLOT@_la_LDFLAGS= \
						  -export-symbols-regex '^pink_'
libpinktrace_easy_@PINKTRACE_PC_SLOT@_la_LIBADD= $(top_builddir)/pinktrace/libpinktrace_@PINKTRACE_PC_SLOT@.la

SPARSE=sparse
SPARSE_CPPFLAGS= $(DEFAULT_INCLUDES) \
		 -I$(shell $(CC) -print-file-name=include) \
		 -I$(shell $(CC) -print-file-name=include-fixed)
# Fix this flag for your architecture!
SPARSE_CPPFLAGS+= -D__x86_64__=1
#SPARSE_CPPFLAGS+= -D__i386__=1
#SPARSE_CPPFLAGS+= -D__ia64__=1
#SPARSE_CPPFLAGS+= -D__powerpc__=1
#SPARSE_CPPFLAGS+= -D__powerpc64__=1
#SPARSE_CPPFLAGS+= -D__arm__=1

.PHONY: sparse-check
sparse-check:
	for src in $(libpinktrace_easy_@PINKTRACE_PC_SLOT@_la_SOURCES); \
	do \
		$(SPARSE) $(DEFS) $(SPARSE_CPPFLAGS) $(AM_CFLAGS) $$src || exit 1; \
	done
#endif # WANT_EASY
