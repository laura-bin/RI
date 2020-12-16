/** *************************************************************************************
 * Exercice sur l'envoi de fichiers
 * ================================
 *
 * Client connecting to ftp server:
 *  - reveive the list of files available for download
 *  - then choose a file to download
 *
 * RI 2020 - Laura Binacchi - Fedora 32
 ****************************************************************************************/

#include <errno.h>
#include <netdb.h>              // gai_strerror
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "const.h"
#include "file.h"
#include "tcp-util.h"

int main(int argc, char *argv[]) {
    char hostname[BUF_SIZE];        // server name or ip address (dot separated)
    int sockfd;                     // socket file descriptor & return value
    struct dl_file *files = NULL;   // list of files received from the server
    uint16_t files_size;            // size of the list received from the server
    struct dl_file *file = NULL;    // file chosen by the user
    uint16_t chosen_file;           // number corresponding to the file chosen by the user (index in the list)
    char id_byte = ID_BYTE;         // protocol identification byte
    ssize_t rv;

    // seed the random
    srand(time(NULL));

    // test the args
    if (argc != 2) {
        fprintf(stderr,"usage: %s hostname\n", argv[0]);
        return EXIT_FAILURE;
    }

    // convert the arg
    strncpy(hostname, argv[1], BUF_SIZE);

    // connect to the server
    sockfd = client_connect(hostname, PORT);
    if (sockfd < 0) {
        if (sockfd == -1) fprintf(stderr, "[client] creating the socket: %s\n", gai_strerror(errno));
        else if (sockfd == -2) perror("[client] connecting to the server");
        return EXIT_FAILURE;
    }

    // send identification byte
    if (send_data(sockfd, &id_byte, 1)) {
        fprintf(stderr, "[client] error while sending the identification byte\n");
        return EXIT_FAILURE;
    }

    // receive the list of downloadable files
    rv = receive_list(sockfd, &files, &files_size);
    if (rv <= 0) {
        fprintf(stderr, "[client] error while receiving the list\n");
        return EXIT_FAILURE;
    }

    // print the list
    puts("\nList of files available for download:");
    print_list(files, files_size);

    // get the the file chosen by the user
    chosen_file = get_file_id(files_size);
    file = get_file_by_id(files, chosen_file);
    free_list(files);

    if (file == NULL) {
        fprintf(stderr, "[client] file index not found in the list");
        return EXIT_FAILURE;
    }

    // send it to the server
    rv = send_list(sockfd, file, 1);

    if (rv < 0) {
        fprintf(stderr, "[client] error while sending the chosen file\n");
        free_list(file);
        return EXIT_FAILURE;
    }

    // receive the file
    if (receive_file(sockfd, file->name)) {
        perror("[client] receiving file");
        free_list(file);
        return EXIT_FAILURE;
    }
    printf("[client] file %s successfully downloaded\n", file->name);
    free_list(file);

    // close the connection
    disconnect(sockfd);

    return EXIT_SUCCESS;
}
