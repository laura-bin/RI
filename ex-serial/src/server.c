/****************************************************************************************
* Exercice sur la serialisation
* =============================
*
* Server implementing the echo protocol (RFC 862)
* & using the TCP library & the serial library
*
* RI 2020 - Laura Binacchi - Fedora 32
****************************************************************************************/

#include <errno.h>
#include <netdb.h>          // gai_strerror
#include <signal.h>         // sigaction
#include <stdio.h>
#include <sys/wait.h>       // WNOHANG
#include <unistd.h>         // fork

#include "constants.h"
#include "data.h"
#include "tcp-util.h"

#define BACKLOG 10  // amount of pending connections allowed

// Save errno after a child death
void sigchld_handler(int s) {
    (void)s;
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

int main() {
    int sockfd, newfd;                  // listen on sockfd, new connection on newfd
    char client_ip[INET6_ADDRSTRLEN];   // string containing a human readable ip address of the client
    struct data_node *node;             // data linked list received
    uint16_t list_size;                 // number of nodes contained in the data list
    ssize_t bytes_received;             // amount of bytes received
    struct sigaction sa;
    
    // open a passive connection
    if ((sockfd = server_listen(PORT, BACKLOG)) < 0) {
        if (sockfd == -1) fprintf(stderr, "[server] creating the socket: %s\n", gai_strerror(errno));
        else if (sockfd == -2) perror("[server] overriding socket options");
        else if (sockfd == -3) perror("[server] binding\n");
        else if (sockfd == -4) perror("[server] listening");
        return 1;
    }

    // function pointer to call on child death 
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    // use the defined handler on sigchld signal (when a child dies)
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("[server] sigaction");
        return 1;
    }

    printf("[server] waiting for connections...\n");

    while(1) {
        // get a new socket file descriptor for the incoming connection
        newfd = server_accept(sockfd, client_ip);
        if (newfd < 0) {
            perror("[server] accepting incoming connection");
            continue;
        }

        printf("[server] connection received from %s...\n", client_ip);
        
        // open a child process
        if (!fork()) {
            disconnect(sockfd); // child doesn't need the listener

            // receive the data list
            bytes_received = receive_list(newfd, &node, &list_size);

            // stop if an error occured
            if (bytes_received < 0) {
                perror("[server] receiving data");
                disconnect(newfd);
                return 1;
            }

            // send back an acknowledgement to the client
            if (send_ack(newfd, bytes_received)) {
                perror("[server] sending acknowledgement");
            }

            // print the data received
            printf("[server:%s]\t%u nodes received (%ld bytes)\n",
                        client_ip, list_size, bytes_received);
            print_list(node, list_size);

            // close the socket
            printf("[server:%s] closing\n", client_ip);
            disconnect(newfd);
            return 0;
        }
    }

    return 0;
}
