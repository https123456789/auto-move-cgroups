#include <libcgroup.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "auto-move-cgroups.h"
#include "config.h"

struct config init_libcgroup(void) {
    printf("Beginning init of cgroups\n");

    int ret;

    if ((ret = cgroup_init())) {
        fprintf(stderr, "cgroup_init failed: %s\n", cgroup_strerror(ret));
        exit(EXIT_FAILURE);
    }

    struct config c = generate_config();

    struct config_group_def *gd = c.groups;

    while (gd != NULL) {
        printf("\tGroup: %s -> %s\n", gd->name, gd->target);

        // Initialize the cgroup
        gd->cgroup = cgroup_new_cgroup(gd->name);
        gd->cpu_controller = cgroup_add_controller(gd->cgroup, "cpu");

        // Actually create the cgroup in the kernel
        cgroup_create_cgroup(gd->cgroup, 0);

        gd = gd->next;
    }

    printf("Initialized cgroups\n");

    return c;
}

void deinit_libcgroup(struct config c) {
    printf("Beginning deinit of cgroups\n");

    struct config_group_def *gd = c.groups;

    while (gd != NULL) {
        printf("\tDeinit group: %s\n", gd->name);

        cgroup_delete_cgroup(gd->cgroup, 0);
        cgroup_free_controllers(gd->cgroup);
        cgroup_free(&gd->cgroup);

        gd = gd->next;
    }

    printf("Deinitialized cgroups\n");
}
