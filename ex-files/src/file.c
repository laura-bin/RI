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


/* PRIVATE FUNCTIONS */

/**
 * Cleans the stdin buffer
 */
void clean_stdin(void) {
    int c;
    do c = getchar();
    while (c != '\n' && c != EOF);
}

/**
 * Gets the size of the file
 *
 * @param filename: file name
 * @param path: file path
 *
 * @return the file size, 0 if an error occured
 */
uint64_t get_file_size(char *filename, char* path) {
    FILE *fp;                   // file pointer
    char filepath[BUF_SIZE];    // file path
    long size;                  // file size

    sprintf(filepath, "%s/%s", path, filename);

    fp = fopen(filepath, "r");
    if (fp == NULL) {
        return 0;
    }

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fclose(fp);

    return size < 0 ? 0 : (uint64_t) size;
}

/* HEADER IMPLEMENTATION */

struct dl_file *append(struct dl_file *files, char *filename, uint64_t filesize) {
    struct dl_file *new_file;   // new file information

    // place the pointer on the last node of the list
    if (files != NULL) {
        while (files->next) {
            files = files->next;
        }
    }

    // fill the new node
    new_file = malloc(sizeof(struct dl_file));
    new_file->name = strdup(filename);
    new_file->size = filesize;
    new_file->next = NULL;

    // add the new node at the end of the list
    if (files != NULL) {
        files->next = new_file;
    }

    return new_file;
}

uint16_t get_list(char *dirname, struct dl_file **out_files) {
    DIR *dir;                       // directory pointer
    struct dirent *ent;             // directory entry pointer
    struct dl_file *file = NULL;    // new downloadable file info
    uint64_t filesize;              // new downloadable file size
    uint16_t size = 0;              // count of elements contained in the list

    *out_files = NULL;

    // open the files directory
    if ((dir = opendir(dirname)) == NULL) {
        return 0;
    }

    // fill the list to send
    while ((ent = readdir (dir)) != NULL) {
        // if the entity is a regular file
        if (ent->d_type == DT_REG) {
            // and if the filesize is not null (no error occured on openig)
            filesize = get_file_size(ent->d_name, dirname);
            if (filesize > 0) {
                // add the new file info to the list
                file = append(file, ent->d_name, filesize);
                if (*out_files == NULL) {
                    *out_files = file;
                }
                size++;
            }
        }
    }

    // close the directory
    closedir(dir);

    return size;
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

void free_list(struct dl_file *files) {
    struct dl_file *next_file;

    while (files) {
        // save the next node pointer
        next_file = files->next;

        // free the string & the node
        free(files->name);
        free(files);

        // iterate on next node
        files = next_file;
    }
}

ssize_t send_list(int sockfd, struct dl_file *files, uint16_t size) {
    char buf_mem[TCP_BUF_SIZE]; // buffer used to send data
    char *start, *buffer;       // pointers used to access the buffer
    uint32_t data_size;         // size of the packet sent
    ssize_t bytes_sent = 0;     // total amount of data sent

    start = buffer = buf_mem;

    // send a packet containing the number of elements that will be sent
    buffer = write_u16(size, buffer);
    if (send_data(sockfd, start, buffer - start)) {
        return -1;
    }

    // send packet containing the file info prefixed by its size
    while (files) {
        // fill the packet with the files data
        // reserve space for the header
        buffer = write_str(files->name, start + sizeof(uint32_t));
        buffer = write_u64(files->size, buffer);

        // fill the packet header with the data size
        data_size = buffer - start - sizeof(uint32_t);
        write_u32(data_size, start);

        // send the packet & stop sending if an error occured
        if (send_data(sockfd, start, buffer - start)) {
            return -1;
        }

        bytes_sent += data_size;
        files = files->next;
    }

    return bytes_sent;
}

ssize_t receive_list(int sockfd, struct dl_file **out_files, uint16_t *out_size) {
    struct dl_file *file = NULL;        // file info
    struct dl_file *prev_file = NULL;   // previous file info kept to link the list
    uint32_t data_size;                 // size of packet expected
    ssize_t bytes_received = 0;         // total amount of data received
    char buf_mem[TCP_BUF_SIZE];         // buffer used to receive data
    char *start, *buffer;               // pointers used to access the buffer
    uint16_t i;                         // packet index

    // init variables
    *out_files = NULL;
    start = buffer = buf_mem;

    // receive first packet: number of elements contained in the list
    if (expect_data(sockfd, start, sizeof(uint16_t))) {
        return -1;
    }

    // fill the returned list size
    read_u16(start, out_size);

    // receive packets containing the data prefixed by its size (1 packet = 1 node)
    for (i = 0; i < *out_size; i++) {
        // receive the packet header containing the packet size
        if (expect_data(sockfd, start, sizeof(uint32_t))) {
            return -1;
        }
        read_u32(start, &data_size);

        // receive the data
        if (expect_data(sockfd, start, data_size)) {
            return -1;
        }
        bytes_received += data_size;

        // fill the file info with the data received
        file = malloc(sizeof(struct dl_file));
        buffer = read_str(start, &file->name);
        buffer = read_u64(buffer, (uint64_t *) &file->size);
        file->next = NULL;

        if (*out_files != NULL) prev_file->next = file;
        else *out_files = file;
        prev_file = file;
    }

    return bytes_received;
}

int send_file(int sockfd, struct dl_file *file, char *dirname) {
    char filepath[BUF_SIZE];            // file path
    int fd;                             // file descriptor
    char size_buf[sizeof(uint64_t)];    // buffer used to send the size of the file
    uint64_t remaining_bytes;           // remaining data amount to send
    ssize_t bytes_sent;                 // amount of bytes sent
    off_t offset = 0;                   // offset used by sendfile

    // open the file
    sprintf(filepath, "%s/%s", dirname, file->name);
    fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        return -1;
    }

    // send file size
    remaining_bytes = file->size;
    write_u64(remaining_bytes, size_buf);
    if (send_data(sockfd, size_buf, sizeof(uint64_t))) {
        return -1;
    }

    // send bytes while bytes remain
    while (remaining_bytes) {
        bytes_sent = sendfile(sockfd, fd, &offset, remaining_bytes);
        if (bytes_sent < 0) {
            close(fd);
            return -1;
        }
        remaining_bytes -= bytes_sent;
    }
    close(fd);
    return 0;
}

int receive_file(int sockfd, struct dl_file *file, char *dirname) {
    FILE *fp;                   // file pointer
    char filepath[BUF_SIZE];    // file path
    uint64_t remaining_bytes;   // remaining data amount to receive
    char buffer[TCP_BUF_SIZE];  // buffer filled with the data received
    ssize_t bytes_expected;     // bytes expected by expect data

    // make directory if it does not exist
    mkdir(dirname, 0700);
    sprintf(filepath, "%s/%s", dirname, file->name);

    // open the file
    fp = fopen(filepath, "wb");
    if (fp == NULL) {
        return -1;
    }

    // receive file size
    if (expect_data(sockfd, buffer, sizeof(uint64_t))) {
        return -1;
    }
    read_u64(buffer, &remaining_bytes);

    // receive bytes while bytes remain
    while (remaining_bytes) {
        bytes_expected = remaining_bytes > TCP_BUF_SIZE ? TCP_BUF_SIZE : remaining_bytes;
        if (expect_data(sockfd, buffer, bytes_expected)) {
            return -1;
        }

        if ((ssize_t) fwrite(buffer, 1, bytes_expected, fp) != bytes_expected) {
            return -1;
        }
        remaining_bytes -= bytes_expected;
    }

    fclose(fp);
    return 0;
}

uint16_t get_chosen_file(uint16_t size) {
    uint16_t input = 0;
    printf("\nChoose a file to download [1-%hu]: ", size);
    while (scanf("%hu", &input) != 1 || input < 1 || input > size) {
        clean_stdin();
        printf("Please enter a valid id [1-%hu]: ", size);
    }
    clean_stdin();
    return input;
}


struct dl_file *get_file_by_index(struct dl_file *files, uint16_t index) {
    uint16_t i;

    if (files == NULL) return NULL;
    for (i = 0; i < index; i++) {
        if (files->next) {
            files = files->next;
        } else {
            return NULL;
        }
    }

    return append(NULL, files->name, files->size);
}
