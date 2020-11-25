/**
 * Réseaux industriels 2020
 * 
 * Client implementing the daytime RFC (RFC 867)
 * with TCP and UDP protocols (add arg for udp)
 * 
 * Laura Binacchi
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "13"		// port number or service name
#define BUF_SIZE 100	// max number of bytes we can get at once 

// Get the pointer to the address (only IPv4 or IPv6)
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) return &(((struct sockaddr_in*)sa)->sin_addr);
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[]) {
	int sockfd;					// socket file descriptor
	int numbytes;				// number of bytes received
	int udp = 0;				// boolean keeping the protocol chosen by the user
	char buf[BUF_SIZE];			// buffer used to read the server'reply
	struct addrinfo hints;		// struct given to getaddrinfo
	struct addrinfo *servinfo;	// linked list filled by get addrinfo
	struct addrinfo *p;			// linked list for iteration
	int rv;						// return value of getaddrinfo
	char s[INET6_ADDRSTRLEN];	// string containing a human readable ip address
	struct sockaddr_storage their_addr;
	socklen_t sin_size;
	
	// Test the params
	if (argc < 2 || argc > 3) {
	    fprintf(stderr,"usage: client hostname [udp]\n");
	    exit(1);
	} else {
		udp = argc == 3 && !strcmp(argv[2], "udp");
	}

	// Fill the hints
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // -> IPv4 or IPv6
	hints.ai_socktype = udp ? SOCK_DGRAM : SOCK_STREAM; // Default: tcp

	// Generate servinfo from hints
	if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
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

	// Network to presentation
	inet_ntop(p->ai_family, get_in_addr(p->ai_addr), s, sizeof s);
	printf("client connecting to %s\n", s);

	// Free the linked list (servinfo & p)
	freeaddrinfo(servinfo);

	
	if (udp) { // UDP connection
		// first send a dummy msg to the udp server
		if ((numbytes = sendto(sockfd, "smth", 13, 0,
             p->ai_addr, p->ai_addrlen)) == -1) {
        	perror("client sendto");
        	exit(1);
		}
		printf("client sent %d bytes to %s\n", numbytes, argv[1]);

		// then wait for the reply
		sin_size = sizeof their_addr;
		if ((numbytes = recvfrom(sockfd, buf, BUF_SIZE-1, 0,
			(struct sockaddr *)&their_addr, &sin_size)) == -1) {
			perror("client recvfrom");
			exit(1);
		}
		printf("listener: got packet from %s\n",
        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s));

	} else { // TCP connection
		// Receive the response from the server into the buffer
		// If numbyte = 0, the server closed the connection
		if ((numbytes = recv(sockfd, buf, BUF_SIZE-1, 0)) == -1) {
	    	perror("client recv");
	    	exit(1);
		}
	}

	printf("client received '%d' bytes\n", numbytes);
	buf[numbytes] = '\0';
	printf("client received: '%s'\n",buf);
	close(sockfd);

	return 0;
}
