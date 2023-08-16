#include "rpc_client_helper.h"
#include "rpc_safety.h"
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>

/* Makes a socket connection to the server on the given IP address and port.
 * Returns the file description for the socket on success;
 * Returns FAILURE otherwise.
 */
int connect_to_server(char *addr, char *port) {
    int sockfd;

    // Create address
    struct addrinfo hints, *servinfo, *rp;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET6;  // IPv6
	hints.ai_socktype = SOCK_STREAM;

	// Get addrinfo of server
	int s = getaddrinfo(addr, port, &hints, &servinfo);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		return FAILED;
	}

	// Connect to first valid result
	for (rp = servinfo; rp != NULL; rp = rp->ai_next) {
		sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sockfd == -1)
			continue;

		if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) != -1)
			break; // success
		
		close(sockfd);
	}
	freeaddrinfo(servinfo);

	if (rp == NULL) {
		print_err(CONNECTION_FAILED);
		return FAILED;
	}

    return sockfd;
}