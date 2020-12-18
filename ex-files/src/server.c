/** *************************************************************************************
 * Exercice sur l'envoi de fichiers
 * ================================
 *
 * Server sending the list of files availaible for download
 * then the file chosen by the client
 * 
 * Add subdirectory "files" with files to download
 * or change the downloadable files directory in const.h
 *
 * RI 2020 - Laura Binacchi - Fedora 32
 ****************************************************************************************/

#include <errno.h>
#include <netdb.h>          // gai_strerror
#include <signal.h>         // sigaction
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>       // WNOHANG
#include <unistd.h>         // fork

#include "const.h"
#include "file.h"
#include "tcp-util.h"

#define BACKLOG 10  // amount of pending connections allowed

// Save errno after a child death
void sigchld_handler(int s) {
    (void)s;
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

int main() {
    int sockfd;                         // server socket file descriptor
    int newfd;                          // client socket file descriptor
    char client_ip[INET6_ADDRSTRLEN];   // string containing the human readable ip address of the client
    struct sigaction sa;                // modified action to call on a child process death
    char id_byte;                       // identification byte received from the client
    struct dl_file *files;              // files info linked list sent to the client
    uint16_t files_size;                // number of elements contained in the list
    ssize_t rsize;                      // returned size
    int rint;                           // returned integer

    // open a passive connection
    sockfd = server_listen(PORT, BACKLOG);
    if (sockfd < 0) {
        switch (sockfd) {
            case ERR_TCP_CREATE_SOCK:
                fprintf(stderr, "[server] creating the socket: %s\n", gai_strerror(errno));
                break;
            case ERR_TCP_OVER_SOCK_OPT:
                perror("[server] overriding socket options");
                break;
            case ERR_TCP_BIND:
                perror("[server] binding\n");
                break;
            case ERR_TCP_PASSIVE_CONNECT:
                perror("[server] listening");
                break;
            default:
                break;
        }
        return EXIT_FAILURE;
    }

    // function pointer to call on child death
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    // use the defined handler on sigchld signal (when a child dies)
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("[server] sigaction");
        return EXIT_FAILURE;
    }

    printf("[server] waiting for connections...\n");

    while (1) {
        // get a new socket file descriptor for the incoming connection
        newfd = server_accept(sockfd, client_ip);
        if (newfd < 0) {
            perror("[server] accepting incoming connection");
            continue;
        }

        printf("[server] connection received from %s...\n", client_ip);

        // open a child process
        if (!fork()) {
            // child doesn't need the listener
            disconnect(sockfd);

            // receive the identification byte
            rint = expect_data(newfd, &id_byte, 1);
            if (rint < 0) {
                if (rint == ERR_TCP_PEER_CLOSED) {
                    fprintf(stderr, "[server:%s] receiving id byte: client has closed the connection\n", client_ip);
                } else {
                    perror("[server] receiving the identification byte");
                }
                disconnect(newfd);
                return EXIT_FAILURE;
            }

            // test the identification byte
            if (id_byte != ID_BYTE) {
                fprintf(stderr, "[server:%s] closing: wrong identification byte\n", client_ip);
                disconnect(newfd);
                return EXIT_FAILURE;
            }

            files_size = get_list(DIR_FILE, &files);

            // send the list
            rsize = send_list(newfd, files, files_size);
            free_list(files);

            if (rsize < 0) {
                fprintf(stderr, "[server:%s] sending the list: %s\n", client_ip, strerror(errno));
                disconnect(newfd);
                return EXIT_FAILURE;
            }

            if (files_size == 0) {
                printf("[server:%s] closing: no file available for download\n", client_ip);
                disconnect(newfd);
                return EXIT_FAILURE;
            }
            printf("[server:%s] list of available files sent\n", client_ip);

            // receive the chosen file
            rsize = receive_list(newfd, &files, &files_size);
            if (rsize < 0) {
                if (rsize == ERR_TCP_PEER_CLOSED) {
                    fprintf(stderr, "[server:%s] receiving chosen file: "
                                "client has closed the connection\n", client_ip);
                } else {
                    perror("[server] receiving chosen file");
                }
                disconnect(newfd);
                return EXIT_FAILURE;
            }

            // send the file
            rint = send_file(newfd, files, DIR_FILE);
            if (rint) {
                fprintf(stderr, "[server:%s] sending %s: %s\n", client_ip, files->name, strerror(errno));
                free_list(files);
                disconnect(newfd);
                return EXIT_FAILURE;
            }
            printf("[server:%s] successfully sent %s\n", client_ip, files->name);
            free_list(files);

            printf("[server:%s] closing\n", client_ip);
            disconnect(newfd);
            return EXIT_SUCCESS;
        }
    }

    return EXIT_SUCCESS;
}
