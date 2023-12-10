#include <errno.h>
#include <linux/cn_proc.h>
#include <linux/connector.h>
#include <linux/netlink.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static volatile char need_exit = 0;

static void termination_handler(int sigcode) {
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

// Attempts to open a netlink socket, returning either the fd or -1
int nl_socket(void) {
    int sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR);

    if (sock == -1) {
        perror("nl_socket");
        return -1;
    }

    return sock;
}

int nl_connect(int nl_sock) {
    struct sockaddr_nl sa_nl;

    sa_nl.nl_family = AF_NETLINK;

    // The netlink socket implementation does not allow people to send data to netlink groups
    // other than 1 by default.
    // (See https://docs.kernel.org/driver-api/connector.html#userspace-usage)
    sa_nl.nl_groups = CN_IDX_PROC;

    sa_nl.nl_pid = getpid();

    if (bind(nl_sock, (struct sockaddr*) &sa_nl, sizeof(sa_nl)) == -1) {
        perror("bind");
        return -1;
    }

    return 0;
}

// Subscribe to process events so we can be notified when a new process is spawned
int set_proc_events_listening(int nl_sock, char enable) {
    // Define the message structure we need to send the subscription request
    struct __attribute__ ((aligned(NLMSG_ALIGNTO))) {
        struct nlmsghdr nl_hdr; // Every netlink message has to start with a header
        struct __attribute__ ((__packed__)) {
            struct cn_msg cn_msg; // Our message data
            enum proc_cn_mcast_op cn_mcast; // The operation we want (either LISTEN or IGNORE)
        };
    } msg;

    memset(&msg, 0, sizeof(msg));

    // Setup the header
    msg.nl_hdr.nlmsg_len = sizeof(msg);
    msg.nl_hdr.nlmsg_pid = getpid();
    msg.nl_hdr.nlmsg_type = NLMSG_DONE;

    // Pass the index and value that correspond to process events and specify (yet) another length
    msg.cn_msg.id.idx = CN_IDX_PROC;
    msg.cn_msg.id.val = CN_VAL_PROC;
    msg.cn_msg.len = sizeof(enum proc_cn_mcast_op);

    // Either enable or disable the broadcasting of messages
    msg.cn_mcast = enable ? PROC_CN_MCAST_LISTEN : PROC_CN_MCAST_IGNORE;

    if (send(nl_sock, &msg, sizeof(msg), 0) == -1) {
        perror("send");
        return -1;
    }

    return 0;
}

int handle_process_events(int nl_sock) {
    struct __attribute__ ((aligned(NLMSG_ALIGNTO))) {
        struct nlmsghdr nl_hdr; // Every netlink message has to start with a header
        struct __attribute__ ((__packed__)) {
            struct cn_msg cn_msg; // The message data
            struct proc_event proc_ev; // The event data
                                       // For more info on the details of this struct, look at
                                       // the contents of /usr/include/linux/cn_proc.h
        };
    } msg;

    while (!need_exit) {
        int recv_res = recv(nl_sock, &msg, sizeof(msg), 0);
        if (recv_res == 0) {
            // Kernel side disconnected for some reason
            // Not much we can do other than note it and stop
            fprintf(stderr, "Kernel disconnected from socket early.\n");
            return 0;
        } else if (recv_res == -1) {
            if (errno == EINTR) continue;
            perror("recv");
            return -1;
        }

        printf("Got message\n");
    }

    return 0;
}

// Before I get lectured about using gotos, please consider the number of repetitive lines
// that I would need to write when I can simply jump to the proper wrapup code and exit.
int main() {
    int nl_sock;
    int exit_status = EXIT_SUCCESS;

    printf("Auto Move CGroups - Ben Landon\n");

    setup_interrupts();

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

    return exit_status;
}
