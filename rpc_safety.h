/*-----------------------------------------------------------------------------
 * Project 2
 * Created by Angel He (angelh1@student.unimelb.edu.au) 09/05/2023
 * rpc_safety.h :
              = the interface of the module `rpc_safety` of the project 
              = defines a list of errors, validity checks, and rules for the
                RPC system
 ----------------------------------------------------------------------------*/

#ifndef RPC_SAFETY_H
#define RPC_SAFETY_H

#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include "rpc.h"

#define SUCCESS 1
#define EMPTY 0
#define FAILED -1
#define TRUE 1
#define FALSE 0

#define MIN_NAME_LEN 1
#define MAX_NAME_LEN UINT16_MAX
#define MIN_NAME_CHAR 32
#define MAX_NAME_CHAR 126
#define MAX_DATA2_LEN UINT32_MAX

// Prefixes (indicating the type of request)
enum PREFIX {FIND_REQ = 1, CALL_REQ = 2, CLOSE_REQ = 3};
// Request status (indicating the type of response)
enum REQ_STATUS {FAILURE_STAT = 1, SUCCESS_STAT = 2};

// Errors
enum ERROR {
    INVALID_PORT,
    INVALID_IP,
    INVALID_NAME,
    INVALID_INPUT,
    INVALID_DATA,
    INVALID_PREFIX,
    INVALID_HANDLE,
    UNKNOWN_REQ,
    FUNC_NOT_FOUND,
    FUNC_CREATION_FAILED,
    CALL_FAILED,
    CONNECTION_FAILED,
    CONNECTION_CLOSED,
    MALLOC_FAILED,
    OVERLENGTH
};


/* Checks the returned value (`n`) of a system call.
 * If `n` indicates an error, prints the given error message 
   and returns FAILURE.
 * Otherwise returns `n` itself.
 */
int check_sys_call(int n, char *msg);

/* Checks the returned value (`n`) of an I/O operation.
 * If `n` indicates an error or is 0, prints the relevant error message 
   and returns FAILURE (for error) or EMPTY (for 0).
 * Otherwise returns `n` itself.
 */
int check_io_err(int n, char *msg);

/* Checks the validity of the port number.
 * If invalid, prints the relevant error message and returns FAILED.
 * Returns SUCCESS otherwise.
 */
int check_port(int port);

/* Checks the validity of the IP address.
 * If invalid, prints the relevant error message and returns FAILED.
 * Returns SUCCESS otherwise.
 */
int check_ip(char *addr);

/* Checks the validity of the name.
 * If invalid, prints the relevant error message and returns FAILED.
 * Returns SUCCESS otherwise.
 */
int check_name(char *name);

/* Checks the validity of the data length.
 * If invalid, prints the relevant error message and returns FAILED.
 * Returns SUCCESS otherwise.
 */
int check_data_len(size_t data_len);

/* Checks the validity of the prefix.
 * If invalid, prints the relevant error message and returns FAILED.
 * Returns SUCCESS otherwise.
 */
int check_prefix(uint32_t prefix);

/* Checks the validity of the rpc_data.
 * If invalid, prints the relevant error message and returns FAILED.
 * Returns SUCCESS otherwise.
 */
int check_rpc_data(rpc_data *data);

/* Prints the corresponding error message to the given index.
 */
void print_err(enum ERROR err);

#endif