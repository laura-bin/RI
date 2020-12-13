/****************************************************************************************
* Exercice sur la serialisation
* =============================
*
* Data linked list manipulation functions
*
* RI 2020 - Laura Binacchi - Fedora 32
****************************************************************************************/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "data.h"
#include "constants.h"
#include "serial-util.h"
#include "tcp-util.h"

/* PRIVATE FUNCTIONS */

/**
 * Generates a random sign (plus or minus)
 *
 * @return 1 for plus, -1 for minus
 */
int random_sign() {
    return rand() % 2 ? 1 : -1;
}

/**
 * Generates a random alphanumeric character
 *
 * @return the randon character
 */
char random_char() {
    char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    return chars[rand() % (sizeof(chars) / sizeof(char) - 1)];
}


/* HEADER PROTOTYPES IMPLEMENTATION */

void print_list(struct data_node *node, uint16_t size) {
    uint16_t i = 0;

    while (node) {
        printf("Node %d/%d {\n"
                "\tint_16:\t%d\n\tint_32:\t%d\n\tint_64:\t%ld\n"
                "\tfloat:\t%f\n\tdouble:\t%f\n\tstring:\t%s\n}\n",
                i + 1, size,
                node->int_16, node->int_32, node->int_64,
                node->f, node->d, node->str);
        node = node->next;
        i++;
    }
}

struct data_node *create_node() {
    struct data_node *node = (struct data_node *) malloc(sizeof(struct data_node));
    int str_len = rand() % 30 + 1;

    node->int_16 = (int16_t) rand();
    node->int_32 = (int32_t) rand() * random_sign();
    node->int_64 = (int64_t) INT64_MAX - rand() * random_sign();
    node->f = rand() % INT16_MAX * M_PI;
    node->d = rand() * M_SQRT2 / 1.467967 * random_sign();
    node->str = malloc(sizeof(char) * str_len + 1);
    for (int i = 0; i < str_len; i++) node->str[i] = random_char();
    node->str[str_len] = '\0';
    node->next = NULL;

    return node;
}

void free_list(struct data_node *node) {
    struct data_node *next_node;

    while (node) {
        // save next node pointer
        next_node = node->next;

        // free the string & the node
        free(node->str);
        free(node);

        // iterate on next node
        node = next_node;
    }
}

ssize_t send_list(int sockfd, struct data_node *node, uint16_t size) {
    char BUFFER[BUF_SIZE];
    char *start, *buffer;
    uint32_t node_size;
    ssize_t bytes_sent = 0;

    start = buffer = BUFFER;

    // send a packet containing the number of nodes that will be sent
    buffer = write_u16(size, buffer);
    if(send_data(sockfd, start, buffer - start)) return -1;

    // send packet containing the node prefixed by its size (1 packet = 1 node)
    while (node) {
        // fill the packet with the node data
        buffer = write_u16(node->int_16, start + sizeof(uint32_t)); // reserve space for the header
        buffer = write_u32(node->int_32, buffer);
        buffer = write_u64(node->int_64, buffer);
        buffer = write_f32(node->f, buffer);
        buffer = write_f64(node->d, buffer);
        buffer = write_str(node->str, buffer);

        // fill the packet header with the data size
        node_size = buffer - start - sizeof(uint32_t);
        write_u32(node_size, start);

        // send the packet & stop sending if an error occured
        if (send_data(sockfd, start, buffer - start)) return -1;
        bytes_sent += node_size;

        node = node->next;
    }

    return bytes_sent;
}

ssize_t receive_list(int sockfd, struct data_node **out_node, uint16_t *out_list_size) {
    struct data_node *previous_node;
    struct data_node *node;
    uint32_t node_size;
    uint16_t i;
    ssize_t bytes_received = 0;
    char BUFFER[BUF_SIZE];
    char *start, *buffer;
    int rv;

    *out_node = NULL;
    start = buffer = BUFFER;

    // receive first packet: number of nodes contained in the list
    rv = expect_data(sockfd, start, sizeof(uint16_t));
    if (rv) return rv;

    // fill the returned list size
    read_u16(start, out_list_size);

    // send packet containing the node prefixed by its size (1 packet = 1 node)
    for (i = 0; i < *out_list_size; i++) {
        // receive the packet header containing the node size
        rv = expect_data(sockfd, start, sizeof(uint32_t));
        if (rv) return rv;
        read_u32(start, &node_size);

        rv = expect_data(sockfd, start, node_size);
        if (rv) return rv;
        bytes_received += node_size;

        node = malloc(sizeof(struct data_node));

        // fill the node with the data received
        buffer = read_u16(start, (uint16_t *) &node->int_16);
        buffer = read_u32(buffer, (uint32_t *) &node->int_32);
        buffer = read_u64(buffer, (uint64_t *) &node->int_64);
        buffer = read_f32(buffer, &node->f);
        buffer = read_f64(buffer, &node->d);
        buffer = read_str(buffer, &node->str);
        node->next = NULL;

        if (*out_node != NULL) previous_node->next = node;
        else *out_node = node;
        previous_node = node;
    }

    return bytes_received;
}

int send_ack(int sockfd, ssize_t ack_bytes) {
    char ack[sizeof(uint64_t)];
    write_u64(ack_bytes, ack);
    return send_data(sockfd, ack, sizeof(uint64_t));
}

ssize_t receive_ack(int sockfd) {
    char ack[sizeof(uint64_t)];
    uint64_t bytes_received;

    int rv = expect_data(sockfd, ack, sizeof(uint64_t));
    if (rv < 0) return rv;

    read_u64(ack, &bytes_received);
    return bytes_received;
}
