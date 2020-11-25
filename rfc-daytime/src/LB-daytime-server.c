/**
 * Réseaux industriels 2020
 * 
 * Server implementing the daytime RFC (RFC 867)
 * with TCP and UDP protocols (add arg for udp)
 * launch as su
 * 
 * Laura Binacchi
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <assert.h>

#define PORT "13"		// port number or service
#define BACKLOG 10		// how many pending connections queue will hold
#define DT_SIZE	64
#define BUF_SIZE 100	// max number of bytes we can get at once 

// Save errno after a child death
void sigchld_handler(int s) {
	(void)s;
	int saved_errno = errno;
	while(waitpid(-1, NULL, WNOHANG) > 0);
	errno = saved_errno;
}

// Get the pointer to the address (only IPv4 or IPv6)
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) return &(((struct sockaddr_in*)sa)->sin_addr);
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd, new_fd;  		// listen on sock_fd, new connection on new_fd
	ssize_t numbytes;			// number of bytes sent
	struct addrinfo hints;		// struct given to getaddrinfo
	struct addrinfo *servinfo;	// linked list filled by get addrinfo
	struct addrinfo *p;			// linked list for iteration
	int rv;						// return value of getaddrinfo
	struct sockaddr_storage their_addr; // address information about the incoming connection
	socklen_t sin_size;			// sizeof struct sockaddr_storage passed to accept	
	char s[INET6_ADDRSTRLEN];	// string containing a human readable ip address
	struct sigaction sa;
	time_t t;
	struct tm *dt;
	char sdt[DT_SIZE];	// formatted string containing the system datetime
	char buf[BUF_SIZE];	// buffer used to wait the client connection
	int yes = 1;
	int udp = 0;

	// Test the params
	if (argc == 2 && !strcmp(argv[1], "udp")) udp = 1;

	// Fill the hints
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = udp ? SOCK_DGRAM : SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;	// use my IP

	// Generate servinfo from hints
	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "server getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {

		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("server socket");
			continue;
		}

		// override default socket options : allow to reuse the port
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			perror("server setsockopt");
			freeaddrinfo(servinfo);
			exit(1);
		}

		// Associate the socket with a port on the local machine (for listening)
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("server bind");
			close(sockfd);
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo);

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	// function pointer to call on child death 
	sa.sa_handler = sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;

	// use the defined handler on sigchld signal (when a child dies)
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	// Open a passive connection only if we are in TCP
	if (!udp && listen(sockfd, BACKLOG) == -1) {
		perror("server listen");
		exit(1);
	}

	printf("server waiting for connections...\n");

	while(1) {  // main accept() loop
		sin_size = sizeof their_addr;

		if (udp) {
			// Wait for someone to connect
			if ((numbytes = recvfrom(sockfd, buf, 1, 0,
				(struct sockaddr *)&their_addr, &sin_size)) == -1) {
				perror("client recvfrom");
				continue;
			}
		} else {
			// get a new socket file descriptor for the incoming connection
			new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
			if (new_fd == -1) {
				perror("server accept");
				continue;
			}
		}

		// network to presentation
		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
		printf("server got connection from %s\n", s);

		if (!fork()) { // this is the child process
			if (!udp) close(sockfd); // child doesn't need the listener

			// set the datetime
			t = time(NULL);
			dt = localtime(&t);
			assert(strftime(sdt, sizeof(sdt), "%c", dt));

			if (udp) {
				if ((numbytes = sendto(sockfd, sdt, strlen(sdt), 0,
					(struct sockaddr *)&their_addr, sizeof(struct sockaddr_storage))) == -1) {
					perror("server sendto");
					exit(1);
				}
			} else {
				// send must return the same value as strlen(sdt)
				// TODO check if all the data is sent
				if ((numbytes = send(new_fd, sdt, strlen(sdt), 0)) == -1)
					perror("server send");
				printf("server sent %ld bytes\n", numbytes);
			}
			if (udp) close(sockfd);
			else close(new_fd);

			exit(0);
		}
	}

	return 0;
}

