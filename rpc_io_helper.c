#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "rpc_io_helper.h"
#include "rpc_safety.h"


/* Fully writes `len` bytes of data from the buffer to the socket.
 * Returns the actual number of bytes written on success;
 * Returns FAILED on failure, or EMPTY if a write() returned 0.
 */
int write_all(int sockfd, char *buf, int len) {
    int total_bytes = 0;   // number of bytes sent
    int bytes_left = len;  // number of bytes left to send
    int n;

    while (total_bytes < len) {
        n = write(sockfd, buf + total_bytes, bytes_left);
        if (n <= 0) 
			return check_io_err(n, "write");
		
        total_bytes += n;
        bytes_left -= n;
    }
	
	// really shouldn't happen here
	if (total_bytes != len) {
		perror("write_all");
		return FAILED;
	}

    return total_bytes;
} 

/* Fully reads `len` bytes of data to the buffer from the socket.
 * Returns the actual number of bytes written on success;
 * Returns FAILED on failure, or EMPTY if a read() read nothing.
 */
int read_all(int sockfd, char *buf, int len) {
	int total_bytes = 0;   // number of bytes received
	int bytes_left = len;  // number of bytes left to receive
	int n;

	memset(buf, 0, len);  // clear this memory block
	while (total_bytes < len) {
		n = read(sockfd, buf + total_bytes, bytes_left);
		if (n <= 0)
			return check_io_err(n, "read");
		
		total_bytes += n;
		bytes_left -= n;
	}
	
	// really shouldn't happen here
	if (total_bytes != len) {
		perror("read_all");
		return FAILED;
	}

	return total_bytes;
}

/* Writes a 16-bit unsigned integer to the socket.
 * Returns the number of bytes written on success;
 * Returns FAILED on failure, or EMPTY if a write() returned 0.
 */
int write_u16(int sockfd, uint16_t u) {
	uint16_t val = htons(u);
	int n = write_all(sockfd, (char *)&val, U16_SIZE);
	if (n <= 0)
		return check_io_err(n, "write_all");
	
	if (n != U16_SIZE) {
		perror("write_u16");
		return FAILED;
	}

	return n;
}

/* Reads a 16-bit unsigned int from the socket and stores it at `u`.
 * Returns the number of bytes read on success;
 * Returns FAILED on failure, or EMPTY if a read() read nothing.
 */
int read_u16(int sockfd, uint16_t *u) {
	int n = read_all(sockfd, (char *)u, U16_SIZE);
	if (n <= 0)
		return check_io_err(n, "read_u16");
	
	*u = ntohs(*u);
	return n;
}

/* Writes a 32-bit unsigned int to the socket.
 * Returns the number of bytes written on success;
 * Returns FAILED on failure, or EMPTY if a write() returned 0.
 */
int write_u32(int sockfd, uint32_t u) {
	uint32_t val = htonl(u);
	int n = write_all(sockfd, (char *)&val, U32_SIZE);
	if (n <= 0)
		check_io_err(n, "write_u32");
	
	if (n != U32_SIZE) {
		perror("write_u32");
		return FAILED;
	}
	return n;
}

/* Reads a 32-bit unsigned int from the socket and stores it at `u`.
 * Returns the number of bytes read on success;
 * Returns FAILED on failure, or EMPTY if a read() read nothing.
 */
int read_u32(int sockfd, uint32_t *u) {
	int n = read_all(sockfd, (char *)u, U32_SIZE);
	if (n <= 0)
		return check_io_err(n, "read_u32");
	
	*u = ntohl(*u);
	return n;
}

/* Writes a 64-bit unsigned int to the socket.
 * Returns the number of bytes written on success;
 * Returns FAILED on failure, or EMPTY if a write() returned 0.
 */
int write_u64(int sockfd, uint64_t u) {
	uint64_t val = htonll(u);
	int n = write_all(sockfd, (char *)&val, U64_SIZE);
	if (n <= 0)
		return check_io_err(n, "write_u64");
	
	if (n != U64_SIZE) {
		perror("write_u64");
		return FAILED;
	}
	return n;
}

/* Reads a 64-bit unsigned int from the socket and stores it at `u`.
 * Returns the number of bytes read on success;
 * Returns FAILED on failure, or EMPTY if a read() read nothing.
 */
int read_u64(int sockfd, uint64_t *u) {
	int n = read_all(sockfd, (char *)u, U64_SIZE);
	if (n <= 0)
		return check_io_err(n, "read_u64");
	
	*u = ntohll(*u);
	return n;
}

/* Converts the unsigned integer `hostll` from host byte order to 
   network byte order.
 * Returns the converted result.
 */ 
uint64_t htonll(uint64_t hostll) {
	uint8_t buf[U64_SIZE]; 
	memcpy(&buf, &hostll, sizeof(buf));

	for (int i = 0; i < U64_SIZE; i++) {
		buf[i] = hostll >> (8 * (8-i-1));
	}
	return *(uint64_t *)&buf;
}

/* Converts the unsigned integer `netll` from network byte order to 
   host byte order.
 * Returns the converted result.
 *
 * Code adapted from: Beej's Guide to Network Programming
 * https://beej.us/guide/bgnet/html/
 * Modifications: Data types, parameter and return types, 
   use of buffer and `memcpy` (everything except for the 3rd line
   in the function definition).
 */ 
uint64_t ntohll(uint64_t netll) {
	uint8_t buf[U64_SIZE];
	memcpy(&buf, &netll, sizeof(buf));
    uint64_t u = ((uint64_t) buf[0] << 56) |
				 ((uint64_t) buf[1] << 48) |
                 ((uint64_t) buf[2] << 40) |
                 ((uint64_t) buf[3] << 32) |
                 ((uint64_t) buf[4] << 24) |
                 ((uint64_t) buf[5] << 16) |
                 ((uint64_t) buf[6] <<  8) |
                 buf[7];
	return u;
}

/* Wrapper function to write a prefix to the socket.
 * Returns the number of bytes written on success;
 * Returns FAILED on failure, or EMPTY if a write() returned 0.
 */
int write_prefix(int sockfd, int prefix) {
	return write_u32(sockfd, prefix);
}

/* Wrapper function to read a prefix from the socket.
 * Returns the prefix read on success;
 * Returns FAILED on failure, or EMPTY if a read() read nothing.
 */
uint32_t read_prefix(int sockfd) {
	uint32_t prefix;
	int n = read_u32(sockfd, &prefix);
	if (n <= 0)
		return n;

	// 32-bit int successfully read
	if (check_prefix(prefix) == FAILED) // but invalid prefix
		return FAILED;
	
	return prefix;
}

/* Writes a name to the socket (its length first, then the actual string).
 * Returns the number of bytes sent on success;
 * Returns FAILED (-1) on failure, or EMPTY (0) if an I/O operation returned 0.
 */
int write_name(int sockfd, char *name) {
	if (check_name(name) == FAILED)
		return FAILED;
	
	// send name length (cannot be over 16-bits under my rules)
    uint16_t name_len = strlen(name);
    int n = write_u16(sockfd, name_len);
	if (n <= 0)
		return n;

    // send name
    n = write_all(sockfd, name, name_len);
	if (n <= 0)
		return n;

	return n;
}

/* Reads a name from the socket (its length first, then the actual string).
 * Returns the name read on success, NULL on failure.
 */
char *read_name(int sockfd) {
	// read length of the name (limited to 16-bits)
	uint16_t name_len;
	int n = read_u16(sockfd, &name_len);
	if (n <= 0)
		return NULL;
	
	// read the actual name
	char *name = malloc(sizeof(char) * (name_len+1));
	if (!name) {
		print_err(MALLOC_FAILED);
		return NULL;
	}

	n = read_all(sockfd, name, name_len);
	if (n <= 0) {
		perror("read_name");
		return NULL;
	}
	name[name_len] = '\0'; // null-terminate the name

	return name;
}
