/** *************************************************************************************
 * Exercice sur les librairies
 * ===========================
 *
 * Programmation d'une bibliotheque de fonctions destinees
 * aux communications TCP/IP (v4 & v6)
 *
 * RI 2020 - Laura Binacchi - Fedora 32
 ****************************************************************************************/

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "tcp-util.h"

/**
 * Gets the IPv4 or IPv6 address
 *
 * @param *sa: sockaddr structure
 *
 * @return the pointer to the sockaddr_in (IPv4) or sockaddr_in6 (IPv6) address
 */
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) return &(((struct sockaddr_in*)sa)->sin_addr);
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int client_connect(char *url, char* service) {
    struct addrinfo hints;          // socket hints: struct given to getaddrinfo
    struct addrinfo *server_info;   // server infos: linked list filled by get addrinfo
    struct addrinfo *p;             // linked list for iteration
    int sockfd;                     // server socket file descriptor
    int err;                        // error number returned by getaddrinfo

    // fill the server hints
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;		// IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;	// TCP

    // generate server informations from hints
    err = getaddrinfo(url, service, &hints, &server_info);
    if (err) {
        // set errno
        errno = err;
        return -1;
    }

    // loop through all the results and connect to the first we can
    for(p = server_info; p != NULL; p = p->ai_next) {

        // get the socket reference from the system
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
            continue;
        }

        // open an active connection to the remote host
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) < 0) {
            close(sockfd);
            continue;
        }

        // stop the loop on the first result
        break;
    }

    // free the linked list (servinfo & p)
    freeaddrinfo(server_info);

    // if p is NULL after the loop, no connection has been successful
    if (p == NULL) {
        return -2;
    }

    return sockfd;
}

int server_listen(char *service, int backlog) {
    struct addrinfo hints;          // socket hints: struct given to getaddrinfo
    struct addrinfo *server_info;   // server infos: linked list filled by get addrinfo
    struct addrinfo *p;             // linked list for iteration
    int sockfd;                     // server socket file descriptor
    int err;                        // error number returned by getaddrinfo
    int yes = 1;                    // override reuse address param

    // fill the hints
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;        // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;    // TCP protocol
    hints.ai_flags = AI_PASSIVE;	    // use my IP

    // generate server informations from hints
    err = getaddrinfo(NULL, service, &hints, &server_info);
    if (err) {
        // set errno
        errno = err;
        return -1;
    }

    // loop through all the results and bind to the first we can
    for(p = server_info; p != NULL; p = p->ai_next) {

        // get the socket reference from the system
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
            continue;
        }

        // override default socket options : allow to reuse the port
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))) {
            freeaddrinfo(server_info);
            return -2;
        }

        // associate the socket with a port on the local machine (for listening)
        if (bind(sockfd, p->ai_addr, p->ai_addrlen)) {
            close(sockfd);
            continue;
        }

        // stop the loop on the first result
        break;
    }

    // free the linked list (servinfo & p)
    freeaddrinfo(server_info);

    if (p == NULL) return -3;

    // open a passive connection
    if (listen(sockfd, backlog)) return -4;

    return sockfd;
}

int server_accept(int sockfd, char *out_client_ip) {
    // address information about the incoming connection
    struct sockaddr_storage incoming_addr;
    socklen_t sin_size = sizeof(struct sockaddr_storage);
    int newfd;  // client socket file descriptor

    newfd = accept(sockfd, (struct sockaddr *)&incoming_addr, &sin_size);
    if (newfd < 0) return -1;

    // network to presentation
    inet_ntop(incoming_addr.ss_family, get_in_addr((struct sockaddr *)&incoming_addr),
                out_client_ip, INET6_ADDRSTRLEN);

    return newfd;
}

int send_data(int sockfd, char *buffer, ssize_t length) {
    ssize_t bytes_sent;

    // while bytes remains, send them
    while (length > 0) {
        //printf("DEBUG [send_data] sending %ld bytes\n", length);

        bytes_sent = send(sockfd, buffer, length, 0);
        if (bytes_sent < 0) return -1;

        //printf("DEBUG [send_data] sent %ld of %ld bytes\n", bytes_sent, length);

        length -= bytes_sent;
        buffer += bytes_sent;
    }

    return 0;
}

ssize_t receive_data(int sockfd, char *out_buffer, ssize_t max_length) {
    return recv(sockfd, out_buffer, max_length, 0);
}

int expect_data(int sockfd, char *out_buffer, ssize_t length) {
    ssize_t bytes_read; // number of bytes read by receive_data

    // while bytes are still expected, read them
    while (length > 0) {
        // printf("DEBUG [expect_data] expecting %ld bytes\n", length);

        bytes_read = receive_data(sockfd, out_buffer, length);
        // if data was still expected & server has closed the connection
        if (bytes_read == 0) return -1;
        // if an error occured
        if (bytes_read < 0) return -2;

        // printf("DEBUG(expect_data): received %ld of %ld\n", bytes_read, length);

        out_buffer += bytes_read; // move the pointer to the next free space in the byte array
        length -= bytes_read;
    }

    return 0;
}

void disconnect(int sockfd) {
    close(sockfd);
}
