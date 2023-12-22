#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "auto-move-cgroups.h"

volatile char need_exit = 0;

// Before I get lectured about using gotos, please consider the number of repetitive lines
// that I would need to write when I can simply jump to the proper wrapup code and exit.
int main() {
    int nl_sock;
    int exit_status = EXIT_SUCCESS;

    printf("Auto Move CGroups - Ben Landon\n");

    setup_interrupts();

    struct config config = init_libcgroup();

    nl_sock = nl_socket();

    if (nl_sock == -1) {
        return EXIT_FAILURE;
    }

    printf("Socket opened\n");

    if (nl_connect(nl_sock) == -1) {
        exit_status = EXIT_FAILURE;
        goto wrapup;
    }

    if (set_proc_events_listening(nl_sock, 1) == -1) {
        exit_status = EXIT_FAILURE;
        goto wrapup;
    }

    printf("Listening for events...\n");

    if (handle_process_events(nl_sock) == -1) {
        exit_status = EXIT_FAILURE;
        goto wrapup;
    }

    if (set_proc_events_listening(nl_sock, 0) == -1) {
        exit_status = EXIT_FAILURE;
        goto wrapup;
    }

    printf("Stopped listening for events...\n");

wrapup:
    printf("Exiting...\n");
    close(nl_sock);
    deinit_libcgroup(config);

    return exit_status;
}
