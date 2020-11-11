/**
 * Réseaux industriels 2020
 * 
 * Client implementing echo protocol (RFC 862)
 * with TCP and UDP protocols (add arg for udp)
 * 
 * Laura Binacchi
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "7"		// port number or service name
#define BUF_SIZE 100	// max number of bytes we can get at once 

// Get the pointer to the address (only IPv4 or IPv6)
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) return &(((struct sockaddr_in*)sa)->sin_addr);
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[]) {
	int sockfd;							// socket file descriptor
	ssize_t bytes_sent;					// number of bytes sent
	ssize_t bytes_received;				// number of bytes received
	int udp = 0;						// boolean keeping the protocol chosen by the user
	struct addrinfo hints;				// struct given to getaddrinfo
	struct addrinfo *servinfo;			// linked list filled by get addrinfo
	struct addrinfo *p;					// linked list for iteration
	int rv;								// return value of getaddrinfo
	char servip[INET6_ADDRSTRLEN];		// string containing a the human readable ip address of the server
	struct sockaddr_storage their_addr;
	socklen_t sin_size;
	char node[BUF_SIZE];				// where the user want to send the message
	char msg_sent[BUF_SIZE];			// message to send to the server
	char msg_received[BUF_SIZE];		// message reveived back from the server
	
	// Test the params
	if (argc < 3 || argc > 4) {
	    fprintf(stderr,"usage: client hostname message [udp]\n");
	    exit(1);
	} else {
		udp = argc == 4 && !strcmp(argv[3], "udp");
	}

	strncpy(node, argv[1], BUF_SIZE);
	strncpy(msg_sent, argv[2], BUF_SIZE);

	// Fill the hints
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // -> IPv4 or IPv6
	hints.ai_socktype = udp ? SOCK_DGRAM : SOCK_STREAM; // Default: tcp

	// Generate servinfo from hints
	if ((rv = getaddrinfo(node, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// Loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {

		// Get the socket reference from the system
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("client socket");
			continue;
		}

		// Open an active connection to the remote host
		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("client connect");
			close(sockfd);
			continue;
		}
		break;
	}


	// If p is NULL after the loop, no connection has been successful
	if (p == NULL) {
		fprintf(stderr, "client failed to create the socket\n");
		freeaddrinfo(servinfo);
		return 2;
	}

	// Free the linked list (servinfo & p)
	freeaddrinfo(servinfo);
	
	if (udp) { // UDP connection
		// send the user's message to the udp server
		if ((bytes_sent = sendto(sockfd, msg_sent, strlen(msg_sent), 0,
				p->ai_addr, p->ai_addrlen)) == -1) {
        	perror("client sendto");
        	exit(1);
		}
		printf("client sent %ld bytes to %s\n", bytes_sent, node);

		// then wait for the reply
		sin_size = sizeof their_addr;
		if ((bytes_received = recvfrom(sockfd, msg_received, BUF_SIZE-1, 0,
				(struct sockaddr *)&their_addr, &sin_size)) == -1) {
			perror("client recvfrom");
			exit(1);
		}

	} else { // TCP connection
		// send the user's message to the tcp server
		if ((bytes_sent = send(sockfd, msg_sent, strlen(msg_sent), 0)) == -1) {
			perror("client send");
			exit(1);
		}
		printf("client sent %ld bytes to %s\n", bytes_sent, node);

		// then wait for the reply
		if ((bytes_received = recv(sockfd, msg_received, BUF_SIZE-1, 0)) == -1) {
	    	perror("client recv");
	    	exit(1);
		}
	}

	printf("client received back %ld bytes\n", bytes_received);
	msg_received[bytes_received] = '\0';
	printf("message received: '%s'\n", msg_received);
	close(sockfd);

	return 0;
}
