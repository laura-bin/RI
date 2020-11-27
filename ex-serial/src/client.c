/****************************************************************************************
* Exercice sur la serialisation
* =============================
*
* Client implementing the echo protocol (RFC 862)
* & using the TCP library & the serial library
*
* arg : list size [1, UINT16_MAX]
*
* RI 2020 - Laura Binacchi - Fedora 32
****************************************************************************************/

#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "constants.h"
#include "data.h"
#include "tcp-util.h"

int main(int argc, char *argv[]) {
    char hostname[BUF_SIZE];        // server name or ip address (dot separated)
    int sockfd, rv;                 // socket file descriptor & return value
    struct data_node *head = NULL;  // first node of the data linked list
    struct data_node *node;         // node of the linked list used for iteration
    size_t list_size;               // number of elements contained in the list
    size_t i;

    // seed the random
    srand(time(NULL));

    // test the args
    if (argc != 3) {
        fprintf(stderr,"usage: %s hostname list_size\n", argv[0]);
        return 1;
    }
    
    // convert the args
    strncpy(hostname, argv[1], BUF_SIZE);
    list_size = strtoul(argv[2], NULL, 10);
    if (list_size < 1 || list_size > UINT16_MAX) {
        fprintf(stderr,"argument list_size must be an integer [1, %d]\n", UINT16_MAX);
        return 1;
    }

    // generate the data list
    head = node = create_node(0);
    for (i = 1; i < list_size; i++) {
        node->next = create_node(i);
        node = node->next;
    }

    print_list(head, list_size);

    // connect to the server
    sockfd = client_connect(hostname, PORT);
    if (sockfd < 0) {
        if (sockfd == -1) fprintf(stderr, "[client] creating the socket: %s\n", gai_strerror(errno));
        else if (sockfd == -2) perror("[client] connecting to the server");
        
        free_list(head);

        return 1;
    }

    // send the list
    if (send_list(sockfd, head, list_size)) {
        perror("[client] sending list");
        free_list(head);
        return 1;
    }

    free_list(head);

    // receive back message
    // rv = expect_data(sockfd, msg_received, strlen(msg_sent));
    // if (rv) {
    //     if (rv == -1) fprintf(stderr, "[client] connection closed by the server before receiving expected amount of data");
    //     else perror("[client] receiving data");
    //     return 1;
    // }

    // // print received message
    // msg_received[strlen(msg_sent)] = '\0';
    // printf("[client] message received: \"%s\"\n", msg_received);

    // close the connection
    disconnect(sockfd);

    return 0;
}
