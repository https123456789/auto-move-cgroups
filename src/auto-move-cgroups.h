#ifndef _AUTO_MOVE_CGROUPS_H_
#define _AUTO_MOVE_CGROUPS_H_

#include "config.h"

extern volatile char need_exit;

struct config init_libcgroup(void);
void deinit_libcgroup(struct config c);

// Signal management
void termination_handler(int sigcode);
void setup_interrupts(void);

// Netlink functions
int nl_socket(void);
int nl_connect(int nl_sock);
int set_proc_events_listening(int nl_sock, char enable);
int handle_process_events(int nl_sock);

#endif // _AUTO_MOVE_CGROUPS_H_
