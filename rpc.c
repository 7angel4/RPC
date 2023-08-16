#include "rpc.h"
#include "rpc_io_helper.h"
#include "array.h"
#include "rpc_safety.h"
#include "rpc_func_manager.h"
#include "rpc_server_helper.h"
#include "rpc_client_helper.h"

#include <stdlib.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define NONBLOCKING
#define MIN_CONCURRENT_CLNTS 10
#define PORT_LEN 6 // length of a port number = max 5 digits, with a null byte

/* Client states */
enum CLNT_STATE {OPEN = 0, CLOSED = 1};

/* Note: We deal with the main logic (i.e. handling requests, manipulating 
   rpc_data...) here, and use the helper modules for the more general / 
   less important "side" logic.
 */

/* Server side */
void rpc_close_server(rpc_server *srv);
int handle_request(rpc_server *srv, int sockfd);
int handle_find(rpc_server *srv, int sockfd);
int handle_call(rpc_server *srv, int sockfd);

/* Client side */
int init_connection(rpc_client *cl);

/* General */
int write_rpc_data(int sockfd, rpc_data *data);
rpc_data *read_rpc_data(int sockfd);
rpc_data *create_rpc_data();
rpc_handle *create_rpc_handle(uint32_t idx);


/* ------------- */
/*  Server side  */
/* ------------- */

struct rpc_server {
    int listening_sd;   // listening socket
    array_t *functions; // registered functions
};

struct rpc_handle {
    uint32_t idx; // index of the handler in the server's RPC functions array
};

/* Initialises server state */
/* RETURNS: rpc_server* on success, NULL on error */
rpc_server *rpc_init_server(int port) {
    if (check_port(port) == FAILED)
        return NULL;

    // Create server
    rpc_server *srv = malloc(sizeof(*srv));
    if (!srv) {
        print_err(MALLOC_FAILED);
        return NULL;
    }

    // Create listening socket
    char port_str[PORT_LEN];
    snprintf(port_str, PORT_LEN, "%d", port);
    int listening_sd = create_listening_socket(port_str);
    if (listening_sd == FAILED) {
        free(srv);
        srv = NULL;
        return NULL;
    }
    srv->listening_sd = listening_sd;
    
    // Create the array structure to hold our RPC functions
    srv->functions = create_array(cmp_func_name, free_rpc_func);
    if (srv->functions == NULL) {
        rpc_close_server(srv); // clean up for previous mallocs
        return NULL;
    }

    // Set up for later - to get rid of zombie processes
    if (set_up_sigchld_handler() == FAILED) {
        rpc_close_server(srv);
        return NULL;
    }

    // Listen on socket - now ready to accept connections
	if (listen(listening_sd, MIN_CONCURRENT_CLNTS) < 0) {
        perror("listen");
        rpc_close_server(srv);
		return NULL;
	}

    return srv;
}

/* Registers a function (mapping from name to handler) */
/* RETURNS: FAILED (-1) on failure */
int rpc_register(rpc_server *srv, char *name, rpc_handler handler) {
    if (srv == NULL || srv->functions == NULL 
            || name == NULL || handler == NULL) {
        print_err(INVALID_INPUT);
        return FAILED; 
    }
    
    if (check_name(name) == FAILED) // invalid name
        return FAILED;
    
    // Check if the name is already registered
    int func_idx = search_array(srv->functions, name);
    if (func_idx != FAILED) { 
        // name found -> replace the original function
        return replace_func(srv->functions, func_idx, handler);
    } 

    // name not found -> create new function and append
    rpc_func *func = create_rpc_func(name, handler);
    if (!func || array_append(srv->functions, func) == FAILED) {
        print_err(FUNC_CREATION_FAILED);
        return FAILED;
    }

    return SUCCESS;
}

/* Start serving requests */
void rpc_serve_all(rpc_server *srv) {
    if (srv == NULL) {
        print_err(INVALID_INPUT);
        return;
    }

    int newsockfd, res;
    while (1) {
        newsockfd = accept_connection(srv->listening_sd);
        if (newsockfd < 0) { // failed
            continue;
        } 
        // child process to handle that connection
        pid_t childpid;
        if ((childpid = fork()) == -1) {
            perror("fork");
            continue;

        } else if (childpid == 0) { // child process
            close(srv->listening_sd); // child doesn't need this
            
            do {
                res = handle_request(srv, newsockfd);
            } while (res > 0); // no error and connection not closed

            close(newsockfd);
            exit(EXIT_SUCCESS);

        } else {
            close(newsockfd); // parent doesn't need this
        }
	}

    // shouldn't reach here
    rpc_close_server(srv);
}

/* Handles a FIND request from the socket.
 * Returns SUCCESS (1) on success of responding to the request
   (i.e. regardless of the result of the FIND),
 * FAILED (-1) on error, or 0 if an I/O operation returned 0.
 */
int handle_find(rpc_server *srv, int sockfd) {
    char *name = read_name(sockfd); // reads length of name and the name
    if (check_name(name) == FAILED) { // really shouldn't happen
        return FAILED;
    }

    int idx, n = 0;
    if ((idx = search_array(srv->functions, name)) == FAILED) {
        // function not found -> respond with failure status
        print_err(FUNC_NOT_FOUND);
        n = write_prefix(sockfd, FAILURE_STAT);
        if (n <= 0)
            return n; // FAILED or EMPTY

    } else { 
        // function found -> respond with success status
        n = write_prefix(sockfd, SUCCESS_STAT);
        if (n <= 0)
            return n;
        // also send the handle
        n = write_u32(sockfd, idx);
        if (n <= 0) 
            return n;
    }

    free(name); // malloced for `name` in read_name()
    name = NULL;
    return SUCCESS;
}

/* Handles a CALL request from the socket.
 * Returns SUCCESS on success of responding to the request
   (i.e. regardless of the result of the CALL),
 * FAILED on error, or 0 if an I/O operation returned 0.
 */
int handle_call(rpc_server *srv, int sockfd) {
    uint32_t idx;
    int n = read_u32(sockfd, &idx); // read the function index (RPC handle)
    if (n <= 0) 
        return n;
    // function input (validity already checked -> NULL if invalid)
    rpc_data *input = read_rpc_data(sockfd);

    // get the actual RPC function
    rpc_func *func = get_elem_at(srv->functions, idx);
    if (input == NULL || func == NULL) {
        if (input == NULL)
            print_err(INVALID_INPUT);
        if (func == NULL) {
            print_err(FUNC_NOT_FOUND);
            if (input != NULL) {  // clean up the data
                rpc_data_free(input);
                input = NULL;
            }
        }
        // call failed
        write_prefix(sockfd, FAILURE_STAT);
        // routine failure, not a system error
        return SUCCESS;
    }
    
    // All good now, let's call the actual remote procedure
    rpc_data *result = (((rpc_handler)(func->handler))(input));
    if (check_rpc_data(result) == FAILED) { // really shouldn't happen
        rpc_data_free(result);
        result = NULL;
        write_prefix(sockfd, FAILURE_STAT);
        return SUCCESS;
    }

    // Tell the client the call succeeded
    n = write_prefix(sockfd, SUCCESS_STAT);
    if (n <= 0)
        return n;

    // "Here's your result"
    n = write_rpc_data(sockfd, result);
    if (n <= 0)
        return n;
    
    // No longer needed
    rpc_data_free(input);
    input = NULL;
    rpc_data_free(result);
    result = NULL;

    return SUCCESS;
}

/* Handles a request from the socket.
 * Returns SUCCESS on success of responding to the request
   (i.e. regardless of the result of that request),
 * FAILED on error, or 0 if an I/O operation returned 0.
 */
int handle_request(rpc_server *srv, int sockfd) {
    int prefix = read_prefix(sockfd);
    if (check_prefix(prefix) == FAILED)
        return FAILED;
    
    int req_result = FAILED;
    switch (prefix) {
        case FIND_REQ: // rpc_find request
            req_result = handle_find(srv, sockfd);
            break;
        
        case CALL_REQ: // rpc_call request
            req_result = handle_call(srv, sockfd);
            break;
        
        case CLOSE_REQ: // explicit closing request
            req_result = EMPTY; // i.e. no more I/O ops
            break;
        
        default:
            print_err(UNKNOWN_REQ);
    }
    return req_result;
}

/* Cleans up server state and closes server */
void rpc_close_server(rpc_server *srv) {
    if (srv == NULL)
        return;
    // close listening socket
    close(srv->listening_sd);

    free_array(srv->functions);
    srv->functions = NULL;
    free(srv);
    srv = NULL;
}


/* ------------- */
/*  Client side  */
/* ------------- */

struct rpc_client {
    char addr[INET6_ADDRSTRLEN];  // IP address
    char port[PORT_LEN];          // port number as a string
    int sockfd;                   // socket for connection
    int state;                    // open or closed?
};

/* Initialises client state */
/* RETURNS: rpc_client* on success, NULL on error */
rpc_client *rpc_init_client(char *addr, int port) {
    // Check input validity
    if (!addr || check_ip(addr) == FAILED || check_port(port) == FAILED) {
        print_err(INVALID_INPUT);
        return NULL;
    }
        
    // Create client
    rpc_client *cl = malloc(sizeof(*cl));
    if (!cl) { // malloc failed
        print_err(MALLOC_FAILED);
        return NULL;
    }

    strcpy(cl->addr, addr);
    snprintf(cl->port, PORT_LEN, "%d", port); // store port number as a string
    cl->state = CLOSED; // no connection yet

    return cl;
}

/* Finds a remote function by name */
/* RETURNS: rpc_handle* on success, NULL on error */
/* rpc_handle* will be freed with a single call to free(3) */
rpc_handle *rpc_find(rpc_client *cl, char *name) {
    if (cl == NULL || name == NULL || check_name(name) == FAILED) {
        print_err(INVALID_INPUT);
        return NULL;
    }

    // initiate a connection request
    init_connection(cl);

    // send FIND request
    int n = write_prefix(cl->sockfd, FIND_REQ);
    if (n <= 0)
        return NULL;
    
    // send name
    n = write_name(cl->sockfd, name);
    if (n <= 0)
        return NULL;
    
    // read the server's response
    int prefix = read_prefix(cl->sockfd);
    if (prefix == FAILURE_STAT) { // find failed
        print_err(FUNC_NOT_FOUND);
        return NULL;
    } else if (check_prefix(prefix) == FAILED) {  // other errors
        return NULL;
    }
    
    // FIND successful -> continue reading for the handle
    uint32_t func_idx;
    n = read_u32(cl->sockfd, &func_idx);
    if (n <= 0)
        return NULL;
    // Create the handle based on the target function's index
    rpc_handle *handle = create_rpc_handle(func_idx);

    return handle; // either a valid handle or NULL
}

/* Initiates a connection for the client (if not yet initialized).
 * Returns FAILED on error, SUCCESS otherwise.
 */
int init_connection(rpc_client *cl) {
    if (cl->state == OPEN) // we can keep using the current socket
        return SUCCESS;
    
    int sockfd = connect_to_server(cl->addr, cl->port);
    if (sockfd == FAILED)
        return FAILED;
    
    cl->sockfd = sockfd;
    cl->state = OPEN;
    return SUCCESS;
}

/* Calls remote function using handle */
/* RETURNS: rpc_data* on success, NULL on error */
rpc_data *rpc_call(rpc_client *cl, rpc_handle *h, rpc_data *payload) {
    if (cl == NULL || h == NULL || check_rpc_data(payload) == FAILED) {
        print_err(INVALID_INPUT);
        return NULL;
    }

    // send request
    int n = write_prefix(cl->sockfd, CALL_REQ);
    if (n <= 0) 
        return NULL;
    // send handle
    n = write_u32(cl->sockfd, h->idx);
    if (n <= 0)
        return NULL;
    // send the data
    n = write_rpc_data(cl->sockfd, payload);
    if (n <= 0)
        return NULL;

    // read response
    int prefix = read_prefix(cl->sockfd);
    if (prefix == FAILURE_STAT) { // call failed
        print_err(CALL_FAILED);
        return NULL;
    } else if (check_prefix(prefix) == FAILED) { // other errors
        return NULL;
    }
    
    return read_rpc_data(cl->sockfd); // either a valid (rpc_data *) or NULL
}

/* Cleans up client state and closes client */
void rpc_close_client(rpc_client *cl) {
    if (cl == NULL) // already closed
        return;
    if (cl->state == OPEN) {
        // Tell the server: "I'm closing"
        write_prefix(cl->sockfd, CLOSE_REQ);
        // We can close without checking the write here
        // Server closes the connection anyway if it finds nothing to read
        close(cl->sockfd);
        cl->state = CLOSED;
    }

    free(cl);
    cl = NULL;
}


/* ------------- */
/*    General    */
/* ------------- */

/* Frees a rpc_data struct */
void rpc_data_free(rpc_data *data) {
    if (data == NULL) {
        return;
    }
    if (data->data2 != NULL) {
        free(data->data2);
        data->data2 = NULL;
    }
    free(data);
    data = NULL;
}

/* Writes a rpc_data struct to the socket.
 * Returns SUCCESS on success, FAILED on failure, 
   or EMPTY if an I/O operation returned 0.
 */
int write_rpc_data(int sockfd, rpc_data *data) {
    // check data validity
    if (check_rpc_data(data) == FAILED) {
        return FAILED;
    }
    
	// write data1
	int n = write_u64(sockfd, data->data1);
    if (n <= 0) {
		return check_io_err(n, "write_rpc_data");
	}
    
	// write data2_len
	n = write_u32(sockfd, data->data2_len);
    if (n <= 0) {
		return check_io_err(n, "write_rpc_data");
	}
    
    // only write data2 if it exists
    if (data->data2_len > 0) {
        void *data2 = data->data2;
	    n = write_all(sockfd, data2, data->data2_len);
		if (n <= 0) {
			return check_io_err(n, "write_rpc_data");
		}
    }

    return SUCCESS;
}

/* Reads a rpc_data struct from the socket.
 * Returns the rpc_data read on success, NULL otherwise.
 */
rpc_data *read_rpc_data(int sockfd) {
    // read data 1
	uint64_t data1;
	int n = read_u64(sockfd, &data1);
    if (n <= 0) {
		return NULL;
	}
    
	// read data2
	uint32_t data2_len;
	n = read_u32(sockfd, &data2_len);
    if (n <= 0) {
		return NULL;
	}

    // store what's been read
    rpc_data *new_data = create_rpc_data();
    if (!new_data) {
		return NULL;
	}
	new_data->data1 = data1;
	new_data->data2_len = data2_len;

    // only read data2 if it exists
    if (data2_len > 0) {
	    new_data->data2 = malloc(data2_len);
        if (new_data->data2 == NULL) {
            print_err(MALLOC_FAILED);
            return NULL;
        }
        n = read_all(sockfd, new_data->data2, data2_len);
        if (n <= 0) {
			return NULL;
		}

    } else { // data2 is empty
        new_data->data2 = NULL;
    }

    // now check if the read data is valid
    if (check_rpc_data(new_data) == FAILED) {
        // Failed -> free all that endeavour
        rpc_data_free(new_data);
        new_data = NULL;
    }
	
	return new_data;
}

/* Allocates memory for and returns a pointer to a rpc_data struct.
 */
rpc_data *create_rpc_data() {
	rpc_data *d = malloc(sizeof(*d));
	if (!d) {
        print_err(MALLOC_FAILED);
        return NULL;
    }
	return d;
}

/* Creates and returns a pointer to a rpc_handle with the given index,
   which represents the index of the corresponding RPC function 
   stored in the server's functions array.
 */
rpc_handle *create_rpc_handle(uint32_t idx) {
	rpc_handle *h = malloc(sizeof(*h));
	if (!h) {
		print_err(MALLOC_FAILED);
		return NULL;
	}
	h->idx = idx;
	return h;
}
