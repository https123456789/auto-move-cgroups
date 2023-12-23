#ifndef _AUTO_MOVE_CGROUPS_H_
#define _AUTO_MOVE_CGROUPS_H_

#include <sys/types.h>
#include "config.h"

extern volatile char need_exit;

// CGroup querying/manipulation
struct config init_libcgroup(void);
void deinit_libcgroup(struct config c);
int belongs_to_cgroup(pid_t pid);

// Signal management
void termination_handler(int sigcode);
void setup_interrupts(void);

// Netlink functions
int nl_socket(void);
int nl_connect(int nl_sock);
int set_proc_events_listening(int nl_sock, char enable);
int handle_process_events(int nl_sock, struct config *config);

// Process management functions
int place_process(int pid, int tid, struct config *config);

#endif // _AUTO_MOVE_CGROUPS_H_
