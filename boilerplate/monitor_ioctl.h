#ifndef MONITOR_IOCTL_H
#define MONITOR_IOCTL_H

#include <linux/ioctl.h>

#define MAX_NAME 32

/* structure passed between engine and kernel module */
struct container_info {
    int pid;
    char name[MAX_NAME];
};

/* ioctl commands */
#define MONITOR_IOC_REGISTER   _IOW('m', 1, struct container_info)
#define MONITOR_IOC_UNREGISTER _IOW('m', 2, struct container_info)

#endif
