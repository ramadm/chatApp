/**
 * Listen for incoming connections and send some data to them
*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

#define MYPORT "3490" // TODO: does this matter?
#define BACKLOG 5 // # of pending connections in the queue

int main(void) {
    int status;
    int host_fd, client_fd;
    struct addrinfo hints, *res;
    struct sockaddr_storage their_addr;
    socklen_t addr_size;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // can use either v4 or v6
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // fill in my IP for me

    // fill res with localhost:myport info
    if (getaddrinfo(NULL, MYPORT, &hints, &res) < 0) {
        fprintf(stderr, "%s", "Failed to get address info\n");
        fprintf(stderr, "Value of error: %d\n", errno);
    }

    // params: (IPv4 or IPv6, stream or datagram, TCP or UDP)
    host_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (host_fd == -1) {
        fprintf(stderr, "%s", "Failed to get socket\n");
        fprintf(stderr, "Value of error: %d\n", errno);
    }

    // now that we have a socket fd, we still have to bind it to the port we specified before
    if (bind(host_fd, res->ai_addr, res->ai_addrlen) == -1) {
        fprintf(stderr, "%s", "Failed to bind port\n");
        fprintf(stderr, "Value of error: %d\n", errno);
    }

    // start listening for connections (woohoo!)
    if (listen(host_fd, BACKLOG) == -1) {
        fprintf(stderr, "%s", "Failed on listen call\n");
        fprintf(stderr, "Value of error: %d\n", errno);
    }

    // pretty weird how this works: we pass in sockfd and it gets a new fd for us (their addr), we 
    // cast &their_addr to a sockaddr* and then accept fills it in with the real sockaddr and sets
    // addr_size correctly (if smaller than the storage)
    addr_size = sizeof(their_addr);
    client_fd = accept(host_fd, (struct sockaddr *)&their_addr, &addr_size);
    if (client_fd == -1) {
        fprintf(stderr, "%s", "Failed to accept connection\n");
        fprintf(stderr, "Value of error: %d\n", errno);
    }

    // Read a line from the console
    char buffer[BUFSIZ];
    int len;
    int bytes_sent;
    while (fgets(buffer, sizeof(buffer), stdin) != 0) {
        len = strlen(buffer);
        bytes_sent = send(client_fd, buffer, len, 0);
        if (bytes_sent == -1) {
            fprintf(stderr, "%s", "Failed to send message\n");
            fprintf(stderr, "Value of error: %d\n", errno);
        } else if (bytes_sent < len) {
            // TODO: send rest of message or w/e
        }
    }

    return 0;

    // TODO: free whatever needs to be freed
}