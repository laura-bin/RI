/****************************************************************************************
* Exercice sur les librairies
* ===========================
*
* Client implementing the echo protocol (RFC 862)
* & using the TCP library
*
* RI 2020 - Laura Binacchi - Fedora 32
****************************************************************************************/

#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>

#include "constants.h"
#include "tcp-util.h"

int main(int argc, char *argv[]) {
    char hostname[BUF_SIZE];        // server name or ip address (dot separated)
    char msg_sent[BUF_SIZE];        // message to send to the server
    char msg_received[BUF_SIZE];    // message reveived back from the server
    int sockfd, rv;                 // socket file descriptor & return value
    
    // test the params
    if (argc != 3) {
        fprintf(stderr,"usage: %s hostname message\n", argv[0]);
        return 1;
    }

    strncpy(hostname, argv[1], BUF_SIZE);

    // connect to the server
    sockfd = client_connect(hostname, PORT);
    if (sockfd < 0) {
        if (sockfd == -1) fprintf(stderr, "[client] creating the socket: %s\n", gai_strerror(errno));
        else if (sockfd == -2) perror("[client] connecting to the server");
        return 1;
    }

    // send message
    // TODO check if argv[2] > BUF_SIZE, here the message is truncated if too long
    strncpy(msg_sent, argv[2], BUF_SIZE);
    if (send_data(sockfd, msg_sent, strlen(msg_sent))) {
        perror("[client] sending message");
        return 1;
    }

    // receive back message
    rv = expect_data(sockfd, msg_received, strlen(msg_sent));
    if (rv) {
        if (rv == -1) fprintf(stderr, "[client] connection closed by the server before receiving expected amount of data");
        else perror("[client] receiving data");
        return 1;
    }

    // print received message
    msg_received[strlen(msg_sent)] = '\0';
    printf("[client] message received: \"%s\"\n", msg_received);

    // close the connection
    disconnect(sockfd);

    return 0;
}
