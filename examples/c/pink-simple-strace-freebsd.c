/**
 * @file
 *
 * Example @ref pink-simple-strace-freebsd.c "pink-simple-strace-freebsd.c"
 **/

/**
 * @example pink-simple-strace-freebsd.c
 *
 * Simple strace like program example written with pinktrace for FreeBSD.
 **/

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <pinktrace/pink.h>

#define MAX_STRING_LEN 128

struct child {
	pid_t pid;
	pink_bitness_t bitness;
	bool insyscall;
	bool inexecve;
	bool printret;
};

/* Utility functions */
static void
print_ret(long ret)
{
	if (ret >= 0)
		printf("= %ld", ret);
	else
		printf("= %ld (%s)", ret, strerror(-ret));
}

static void
print_open_flags(long flags)
{
	long aflags;
	bool found;

	found = true;

	/* Check out access flags */
	aflags = flags & 3;
	switch (aflags) {
	case O_RDONLY:
		printf("O_RDONLY");
		break;
	case O_WRONLY:
		printf("O_WRONLY");
		break;
	case O_RDWR:
		printf("O_RDWR");
		break;
	default:
		/* Nothing found */
		found = false;
	}

	if (flags & O_CREAT) {
		printf("%s | O_CREAT", found ? "" : "0");
		found = true;
	}

	if (!found)
		printf("%#x", (unsigned)flags);
}

/* A very basic decoder for open(2) system call. */
static void
decode_open(pid_t pid, pink_bitness_t bitness)
{
	long flags;
	char buf[MAX_STRING_LEN];

	if (!pink_decode_string(pid, bitness, 0, buf, MAX_STRING_LEN)) {
		perror("pink_decode_string");
		return;
	}
	if (!pink_util_get_arg(pid, bitness, 1, &flags)) {
		perror("pink_util_get_arg");
		return;
	}

	printf("open(\"%s\", ", buf);
	print_open_flags(flags);
	putchar(')');
}

/* A very basic decoder for execve(2) system call. */
static void
decode_execve(pid_t pid, pink_bitness_t bitness)
{
	bool nil;
	unsigned i;
	long arg;
	char buf[MAX_STRING_LEN];
	const char *sep;

	if (!pink_decode_string(pid, bitness, 0, buf, MAX_STRING_LEN)) {
		perror("pink_decode_string");
		return;
	}
	if (!pink_util_get_arg(pid, bitness, 1, &arg)) {
		perror("pink_util_get_arg");
		return;
	}

	printf("execve(\"%s\", [", buf);

	for (i = 0, nil = false, sep = "";;sep = ", ") {
		if (!pink_decode_string_array_member(pid, bitness, arg, ++i, buf, MAX_STRING_LEN, &nil)) {
			perror("pink_decode_string_array_member");
			return;
		}

		printf("%s\"%s\"", sep, buf);

		if (nil) {
			printf("], envp[])");
			break;
		}
	}
}

/* A very basic decoder for bind() and connect() calls */
static void
decode_socketcall(pid_t pid, pink_bitness_t bitness, const char *scname)
{
	long fd;
	char ip[100];
	pink_socket_address_t addr;

	if (!pink_decode_socket_address(pid, bitness, 1, &fd, &addr)) {
		perror("pink_decode_socket_address");
		return;
	}

	printf("%s(%ld, ", scname, fd);

	switch (addr.family) {
	case -1: /* NULL */
		printf("NULL");
		break;
	case AF_UNIX:
		printf("{sa_family=AF_UNIX, path=\"%s\"}", addr.u.sa_un.sun_path);
		break;
	case AF_INET:
		inet_ntop(AF_INET, &addr.u.sa_in.sin_addr, ip, sizeof(ip));
		printf("{sa_family=AF_INET, sin_port=htons(%d), sin_addr=inet_addr(\"%s\")}",
				ntohs(addr.u.sa_in.sin_port), ip);
		break;
#if PINK_HAVE_IPV6
	case AF_INET6:
		inet_ntop(AF_INET6, &addr.u.sa6.sin6_addr, ip, sizeof(ip));
		printf("{sa_family=AF_INET6, sin_port=htons(%d), sin6_addr=inet_addr(\"%s\")}",
				ntohs(addr.u.sa6.sin6_port), ip);
		break;
#endif
	default: /* Unknown family */
		printf("{sa_family=???}");
		break;
	}

	printf(", %u)", addr.length);
}

static void
handle_sigtrap(struct child *son)
{
	long scno, ret;
	const char *scname;

	/* We get this event twice, one at entering a
	 * system call and one at exiting a system
	 * call. */
	if (son->insyscall) {
		if (!pink_util_get_return(son->pid, &ret))
			err(1, "pink_util_get_return");
		if (son->inexecve) {
			son->inexecve = false;
			if (ret == 0) { /* execve was successful */
				/* Update bitness */
				son->bitness = pink_bitness_get(son->pid);
				if (son->bitness == PINK_BITNESS_UNKNOWN)
					err(1, "pink_bitness_get");
				printf(" = 0 (Updating the bitness of child %i to %s mode)\n",
					son->pid, pink_bitness_name(son->bitness));
				son->printret = false;
				return;
			}
		}
		son->insyscall = false;
		if (!son->printret) {
			son->printret = true;
			return;
		}
		/* Exiting the system call, print the
		 * return value. */
		putchar(' ');
		print_ret(ret);
		putchar('\n');
	}
	else {
		son->insyscall = true;
		/* Get the system call number and call
		 * the appropriate decoder. */
		if (!pink_util_get_syscall(son->pid, son->bitness, &scno))
			err(1, "pink_util_get_syscall");
		scname = pink_name_syscall(scno, son->bitness);
		assert(scname != NULL);

		if (!strcmp(scname, "execve"))
			son->inexecve = true;

		if (!strcmp(scname, "open"))
			decode_open(son->pid, son->bitness);
		else if (!strcmp(scname, "execve"))
			decode_execve(son->pid, son->bitness);
		else if (!strcmp(scname, "bind") || !strcmp(scname, "connect"))
			decode_socketcall(son->pid, son->bitness, scname);
		else
			printf("%s()", scname);
	}
}

int
main(int argc, char **argv)
{
	int sig, status, exit_code;
	struct child son;

	/* Parse arguments */
	if (argc < 2) {
		fprintf(stderr, "Usage: %s program [argument...]\n", argv[0]);
		return EXIT_FAILURE;
	}

	/* Fork */
	if ((son.pid = fork()) < 0) {
		perror("fork");
		return EXIT_FAILURE;
	}
	else if (!son.pid) { /* child */
		/* Set up for tracing */
		if (!pink_trace_me()) {
			perror("pink_trace_me");
			_exit(EXIT_FAILURE);
		}
		/* Stop and let the parent continue the execution. */
		kill(getpid(), SIGSTOP);

		++argv;
		execvp(argv[0], argv);
		perror("execvp");
		_exit(-1);
	}
	else {
		waitpid(son.pid, &status, 0);
		assert(WIFSTOPPED(status));
		assert(WSTOPSIG(status) == SIGSTOP);

		/* Figure out the bitness of the traced child. */
		son.bitness = pink_bitness_get(son.pid);
		if (son.bitness == PINK_BITNESS_UNKNOWN)
			err(1, "pink_bitness_get");
		printf("Child %i runs in %s mode\n", son.pid, pink_bitness_name(son.bitness));

		son.inexecve = son.insyscall = false;
		son.printret = true;
		sig = exit_code = 0;
		for (;;) {
			/* At this point the traced child is stopped and needs
			 * to be resumed.
			 */
			if (!pink_trace_syscall(son.pid, sig)) {
				perror("pink_trace_syscall");
				return (errno == ESRCH) ? 0 : 1;
			}
			sig = 0;

			/* Wait for the child */
			if ((son.pid = waitpid(son.pid, &status, 0)) < 0) {
				perror("waitpid");
				return (errno == ECHILD) ? 0 : 1;
			}

			/* Check the event */
			if (WIFSTOPPED(status)) {
				if (WSTOPSIG(status) == SIGTRAP)
					handle_sigtrap(&son);
				else {
					/* Child received a genuine signal.
					 * Send it to the child. */
					sig = WSTOPSIG(status);
				}
			}
			else if (WIFEXITED(status)) {
				exit_code = WEXITSTATUS(status);
				printf("Child %i exited normally with return code %d\n",
						son.pid, exit_code);
				break;
			}
			else if (WIFSIGNALED(status)) {
				exit_code = 128 + WTERMSIG(status);
				printf("Child %i exited with signal %d\n",
						son.pid, WTERMSIG(status));
				break;
			}
		}

		return exit_code;
	}
}
