dnl Program, version
AC_PREREQ([2.59])

m4_define([pinktrace_version_major],  [0])
m4_define([pinktrace_version_minor],  [1])
m4_define([pinktrace_version_micro],  [3])
m4_define([pinktrace_version_suffix], [])
m4_define([pinktrace_version_full],   [pinktrace_version_major.pinktrace_version_minor.pinktrace_version_micro[]pinktrace_version_suffix])
m4_define([pinktrace_version],        [pinktrace_version_full])
m4_define([pinktrace_lib_current],    [m4_eval(100 * pinktrace_version_major + pinktrace_version_minor)])
m4_define([pinktrace_lib_revision],   [pinktrace_version_micro])

AC_INIT([pinktrace], [pinktrace_version], [http://github.com/alip/pinktrace/issues], [pinktrace])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

AC_CANONICAL_HOST

VERSION_MAJOR=pinktrace_version_major
VERSION_MINOR=pinktrace_version_minor
VERSION_MICRO=pinktrace_version_micro
VERSION_SUFFIX=pinktrace_version_suffix
VERSION_FULL=pinktrace_version_full
VERSION=pinktrace_version
VERSION_LIB_CURRENT=pinktrace_lib_current
VERSION_LIB_REVISION=pinktrace_lib_revision

AC_SUBST([VERSION_MAJOR])
AC_SUBST([VERSION_MINOR])
AC_SUBST([VERSION_MICRO])
AC_SUBST([VERSION_FULL])
AC_SUBST([VERSION_SUFFIX])
AC_SUBST([VERSION_LIB_CURRENT])
AC_SUBST([VERSION_LIB_REVISION])

AM_INIT_AUTOMAKE([dist-bzip2 no-dist-gzip std-options foreign])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl Git revision
if test -d "${GIT_DIR:-${ac_top_srcdir:-./}/.git}" ; then
	GITHEAD=`git describe 2>/dev/null`
	if test -z "${GITHEAD}" ; then
		GITHEAD=`git rev-parse HEAD`
	fi
	if test -n "`git diff-index -m --name-only HEAD`" ; then
		GITHEAD="${GITHEAD}-dirty"
	fi
else
	GITHEAD=
fi
AC_MSG_CHECKING([for git head])
AC_ARG_WITH([git-head],
	AS_HELP_STRING([--with-git-head], [Specify Git head]),
	GITHEAD=`eval echo $withval`,
	GITHEAD=`eval echo $GITHEAD`)
AC_MSG_RESULT([$GITHEAD])
AC_SUBST([GITHEAD])

PINKTRACE_PC_VERSION="${VERSION_FULL}"
PINKTRACE_PC_SLOT="${VERSION_MAJOR}.${VERSION_MINOR}${VERSION_SUFFIX}"
PINKTRACE_PC_SLOT="${PINKTRACE_PC_SLOT}_"`echo ${GITHEAD} | sed -e 's,\(........\(-dirty\)\?\)$,#\1,' -e 's,.*#,,'`

AC_MSG_CHECKING([for slot])
AC_MSG_RESULT([$PINKTRACE_PC_SLOT])

PINKTRACE_CFLAGS=
PINKTRACE_PC_CFLAGS=
PINKTRACE_PC_LIBS="-lpinktrace_${PINKTRACE_PC_SLOT}"
PINKTRACE_EASY_PC_LIBS="-lpinktrace_easy_${PINKTRACE_PC_SLOT}"

dnl Toolchain checks
AC_PROG_CC_C99
if test x"$ac_cv_prog_cc_c99" = x"no"; then
	AC_MSG_ERROR([pinktrace requires a C compiler that supports ISO C99!])
fi
AC_USE_SYSTEM_EXTENSIONS
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_EGREP
AC_PROG_SED
AC_PROG_LN_S
LT_INIT

dnl Check for supported operating system
SYSSRC_DEFAULT=/usr/src/sys
AC_ARG_VAR([SYSSRC], [Path to the system sources for FreeBSD (default: /usr/src/sys)])
AC_MSG_CHECKING([for supported operating system])
case "$host_os" in
freebsd*)
	opsys="freebsd"
	AC_MSG_RESULT([$host_os])
	PINKTRACE_CFLAGS="${PINKTRACE_CFLAGS} -DPINKTRACE_FREEBSD"
	PINKTRACE_PC_CFLAGS="${PINKTRACE_PC_CFLAGS} -DPINKTRACE_FREEBSD"
	dnl Check for kernel sources, required to generate syscall names.
	AC_MSG_CHECKING([for kernel sources])
	if test x"$SYSSRC" = x ; then
		if test -d "$SYSSRC_DEFAULT" ; then
			SYSSRC="$SYSSRC_DEFAULT"
		else
			AC_MSG_RESULT([NO!])
			AC_MSG_ERROR([System sources not found under $SYSSRC_DEFAULT])
		fi
	elif ! test -d "$SYSSRC" ; then
		AC_MSG_RESULT([NO!])
		AC_MSG_ERROR([Not a directory: $SYSSRC])
	fi
	AC_MSG_RESULT([$SYSSRC])
	AC_SUBST([SYSSRC])
	;;
linux*)
	opsys="linux"
	AC_MSG_RESULT([$host_os])
	AC_MSG_CHECKING([for supported kernel])
	kernel_version=`uname -r | cut -d'-' -f1`
	case "$kernel_version" in
	3.*|2.6*)
		AC_MSG_RESULT([$kernel_version])
		;;
	2.5*)
		kernel_minor_version=`echo $kernel_version | cut -d'.' -f3`
		if test "$kernel_minor_version" -lt 46; then
			AC_MSG_RESULT([NO!])
			AC_MSG_ERROR([Linux kernel version $kernel_version is not supported by pinktrace!])
		fi
		AC_MSG_RESULT([$kernel_version])
		;;
	*)
		AC_MSG_RESULT([NO!])
		AC_MSG_ERROR([Linux kernel version $kernel_version is not supported by pinktrace!])
		;;
	esac
	PINKTRACE_CFLAGS="${PINKTRACE_CFLAGS} -DPINKTRACE_LINUX"
	PINKTRACE_PC_CFLAGS="${PINKTRACE_PC_CFLAGS} -DPINKTRACE_LINUX"
	;;
*)
	AC_MSG_RESULT([NO!])
	AC_MSG_ERROR([Operating system $host_os is not supported by pinktrace!])
	;;
esac
AM_CONDITIONAL([FREEBSD], test x"$opsys" = x"freebsd")
AM_CONDITIONAL([LINUX], test x"$opsys" = x"linux")

dnl Check for supported architecture
AC_MSG_CHECKING([for supported architecture])
case "$host_cpu" in
i[[3456]]86|pentium)
	arch=i386
	AC_DEFINE([I386], 1, [Define for the i386 architecture.])
	;;
x86?64*|amd64)
	arch=x86_64
	AC_DEFINE([X86_64], 1, [Define for the AMD x86-64 architecture.])
	;;
ia64)
	arch=ia64
	if test x"$opsys" = x"freebsd" ; then
		AC_MSG_RESULT([NO!])
		AC_MSG_ERROR([Architecture $host_cpu is not supported by pinktrace])
	fi
	AC_DEFINE([IA64], 1, [Define for the IA64 architecture.])
	;;
powerpc64*)
	arch=powerpc64
	if test x"$opsys" = x"freebsd" ; then
		AC_MSG_RESULT([NO!])
		AC_MSG_ERROR([Architecture $host_cpu is not supported by pinktrace])
	fi
	AC_DEFINE([POWERPC64], 1, [Define for the POWERPC64 architecture.])
	;;
powerpc*)
	arch=powerpc
	if test x"$opsys" = x"freebsd" ; then
		AC_MSG_RESULT([NO!])
		AC_MSG_ERROR([Architecture $host_cpu is not supported by pinktrace])
	fi
	AC_DEFINE([POWERPC], 1, [Define for the POWERPC architecture.])
	;;
arm*)
	arch=arm
	if test x"$opsys" = x"freebsd" ; then
		AC_MSG_RESULT([NO!])
		AC_MSG_ERROR([Architecture $host_cpu is not supported by pinktrace])
	fi
	AC_DEFINE([ARM], 1, [Define for the ARM architecture.])
	;;
*)
	AC_MSG_RESULT([NO!])
	AC_MSG_ERROR([Architecture $host_cpu is not supported by pinktrace])
	;;
esac
AC_MSG_RESULT([$arch])

AM_CONDITIONAL(I386, test x"$arch" = x"i386")
AM_CONDITIONAL(X86_64, test x"$arch" = x"x86_64")
AM_CONDITIONAL(IA64, test x"$arch" = x"ia64")
AM_CONDITIONAL(POWERPC, test x"$arch" = x"powerpc")
AM_CONDITIONAL(POWERPC64, test x"$arch" = x"powerpc64")
AM_CONDITIONAL(ARM, test x"$arch" = x"arm")

if test x"$arch" = x"x86_64" ; then
	PINKTRACE_BITNESS_COUNT_SUPPORTED=2
else
	PINKTRACE_BITNESS_COUNT_SUPPORTED=1
fi
AC_SUBST([PINKTRACE_BITNESS_COUNT_SUPPORTED])
AM_CONDITIONAL(BITNESS_ONE, test x"$PINKTRACE_BITNESS_COUNT_SUPPORTED" = x"1")
AM_CONDITIONAL(BITNESS_TWO, test x"$PINKTRACE_BITNESS_COUNT_SUPPORTED" = x"2")

if test x"$arch" = x"i386" -o x"$arch" = x"powerpc" -o x"$arch" = x"arm" ; then
	PINKTRACE_BITNESS_DEFAULT="PINK_BITNESS_32"
else
	PINKTRACE_BITNESS_DEFAULT="PINK_BITNESS_64"
fi
AC_SUBST([PINKTRACE_BITNESS_DEFAULT])

if test x"$arch" = x"i386" -o x"$arch" = x"powerpc" -o x"$arch" = x"arm" ; then
	PINKTRACE_BITNESS_64_SUPPORTED=0
else
	PINKTRACE_BITNESS_64_SUPPORTED=1
fi
AC_SUBST([PINKTRACE_BITNESS_64_SUPPORTED])

if test x"$arch" = x"ia64" -o x"$arch" = x"powerpc64" ; then
	PINKTRACE_BITNESS_32_SUPPORTED=0
else
	PINKTRACE_BITNESS_32_SUPPORTED=1
fi
AC_SUBST([PINKTRACE_BITNESS_32_SUPPORTED])

if test x"$arch" = x"arm" ; then
	PINKTRACE_CFLAGS="${PINKTRACE_CFLAGS} -DPINKTRACE_INVALID_SYSCALL=0xbad"
	PINKTRACE_PC_CFLAGS="${PINKTRACE_PC_CFLAGS} -DPINKTRACE_INVALID_SYSCALL=0xbad"
else
	PINKTRACE_CFLAGS="${PINKTRACE_CFLAGS} -DPINKTRACE_INVALID_SYSCALL=0xbadca11"
	PINKTRACE_PC_CFLAGS="${PINKTRACE_PC_CFLAGS} -DPINKTRACE_INVALID_SYSCALL=0xbadca11"
fi

dnl Check for types
AC_TYPE_PID_T
AC_TYPE_SIZE_T

dnl Check for sizes
AC_CHECK_SIZEOF([int])
AC_CHECK_SIZEOF([long])
AC_CHECK_SIZEOF([long long])
AC_CHECK_SIZEOF([pid_t])

dnl Check headers
REQUIRED_HEADERS="sys/ptrace.h sys/types.h sys/wait.h netinet/in.h sys/un.h"
if test x"$opsys" = x"linux" ; then
	REQUIRED_HEADERS="${REQUIRED_HEADERS} linux/ptrace.h"
fi
for header in $REQUIRED_HEADERS ; do
	AC_CHECK_HEADER([$header], [],
					AC_MSG_ERROR([Required header $header not found!]))
done
AC_CHECK_HEADERS([machine/reg.h machine/psl.h sys/reg.h], [], [])

dnl Check types
AC_CHECK_TYPES([struct pt_all_user_regs, struct ia64_fpreg],,,[#include <sys/ptrace.h>])

dnl Check for gcc
AM_CONDITIONAL(GCC, test x"$GCC" = x"yes")

dnl Check for IPV6 support
AC_ARG_ENABLE([ipv6],
	      [AS_HELP_STRING([--disable-ipv6], [disable IPV6 support])],
	      WANT_IPV6="$enableval",
	      WANT_IPV6="yes")
if test x"$WANT_IPV6" = x"yes" ; then
	APR_CHECK_SOCKADDR_IN6
	CHECK_AF_INET6_DEFINE
	AC_MSG_CHECKING([for IPV6 support])
	if test x"$have_sockaddr_in6" = x"1" ; then
		if test x"$have_af_inet6" = x"1" ; then
			AC_MSG_RESULT([yes])
		else
			AC_MSG_RESULT([no -- no AF_INET6 define])
			WANT_IPV6=no
		fi
	else
		AC_MSG_RESULT([no -- no sockaddr_in6])
		WANT_IPV6=no
	fi
else
	AC_MSG_CHECKING([for IPV6 support])
	AC_MSG_RESULT([no])
fi
AM_CONDITIONAL(WANT_IPV6, test x"$WANT_IPV6" = x"yes")
if test x"$WANT_IPV6" = x"yes" ; then
	PINKTRACE_CFLAGS="${PINKTRACE_CFLAGS} -DPINKTRACE_HAVE_IPV6=1"
	PINKTRACE_PC_CFLAGS="${PINKTRACE_PC_CFLAGS} -DPINKTRACE_HAVE_IPV6=1"
else
	PINKTRACE_CFLAGS="${PINKTRACE_CFLAGS} -DPINKTRACE_HAVE_IPV6=0"
	PINKTRACE_PC_CFLAGS="${PINKTRACE_PC_CFLAGS} -DPINKTRACE_HAVE_IPV6=0"
fi

dnl Check for Netlink support
AC_CHECK_HEADER([linux/netlink.h],
		[AC_CHECK_TYPES([struct sockaddr_nl],
				[WANT_NETLINK="yes"],
				[WANT_NETLINK="no"],
				[#include <stddef.h>
				 #include <sys/socket.h>
				 #include <asm/types.h>
				 #include <linux/netlink.h>])],
		[WANT_NETLINK="no"],
		[#include <stddef.h>
		 #include <sys/socket.h>
		 #include <asm/types.h>])
AC_MSG_CHECKING([for Netlink support])
AC_MSG_RESULT([$WANT_NETLINK])
AM_CONDITIONAL(WANT_NETLINK, test x"$WANT_NETLINK" = x"yes")
if test x"$WANT_NETLINK" = x"yes" ; then
	PINKTRACE_CFLAGS="${PINKTRACE_CFLAGS} -DPINKTRACE_HAVE_NETLINK=1"
	PINKTRACE_PC_CFLAGS="${PINKTRACE_PC_CFLAGS} -DPINKTRACE_HAVE_NETLINK=1"
else
	PINKTRACE_CFLAGS="${PINKTRACE_CFLAGS} -DPINKTRACE_HAVE_NETLINK=0"
	PINKTRACE_PC_CFLAGS="${PINKTRACE_PC_CFLAGS} -DPINKTRACE_HAVE_NETLINK=0"
fi

dnl pinktrace-easy
AC_ARG_ENABLE([easy],
	      [AS_HELP_STRING([--disable-easy], [Do not build pinktrace_easy library (availability: Linux)])],
	      [WANT_EASY="$enableval"],
	      [if test x"$opsys" = x"linux";
	       then WANT_EASY="yes"
	       else AC_MSG_ERROR([pinktrace_easy is only available on Linux])
	       fi])
if test x"$WANT_EASY" = x"yes" ; then
	AC_CHECK_HEADER([sys/queue.h], [], AC_MSG_ERROR([pinktrace_easy requires sys/queue.h]))
	AC_CHECK_HEADER([alloca.h], [], AC_MSG_ERROR([pinktrace_easy requires alloca.h]))
	AC_FUNC_ALLOCA

	if test x"$opsys" = x"freebsd" ; then
		AC_MSG_ERROR([pinktrace_easy is not available for FreeBSD])
dnl 		AC_MSG_CHECKING([whether ptrace() defines required requests])
dnl 		AC_PREPROC_IFELSE([AC_LANG_SOURCE([
dnl #include <sys/types.h>
dnl #include <sys/ptrace.h>
dnl
dnl #ifndef PT_LWPINFO
dnl #error nope
dnl #endif
dnl
dnl #ifndef PL_FLAG_SCE
dnl #error nope
dnl #endif
dnl
dnl #ifndef PL_FLAG_SCX
dnl #error nope
dnl #endif
dnl
dnl #ifndef PL_FLAG_EXEC
dnl #error nope
dnl #endif
dnl
dnl #ifndef PL_FLAG_FORKED
dnl #error nope
dnl #endif
dnl
dnl #ifndef PT_FOLLOW_FORK
dnl #error nope
dnl #endif
dnl ])],
dnl 	[AC_MSG_RESULT([yes])],
dnl 	[AC_MSG_RESULT([NO!])
dnl 	 AC_MSG_ERROR([required ptrace() defines are missing!])])
dnl 	#elif test x"$opsys" = x"linux" ; then
	fi
fi
AM_CONDITIONAL([WANT_EASY], test x"$WANT_EASY" = x"yes")

dnl Extra CFLAGS
WANTED_CFLAGS="-pedantic -Wall -W -Wextra -Wbad-function-cast -Wcast-align -Wcast-qual -Wfloat-equal -Wformat=2 -Wformat-security -Wformat-nonliteral -Winit-self -Winline -Wlogical-op -Wmissing-prototypes -Wmissing-declarations -Wmissing-format-attribute -Wmissing-noreturn -Wpointer-arith -Wredundant-decls -Wshadow -Wswitch-default -Wunused -Wvla"
for flag in $WANTED_CFLAGS; do
	AX_CHECK_COMPILER_FLAGS([$flag], [PINKTRACE_CFLAGS="$PINKTRACE_CFLAGS $flag"],)
done
AC_SUBST([PINKTRACE_CFLAGS])

dnl pkg-config
AC_SUBST([PINKTRACE_PC_CFLAGS])
AC_SUBST([PINKTRACE_PC_LIBS])
AC_SUBST([PINKTRACE_PC_VERSION])
AC_SUBST([PINKTRACE_PC_SLOT])
AC_SUBST([PINKTRACE_EASY_PC_LIBS])

dnl Library checks
PKG_PROG_PKG_CONFIG([0.9.0])
PKG_CHECK_MODULES([check], [check >= 0.9.4], [have_libcheck=yes], [have_libcheck=no])
AM_CONDITIONAL([HAVE_LIBCHECK], test "x$have_libcheck" = "xyes")

dnl Doxygen
AC_MSG_CHECKING([whether to enable doxygen])
AC_ARG_ENABLE([doxygen],
	      AS_HELP_STRING([--enable-doxygen], [Enable 'make doxygen' (developer docs)]),
	      [HAVE_DOXYGEN=$enableval
	       AC_MSG_RESULT([$enableval])],
	      [AC_MSG_RESULT([autodetect])
	       AC_CHECK_PROG(HAVE_DOXYGEN, [doxygen], [yes], [no])])
AC_SUBST([HAVE_DOXYGEN])
AM_CONDITIONAL([HAVE_DOXYGEN], test "x$HAVE_DOXYGEN" = "xyes")

dnl Ruby interface
AC_ARG_VAR([RUBY], [Path to the Ruby interpreter])
AC_MSG_CHECKING([whether to build the Ruby interface])
AC_ARG_ENABLE([ruby],
	      AS_HELP_STRING([--enable-ruby], [Enable Ruby interface (default: disable)]),
	      enable_ruby="$enableval",
	      enable_ruby=no)
AC_MSG_RESULT([$enable_ruby])

if test x"$enable_ruby" = x"yes" ; then
	AC_PATH_PROG([RUBY], ruby)
	if test x"$RUBY" = x ; then
		AC_MSG_ERROR([Ruby is required for --enable-ruby])
	fi

	AC_MSG_CHECKING([for Ruby archdir])
	RUBY_ARCHDIR=`$RUBY -r rbconfig -e 'puts Config::CONFIG@<:@"archdir"@:>@'`
	if test x"$RUBY_ARCHDIR" = x ; then
		AC_MSG_ERROR([Could not find Ruby archdir])
	fi
	AC_MSG_RESULT([$RUBY_ARCHDIR])
	AC_SUBST([RUBY_ARCHDIR])

	AC_MSG_CHECKING([for Ruby sitearchdir])
	RUBY_SITEARCHDIR=`$RUBY -r rbconfig -e 'puts Config::CONFIG@<:@"sitearchdir"@:>@'`
	if test x"$RUBY_SITEARCHDIR" = x ; then
		AC_MSG_ERROR([Could not find Ruby sitearchdir])
	fi
	AC_MSG_RESULT([$RUBY_SITEARCHDIR])
	AC_SUBST([RUBY_SITEARCHDIR])

	AC_MSG_CHECKING([for Ruby soname])
	RUBY_SONAME=`$RUBY -r rbconfig -e 'puts Config::CONFIG@<:@"RUBY_SO_NAME"@:>@'`
	if test x"$RUBY_SONAME" = x ; then
		AC_MSG_ERROR([Could not find Ruby soname])
	fi
	AC_MSG_RESULT([$RUBY_SONAME])
	AC_SUBST([RUBY_SONAME])

	AC_ARG_WITH([ruby-install-dir],
		    AS_HELP_STRING([--with-ruby-install-dir=dir], [Specify Ruby installation dir]),
		    RUBY_INSTALL_DIR=`eval echo $withval`,
		    RUBY_INSTALL_DIR=)
	AC_MSG_CHECKING([for Ruby installation dir])
	if test x"$RUBY_INSTALL_DIR" = x ; then
		RUBY_INSTALL_DIR="$RUBY_SITEARCHDIR"
	fi
	AC_MSG_RESULT([$RUBY_INSTALL_DIR])
	AC_SUBST([RUBY_INSTALL_DIR])

	AC_MSG_CHECKING([for Ruby hdrdir])
	RUBY_HDRDIR=`$RUBY -r rbconfig -e 'puts Config::CONFIG@<:@"rubyhdrdir"@:>@'`
	if test x"$RUBY_HDRDIR" = x ; then
		AC_MSG_ERROR([Could not find Ruby hdrdir])
	elif test x"$RUBY_HDRDIR" = x"nil" ; then
		RUBY_HDRDIR=$RUBY_ARCHDIR
	fi
	AC_MSG_RESULT([$RUBY_HDRDIR])
	AC_SUBST([RUBY_HDRDIR])

	AC_MSG_CHECKING([for Ruby hdrarchdir])
	if test x"$RUBY_ARCHDIR" = x"$RUBY_HDRDIR" ; then
		RUBY_HDRARCHDIR=$RUBY_ARCHDIR
	else
		RUBY_HDRARCHDIR=`$RUBY -r rbconfig -e 'puts File.join(Config::CONFIG@<:@"rubyhdrdir"@:>@, Config::CONFIG@<:@"arch"@:>@'\)`
		if test x"$RUBY_HDRARCHDIR" = x ; then
			AC_MSG_ERROR([Could not find Ruby hdrarchdir])
		fi
	fi
	AC_MSG_RESULT([$RUBY_HDRARCHDIR])
	AC_SUBST([RUBY_HDRARCHDIR])

	dnl We need a way to check if rb_last_status_get() is available.
	dnl Since it's an internal function only available to modules as of 1.9.0;
	dnl AC_COMPILE_IFELSE or AC_LINK_IFELSE doesn't work.
	dnl So we check if ruby is newer than 1.9 or not.
	AC_MSG_CHECKING([whether we can use rb_last_status_get()])
	if $RUBY -e '(VERSION rescue RUBY_VERSION) > "1.9.0" or exit 1' >/dev/null 2>/dev/null ; then
		AC_MSG_RESULT([yes, rb_last_status_get() is available])
		AC_DEFINE([HAVE_RB_LAST_STATUS_GET], 1, [Define for rb_last_status_get() function])
	else
		AC_MSG_RESULT([no])
	fi

	# Extra cflags for Ruby to quiet useless warnings
	WANTED_RUBY_CFLAGS="-Wno-strict-prototypes -Wno-missing-prototypes -Wno-redundant-decls -Wno-missing-noreturn -Wno-unused"
	PINKTRACE_RUBY_CFLAGS=
	for flag in $WANTED_RUBY_CFLAGS ; do
		AX_CHECK_COMPILER_FLAGS([$flag], [PINKTRACE_RUBY_CFLAGS="$PINKTRACE_RUBY_CFLAGS $flag"],)
	done
	AC_SUBST([PINKTRACE_RUBY_CFLAGS])
fi
AM_CONDITIONAL([ENABLE_RUBY], test x"$enable_ruby" = x"yes")

dnl Ruby documentation
AC_MSG_CHECKING([whether to build the Ruby documentation])
AC_ARG_VAR([RDOC], [Path to rdoc documentation tool])
AC_ARG_ENABLE([ruby-doc],
	      AS_HELP_STRING([--enable-ruby-doc],
			     [Enable Ruby documentation, requires --enable-ruby (default: disable)]),
	      enable_ruby_doc="$enableval",
	      enable_ruby_doc=no)
AC_MSG_RESULT([$enable_ruby_doc])

if test x"$enable_ruby_doc" = x"yes" && test x"$enable_ruby" = x"no"; then
	AC_MSG_ERROR([--enable-ruby-doc requires --enable-ruby])
fi

if test x"$enable_ruby_doc" = x"yes" ; then
	AC_PATH_PROG([RDOC], rdoc)
	if test x"$RDOC" = x ; then
		AC_MSG_ERROR([rdoc is required for --enable-ruby-doc])
	fi
fi
AM_CONDITIONAL([ENABLE_RUBY_DOC], test x"$enable_ruby_doc" = x"yes")

dnl Python interface
AC_ARG_VAR([PYTHON], [path to the Python interpreter])
AC_MSG_CHECKING([whether to build the Python interface])
AC_ARG_ENABLE([python],
	      AS_HELP_STRING([--enable-python], [Enable Python interface (default: disable)]),
	      enable_python="$enableval",
	      enable_python=no)
AC_MSG_RESULT([$enable_python])

if test x"$enable_python" = x"yes" ; then
	AM_PATH_PYTHON([2.4])
	if test x"$PYTHON" = x ; then
		AC_MSG_ERROR([Python is required for --enable-python])
	fi

	AC_MSG_CHECKING([for headers required to compile python extensions])
	py_prefix=`$PYTHON -c "import sys; sys.stdout.write(sys.prefix)"`
	py_exec_prefix=`$PYTHON -c "import sys; sys.stdout.write(sys.exec_prefix)"`
	PYTHON_INCLUDE_DIR="${py_prefix}/include/python${PYTHON_VERSION}"
	if test "$py_prefix" != "$py_exec_prefix"; then
		PYTHON_INCLUDE_DIR="$PYTHON_INCLUDE_DIR -I${py_exec_prefix}/include/python${PYTHON_VERSION}"
	fi
	AC_MSG_RESULT([$PYTHON_INCLUDE_DIR])
	AC_SUBST([PYTHON_INCLUDE_DIR])

	AC_MSG_CHECKING([for Python install directory])
	AC_ARG_WITH([python-install-dir],
				AS_HELP_STRING([--with-python-install-dir=dir],
							   [Specify Python installation directory]),
				PYTHON_INSTALL_DIR=`eval echo $withval`,
				PYTHON_INSTALL_DIR=)
	if test x"$PYTHON_INSTALL_DIR" = x ; then
		PYTHON_INSTALL_DIR="$pyexecdir"
	fi
	AC_MSG_RESULT([$PYTHON_INSTALL_DIR])
	AC_SUBST([PYTHON_INSTALL_DIR])

	if test x"$WANT_IPV6" = x"yes" ; then
		PYTHON_HAVE_IPV6="True"
	else
		PYTHON_HAVE_IPV6="False"
	fi
	AC_SUBST([PYTHON_HAVE_IPV6])

	if test x"$WANT_NETLINK" = x"yes" ; then
		PYTHON_HAVE_NETLINK="True"
	else
		PYTHON_HAVE_NETLINK="False"
	fi
	AC_SUBST([PYTHON_HAVE_NETLINK])
fi
AM_CONDITIONAL([ENABLE_PYTHON], test x"$enable_python" = x"yes")

dnl Python documentation
AC_ARG_VAR([EPYDOC], [Path to the epydoc documentation tool])
AC_MSG_CHECKING([whether to build the Python documentation])
AC_ARG_ENABLE([python-doc],
	      AS_HELP_STRING([--enable-python-doc],
			     [Enable Python documentation, requires --enable-python (default: disable)]),
	      enable_python_doc="$enableval",
	      enable_python_doc=no)
AC_MSG_RESULT([$enable_python_doc])

if test x"$enable_python_doc" = x"yes" && test x"$enable_python" = x"no"; then
	AC_MSG_ERROR([--enable-python-doc requires --enable-python])
fi

if test x"$enable_python_doc" = x"yes" ; then
	AC_PATH_PROG([EPYDOC], epydoc)
	if test x"$EPYDOC" = x ; then
		AC_MSG_ERROR([epydoc (http://epydoc.sourceforge.net) is required to build Python documentation])
	fi
fi
AM_CONDITIONAL([ENABLE_PYTHON_DOC], test x"$enable_python_doc" = x"yes")

dnl Enable site copy/generation
AC_MSG_CHECKING([whether site should be copied])
AC_ARG_ENABLE([site-copy],
	      AS_HELP_STRING([--enable-site-copy], [Enable site copy (default: disable)]),
	      enable_site_copy="$enableval",
	      enable_site_copy=no)
AC_MSG_RESULT([$enable_site_copy])

AC_MSG_CHECKING([whether site should be generated])
AC_ARG_ENABLE([site-gen],
	      AS_HELP_STRING([--enable-site-gen], [Enable site generation (default: disable)]),
	      enable_site_gen="$enableval",
	      enable_site_gen=no)
AC_MSG_RESULT([$enable_site_gen])

if test x"$enable_site_copy" = x"yes" -a x"$enable_site_gen" = x"yes"; then
	AC_MSG_ERROR([--enable-site-copy and --enable-site-gen are mutually exclusive options])
fi

AC_ARG_VAR([JEKYLL], [Path to jekyll])
AC_ARG_VAR([RSYNC], [Path to rsync])
if test x"$enable_site_copy" = x"yes"; then
	AC_PATH_PROG([RSYNC], rsync)
	if test x"$RSYNC" = x; then
		AC_MSG_ERROR([rsync is required to copy site])
	fi
fi

if test x"$enable_site_gen" = x"yes"; then
	AC_PATH_PROG([JEKYLL], jekyll)
	if test x"$JEKYLL" = x; then
		AC_MSG_ERROR([jekyll (http://jekyllrb.com/) is required to generate site])
	fi
fi

if test x"$enable_site_copy" = x"yes" -o x"$enable_site_gen" = x"yes"; then
	AC_MSG_CHECKING([for site install directory])
	AC_ARG_WITH([site-install-dir],
		    AS_HELP_STRING([--with-site-install-dir=dir], [Specify site installation directory]),
		    SITE_INSTALL_DIR=`eval echo $withval`,
		    SITE_INSTALL_DIR=)
	if test x"$SITE_INSTALL_DIR" = x; then
		AC_MSG_ERROR([Site install directory not specified])
	fi
	AC_MSG_RESULT([$SITE_INSTALL_DIR])
	AC_SUBST([SITE_INSTALL_DIR])
fi
AM_CONDITIONAL([ENABLE_SITE], test x"$enable_site_copy" = x"yes" -o x"$enable_site_gen" = x"yes")
AM_CONDITIONAL([ENABLE_SITE_COPY], test x"$enable_site_copy" = x"yes")
AM_CONDITIONAL([ENABLE_SITE_GEN], test x"$enable_site_gen" = x"yes")

dnl Built sources
GENERATED_FILE=misc/generated-file.txt
AC_SUBST_FILE([GENERATED_FILE])

dnl Output
AC_CONFIG_FILES([
		 Makefile
		 doc/_config.yml
		 doc/Makefile
		 doc/api/Makefile
		 doc/api/c/Makefile
		 doc/api/c/doxygen.conf
		 doc/api/python/Makefile
		 doc/api/ruby/Makefile
		 examples/Makefile
		 examples/c/Makefile
		 examples/python/Makefile
		 examples/ruby/Makefile
		 include/pinktrace/about.h
		 include/pinktrace/bitness.h
		 python/Makefile
		 python/pinktrace/Makefile
		 ruby/Makefile
		 pkg-config/Makefile
		 src/Makefile
		 src/easy/Makefile
		 src/freebsd/Makefile
		 src/freebsd/amd64/Makefile
		 src/linux/Makefile
		 src/linux/x86/Makefile
		 src/linux/x86_64/Makefile
		 src/linux/ia64/Makefile
		 src/linux/powerpc/Makefile
		 src/linux/arm/Makefile
		 tests/Makefile
		 tests/easy/Makefile])
AC_OUTPUT

dnl User message
echo
echo "pinktrace was configured successfully!"
echo "-----------------------------------------------------"
echo "version:                   ${VERSION_FULL}"
echo "slot:                      ${PINKTRACE_PC_SLOT}"
echo "opsys:                     ${opsys}"
echo "arch:                      ${arch}"
echo "supported bitness count:   ${PINKTRACE_BITNESS_COUNT_SUPPORTED}"
echo "default bitness:           ${PINKTRACE_BITNESS_DEFAULT}"
echo "ipv6 support:              ${WANT_IPV6}"
echo "netlink support:           ${WANT_NETLINK}"
echo "pinktrace-easy:            ${WANT_EASY}"
echo "libcheck:                  ${have_libcheck}"
echo "doxygen:                   ${HAVE_DOXYGEN}"
echo "python bindings:           ${enable_python}"
echo "python install directory:  ${PYTHON_INSTALL_DIR}"
echo "python api documentation:  ${enable_python_doc}"
echo "ruby bindings:             ${enable_ruby}"
echo "ruby install directory:    ${RUBY_INSTALL_DIR}"
echo "ruby api documentation:    ${enable_ruby_doc}"
echo "site copy:                 ${enable_site_copy}"
echo "site generation:           ${enable_site_gen}"
echo "executables:"
echo "  doxygen:                 doxygen"
echo "  python:                  ${PYTHON}"
echo "  epydoc:                  ${EPYDOC}"
echo "  ruby:                    ${RUBY}"
echo "  rdoc:                    ${RDOC}"
echo "  rsync:                   ${RSYNC}"
echo "  jekyll:                  ${JEKYLL}"
echo "-----------------------------------------------------"
echo "Now run make (or gmake on FreeBSD) to build pinktrace"
echo "-----------------------------------------------------"
