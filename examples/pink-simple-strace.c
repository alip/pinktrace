/* vim: set cino= fo=croql sw=8 ts=8 sts=0 noet cin fdm=syntax : */

/**
 * \file
 *
 * Example \ref pink-simple-strace-linux.c "pink-simple-strace-linux.c" .
 **/

/**
 * \example pink-simple-strace-linux.c
 *
 * Simple strace like program example written with pinktrace for Linux.
 **/

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <pinktrace/pink.h>

struct process {
	pid_t pid;
	short abi;
	bool insyscall;
	struct pink_regset *regs;
};

/* Utility functions */
static void
process_update(struct process *proc)
{
	int r;

	/*
	 * Update process registers and system call ABI indicator, ie
	 * "personality".
	 */
	if ((r = pink_regset_fill(proc->pid, proc->regs)) < 0 ||
	    (r = pink_read_abi(proc->pid, proc->regs, &proc->abi)) < 0)
	{
		errno = -r;
		perror("pink_regset_fill");
	}
}

static void
print_ret(struct process *proc)
{
	int r;
	long ret;
	int err;

	if ((r = pink_read_retval(proc->pid, proc->regs, &ret, &err)) < 0) {
		errno = -r;
		perror("pink_read_retval");
		return;
	}

	if (err)
		printf("= %d (%s)", -err, strerror(-err));
	else
		printf("= %ld", ret);
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
decode_open(struct process *proc)
{
	int r;
	long addr, flags;
	char buf[PATH_MAX];

	if ((r = pink_read_argument(proc->pid, proc->regs, 0, &addr)) < 0) {
		errno = -r;
		perror("pink_read_argument");
		return;
	}
	if ((r = pink_read_string(proc->pid, proc->regs, addr, buf, PATH_MAX)) < 0) {
		errno = -r;
		perror("pink_read_string");
		return;
	}
	if (!pink_read_argument(proc->pid, proc->regs, 1, &flags)) {
		errno = -r;
		perror("pink_read_argument");
		return;
	}

	printf("open(\"%s\", ", buf);
	print_open_flags(flags);
	fputc(')', stdout);
}

/* A very basic decoder for execve(2) system call. */
static void
decode_execve(struct process *proc)
{
	bool nil;
	unsigned i;
	int r;
	long arg;
	char buf[PATH_MAX];
	const char *sep;

	if ((r = pink_read_argument(proc->pid, proc->regs, 0, &arg)) < 0) {
		errno = -r;
		perror("pink_read_argument");
		return;
	}
	if ((r = pink_read_string(proc->pid, proc->regs, 0, buf, PATH_MAX)) < 0) {
		errno = -r;
		perror("pink_read_string");
		return;
	}
	if ((r = pink_read_argument(proc->pid, proc->regs, 1, &arg)) < 0) {
		errno = -r;
		perror("pink_read_argument");
		return;
	}

	printf("execve(\"%s\", [", buf);

	for (i = 0, nil = false, sep = "";;sep = ", ") {
		if ((r = pink_read_string_array(proc->pid, proc->regs,
						arg, ++i,
						buf, PATH_MAX, &nil)) < 0)
		{
			errno = -r;
			perror("pink_read_string_array");
			return;
		}
		printf("%s", sep);
		fputc('"', stdout);
		printf("%s", buf);
		fputc('"', stdout);

		if (nil) {
			printf("], envp[])");
			break;
		}
	}
}

/* A very basic decoder for bind() and connect() calls */
static void
decode_socketcall(struct process *proc, const char *scname)
{
	int fd, r;
	long addr, subcall;
	const char *path, *subname;
	char ip[100];
	struct pink_sockaddr sockaddr;

	subcall = -1;
	if (!strcmp(scname, "socketcall") &&
	    (r = pink_read_socket_subcall(proc->pid, proc->regs, true, &subcall)) < 0)
	{
		errno = -r;
		perror("pink_read_socket_subcall");
		return;
	}

	switch (subcall) {
	case -1:
	case PINK_SOCKET_SUBCALL_BIND:
	case PINK_SOCKET_SUBCALL_CONNECT:
		break;
	default:
		subname = pink_name_socket_subcall(subcall);
		if (!(!strcmp(subname, "bind") || !strcmp(subname, "connect"))) {
			/* Print the name only */
			printf("%s()", subname);
		}
		return;
	}

	if ((r = pink_read_socket_address(proc->pid, proc->regs, false, 1,
					  &fd, &sockaddr)) < 0)
	{
		errno = -r;
		perror("pink_read_socket_address");
		return;
	}

	printf("%s(%d, ", (subcall > 0) ? subname : scname, fd);

	switch (sockaddr.family) {
	case -1: /* NULL */
		printf("NULL");
		break;
	case AF_UNIX:
		printf("{sa_family=AF_UNIX, path=");
		path = sockaddr.u.sa_un.sun_path;
		if (path[0] == '\0' && path[1] != '\0') /* Abstract UNIX socket */
			printf("\"@%s\"}", ++path);
		else
			printf("\"%s\"}", path);
		break;
	case AF_INET:
		inet_ntop(AF_INET, &sockaddr.u.sa_in.sin_addr, ip, sizeof(ip));
		printf("{sa_family=AF_INET, sin_port=htons(%d), sin_sockaddr=inet_addr(\"%s\")}",
		       ntohs(sockaddr.u.sa_in.sin_port), ip);
		break;
	case AF_INET6:
		inet_ntop(AF_INET6, &sockaddr.u.sa6.sin6_addr, ip, sizeof(ip));
		printf("{sa_family=AF_INET6, sin_port=htons(%d), sin6_sockaddr=inet_addr(\"%s\")}",
		       ntohs(sockaddr.u.sa6.sin6_port), ip);
		break;
#if PINK_HAVE_NETLINK
	case AF_NETLINK:
		printf("{sa_family=AF_NETLINK, nl_pid=%d, nl_groups=%08x}",
		       sockaddr.u.nl.nl_pid, sockaddr.u.nl.nl_groups);
		break;
#endif /* PINKTRACE_HAVE_NETLINK */
	default: /* Unknown family */
		printf("{sa_family=???}");
		break;
	}

	printf(", %u)", sockaddr.length);
}

static void
handle_syscall(struct process *proc)
{
	int r;
	long scno;
	const char *scname;

	/* We get this event twice, one at entering a
	 * system call and one at exiting a system
	 * call. */
	if (proc->insyscall) {
		/* Exiting the system call, print the
		 * return value. */
		proc->insyscall = false;
		fputc(' ', stdout);
		print_ret(proc);
		fputc('\n', stdout);
	}
	else {
		/* Get the system call number and call
		 * the appropriate decoder. */
		proc->insyscall = true;
		if ((r = pink_read_syscall(proc->pid, proc->regs, &scno)) < 0) {
			errno = -r;
			perror("pink_read_syscall");
			return;
		}
		scname = pink_name_syscall(scno, proc->abi);
		if (!scname)
			printf("%ld()", scno);
		else if (!strcmp(scname, "open"))
			decode_open(proc);
		else if (!strcmp(scname, "execve"))
			decode_execve(proc);
		else if (!strcmp(scname, "socketcall") || !strcmp(scname, "bind") || !strcmp(scname, "connect"))
			decode_socketcall(proc, scname);
		else
			printf("%s()", scname);
	}
}

int
main(int argc, char **argv)
{
	int r, sig, status, exit_code;
	enum pink_event event;
	struct process proc;

	/* Parse arguments */
	if (argc < 2) {
		fprintf(stderr, "Usage: %s program [argument...]\n", argv[0]);
		return EXIT_FAILURE;
	}

	/* Initialize */
	if ((r = pink_regset_alloc(&proc.regs)) < 0) {
		errno = -r;
		perror("pink_regset_alloc");
		return EXIT_FAILURE;
	}

	/* Fork */
	if ((proc.pid = fork()) < 0) {
		perror("fork");
		return EXIT_FAILURE;
	}
	else if (!proc.pid) { /* child */
		/* Set up for tracing */
		if ((r = pink_trace_me()) < 0) {
			errno = -r;
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
		waitpid(proc.pid, &status, 0);
		event = pink_event_decide(status);
		assert(event == 0);

		/* Set up the tracing options. */
		if ((r = pink_trace_setup(proc.pid, PINK_TRACE_OPTION_SYSGOOD |
					  PINK_TRACE_OPTION_EXEC)) < 0) {
			errno = -r;
			perror("pink_trace_setup");
			pink_trace_kill(proc.pid, -1, SIGKILL);
			return EXIT_FAILURE;
		}

		/* Figure out the abi of the traced child. */
		process_update(&proc);
		printf("Child %d runs in ABI:%d\n", proc.pid, proc.abi);

		proc.insyscall = false;
		sig = exit_code = 0;
		for (;;) {
			/* At this point the traced child is stopped and needs
			 * to be resumed.
			 */
			if ((r = pink_trace_syscall(proc.pid, sig)) < 0) {
				perror("pink_trace_syscall");
				return (r == -ESRCH) ? 0 : 1;
			}
			sig = 0;

			/* Wait for the child */
			if ((proc.pid = waitpid(proc.pid, &status, 0)) < 0) {
				perror("waitpid");
				return (errno == ECHILD) ? 0 : 1;
			}

			if (WIFEXITED(status)) {
				exit_code = WEXITSTATUS(status);
				printf("Child %d exited normally with return code %d\n",
				       proc.pid, exit_code);
				break;
			} else if (WIFSIGNALED(status)) {
				exit_code = 128 + WTERMSIG(status);
				printf("Child %d exited with signal %d\n",
				       proc.pid, WTERMSIG(status));
				break;
			}

			/* Check the event. */
			event = pink_event_decide(status);
			switch (event) {
			case 0:
				process_update(&proc);
				handle_syscall(&proc);
				break;
			case PINK_EVENT_EXEC:
				/* Update abi */
				process_update(&proc);
				break;
			default:
				; /* Nothing */
			}
		}

		return exit_code;
	}
}
