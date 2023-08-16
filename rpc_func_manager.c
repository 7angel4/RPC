#include "rpc_func_manager.h"
#include "rpc_safety.h"
#include <string.h>
#include <stdlib.h>

/* Creates a function with the given name and handler.
 * Returns the pointer to the rpc_func on success, NULL otherwise.
 */
rpc_func *create_rpc_func(char *name, rpc_handler handler) {
    if (!name || !handler || check_name(name) == FAILED) {
        return NULL;
    }

    rpc_func *f = malloc(sizeof(*f));
    if (!f) {
        print_err(MALLOC_FAILED);
        return NULL;
    }
    
    f->name = strdup(name);
    f->handler = handler;
    return f;
}

/* Compares the RPC function's name to a string.
 * Returns 0 if they are equal,
 * negative if the function's name compares less than the string, 
 * positive otherwise.
 */
int cmp_func_name(void *func, void *s) {
    return strcmp(((rpc_func *)func)->name, (char *)s);
}

/* Replaces the handler of the RPC function at index `idx` 
   in the array `functions`.
 * Returns SUCCESS on success, FAILED otherwise.
 */
int replace_func(array_t *functions, int idx, rpc_handler new_handler) {
	if (!functions || !is_valid_idx(functions, idx) || !new_handler) {
        return FAILED;
    }
	
    rpc_func *func = get_elem_at(functions, idx);
    if (func == NULL) {
        return FAILED;
    }

	func->handler = new_handler;
    return SUCCESS;
}

/* Frees the RPC function.
 */
void free_rpc_func(void *f) {
    if (f == NULL)
        return;

    rpc_func *func = f;
    free(func->name);
    func->name = NULL;
    free(func);
    func = NULL;
}
