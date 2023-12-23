#include <errno.h>
#include <linux/cn_proc.h>
#include <linux/connector.h>
#include <linux/netlink.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "auto-move-cgroups.h"

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
//
// The proc connector is a bit messy to work with; you have to send a message inside of a message
// inside of yet another message. The internet doesn't have a whole lot about the conenctor but I
// did manage to find a nice article (taken from the Wayback machine) that discusses it.
// https://nick-black.com/dankwiki/index.php/The_Proc_Connector_and_Socket_Filters
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

int handle_process_events(int nl_sock, struct config *config) {
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

        // The kernel views PIDs a bit different than programs in the userspace.
        // In the kernel, TGIDs are equivalent to PIDs in userspace and vice-versa.
        // Thus, we need to re-interpret the TGID and PID fields we recieve.
        //
        // A good explantation
        // https://stackoverflow.com/a/9306150/15566643
        switch (msg.proc_ev.what) {
            case PROC_EVENT_NONE:
                printf("Process event stream is open.\n");
                break;
            case PROC_EVENT_FORK:
                /*printf(
                    "fork: ppid=%d,ptid=%d -> pid=%d,tid=%d\n",
                    msg.proc_ev.event_data.fork.parent_tgid,
                    msg.proc_ev.event_data.fork.parent_pid,
                    msg.proc_ev.event_data.fork.child_tgid,
                    msg.proc_ev.event_data.fork.child_pid
                );*/
                break;
            case PROC_EVENT_EXEC:
                place_process(
                    msg.proc_ev.event_data.exec.process_tgid,
                    msg.proc_ev.event_data.exec.process_pid,
                    config
                );
                break;
            // We only care about when new processes are spawned, so we only handle exec and fork
            // events.
            default:
                break;
        }
    }

    return 0;
}
