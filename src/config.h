/**
 * ABOUT
 *
 * This is the config file. It is used to configure the various features of this program. One of
 * the more important things that this config file does is specify the cgroups that the program
 * will use.
 */

#ifndef _AUTO_MOVE_CGROUPS_CONFIG_H_
#define _AUTO_MOVE_CGROUPS_CONFIG_H_

#include <libcgroup.h>
#include <stdint.h>

struct config_group_def {
    const char *name;
    const char *target;
    struct config_group_def *next;
    struct cgroup *cgroup;
    struct cgroup_controller *cpu_controller;
};

struct config {
    struct config_group_def *groups;
    size_t group_count;
};

struct config generate_config(void);

#endif // _AUTO_MOVE_CGROUPS_CONFIG_H_
