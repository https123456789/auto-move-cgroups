#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "auto-move-cgroups.h"
#include "config.h"

int place_process(int pid, int tid, struct config *config) {
    char exe_path[4096];
    char exe_true_path[4096];
    int exe_filterable = 1;

    bzero(exe_true_path, 4096); // readlink doesn't set a null terminator

    snprintf(
        exe_path,
        4096,
        "/proc/%d/exe",
        tid
    );

    if (readlink(exe_path, exe_true_path, 4096) < 0) {
        snprintf(exe_true_path, 4096, "readlink: %s", strerror(errno));
        exe_filterable = 0;
    }

    if (!exe_filterable) {
        fprintf(
            stderr,
            "Can't place process (pid: %d, tid: %d) because readlink failed\n",
            pid,
            tid
        );
        return -1;
    }

    // Iterate through all of the cgroups specified in the config and attempt to match the true exe
    // path with one of the config targets
    struct config_group_def *gd = config->groups;

    while (gd != NULL) {
        if (strcmp(exe_true_path, gd->target) == 0) {
            break;
        }

        gd = gd->next;
    }

    // No matches were found
    if (gd == NULL) {
        return 1;
    }

    printf(
        "exec: pid=%d,tid=%d\n\texe: %s\n",
        pid,
        tid,
        exe_true_path
    );

    printf("\n\n%s\n\n", exe_true_path);

    cgroup_attach_task_pid(gd->cgroup, pid);

    return 0;
}
