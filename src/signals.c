#include <signal.h>
#include <stdio.h>
#include "auto-move-cgroups.h"

void termination_handler(int sigcode) {
    printf("\n"); // Print a newline so the output looks nicer
    printf("Recived termination request.\n");
    need_exit = 1;
}

// Signal handling
void setup_interrupts(void) {
    struct sigaction new_action, old_action;

    // Set up the structure to specify the new action
    new_action.sa_handler = termination_handler;
    sigemptyset (&new_action.sa_mask);
    new_action.sa_flags = 0;

    // SIGINT
    sigaction (SIGINT, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN) {
        sigaction (SIGINT, &new_action, NULL);
    }

    // SIGHUP
    sigaction (SIGHUP, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN) {
        sigaction (SIGHUP, &new_action, NULL);
    }

    // SIGTERM
    sigaction (SIGTERM, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN) {
        sigaction (SIGTERM, &new_action, NULL);
    }
}
