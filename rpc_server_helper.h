/*-----------------------------------------------------------------------------
 * Project 2
 * Created by Angel He (angelh1@student.unimelb.edu.au) 09/05/2023
 * rpc_server_helper.h :
              = the interface of the module `rpc_server_helper` of the project 
              = includes some general helper functions for the server side
 ----------------------------------------------------------------------------*/

#ifndef RPC_SERVER_HELPER_H
#define RPC_SERVER_HELPER_H


/* Creates a listening socket that listens on the given port.
 * Returns the new socket's file descriptor on success;
 * Returns FAILED (-1) on failure.
 */
int create_listening_socket(char* service);

/* Handler for SIGCHLD.
 *
 * Code adapted from: Beej's Guide to Network Programming
 * https://beej.us/guide/bgnet/html/
 * Author: Brian “Beej Jorgensen” Hall
 * Also looked up "man waitpid"
 * Modifications: comments and coding style
 */
void sigchld_handler(int s);

/* Sets up the SIGCHLD handler.
 * Returns FAILED if the set-up failed, SUCCESS otherwise.
 *
 * Code adapted from: Beej's Guide to Network Programming
 * https://beej.us/guide/bgnet/html/
 * Author: Brian “Beej Jorgensen” Hall
 * Modifications: converted to function with return values
 */
int set_up_sigchld_handler();

/* Accepts a connection request on the queue of pending connections 
   for the listening socket.
 * Returns a file descriptor for the accepted socket on success;
 * Returns FAILURE (-1) on failure.
 */
int accept_connection(int listening_sd);


#endif