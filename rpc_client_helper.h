/*-----------------------------------------------------------------------------
 * Project 2
 * Created by Angel He (angelh1@student.unimelb.edu.au) 09/05/2023
 * rpc_server_helper.h :
              = the interface of the module `rpc_client_helper` of the project 
              = includes some general helper functions for the client side
 ----------------------------------------------------------------------------*/

#ifndef RPC_CLIENT_HELPER_H
#define RPC_CLIENT_HELPER_H

/* Yes, short, but extensible. */

/* Makes a socket connection to the server on the given IP address and port.
 * Returns the file description for the socket on success;
 * Returns FAILURE otherwise.
 */
int connect_to_server(char *addr, char *port);

#endif