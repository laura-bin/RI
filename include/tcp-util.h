#pragma once
/** *************************************************************************************
 * Exercice sur les librairies
 * ===========================
 *
 * TCP/IP (v4 & v6) communication library
 *
 * RI 2020 - Laura Binacchi - Fedora 32
 ****************************************************************************************/

#include <sys/types.h>

#define ERR_TCP_CREATE_SOCK     -1
#define ERR_TCP_ACTIVE_CONNECT  -2
#define ERR_TCP_OVER_SOCK_OPT   -3
#define ERR_TCP_BIND            -4
#define ERR_TCP_PASSIVE_CONNECT -5
#define ERR_TCP_PEER_CLOSED     -6
#define ERR_TCP_RECV_DATA       -7

/**
 * Initiates an active TCP connection :
 * creates the socket & connects to a server listening
 *
 * @param url: url or ip address separated by dots
 * @param service: port number or service
 *
 * @return either
 *      socket file descriptor if it was successfully created
 *      ERR_TCP_CREATE_SOCK if an error occured on socket creation
 *          (errno is set with the gai error)
 *      ERR_TCP_ACTIVE_CONNECT if an error occured while connecting to the server
 *          (errno is set)
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
 *      socket file descriptor if it was successfully created
 *      ERR_TCP_CREATE_SOCK if an error occured on socket creation
 *      ERR_TCP_OVER_SOCK_OPT if an eror occured while overriding the default socket options
 *      ERR_TCP_BIND if the server failed to bind
 *      ERR_TCP_PASSIVE_CONNECT if the server failed to open a passive connection
 *      errno is set
 */
int server_listen(char *service, int backlog);

/**
 * Accepts the connection from a client
 *
 * @param sockfd: server socket file descriptor
 * @param out_client_ip: returned client ip address
 *
 * @return either
 *      the active connection socket file descriptor
 *      -1 if an error occured
 *      errno is set
 */
int server_accept(int sockfd, char *out_client_ip);

/**
 * Sends data to the remote host
 *
 * @param sockfd: socket file descriptor
 * @param buffer: buffer containing the data
 * @param length: buffer length
 *
 * @return either
 *      0 if all bytes have been sent
 *      -1 if an error occured
 *      errno is set
 */
int send_data(int sockfd, char *buffer, ssize_t length);

/**
 * Receives data from the remote host
 *
 * @param sockfd: socket file descriptor
 * @param out_buffer: returned buffer containing the data
 * @param max_length: total allocated memory available for the buffer
 *
 * @return either
 *      the amount of bytes received
 *      -1 if an error occured
 *      errno is set
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
 *      0 if all the data expected have been received
 *      ERR_TCP_PEER_CLOSED if the remote closed the connection before receiving
 *          the amount of bytes expected (errno is not set)
 *      ERR_TCP_RECV_DATA if an eror occured while receiving the data (errno is set)
 */
int expect_data(int sockfd, char *out_buffer, ssize_t length);

/**
 * Closes the socket
 *
 * @param sockfd: socket file descriptor
 */
void disconnect(int sockfd);
