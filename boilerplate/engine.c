#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sched.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/ioctl.h>

#include "monitor_ioctl.h"

#define STACK_SIZE (1024 * 1024)

struct child_args {
    char id[32];
    char rootfs[256];
    char program[256];
};

static int monitor_fd = -1;

/* register container to kernel monitor */
void monitor_register(pid_t pid, const char *name)
{
    if (monitor_fd < 0)
        return;

    struct container_info ci;
    ci.pid = pid;
    strncpy(ci.name, name, sizeof(ci.name));

    ioctl(monitor_fd, MONITOR_IOC_REGISTER, &ci);
}

/* unregister container */
void monitor_unregister(pid_t pid, const char *name)
{
    if (monitor_fd < 0)
        return;

    struct container_info ci;
    ci.pid = pid;
    strncpy(ci.name, name, sizeof(ci.name));

    ioctl(monitor_fd, MONITOR_IOC_UNREGISTER, &ci);
}

/* child process (container) */
static int container_child(void *arg)
{
    struct child_args *a = arg;

    sethostname(a->id, strlen(a->id));

    if (chroot(a->rootfs) != 0) {
        perror("chroot");
        exit(1);
    }

    chdir("/");

    char *const argv[] = { a->program, NULL };
    execvp(argv[0], argv);

    perror("exec");
    return 1;
}

int main(int argc, char *argv[])
{
    if (argc < 5) {
        printf("Usage: %s run <id> <rootfs> <program>\n", argv[0]);
        return 1;
    }

    monitor_fd = open("/dev/monitor", O_RDWR);

    struct child_args args;
    strncpy(args.id, argv[2], sizeof(args.id));
    strncpy(args.rootfs, argv[3], sizeof(args.rootfs));
    strncpy(args.program, argv[4], sizeof(args.program));

    void *stack = malloc(STACK_SIZE);
    if (!stack) {
        perror("malloc");
        return 1;
    }

    pid_t pid = clone(container_child,
                      stack + STACK_SIZE,
                      CLONE_NEWUTS | CLONE_NEWPID | SIGCHLD,
                      &args);

    if (pid < 0) {
        perror("clone");
        return 1;
    }

    printf("[%s] started with pid %d\n", args.id, pid);

    monitor_register(pid, args.id);

    waitpid(pid, NULL, 0);

    monitor_unregister(pid, args.id);

    return 0;
}
