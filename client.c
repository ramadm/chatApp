/**
 * Connect to server and print the message you receive to the console
*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <threads.h>

#ifdef __STDC_NO_THREADS__
#error Program cannot be built without multithreading.
#endif

#define MYPORT "3490" // arbitrarily picked this
#define BUFFER_SIZE 1024

/* Attempts to receive inbound messages from the server continuously */
int run_recv(void *sock_fd_ptr) {
    int sock_fd = *(int *)sock_fd_ptr;
    char buf[BUFFER_SIZE];
    int bytes_recv;
    while ((bytes_recv = recv(sock_fd, buf, BUFFER_SIZE, 0)) != 0) {
        buf[bytes_recv] = '\0';
        if (bytes_recv == -1) {
            fprintf(stderr, "%s", "Failed on recv() call\n");
            fprintf(stderr, "Value of error: %d\n", errno);
        }
        printf("%s", buf);
    }

    return 0;
}

int main(void) {
    int status;
    int host_fd;
    struct addrinfo hints, *res;
    struct sockaddr_storage their_addr;
    socklen_t addr_size;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // can use either v4 or v6
    hints.ai_socktype = SOCK_STREAM; // stream vs. datagram
    hints.ai_flags = AI_PASSIVE; // fill in my IP for me

    // fill res with localhost:myport info
    if (getaddrinfo(NULL, MYPORT, &hints, &res) < 0) {
        fprintf(stderr, "%s", "Failed to get address info\n");
        fprintf(stderr, "Value of error: %d\n", errno);
        return 1;
    }

    // params: (IPv4 or IPv6, stream or datagram, TCP or UDP)
    host_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (host_fd == -1) {
        fprintf(stderr, "%s", "Failed to get socket\n");
        fprintf(stderr, "Value of error: %d\n", errno);
        return 1;
    }

    // no need to bind if we're connect()ing!
    connect(host_fd, res->ai_addr, res->ai_addrlen);

    // now if all went well we have a socket fd to talk on !!
    // attempt to receive a message
    thrd_t t;
    thrd_create(&t, run_recv, &host_fd);

    // Read a line from the console
    char buffer[BUFSIZ];
    int len;
    int bytes_sent;
    while (fgets(buffer, sizeof(buffer), stdin) != 0) {
        len = strlen(buffer);
        bytes_sent = send(host_fd, buffer, len, 0);
        if (bytes_sent == -1) {
            fprintf(stderr, "%s", "Failed to send message\n");
            fprintf(stderr, "Value of error: %d\n", errno);
        } else if (bytes_sent < len) {
            // TODO: send rest of message or w/e
        }
    }    

    // TODO: create separate send and receive threads so these can run simultaneously
    int t_res;
    thrd_join(t, &t_res);
    printf("Thread exited with code %d\n", t_res);

    // TODO: whatever other freeing needs to be done
    freeaddrinfo(res);

    return 0;
}