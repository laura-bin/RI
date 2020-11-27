/****************************************************************************************
* Exercice sur la serialisation
* =============================
*
* Data linked list manipulation functions
*
* RI 2020 - Laura Binacchi - Fedora 32
****************************************************************************************/

#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "data.h"
#include "constants.h"
#include "serial-util.h"
#include "tcp-util.h"

/**
 * Generates a random sign (plus or minus)
 * 
 * @return 1 for plus, - 1 for minus
 */
int random_sign() {
    int random = rand() % 2;
    return random ? 1 : -1;
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

/**
 * Prints a list of nodes
 * 
 * @param node: head node pointer
 * @param size: number of nodes to print
 */
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

/**
 * Creates a new node with random values
 * 
 * @return the pointer of the node created
 */
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

/**
 * Frees the data linked list
 * 
 * @param node: head node pointer
 */
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


/**
 * Sends a data linked list
 * 
 * @param sockfd: tcp connection socket file descriptor
 * @param node: head node pointer
 * @param size: number of elements contained in the list
 * 
 * @return 0 if all data has been sent
 */
int send_list(int sockfd, struct data_node *node, uint16_t size) {
    char BUFFER[BUF_SIZE];
    char *start, *buffer;
    int rv;

    start = buffer = BUFFER;

    buffer = write_u16(buffer, size);
    while (node) {
        buffer = write_u16(buffer, node->int_16);
        buffer = write_u32(buffer, node->int_32);
        buffer = write_u64(buffer, node->int_64);
        buffer = write_f32(buffer, node->f);
        buffer = write_f64(buffer, node->d);
        buffer = write_str(buffer, node->str);

        rv = send_data(sockfd, start, buffer - start);
        if (rv) return rv;

        node = node->next;
        buffer = start;
    }

    return 0;
}

/**
 * Receives a data linked list
 * 
 * @param sockfd: tcp connection socket file descriptor
 * 
 * @return the data linked list received
 */
struct data_node *receive_list(int sockfd) {
    struct data_node *node;
    return node;
}
