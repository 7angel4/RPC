#include "rpc_server_helper.h"
#include "rpc_safety.h"
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>

/* Creates a listening socket that listens on the given port.
 * Returns the new socket's file descriptor on success;
 * Returns FAILED (-1) on failure.
 */
int create_listening_socket(char* port) {
	int s, sockfd;
	struct addrinfo hints, *res;

	// Create address to listen on (with given port number)
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET6;       // IPv6
	hints.ai_socktype = SOCK_STREAM;  // Connection-mode byte streams (TCP)
	hints.ai_flags = AI_PASSIVE;      // for bind, listen, accept
	
    s = getaddrinfo(NULL, port, &hints, &res);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		return FAILED;
	}

	// Create socket
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sockfd < 0) {
		perror("socket");
		return FAILED;
	}

	// Reuse port if possible
	int enable = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	// Bind address to the socket
	if (bind(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
		perror("bind");
		return FAILED;
	}
	freeaddrinfo(res);

	return sockfd;
}

/* Handler for SIGCHLD.
 *
 * Code adapted from: Beej's Guide to Network Programming
 * https://beej.us/guide/bgnet/html/
 * Author: Brian “Beej Jorgensen” Hall
 * Also looked up "man waitpid"
 * Modifications: comments and coding style
 */
void sigchld_handler(int s) {
    // save errno in case waitpid overwrites it
    int saved_errno = errno;
	// prevents waitpid from blocking so we can do other stuff
    while (waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

/* Sets up the SIGCHLD handler.
 * Returns FAILED if the set-up failed, SUCCESS otherwise.
 *
 * Code adapted from: Beej's Guide to Network Programming
 * https://beej.us/guide/bgnet/html/
 * Author: Brian “Beej Jorgensen” Hall
 * Modifications: converted to function with return values
 */
int set_up_sigchld_handler() {
	struct sigaction sa;
    sa.sa_handler = sigchld_handler; // reap the dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        return FAILED;
    }
	return SUCCESS;
}

/* Accepts a connection request on the queue of pending connections 
   for the listening socket.
 * Returns a file descriptor for the accepted socket on success;
 * Returns FAILURE (-1) on failure.
 */
int accept_connection(int listening_sd) {
    struct sockaddr_in cl_addr;
    socklen_t cl_len = sizeof cl_addr;
    int newsockfd = accept(listening_sd, (struct sockaddr*)&cl_addr, &cl_len);
    if (newsockfd < 0) {
        perror("accept");
        return FAILED;
    } 

    return newsockfd;
}


