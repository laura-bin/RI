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
    int sockfd;                     // socket file descriptor
    struct dl_file *files = NULL;   // list of files received from the server
    uint16_t files_size;            // size of the list received from the server
    struct dl_file *file = NULL;    // file chosen by the user
    uint16_t chosen_file;           // number corresponding to the file chosen by the user (index in the list)
    char id_byte = ID_BYTE;         // protocol identification byte

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
        if (sockfd == ERR_TCP_CREATE_SOCK) {
            fprintf(stderr, "[client] creating the socket: %s\n", gai_strerror(errno));
        } else {
            perror("[client] connecting to the server");
        }
        return EXIT_FAILURE;
    }

    // send identification byte
    if (send_data(sockfd, &id_byte, 1)) {
        perror("[client] sending the identification byte");
        return EXIT_FAILURE;
    }

    // receive the list of downloadable files
    if (receive_list(sockfd, &files, &files_size) < 0) {
        fprintf(stderr, "[client] error while receiving the list\n");
        return EXIT_FAILURE;
    }

    // print the list
    puts("\nList of files available for download:");
    print_list(files, files_size);
    if (files == NULL) {
        return EXIT_FAILURE;
    }

    // get the the file chosen by the user
    chosen_file = get_chosen_file(files_size);
    file = get_file_by_index(files, chosen_file-1);
    free_list(files);

    if (file == NULL) {
        fprintf(stderr, "[client] file index not found in the list");
        return EXIT_FAILURE;
    }

    // send it to the server
    if (send_list(sockfd, file, 1) < 0) {
        fprintf(stderr, "[client] error while sending the chosen file\n");
        free_list(file);
        return EXIT_FAILURE;
    }

    // receive the file
    printf("\nDownloading %s...\n", file->name);
    if (receive_file(sockfd, file, DIR_DL)) {
        perror("[client] receiving file");
        free_list(file);
        return EXIT_FAILURE;
    }
    puts("Success");
    free_list(file);

    // close the connection
    disconnect(sockfd);

    return EXIT_SUCCESS;
}
