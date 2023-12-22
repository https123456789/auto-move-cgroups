#include <libcgroup.h>
#include <stdio.h>
#include <stdlib.h>
#include "auto-move-cgroups.h"
#include "config.h"

void init_libcgroup(void) {
    int ret;

    if ((ret = cgroup_init())) {
        fprintf(stderr, "cgroup_init failed: %s\n", cgroup_strerror(ret));
        exit(EXIT_FAILURE);
    }

    printf("Initialized cgroups\n");

    struct config c = generate_config();

    printf("Config specifies %ld groups.\n", c.group_count);

    exit(0);
}
