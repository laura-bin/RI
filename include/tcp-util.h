#pragma once
/****************************************************************************************
* Exercice sur les librairies
* ===========================
*
* Programmation d'une bibliotheque de fonctions destinees 
* aux communications TCP/IP (v4 & v6)
*
* RI 2020 - Laura Binacchi - Fedora 32
****************************************************************************************/

#include <sys/types.h>

/**
 * Initiates an active TCP connection :
 * creates the socket & connects to a server listening
 * 
 * @param url: url or ip address separated by dots
 * @param service: port number or service
 *
 * @return either
 *      -  socket file descriptor if it was successfully created
 *      - -1 if an error occured on socket creation
 *      - -2 if an error occured when connecting to the server
 */
int client_connect(char *url, char *service);

/**
 * Initiates a passive TCP connection :
 * creates the socket & listens for active connections
 * 
 * @param service: port number or service
 * @param backlog: amount of pending connections allowed
 *
 * @return either
 *      -  socket file descriptor if it was successfully created
 *      - -1 if an error occured on socket creation
 *      - -2 if an eror occured while overriding the default socket options
 *      - -3 if the server failed to bind
 *      - -4 if the server failed to open a passive connection
 */
int server_listen(char *service, int backlog);

/**
 * Accepts the connection from a client
 * 
 * @param sockfd: server socket file descriptor
 * @param out_client_ip: returned client ip address
 * 
 * @return active connection socket file descriptor, -1 if an error occured
 */
int server_accept(int sockfd, char *out_client_ip);

/**
 * Sends data to the remote host
 * 
 * @param sockfd: socket file descriptor
 * @param buffer: buffer containing the data
 * @param length: buffer length
 * 
 * @return 0 if all bytes have been sent, -1 if an error occured
 */
int send_data(int sockfd, char *buffer, ssize_t length);

/**
 * Receives data from the remote host
 *
 * @param sockfd: socket file descriptor
 * @param out_buffer: returned buffer containing the data
 * @param max_length: total allocated memory available for the buffer
 *
 * @return number of bytes received, -1 if an error occured
 */
ssize_t receive_data(int sockfd, char *out_buffer, ssize_t max_length);

/**
 * Expects a given amount of data from the remote host
 *
 * @param sockfd: socket file descriptor
 * @param out_buffer: returned buffer containing the data
 * @param length: blocks until that amount of bytes have been received
 *
 * @return either
 *      -  0 if all the data expected have been received
 *      - -1 if the remote closed the connection before receiving the amount of bytes expected
 *      - -2 if an eror occured while receiving the data
 */
int expect_data(int sockfd, char *out_buffer, ssize_t length);

/**
 * Closes the socket
 *
 * @param sockfd: socket file descriptor
 */
void disconnect(int sockfd);
