/** *************************************************************************************
 * Exercice sur l'envoi de fichiers
 * ================================
 *
 * Server sending the list of files availaible for download
 * then the file chosen by the client
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
    struct dl_file *files;                     // files info linked list sent to the client
    uint16_t files_size;                 // number of elements contained in the list
    ssize_t rv;

    // open a passive connection
    if ((sockfd = server_listen(PORT, BACKLOG)) < 0) {
        if (sockfd == -1) fprintf(stderr, "[server] creating the socket: %s\n", gai_strerror(errno));
        else if (sockfd == -2) perror("[server] overriding socket options");
        else if (sockfd == -3) perror("[server] binding\n");
        else if (sockfd == -4) perror("[server] listening");
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
            rv = expect_data(newfd, &id_byte, 1);
            if (rv < 0) {
                if (rv == -1) fprintf(stderr, "[server:%s] receiving id byte: client has closed the connection\n", client_ip);
                else perror("[server] receiving id byte");
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
            if (send_list(newfd, files, files_size) < 0) {
                fprintf(stderr, "[server:%s] sending the list: %s\n", client_ip, strerror(errno));
                free_list(files);
                disconnect(newfd);
                return EXIT_FAILURE;
            }
            printf("[server:%s] list of available files sent\n", client_ip);
            free_list(files);

            // receive the chosen file
            rv = receive_list(newfd, &files, &files_size);
            if (rv < 0) {
                if (rv == -1) fprintf(stderr, "[server:%s] receiving chosen file: client has closed the connection\n", client_ip);
                else perror("[server] receiving chosen file");
                disconnect(newfd);
                return EXIT_FAILURE;
            }

            // send the file
            if (send_file(newfd, files->name)) {
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




// /* Server code */
// /* TODO : Modify to meet your need */
// #include <stdio.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <stdlib.h>
// #include <errno.h>
// #include <string.h>
// #include <arpa/inet.h>
// #include <unistd.h>
// #include <netinet/in.h>
// #include <sys/stat.h>
// #include <fcntl.h>
// #include <sys/sendfile.h>

// #define PORT_NUMBER     5000
// #define SERVER_ADDRESS  "192.168.1.7"
// #define FILE_TO_SEND    "hello.c"

// int main(int argc, char **argv)
// {
//         int sockfd;
//         int peer_socket;
//         socklen_t       sock_len;
//         ssize_t len;
//         struct sockaddr_in      server_addr;
//         struct sockaddr_in      peer_addr;
//         int fd;
//         int sent_bytes = 0;
//         char file_size[256];
//         struct stat file_stat;
//         int offset;
//         int remain_data;



//         fd = open(FILE_TO_SEND, O_RDONLY);
//         if (fd == -1)
//         {
//                 fprintf(stderr, "Error opening file --> %s", strerror(errno));

//                 exit(EXIT_FAILURE);
//         }

//         /* Get file stats */
//         if (fstat(fd, &file_stat) < 0)
//         {
//                 fprintf(stderr, "Error fstat --> %s", strerror(errno));

//                 exit(EXIT_FAILURE);
//         }

//         fprintf(stdout, "File Size: \n%d bytes\n", file_stat.st_size);

//         sock_len = sizeof(struct sockaddr_in);
//         /* Accepting incoming peers */
//         peer_socket = accept(sockfd, (struct sockaddr *)&peer_addr, &sock_len);
//         if (peer_socket == -1)
//         {
//                 fprintf(stderr, "Error on accept --> %s", strerror(errno));

//                 exit(EXIT_FAILURE);
//         }
//         fprintf(stdout, "Accept peer --> %s\n", inet_ntoa(peer_addr.sin_addr));

//         sprintf(file_size, "%d", file_stat.st_size);

//         /* Sending file size */
//         len = send(peer_socket, file_size, sizeof(file_size), 0);
//         if (len < 0)
//         {
//               fprintf(stderr, "Error on sending greetings --> %s", strerror(errno));

//               exit(EXIT_FAILURE);
//         }

//         fprintf(stdout, "Server sent %d bytes for the size\n", len);

//         offset = 0;
//         remain_data = file_stat.st_size;
//         /* Sending file data */
//         while (((sent_bytes = sendfile(peer_socket, fd, &offset, BUFSIZ)) > 0) && (remain_data > 0))
//         {
//                 fprintf(stdout, "1. Server sent %d bytes from file's data, offset is now : %d and remaining data = %d\n", sent_bytes, offset, remain_data);
//                 remain_data -= sent_bytes;
//                 fprintf(stdout, "2. Server sent %d bytes from file's data, offset is now : %d and remaining data = %d\n", sent_bytes, offset, remain_data);
//         }

//         close(peer_socket);
//         close(server_socket);

//         return 0;