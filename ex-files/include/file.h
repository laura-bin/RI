#pragma once
/** *************************************************************************************
 * Exercice sur l'envoi de fichiers
 * ================================
 *
 * File information structure & manipulation functions
 *
 * RI 2020 - Laura Binacchi - Fedora 32
 ****************************************************************************************/

#include <stdint.h>
#include <stddef.h>



/** List of downloadable files */
struct dl_file {
    int16_t id;
    char *name;
    char *path;
    uint64_t size;
    struct dl_file *next;
};

struct dl_file *append(struct dl_file *file, char *filename, char *dirname);

/**
 * Fills the list with the files in a given directory
 *
 * @param dirname: name of the directory in which search the files
 * @param out_files: list to fill
 *
 * @return either:
 *      the count of elements added in the list
 *      -1 if an error occured
 */
uint16_t get_list(char *dirname, struct dl_file **out_files);

/**
 */
struct dl_file *get_file_by_id(struct dl_file *files, uint16_t chosen_file);

/**
 * Prints the list of files information or an error message if the list is empty
 *
 * @param file: list of files information
 * @param size: count of elements contained in the list
 */
void print_list(struct dl_file *file, uint16_t size);

/**
 * Frees the list of files information
 *
 * @param files: list of files information
 */
void free_list(struct dl_file *file);

/**
 * Sends a serialized list of files information
 *  - first packet contains the count of elements in the list
 *  - the each file information is sent in a packet prefixed by the data size
 *
 * @param sockfd: tcp connection socket file descriptor
 * @param file: list of files information
 * @param size: count of elements contained in the list
 *
 * @return the amount of bytes sent or -1 if an error occured
 */
ssize_t send_list(int sockfd, struct dl_file *file, uint16_t size);

/**
 * Receives a linked list of files information
 *
 * @param sockfd: tcp connection socket file descriptor
 * @param out_file: returned list containing the data received
 * @param out_size: returned list size (count of elements contained in the list)
 *
 * @return number of bytes received or -1 if an error occured
 */
ssize_t receive_list(int sockfd, struct dl_file **out_file, uint16_t *out_size);

uint16_t get_file_id(uint16_t size);

/**
 * Sends a file
 *
 * @param sockfd: destination socket file descriptor
 * @param filename: name of the file to send
 *
 * @return either:
 *          0 il all the data have been sent
 *         -1 if an error occured & sets errno
 */
int send_file(int sockfd, char *filename);

int receive_file(int sockfd, char *filename);
