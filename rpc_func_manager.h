/*-----------------------------------------------------------------------------
 * Project 2
 * Created by Angel He (angelh1@student.unimelb.edu.au) 09/05/2023
 * rpc_func_manager.h :
              = the interface of the module `rpc_func_manager` of the project 
              = includes the general functions for managing RPC functions
 ----------------------------------------------------------------------------*/

#ifndef RPC_FUNC_MANAGER_H
#define RPC_FUNC_MANAGER_H

#include "array.h"
#include "rpc.h"

// storage structure in server
typedef struct {
    char *name;
    rpc_handler handler;
} rpc_func;

/* Creates a function with the given name and handler.
 * Returns the pointer to the rpc_func on success, NULL otherwise.
 */
rpc_func *create_rpc_func(char *name, rpc_handler handler);

/* Compares the RPC function's name to a string.
 * Returns 0 if they are equal,
 * negative if the function's name compares less than the string, 
 * positive otherwise.
 */
int cmp_func_name(void *func, void *s);

/* Replaces the handler of the RPC function at index `idx` 
   in the array `functions`.
 * Returns SUCCESS on success, FAILED otherwise
 */
int replace_func(array_t *functions, int idx, rpc_handler new_handler);

/* Frees the RPC function.
 */
void free_rpc_func(void *f);

#endif