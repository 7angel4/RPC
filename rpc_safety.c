#include "rpc_safety.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <assert.h>

/* Error messages */
static const char *ERROR_MSG[] = {
    "Invalid port",
    "Invalid IP address",
    "Invalid function name",
    "Invalid input",
    "Invalid data",
    "Invalid or no prefix",
    "Invalid handle",
    "Unknown request",
    "Function not found",
    "Function creation failed",
    "RPC call failed",
    "Connection failed",
    "Connection closed",
    "Memory allocation failed",
    "Overlength error"
};


/******* Private functions *******/
int is_valid_name(char *name);
int is_valid_ip(char *addr);
int is_valid_port(int port);
int is_valid_prefix(uint32_t prefix);
int is_valid_data2_len(size_t data_len);
int is_valid_data(rpc_data *data);


/* Checks the returned value (`n`) of a system call.
 * If `n` indicates an error, prints the given error message 
   and returns FAILURE.
 * Otherwise returns `n` itself.
 */
int check_sys_call(int n, char *msg) {
    if (n < 0) {
        perror(msg);
        return FAILED;
    }
    return n;
}

/* Checks the returned value (`n`) of an I/O operation.
 * If `n` indicates an error or is 0, prints the relevant error message 
   and returns FAILURE (for error) or EMPTY (for 0).
 * Otherwise returns `n` itself.
 */
int check_io_err(int n, char *msg) {
    if (n == 0) {
        print_err(CONNECTION_CLOSED);
        return EMPTY;
    } else if (n < 0) {
        perror(msg);
        return FAILED;
    }
    return n;
}

/* Checks the validity of the port number.
 * If invalid, prints the relevant error message and returns FAILED.
 * Returns SUCCESS otherwise.
 */
int check_port(int port) {
    if (is_valid_port(port) == FALSE) {
        print_err(INVALID_PORT);
        return FAILED;
    }
    return SUCCESS;
}

/* Checks the validity of the IP address.
 * If invalid, prints the relevant error message and returns FAILED.
 * Returns SUCCESS otherwise.
 */
int check_ip(char *addr) {
    if (is_valid_ip(addr) == FALSE) {
        print_err(INVALID_IP);
        return FAILED;
    }
    return SUCCESS;
}

/* Checks the validity of the name.
 * If invalid, prints the relevant error message and returns FAILED.
 * Returns SUCCESS otherwise.
 */
int check_name(char *name) {
    if (is_valid_name(name) == FALSE) {
        print_err(INVALID_NAME);
        return FAILED;
    }
    return SUCCESS;
}

/* Checks the validity of the prefix.
 * If invalid, prints the relevant error message and returns FAILED.
 * Returns SUCCESS otherwise.
 */
int check_prefix(uint32_t prefix) {
    if (is_valid_prefix(prefix) == FALSE) {
        print_err(INVALID_PREFIX);
        return FAILED;
    }
    return SUCCESS;
}

/* Checks the validity of the rpc_data.
 * If invalid, prints the relevant error message and returns FAILED.
 * Returns SUCCESS otherwise.
 */
int check_rpc_data(rpc_data *data) {
    if (is_valid_data(data) == FALSE) {
        print_err(INVALID_DATA);
        return FAILED;
    }
    return SUCCESS;
}

/* Prints the corresponding error message to the given index.
 */
void print_err(enum ERROR err) {
    fprintf(stderr, "%s\n", ERROR_MSG[err]);
}


/* Returns TRUE if the name is valid, FALSE otherwise.
 */
int is_valid_name(char *name) {
    if (name == NULL) {
        print_err(INVALID_NAME);
        return FALSE;
    }
    // check length
    size_t len = strlen(name);
    if (len < MIN_NAME_LEN || len > MAX_NAME_LEN)
        return FALSE;
    
    // check characters
    for (int i = 0; i < len; i++) {
        if (name[i] < MIN_NAME_CHAR || name[i] > MAX_NAME_CHAR)
            return FALSE;
    }
    return TRUE;
}

/* Returns TRUE if the address is a valid IPv6 address, FALSE otherwise.
 */
int is_valid_ip(char *addr) {
    if (addr == NULL || !*addr) {
        return FALSE;
    }
    struct sockaddr_in sa;
    return inet_pton(AF_INET6, addr, &(sa.sin_addr)) != 0 ? TRUE : FALSE;
}

/* Returns TRUE if the port number is valid, FALSE otherwise.
 */
int is_valid_port(int port) {
    // port 0 is reserved and cannot be used for TCP
    return port >= 1 && port <= UINT16_MAX;
}

/* Returns TRUE if the prefix is valid, FALSE otherwise.
 */
int is_valid_prefix(uint32_t prefix) {
	// assume FIND_REQ is the first in the enum PREFIX
	// and CLOSE_REQ is the last
	return prefix >= FIND_REQ && prefix <= CLOSE_REQ;
}

/* Returns TRUE if the data length is valid, FALSE otherwise.
 */
int is_valid_data2_len(size_t data2_len) {
    // data2_len must be in [0, UINT32_MAX]
    return data2_len >= 0 && data2_len <= MAX_DATA2_LEN;
}

/* Returns TRUE if the data is valid, FALSE otherwise.
 */
int is_valid_data(rpc_data *data) {
    // Case 1: NULL data
    if (data == NULL)
        return FALSE;

    // Case 2: invalid data2 length
    if (is_valid_data2_len(data->data2_len) == FALSE) {
        if (data->data2_len > MAX_DATA2_LEN) {
            // data2_len is too large for this packet format
            print_err(OVERLENGTH);
        }
        return FALSE;
    }

    // Case 3: data2_len inconsistent with data2
    if (data->data2_len == 0 && data->data2 != NULL)
        return FALSE;
    if (data->data2_len != 0 && data->data2 == NULL)
        return FALSE;
    
    return TRUE;
}