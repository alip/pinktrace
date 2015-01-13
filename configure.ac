dnl Program, version
AC_PREREQ(2.59)

dnl Versioning
dnl major: User interface changes
dnl minor: Major changes without UI breakage
dnl micro: Every other change increases micro version
dnl suffix: For prerelease, alpha, beta versions
m4_define([pinktrace_version_major],  [0])
m4_define([pinktrace_version_minor],  [9])
m4_define([pinktrace_version_micro],  [1])
m4_define([pinktrace_version_suffix], [])
m4_define([pinktrace_version_full],   [pinktrace_version_major.pinktrace_version_minor.pinktrace_version_micro[]pinktrace_version_suffix])
m4_define([pinktrace_version],        [pinktrace_version_full])
m4_define([pinktrace_lib_current],    [m4_eval(100 * pinktrace_version_major + pinktrace_version_minor)])
m4_define([pinktrace_lib_revision],   [pinktrace_version_micro])

AC_INIT([pinktrace], [pinktrace_version], [alip@exherbo.org], [pinktrace])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([pinktrace/pink.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE([dist-bzip2 no-dist-gzip std-options subdir-objects foreign])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

PINK_VERSION_MAJOR=pinktrace_version_major
PINK_VERSION_MINOR=pinktrace_version_minor
PINK_VERSION_MICRO=pinktrace_version_micro
PINK_VERSION_SUFFIX=pinktrace_version_suffix
PINK_VERSION_FULL=pinktrace_version_full
PINK_VERSION=pinktrace_version
PINK_VERSION_LIB_CURRENT=pinktrace_lib_current
PINK_VERSION_LIB_REVISION=pinktrace_lib_revision

AC_SUBST([PINK_VERSION_MAJOR])
AC_SUBST([PINK_VERSION_MINOR])
AC_SUBST([PINK_VERSION_MICRO])
AC_SUBST([PINK_VERSION_FULL])
AC_SUBST([PINK_VERSION_SUFFIX])
AC_SUBST([PINK_VERSION])
AC_SUBST([PINK_VERSION_LIB_CURRENT])
AC_SUBST([PINK_VERSION_LIB_REVISION])

dnl Git revision
GITHEAD=
GITVERSION=
if test -d "${GIT_DIR:-${ac_top_srcdir:-./}/.git}" ; then
	GITHEAD=`git describe 2>/dev/null`
	if test -z "${GITHEAD}" ; then
		GITHEAD=`git rev-parse --short HEAD`
	fi
	if test -n "`git diff-index -m --name-only HEAD`" ; then
		GITHEAD="${GITHEAD}-dirty"
	fi
fi
AC_MSG_CHECKING([for git head])
AC_ARG_WITH([git-head],
	AS_HELP_STRING([--with-git-head], [Specify Git head]),
	GITHEAD=`eval echo $withval`,
	GITHEAD=`eval echo $GITHEAD`)
AC_MSG_RESULT([$GITHEAD])
AC_SUBST([GITHEAD])

if test -n "${GITHEAD}" ; then
	GITVERSION="-${GITHEAD}"
fi
AC_SUBST([GITVERSION])

PINKTRACE_PC_VERSION="${PINK_VERSION_FULL}"
PINKTRACE_PC_SLOT="${PINK_VERSION_MAJOR}.${PINK_VERSION_MINOR}${PINK_VERSION_SUFFIX}"
AC_SUBST([PINKTRACE_PC_VERSION])
AC_SUBST([PINKTRACE_PC_SLOT])

AC_MSG_CHECKING([for pinktrace slot])
AC_MSG_RESULT([$PINKTRACE_PC_SLOT])

PINKTRACE_CFLAGS=
PINKTRACE_PC_CFLAGS=
PINKTRACE_PC_LIBS="-lpinktrace_${PINKTRACE_PC_SLOT}"

dnl remove the check for c++ and fortran compiler
m4_defun([_LT_AC_LANG_CXX_CONFIG], [:])
m4_defun([_LT_AC_LANG_F77_CONFIG], [:])

dnl toolchain checks
AC_USE_SYSTEM_EXTENSIONS
AC_PROG_CC_C99
if test x"$ac_cv_prog_cc_c99" = x"no"; then
	AC_MSG_ERROR([sydbox requires a C compiler that supports ISO C99!])
fi
AC_PROG_LD
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_SED
LT_INIT

AM_CONDITIONAL([HAVE_GNU_CC], [test x"$GCC" = x"yes"])
AM_CONDITIONAL([HAVE_GNU_LD], [test x"$with_gnu_ld" = x"yes"])

dnl check for supported operating system
AC_CANONICAL_HOST
AC_MSG_CHECKING([for supported operating system])
case "$host_os" in
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
			AC_MSG_ERROR([Linux kernel version $kernel_version is not supported by pinktrace])
		fi
		AC_MSG_RESULT([$kernel_version])
		;;
	*)
		AC_MSG_RESULT([NO!])
		AC_MSG_ERROR([Linux kernel version $kernel_version is not supported by pinktrace])
		;;
	esac
	;;
*)
	AC_MSG_RESULT([NO!])
	AC_MSG_ERROR([Operating system $host_os is not supported by pinktrace])
	;;
esac

dnl check for supported architecture
AC_MSG_CHECKING([for supported architecture])
case "$host_cpu" in
i[[3456]]86|pentium)
	arch=i386
	AC_DEFINE([I386], 1, [Define for the i386 architecture.])
	;;
x86?64*)
	AC_PREPROC_IFELSE([AC_LANG_PROGRAM([[#ifndef __ILP32__
# error not x32
#endif]], [[]])],[arch=x32],[arch=x86_64])
	if test "$arch" = "x86_64"; then
		AC_DEFINE([X86_64], 1, [Define for the 64bit AMD x86-64 architecture.])
	else
		AC_DEFINE([X32], 1, [Define for the 32bit AMD x86-64 architecture.])
	fi
	;;
powerpc*)
	arch=powerpc
	AC_DEFINE([POWERPC], 1, [Define for the PowerPC architecture.])
	AC_TRY_COMPILE(
[#ifndef __LP64__
 # error 32 bit
 #endif], [], ppc_bits=64, ppc_bits=32)
	if test "$ppc_bits" = "64"; then
		AC_DEFINE([POWERPC64], 1, [Define for the PowerPC64 architecture.])
	fi
	;;
arm*)
	arch=arm
	AC_DEFINE([ARM], 1, [Define for the ARM architecture.])
	;;
ia64)
	AC_MSG_RESULT([NO!])
	AC_MSG_ERROR([pinktrace ia64 support was disabled due to lack of testing])
	;;
*)
	AC_MSG_RESULT([NO!])
	AC_MSG_ERROR([Architecture $host_cpu is not supported by pinktrace])
	;;
esac
AC_MSG_RESULT([$arch])
AC_SUBST([arch])

if test x"$arch" = x"i386"; then
	PINK_ARCH_I386=1
else
	PINK_ARCH_I386=0
fi
if test x"$arch" = x"x86_64"; then
	PINK_ARCH_X86_64=1
else
	PINK_ARCH_X86_64=0
fi
if test x"$arch" = x"x32"; then
	PINK_ARCH_X32=1
else
	PINK_ARCH_X32=0
fi
if test x"$arch" = x"ia64"; then
	PINK_ARCH_IA64=1
else
	PINK_ARCH_IA64=0
fi
if test x"$arch" = x"powerpc"; then
	PINK_ARCH_POWERPC=1
else
	PINK_ARCH_POWERPC=0
fi
if test x"$arch" = x"powerpc64"; then
	PINK_ARCH_POWERPC64=1
else
	PINK_ARCH_POWERPC64=0
fi
if test x"$arch" = x"arm"; then
	PINK_ARCH_ARM=1
else
	PINK_ARCH_ARM=0
fi
AC_SUBST([PINK_ARCH_I386])
AC_SUBST([PINK_ARCH_X86_64])
AC_SUBST([PINK_ARCH_X32])
AC_SUBST([PINK_ARCH_IA64])
AC_SUBST([PINK_ARCH_POWERPC])
AC_SUBST([PINK_ARCH_POWERPC64])
AC_SUBST([PINK_ARCH_ARM])

if test x"$arch" = x"arm" ; then
	PINK_SYSCALL_INVALID="0xbad"
else
	PINK_SYSCALL_INVALID="0xbadca11"
fi
AC_SUBST([PINK_SYSCALL_INVALID])

dnl check for types
AC_TYPE_PID_T
AC_TYPE_SIZE_T

dnl check for sizes
AC_CHECK_SIZEOF([int])
AC_CHECK_SIZEOF([long])
AC_CHECK_SIZEOF([long long])
AC_CHECK_SIZEOF([short])
AC_CHECK_SIZEOF([int16_t])
AC_CHECK_SIZEOF([uint16_t])
AC_CHECK_SIZEOF([int32_t])
AC_CHECK_SIZEOF([uint32_t])
AC_CHECK_SIZEOF([int64_t])
AC_CHECK_SIZEOF([off_t])
AC_CHECK_SIZEOF([off64_t])

dnl check for pinktrace headers
AC_CHECK_HEADER([sys/types.h],  [], AC_MSG_ERROR([I need sys/types.h]))
AC_CHECK_HEADER([sys/wait.h],   [], AC_MSG_ERROR([I need sys/wait.h]))
AC_CHECK_HEADER([sys/ptrace.h], [], AC_MSG_ERROR([I need sys/ptrace.h]))
AC_CHECK_HEADER([sys/socket.h], [], AC_MSG_ERROR([I need sys/socket.h]))
AC_CHECK_HEADER([netinet/in.h], [], AC_MSG_ERROR([I need netinet/in.h]))
AC_CHECK_HEADER([sys/un.h],     [], AC_MSG_ERROR([I need sys/un.h]))
AC_CHECK_HEADERS([sys/reg.h sys/uio.h], [], [])

dnl check for functions
AC_CHECK_FUNCS([pipe2])

dnl check for system calls
AC_CHECK_DECL([SYS_tkill],  [PINK_HAVE_TKILL=1],  [PINK_HAVE_TKILL=0],  [#include <sys/syscall.h>])
AC_CHECK_DECL([SYS_tgkill], [PINK_HAVE_TGKILL=1], [PINK_HAVE_TGKILL=0], [#include <sys/syscall.h>])
AC_SUBST([PINK_HAVE_TKILL])
AC_SUBST([PINK_HAVE_TGKILL])

AC_CHECK_FUNCS([process_vm_readv],
	       [PINK_HAVE_PROCESS_VM_READV=1],
	       [AC_CHECK_DECL([__NR_process_vm_readv],
			      [PINK_HAVE_PROCESS_VM_READV=1],
			      [PINK_HAVE_PROCESS_VM_READV=0],
			      [#include <asm/unistd.h>])])
AC_CHECK_FUNCS([process_vm_writev],
	       [PINK_HAVE_PROCESS_VM_WRITEV=1],
	       [AC_CHECK_DECL([__NR_process_vm_writev],
			      [PINK_HAVE_PROCESS_VM_WRITEV=1],
			      [PINK_HAVE_PROCESS_VM_WRITEV=0],
			      [#include <asm/unistd.h>])])
AC_SUBST([PINK_HAVE_PROCESS_VM_READV])
AC_SUBST([PINK_HAVE_PROCESS_VM_WRITEV])

dnl check for types
AC_CHECK_TYPES([struct pt_all_user_regs, struct ia64_fpreg, struct ptrace_peeksiginfo_args],,,[#include <sys/ptrace.h>])

dnl We need a hack to include linux/ptrace.h after sys/ptrace.h on some systems.
AC_CHECK_HEADERS([linux/ptrace.h])
m4_define([include_linux_ptrace_h],
	  [#ifdef HAVE_LINUX_PTRACE_H
	   #ifdef __ia64__
	   #ifdef HAVE_STRUCT_IA64_FPREG
	   #define ia64_fpreg XXX_ia64_fpreg
	   #endif
	   #ifdef HAVE_STRUCT_PT_ALL_USER_REGS
	   #define pt_all_user_regs XXX_pt_all_user_regs
	   #endif
	   #endif
           #ifdef HAVE_STRUCT_PTRACE_PEEKSIGINFO_ARGS
           #define ptrace_peeksiginfo_args XXX_ptrace_peeksiginfo_args
           #endif
	   #include <linux/ptrace.h>
	   #ifdef __ia64__
	   #undef ia64_fpreg
	   #undef pt_all_user_regs
	   #endif
	   #ifdef HAVE_STRUCT_PTRACE_PEEKSIGINFO_ARGS
	   #undef ptrace_peeksiginfo_args
	   #endif
	   #endif])
m4_define([include_ptrace_h],
	  [#include <sys/ptrace.h>
	   include_linux_ptrace_h
	  ])

AC_CHECK_DECL([PTRACE_TRACEME],    [], [AC_MSG_ERROR([I need PTRACE_TRACEME])],    [include_ptrace_h])
AC_CHECK_DECL([PTRACE_CONT],       [], [AC_MSG_ERROR([I need PTRACE_CONT])],       [include_ptrace_h])
AC_CHECK_DECL([PTRACE_KILL],       [], [AC_MSG_ERROR([I need PTRACE_KILL])],       [include_ptrace_h])
AC_CHECK_DECL([PTRACE_SINGLESTEP], [], [AC_MSG_ERROR([I need PTRACE_SINGLESTEP])], [include_ptrace_h])
AC_CHECK_DECL([PTRACE_SYSCALL],    [], [AC_MSG_ERROR([I need PTRACE_SYSCALL])],    [include_ptrace_h])
AC_CHECK_DECL([PTRACE_ATTACH],     [], [AC_MSG_ERROR([I need PTRACE_ATTACH])],     [include_ptrace_h])
AC_CHECK_DECL([PTRACE_DETACH],     [], [AC_MSG_ERROR([I need PTRACE_DETACH])],     [include_ptrace_h])
AC_CHECK_DECL([PTRACE_PEEKUSER],   [], [AC_MSG_ERROR([I need PTRACE_PEEKUSER])],   [include_ptrace_h])
AC_CHECK_DECL([PTRACE_PEEKDATA],   [], [AC_MSG_ERROR([I need PTRACE_PEEKDATA])],   [include_ptrace_h])
AC_CHECK_DECL([PTRACE_POKEUSER],   [], [AC_MSG_ERROR([I need PTRACE_POKEUSER])],   [include_ptrace_h])
AC_CHECK_DECL([PTRACE_GETREGS],    [], [AC_MSG_ERROR([I need PTRACE_GETREGS])],    [include_ptrace_h])
AC_CHECK_DECL([PTRACE_SETREGS],    [], [AC_MSG_ERROR([I need PTRACE_SETREGS])],    [include_ptrace_h])

AC_CHECK_DECL([PTRACE_GETREGSET],         [PINK_HAVE_GETREGSET=1],         [PINK_HAVE_GETREGSET=0],         [include_ptrace_h])
AC_CHECK_DECL([PTRACE_SETREGSET],         [PINK_HAVE_SETREGSET=1],         [PINK_HAVE_SETREGSET=0],         [include_ptrace_h])
AC_CHECK_DECL([PTRACE_SETOPTIONS],        [PINK_HAVE_SETUP=1],             [PINK_HAVE_SETUP=0],             [include_ptrace_h])
AC_CHECK_DECL([PTRACE_O_TRACESYSGOOD],    [PINK_HAVE_OPTION_SYSGOOD=1],    [PINK_HAVE_OPTION_SYSGOOD=0],    [include_ptrace_h])
AC_CHECK_DECL([PTRACE_O_TRACEFORK],       [PINK_HAVE_OPTION_FORK=1],       [PINK_HAVE_OPTION_FORK=0],       [include_ptrace_h])
AC_CHECK_DECL([PTRACE_O_TRACEVFORK],      [PINK_HAVE_OPTION_VFORK=1],      [PINK_HAVE_OPTION_VFORK=0],      [include_ptrace_h])
AC_CHECK_DECL([PTRACE_O_TRACECLONE],      [PINK_HAVE_OPTION_CLONE=1],      [PINK_HAVE_OPTION_CLONE=0],      [include_ptrace_h])
AC_CHECK_DECL([PTRACE_O_TRACEEXEC],       [PINK_HAVE_OPTION_EXEC=1],       [PINK_HAVE_OPTION_EXEC=0],       [include_ptrace_h])
AC_CHECK_DECL([PTRACE_O_TRACEVFORKDONE],  [PINK_HAVE_OPTION_VFORK_DONE=1], [PINK_HAVE_OPTION_VFORK_DONE=0], [include_ptrace_h])
AC_CHECK_DECL([PTRACE_O_TRACEEXIT],       [PINK_HAVE_OPTION_EXIT=1],       [PINK_HAVE_OPTION_EXIT=0],       [include_ptrace_h])
AC_CHECK_DECL([PTRACE_O_TRACESECCOMP],    [PINK_HAVE_OPTION_SECCOMP=1],    [PINK_HAVE_OPTION_SECCOMP=0],    [include_ptrace_h])
AC_CHECK_DECL([PTRACE_O_EXITKILL],        [PINK_HAVE_OPTION_EXITKILL=1],   [PINK_HAVE_OPTION_EXITKILL=0],   [include_ptrace_h])
AC_CHECK_DECL([PTRACE_EVENT_FORK],        [PINK_HAVE_EVENT_FORK=1],        [PINK_HAVE_EVENT_FORK=0],        [include_ptrace_h])
AC_CHECK_DECL([PTRACE_EVENT_VFORK],       [PINK_HAVE_EVENT_VFORK=1],       [PINK_HAVE_EVENT_VFORK=0],       [include_ptrace_h])
AC_CHECK_DECL([PTRACE_EVENT_CLONE],       [PINK_HAVE_EVENT_CLONE=1],       [PINK_HAVE_EVENT_CLONE=0],       [include_ptrace_h])
AC_CHECK_DECL([PTRACE_EVENT_EXEC],        [PINK_HAVE_EVENT_EXEC=1],        [PINK_HAVE_EVENT_EXEC=0],        [include_ptrace_h])
AC_CHECK_DECL([PTRACE_EVENT_VFORK_DONE],  [PINK_HAVE_EVENT_VFORK_DONE=1],  [PINK_HAVE_EVENT_VFORK_DONE=0],  [include_ptrace_h])
AC_CHECK_DECL([PTRACE_EVENT_EXIT],        [PINK_HAVE_EVENT_EXIT=1],        [PINK_HAVE_EVENT_EXIT=0],        [include_ptrace_h])
AC_CHECK_DECL([PTRACE_EVENT_SECCOMP],     [PINK_HAVE_EVENT_SECCOMP=1],     [PINK_HAVE_EVENT_SECCOMP=0],     [include_ptrace_h])
AC_CHECK_DECL([PTRACE_EVENT_STOP],        [PINK_HAVE_EVENT_STOP=1],        [PINK_HAVE_EVENT_STOP=0],        [include_ptrace_h])
AC_CHECK_DECL([PTRACE_GETEVENTMSG],       [PINK_HAVE_GETEVENTMSG=1],       [PINK_HAVE_GETEVENTMSG=0],       [include_ptrace_h])
AC_CHECK_DECL([PTRACE_GETSIGINFO],        [PINK_HAVE_GETSIGINFO=1],        [PINK_HAVE_GETSIGINFO=0],        [include_ptrace_h])
dnl XXX musl-libc doesn't define SYSEMU constants but the functionality may be available.
AC_CHECK_DECL([PTRACE_SYSEMU],            [PINK_HAVE_SYSEMU=1],            [PINK_HAVE_SYSEMU=0],            [include_ptrace_h])
AC_CHECK_DECL([PTRACE_SYSEMU_SINGLESTEP], [PINK_HAVE_SYSEMU_SINGLESTEP=1], [PINK_HAVE_SYSEMU_SINGLESTEP=0], [include_ptrace_h])
dnl If PTRACE_SEIZE_DEVEL is available, we don't use PTRACE_SEIZE, PTRACE_INTERRUPT or PTRACE_LISTEN!
dnl Note: We don't include <sys/ptrace.h> for these checks because glibc may have a definition of PTRACE_SEIZE_DEVEL itself.
dnl For other libc's which we care about (read musl-libc) PTRACE_SEIZE_DEVEL was never defined anyway.
dnl (Judging by the fact that git log -SPINK_SEIZE_DEVEL --all returns nothing under musl.git)
AC_CHECK_DECL([PTRACE_SEIZE_DEVEL], [seize_in_development=1], [seize_in_development=0], [include_linux_ptrace_h])
if test x"$seize_in_development" = x"1"; then
	AC_MSG_WARN([PTRACE_SEIZE functionality in development, not using])
else
	AC_CHECK_DECL([PTRACE_SEIZE],     [PINK_HAVE_SEIZE=1],     [PINK_HAVE_SEIZE=0],     [include_ptrace_h])
	AC_CHECK_DECL([PTRACE_LISTEN],    [PINK_HAVE_LISTEN=1],    [PINK_HAVE_LISTEN=0],    [include_ptrace_h])
	AC_CHECK_DECL([PTRACE_INTERRUPT], [PINK_HAVE_INTERRUPT=1], [PINK_HAVE_INTERRUPT=0], [include_ptrace_h])
fi

case x"${PINK_HAVE_SEIZE}${PINK_HAVE_INTERRUPT}${PINK_HAVE_LISTEN}" in
x111)
	PTRACE_SUPPORTS_SEIZE=1
	;;
*)
	PTRACE_SUPPORTS_SEIZE=0
	;;
esac
AC_SUBST([PTRACE_SUPPORTS_SEIZE])
AC_DEFINE_UNQUOTED([PTRACE_SUPPORTS_SEIZE], [$PTRACE_SUPPORTS_SEIZE], [ptrace() seize support])

AC_SUBST([PINK_HAVE_GETREGSET])
AC_SUBST([PINK_HAVE_SETREGSET])
AC_SUBST([PINK_HAVE_SETUP])
AC_SUBST([PINK_HAVE_OPTION_SYSGOOD])
AC_SUBST([PINK_HAVE_OPTION_FORK])
AC_SUBST([PINK_HAVE_OPTION_VFORK])
AC_SUBST([PINK_HAVE_OPTION_CLONE])
AC_SUBST([PINK_HAVE_OPTION_EXEC])
AC_SUBST([PINK_HAVE_OPTION_VFORK_DONE])
AC_SUBST([PINK_HAVE_OPTION_EXIT])
AC_SUBST([PINK_HAVE_OPTION_SECCOMP])
AC_SUBST([PINK_HAVE_OPTION_EXITKILL])
AC_SUBST([PINK_HAVE_EVENT_FORK])
AC_SUBST([PINK_HAVE_EVENT_VFORK])
AC_SUBST([PINK_HAVE_EVENT_CLONE])
AC_SUBST([PINK_HAVE_EVENT_EXEC])
AC_SUBST([PINK_HAVE_EVENT_VFORK_DONE])
AC_SUBST([PINK_HAVE_EVENT_EXIT])
AC_SUBST([PINK_HAVE_EVENT_SECCOMP])
AC_SUBST([PINK_HAVE_EVENT_STOP])
AC_SUBST([PINK_HAVE_GETEVENTMSG])
AC_SUBST([PINK_HAVE_GETSIGINFO])
AC_SUBST([PINK_HAVE_SYSEMU])
AC_SUBST([PINK_HAVE_SYSEMU_SINGLESTEP])
AC_SUBST([PINK_HAVE_SEIZE])
AC_SUBST([PINK_HAVE_INTERRUPT])
AC_SUBST([PINK_HAVE_LISTEN])

dnl check for large file support
AC_SYS_LARGEFILE

dnl check for IPV6 support
AC_ARG_ENABLE([ipv6],
	      [AS_HELP_STRING([--disable-ipv6], [disable IPV6 support])],
	      [WANT_IPV6="$enableval"],
	      [WANT_IPV6="yes"])
if test x"$WANT_IPV6" = x"yes" ; then
	AC_CHECK_DECL([AF_INET6], [],
		      [AC_MSG_ERROR([I need AF_INET6 for IPV6])],
		      [#include <sys/types.h>
		       #include <sys/socket.h>
		       #include <netinet/in.h>])
	AC_CHECK_TYPE([struct sockaddr_in6], [],
		      [AC_MSG_ERROR([I need struct sockaddr_in6 for IPV6])],
		      [#include <sys/types.h>
		       #include <netinet/in.h>])
fi
AC_MSG_CHECKING([for IPV6 support])
AC_MSG_RESULT([$WANT_IPV6])
if test x"$WANT_IPV6" = x"yes"; then
	PINK_HAVE_IPV6=1
else
	PINK_HAVE_IPV6=0
fi
AC_SUBST([PINK_HAVE_IPV6])
AM_CONDITIONAL(PINK_HAVE_IPV6, test x"$PINK_HAVE_IPV6" = x"1")

dnl check for Netlink support
AC_CHECK_HEADER([linux/netlink.h],
		[AC_CHECK_TYPE([struct sockaddr_nl],
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
if test x"$WANT_NETLINK" = x"yes"; then
	PINK_HAVE_NETLINK=1
else
	PINK_HAVE_NETLINK=0
fi
AC_SUBST([PINK_HAVE_NETLINK])
AM_CONDITIONAL(PINK_HAVE_NETLINK, test x"$PINK_HAVE_NETLINK" = x"1")

dnl extra CFLAGS
PINKTRACE_WANTED_CFLAGS="${SYDBOX_WANTED_CFLAGS}"
for flag in $PINKTRACE_WANTED_CFLAGS ; do
	AX_CHECK_COMPILER_FLAGS([$flag], [PINKTRACE_CFLAGS="$PINKTRACE_CFLAGS $flag"],)
done
AC_SUBST([PINKTRACE_CFLAGS])

dnl pkg-config
AC_SUBST([PINKTRACE_PC_CFLAGS])
AC_SUBST([PINKTRACE_PC_LIBS])
AC_SUBST([PINKTRACE_PC_VERSION])
AC_SUBST([PINKTRACE_PC_SLOT])

dnl installed tests
AC_MSG_CHECKING([whether tests should be installed])
AC_ARG_ENABLE([installed-tests],
	      AS_HELP_STRING([--enable-installed-tests], [Install tests]),
	      [ENABLE_INSTALLED_TESTS=$enableval],
	      [ENABLE_INSTALLED_TESTS=no])
AC_MSG_RESULT([$ENABLE_INSTALLED_TESTS])
if test x"$ENABLE_INSTALLED_TESTS" = x"yes"; then
	if test x"$HAVE_CHECK" = x"no"; then
		AC_MSG_ERROR([Installed tests require check-$CHECK_REQ or newer])
	fi
fi
AM_CONDITIONAL([ENABLE_INSTALLED_TESTS], test "x$ENABLE_INSTALLED_TESTS" = x"yes")

dnl doxygen
AC_MSG_CHECKING([whether doxygen should be enabled])
AC_ARG_ENABLE([doxygen],
	      AS_HELP_STRING([--enable-doxygen], [Enable 'make doxygen' (developer docs)]),
	      [HAVE_DOXYGEN=$enableval
	       AC_MSG_RESULT([$enableval])],
	      [AC_MSG_RESULT([autodetect])
	       AC_CHECK_PROG(HAVE_DOXYGEN, [doxygen], [yes], [no])])
AC_SUBST([HAVE_DOXYGEN])
AM_CONDITIONAL([HAVE_DOXYGEN], test "x$HAVE_DOXYGEN" = "xyes")
AC_PATH_PROG([HAVE_DOT], [dot])

dnl built sources
GENERATED_FILE=misc/generated-file.txt
AC_SUBST_FILE([GENERATED_FILE])

dnl Python interface
AC_ARG_VAR([PYTHON], [path to the Python interpreter])
AC_MSG_CHECKING([whether to build the Python interface])
AC_ARG_ENABLE([python],
	      AS_HELP_STRING([--enable-python], [Enable Python interface (default: disable)]),
	      enable_python="$enableval",
	      enable_python=no)
AC_MSG_RESULT([$enable_python])

if test x"$enable_python" = x"yes" ; then
	AM_PATH_PYTHON([2.7.0])
	if test x"$PYTHON" = x ; then
		AC_MSG_ERROR([Python is required for --enable-python])
	fi

	PY_PREFIX=`$PYTHON -c 'import sys ; sys.stdout.write(sys.prefix)'`
	PYTHON_ABIFLAGS=`$PYTHON -c 'import sys ; sys.stdout.write(getattr(sys, "abiflags", ''))'`
	PYTHON_LIBS="-lpython${PYTHON_VERSION}${PYTHON_ABIFLAGS}"
	PYTHON_CFLAGS="-I$PY_PREFIX/include/python${PYTHON_VERSION}${PYTHON_ABIFLAGS}"

	AC_SUBST([PYTHON_ABIFLAGS])
	AC_SUBST([PYTHON_LIBS])
	AC_SUBST([PYTHON_CFLAGS])

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

dnl output
AC_CONFIG_FILES([Makefile
		 doc/Makefile
		 doc/api/Makefile
		 doc/api/c/Makefile
		 doc/api/c/doxygen.conf
		 doc/api/python/Makefile
		 docs/Makefile
		 pkg-config/Makefile
		 pinktrace/Makefile
		 pinktrace/about.h
		 pinktrace/system.h
		 pinktrace/linux/Makefile
		 pinktrace/linux/i386/Makefile
		 pinktrace/linux/x32/Makefile
		 pinktrace/linux/x86_64/Makefile
		 pinktrace/linux/ia64/Makefile
		 pinktrace/linux/powerpc/Makefile
		 pinktrace/linux/arm/Makefile
		 python/Makefile
		 python/pinktrace/Makefile])
AC_OUTPUT
