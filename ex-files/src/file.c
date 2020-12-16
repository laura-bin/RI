/** *************************************************************************************
 * Exercice sur l'envoi de fichiers
 * ================================
 *
 * File information structure & manipulation functions
 *
 * RI 2020 - Laura Binacchi - Fedora 32
 ****************************************************************************************/

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <unistd.h>

#include "const.h"
#include "file.h"
#include "serial-util.h"
#include "tcp-util.h"


/* PRIVATE FUNCTION */

/**
 * Cleans the stdin buffer
 */
void clean_stdin(void) {
    int c;
    do c = getchar();
    while (c != '\n' && c != EOF);
}

uint64_t get_file_size(char* filepath) {
    FILE *fp;                   // file pointer
    uint64_t size;

    fp = fopen(filepath, "r");
    if (fp == NULL) return 0;
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fclose(fp);

    return size;
}

struct dl_file *append(struct dl_file *file, char *filename, char *dirname) {
    struct dl_file *new_file;
    char filepath[1024];        // file path

    // place the pointer on the last node of the list
    if (file != NULL) {
        while (file->next) file = file->next;
    }

    // init the new node
    new_file = malloc(sizeof(struct dl_file));

    // set id
    if (file == NULL) new_file->id = 1;
    else new_file->id = file->id + 1;

    // set name
    new_file->name = strdup(filename);

    // set size
    if (dirname) {
        sprintf(filepath, "%s/%s", dirname, filename);
        new_file->size = get_file_size(filepath);
    } else {
        new_file->size = 0;
    }

    // set next
    new_file->next = NULL;

    // add the new node at the end of the list
    if (file != NULL) file->next = new_file;

    return new_file;
}

uint16_t get_list(char *dirname, struct dl_file **out_files) {
    DIR *dir;                   // directory pointer
    struct dirent *ent;         // directory entry pointer
    struct dl_file *file = NULL;       // downloadable file info
    uint16_t size = 0;          // count of elements contained in the list

    // open the files directory
    if ((dir = opendir(dirname)) == NULL) return -1;

    // init variables
    size = 0;
    *out_files = NULL;

    // fill the list to send
    while ((ent = readdir (dir)) != NULL) {
        // if the entity is a regular file, add it to the list
        if (ent->d_type == DT_REG) {
            file = append(file, ent->d_name, dirname);
            if (*out_files == NULL) *out_files = file;
            size++;
        }
    }

    // close the directory
    closedir(dir);

    return size;
}

struct dl_file *get_file_by_id(struct dl_file *files, uint16_t chosen_file) {
    uint16_t i;

    if (files == NULL) return NULL;
    for (i = 1; i < chosen_file; i++) {
        if (files->next) files = files->next;
        else return NULL;
    }

    return append(NULL, files->name, NULL);
}

void print_list(struct dl_file *file, uint16_t size) {
    uint16_t i = 1;
    if (file == NULL) puts("No file available for download");
    while (file && i <= size) {
        printf("%6d - %-48s %16ld bytes\n", i, file->name, file->size);
        file = file->next;
        i++;
    }
}

void free_list(struct dl_file *file) {
    struct dl_file *next_file;

    while (file) {
        // save the next node pointer
        next_file = file->next;

        // free the string & the node
        free(file->name);
        free(file);

        // iterate on next node
        file = next_file;
    }
}

ssize_t send_list(int sockfd, struct dl_file *file, uint16_t size) {
    char buf_mem[BUF_SIZE];     // buffer used to send data
    char *start, *buffer;       // pointers used to access the buffer
    uint32_t data_size;         // size of packet sent
    ssize_t bytes_sent = 0;     // total amount of data sent

    start = buffer = buf_mem;

    // send a packet containing the number of elements that will be sent
    buffer = write_u16(size, buffer);
    if(send_data(sockfd, start, buffer - start)) return -1;

    // send packet containing the file info prefixed by its size
    while (file) {
        // fill the packet with the files data
        buffer = write_u16(file->id, start + sizeof(uint32_t)); // reserve space for the header
        buffer = write_str(file->name, buffer);
        buffer = write_u64(file->size, buffer);

        // fill the packet header with the data size
        data_size = buffer - start - sizeof(uint32_t);
        write_u32(data_size, start);

        // send the packet & stop sending if an error occured
        if (send_data(sockfd, start, buffer - start)) return -1;
        bytes_sent += data_size;

        file = file->next;
    }

    return bytes_sent;
}

ssize_t receive_list(int sockfd, struct dl_file **out_file, uint16_t *out_size) {
    struct dl_file *file = NULL;           // file info
    struct dl_file *prev_file = NULL;      // previous file info kept to link the list
    uint32_t data_size;             // size of packet expected
    ssize_t bytes_received = 0;     // total amount of data received
    char buf_mem[BUF_SIZE];         // buffer used to receive data
    char *start, *buffer;           // pointers used to access the buffer
    int rv;
    uint16_t i;

    // init variables
    *out_file = NULL;
    start = buffer = buf_mem;

    // receive first packet: number of elements contained in the list
    rv = expect_data(sockfd, start, sizeof(uint16_t));
    if (rv) return rv;

    // fill the returned list size
    read_u16(start, out_size);

    // receive packets containing the data prefixed by its size (1 packet = 1 node)
    for (i = 0; i < *out_size; i++) {
        // receive the packet header containing the packet size
        rv = expect_data(sockfd, start, sizeof(uint32_t));
        if (rv) return rv;
        read_u32(start, &data_size);

        rv = expect_data(sockfd, start, data_size);
        if (rv) return rv;
        bytes_received += data_size;

        file = malloc(sizeof(struct dl_file));

        // fill the file info with the data received
        buffer = read_u16(start, (uint16_t *) &file->id);
        buffer = read_str(buffer, &file->name);
        buffer = read_u64(buffer, &file->size);
        file->next = NULL;

        if (*out_file != NULL) prev_file->next = file;
        else *out_file = file;
        prev_file = file;
    }

    return bytes_received;
}

uint16_t get_file_id(uint16_t size) {
    uint16_t input = 0;
    printf("\nChoose a file to download [1-%hu]: ", size);
    while (scanf("%hu", &input) != 1 || input < 1 || input > size) {
        clean_stdin();
        printf("Please enter a valid id [1-%hu]: ", size);
    }
    clean_stdin();
    return input;
}

int send_file(int sockfd, char *filename) {
    int fd;                     // file descriptor
    char filepath[1024];        // file path
    ssize_t size;               // file size
    ssize_t bytes_sent;         // amount of bytes sent
    off_t offset = 0;           // offset used by sendfile
    char size_buf[sizeof(uint64_t)];

    puts("sendfile");

    sprintf(filepath, "%s/%s", DIR_FILE, filename);
    size = get_file_size(filepath);

    // send file size
    write_u64(size, size_buf);
    if (send_data(sockfd, size_buf, sizeof(uint64_t))) return -1;

    fd = open(filepath, O_RDONLY);
    if (fd < 0) return -1;

    // send bytes while bytes remain
    while (size) {
        printf("offset: %ld\n", offset);

        bytes_sent = sendfile(sockfd, fd, &offset, size);
        printf("bytes sent: %lu\n", bytes_sent);

        if (bytes_sent < 0) {
            close(fd);
            return bytes_sent;
        }
        size -= bytes_sent;
    }

    close(fd);

    return 0;
}

int receive_file(int sockfd, char *filename) {
    FILE *fp;                     // file pointer
    char filepath[1024];        // file path
    char buffer[BUF_SIZE];
    uint64_t size;
    ssize_t bytes_expected;

    // make directory if it does not exist
    mkdir(DIR_DL, 0700);

    // open the file
    sprintf(filepath, "%s/%s", DIR_DL, filename);
    fp = fopen(filepath, "wb");
    if (fp == NULL) return -1;

    // receive file size
    if (expect_data(sockfd, buffer, sizeof(uint64_t))) return -1;
    read_u64(buffer, &size);

    while (size) {
        bytes_expected = size > BUF_SIZE ? BUF_SIZE : size;
        if (expect_data(sockfd, buffer, bytes_expected)) return -1;

        if ((ssize_t)fwrite(buffer, 1, bytes_expected, fp) != bytes_expected) return -1;
        size -= bytes_expected;
    }

    fclose(fp);
    return 0;
}