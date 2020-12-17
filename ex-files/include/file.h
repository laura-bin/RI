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

/** List of downloadable files information */
struct dl_file {
    char *name;
    uint64_t size;
    struct dl_file *next;
};

/**
 * Appends a new file at the end of the list
 *
 * @param files: list of file information
 * @param filename: name of the new file to add
 * @param filesize: size of the new file to add
 *
 * @return the pointer of the new file added, NULL if an error occured
 */
struct dl_file *append(struct dl_file *files, char *filename, uint64_t filesize);

/**
 * Fills the list with the files in a given directory
 *
 * @param dirname: name of the directory in which search the files
 * @param out_files: list to fill
 *
 * @return either:
 *      the count of elements contained in the list
 *      0 if an error occured
 */
uint16_t get_list(char *dirname, struct dl_file **out_files);

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
void free_list(struct dl_file *files);

/**
 * Sends a serialized list of files information:
 * first packet contains the count of elements in the list
 * then each file information is sent in a packet prefixed by the data size
 *
 * @param sockfd: tcp connection socket file descriptor
 * @param files: list of files information
 * @param size: count of elements contained in the list
 *
 * @return the amount of bytes sent or -1 if an error occured (errno is set)
 */
ssize_t send_list(int sockfd, struct dl_file *files, uint16_t size);

/**
 * Receives a linked list of files information
 *
 * @param sockfd: tcp connection socket file descriptor
 * @param out_file: returned list containing the data received
 * @param out_size: returned list size (count of elements contained in the list)
 *
 * @return number of bytes received or -1 if an error occured (errno is set)
 */
ssize_t receive_list(int sockfd, struct dl_file **out_file, uint16_t *out_size);

/**
 * Sends a file prefixed by its size
 *
 * @param sockfd: destination socket file descriptor
 * @param file: file to send
 * @param dirname: name of the directory where the file is located
 *
 * @return either:
 *      0 il all the data have been sent
 *      -1 if an error occured
 *      errno is set
 */
int send_file(int sockfd, struct dl_file *file, char *dirname);

/**
 * Receives a file prefixed by its size
 *
 * @param sockfd: socket file descriptor
 * @param file: file to receive
 * @param dirname: download directory
 *
 * @return either:
 *      0 if all the data have been received
 *      -1 if an error occured
 *      errno is set
 */
int receive_file(int sockfd, struct dl_file *file, char* dirname);

/**
 * Gets the index of the file chosen by the user
 *
 * @param size: list size
 *
 * @return the index of the file chose
 */
uint16_t get_chosen_file(uint16_t size);


/**
 * Gets a file by its index in the list
 *
 * @param files: files list
 * @param index: index of the file searched in the list
 *
 * @return a new file pointer, NULL if the file was not found
 */
struct dl_file *get_file_by_index(struct dl_file *files, uint16_t index);
