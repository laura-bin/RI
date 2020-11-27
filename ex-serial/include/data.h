#pragma once
/****************************************************************************************
* Exercice sur la serialisation
* =============================
*
* Data linked list structure & manipulation functions prototypes
*
* RI 2020 - Laura Binacchi - Fedora 32
****************************************************************************************/

#include <stdint.h>
#include <stddef.h>

/** Data linked list structure */
struct data_node {
    int16_t int_16;
    int32_t int_32;
    int64_t int_64;
    float   f;
    double  d;
    char    *str;
    struct data_node *next;
};

/**
 * Prints a list of nodes
 * 
 * @param node: head node pointer
 * @param size: number of nodes to print
 */
void print_list(struct data_node *node, uint16_t size);

/**
 * Creates a new node with random values
 * 
 * @return the pointer of the node created
 */
struct data_node *create_node();

/**
 * Frees the data linked list
 * 
 * @param node: head node pointer
 */
void free_list(struct data_node *node);

/**
 * Sends a data linked list
 * 
 * @param sockfd: tcp connection socket file descriptor
 * @param node: head node pointer
 * @param size: number of elements contained in the list
 * 
 * @return the amount of bytes sent or -1 if an error occured
 */
ssize_t send_list(int sockfd, struct data_node *node, uint16_t size);

/**
 * Receives a data linked list
 * 
 * @param sockfd: tcp connection socket file descriptor
 * @param out_node: returned linked list pointer containing the data received
 * @param out_list_size: returned list size (number of nodes contained)
 * 
 * @return number of bytes received, -1 if an error occured
 */
ssize_t receive_list(int sockfd, struct data_node **out_node, uint16_t *out_list_size);

/**
 * Sends an acknowledgement containing the amount of data received
 * 
 * @param sockfd: tcp connection socket file descriptor
 * @param ack_bytes: amount of data received by the server
 * 
 * @return the return value of send (0 if all bytes have been send,
 *      -1 if an error occured & set errno)
 */
int send_ack(int sockfd, ssize_t ack_bytes);

/**
 * Receives an acknowledgement containing the amount of data received
 * 
 * @param sockfd: tcp connection socket file descriptor
 * 
 * @return the amount of data received by the server or the error code from expect_data
 */
ssize_t receive_ack(int sockfd);