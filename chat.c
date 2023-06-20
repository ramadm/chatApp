/**
 * Simple chat application
 * Usage ./chat to host, ./chat <IP> or ./chat localhost to connect as a client
 * 
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
#include <unistd.h>

#ifdef __STDC_NO_THREADS__
#error Program cannot be built without multithreading.
#endif

#define USAGE_STR "Usage: Run without command line arguments to host a server.\nType " \
        "./chat <IP> or ./chat localhost to connect as a client.\n"
#define MYPORT "3490" // arbitrarily picked this
#define BUFFER_SIZE 1024
#define BACKLOG 5

/* Receive and print incoming messages. Runs continuously until connection is closed. */
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
        printf("Them: %s", buf);
    }

    return 0;
}


int main(int argc, char *argv[]) {
    if (argc > 2) {
        fprintf(stderr, "%s", USAGE_STR);
        return 1;
    }

    int local_fd, con_fd;
    struct addrinfo hints, *res;
    struct sockaddr_storage their_addr;
    socklen_t addr_size;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // can use either v4 or v6
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // fill in my IP for me

    // host a server and wait for a connection
    if (argc == 1) {
        // fill res with addrinfo for localhost:MYPORT
        if (getaddrinfo(NULL, MYPORT, &hints, &res) < 0) {
            fprintf(stderr, "Failed to get addr info with errno code %d.\n", errno);
            return 1;
        }

        // params: (IPv4 or IPv6, stream or datagram, TCP or UDP)
        local_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (local_fd == -1) {
            fprintf(stderr, "Failed to get socket with errno code %d.\n", errno);
            return 1;
        }

        // associates the fd w/ a port, only needed for the server
        if (bind(local_fd, res->ai_addr, res->ai_addrlen) == -1) {
            fprintf(stderr, "Failed bind() call with errno code %d.\n", errno);
            return 1;
        }

        // start listening for incoming connections (woohoo!)
        if (listen(local_fd, BACKLOG) == -1) {
            fprintf(stderr, "Failed on listen() call with errno code %d.\n", errno);
            return 1;
        }

        addr_size = sizeof(their_addr);
        con_fd = accept(local_fd, (struct sockaddr *)&their_addr, &addr_size);
        if (con_fd == -1) {
            fprintf(stderr, "Failed to accept connection with errno code %d.\n", errno);
            return 1;
        }

    // connect to the specified ip as a client    
    } else {
        char* addr_str = argv[1];
        if (strncmp(argv[1], "localhost", sizeof("localhost")) == 0) {
            addr_str = NULL;
        }
        
        // fill res with addrinfo for localhost:MYPORT
        if (getaddrinfo(addr_str, MYPORT, &hints, &res) < 0) {
            fprintf(stderr, "Failed to get addr info with errno code %d.\n", errno);
            return 1;
        }

        // params: (IPv4 or IPv6, stream or datagram, TCP or UDP)
        con_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (con_fd == -1) {
            fprintf(stderr, "Failed to get socket with errno code %d.\n", errno);
            return 1;
        }

        if (connect(con_fd, res->ai_addr, res->ai_addrlen) == -1) {
            fprintf(stderr, "Failed to connect to host with errno code %d.\n", errno);
            return 1;
        }
    }

    // At this point we have established a connection! Start running chat service.
    // Run a separate thread to receive messages so we can send and recv simultaneously!
    thrd_t t_recv;
    thrd_create(&t_recv, run_recv, &con_fd);

    // Read a line from the console
    char buffer[BUFSIZ];
    int len;
    int bytes_sent;
    while (fgets(buffer, sizeof(buffer), stdin) != 0) {
        len = strlen(buffer);
        bytes_sent = send(con_fd, buffer, len, 0);
        if (bytes_sent == -1) {
            fprintf(stderr, "Failed to send message with errno code %d.\n", errno);
        } else if (bytes_sent < len) {
            // TODO: send rest of message or w/e
        }
    }

    // Wait for thread to close
    int t_res;
    thrd_join(t_recv, &t_res);
    printf("Thread exited with code %d\n", t_res);

    return 0;
}