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

#define MYPORT "3490" // arbitrarily picked this
#define BUFFER_SIZE 1024

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
    }

    // params: (IPv4 or IPv6, stream or datagram, TCP or UDP)
    host_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (host_fd == -1) {
        fprintf(stderr, "%s", "Failed to get socket\n");
        fprintf(stderr, "Value of error: %d\n", errno);
    }

    // no need to bind if we're connect()ing!
    connect(host_fd, res->ai_addr, res->ai_addrlen);

    // now if all went well we have a socket fd to talk on !!
    // attempt to receive a message
    char buf[BUFFER_SIZE];
    int bytes_recv;
    while ((bytes_recv = recv(host_fd, buf, BUFFER_SIZE, 0)) != 0) {
        buf[bytes_recv] = '\0';
        if (bytes_recv == -1) {
                fprintf(stderr, "%s", "Failed on recv() call\n");
                fprintf(stderr, "Value of error: %d\n", errno);
        }
        printf("%s", buf);
    }
    // TODO: whatever other freeing needs to be done
    freeaddrinfo(res);



    return 0;
}