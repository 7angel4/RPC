/*-----------------------------------------------------------------------------
 * Project 2
 * Created by Angel He (angelh1@student.unimelb.edu.au) 09/05/2023
 * rpc_io_helper.h :
              = the interface of the module `rpc_io_helper` of the project 
              = includes the helper functions for I/O operations (read & write)
                on a variety of data types
 ----------------------------------------------------------------------------*/

#ifndef RPC_IO_HELPER_H
#define RPC_IO_HELPER_H

#include <stdint.h>

// size of some fixed width data types, in bytes
#define U64_SIZE 8
#define U32_SIZE 4
#define U16_SIZE 2


/* Fully writes `len` bytes of data from the buffer to the socket.
 * Returns the actual number of bytes written on success;
 * Returns FAILED on failure, or EMPTY if a write() returned 0.
 */
int write_all(int sockfd, char *buf, int len);

/* Fully reads `len` bytes of data to the buffer from the socket.
 * Returns the actual number of bytes written on success;
 * Returns FAILED on failure, or EMPTY if a read() read nothing.
 */
int read_all(int sockfd, char *buf, int len);

/* Writes a 16-bit unsigned integer to the socket.
 * Returns the number of bytes written on success;
 * Returns FAILED on failure, or EMPTY if a write() returned 0.
 */
int write_u16(int sockfd, uint16_t u);

/* Reads a 16-bit unsigned int from the socket and stores it at `u`.
 * Returns the number of bytes read on success;
 * Returns FAILED on failure, or EMPTY if a read() read nothing.
 */
int read_u16(int sockfd, uint16_t *u);

/* Writes a 32-bit unsigned int to the socket.
 * Returns the number of bytes written on success;
 * Returns FAILED on failure, or EMPTY if a write() returned 0.
 */
int write_u32(int sockfd, uint32_t u);

/* Reads a 32-bit unsigned int from the socket and stores it at `u`.
 * Returns the number of bytes read on success;
 * Returns FAILED on failure, or EMPTY if a read() read nothing.
 */
int read_u32(int sockfd, uint32_t *u);

/* Writes a 64-bit unsigned int to the socket.
 * Returns the number of bytes written on success;
 * Returns FAILED on failure, or EMPTY if a write() returned 0.
 */
int write_u64(int sockfd, uint64_t u);

/* Reads a 64-bit unsigned int from the socket and stores it at `u`.
 * Returns the number of bytes read on success;
 * Returns FAILED on failure, or EMPTY if a read() read nothing.
 */
int read_u64(int sockfd, uint64_t *u);

/* Converts the unsigned integer `hostll` from host byte order to 
   network byte order.
 * Returns the converted result.
 */ 
uint64_t htonll(uint64_t hostll);

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
uint64_t ntohll(uint64_t netll);

/* Wrapper function to write a prefix to the socket.
 * Returns the number of bytes written on success;
 * Returns FAILED on failure, or EMPTY if a write() returned 0.
 */
int write_prefix(int sockfd, int type);

/* Wrapper function to read a prefix from the socket.
 * Returns the prefix read on success;
 * Returns FAILED on failure, or EMPTY if a read() read nothing.
 */
uint32_t read_prefix(int sockfd);

/* Writes a name to the socket (its length first, then the actual string).
 * Returns the number of bytes sent on success;
 * Returns FAILED (-1) on failure, or EMPTY (0) if an I/O operation returned 0.
 */
int write_name(int sockfd, char *name);

/* Reads a name from the socket (its length first, then the actual string).
 * Returns the name read on success, NULL on failure.
 */
char *read_name(int sockfd);


#endif