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
void print_list(struct data_node *node, size_t size);

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
