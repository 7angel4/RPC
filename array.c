#include <stdio.h>
#include <stdlib.h>
#include "array.h"
#include "rpc_safety.h"

struct array {
    void **A;                    // the actual array
    int n;                       // number of occupied positions
    int capacity;                // maximum number of values
    int (*cmp) (void *, void *); // for searching values
    void (*free_elem) (void *);  // for freeing the elements
};


/* Creates and returns an empty array, 
   attached with the given comparison and freeing functions.
 */
array_t *create_array(int (*cmp) (void *, void *), void (*free_elem) (void *)) {
    array_t *arr = malloc(sizeof(*arr));
    if (!arr) {
        print_err(MALLOC_FAILED);
        return NULL;
    }

    arr->A = malloc(sizeof(*(arr->A)) * INIT_SIZE);
    if (!arr->A) {
        print_err(MALLOC_FAILED);
        return NULL;
    }
    
    arr->n = 0;
    arr->capacity = INIT_SIZE;
    arr->cmp = cmp;
    arr->free_elem = free_elem;

    return arr;
}

/* Frees the memory used by the array, including its elements.
 */
void free_array(array_t *arr) {
    if (!arr)
        return;
        
    // freeing inside out
    for (int i = 0; i < arr->n; i++) {
        arr->free_elem(arr->A[i]);
        arr->A[i] = NULL;
    }
    free(arr->A);
    arr->A = NULL;
    free(arr);
    arr = NULL;
}

/* Ensures that the array's capacity exceeds its number of elements
 * i.e. there is space for adding new elements.
 * Returns SUCCESS on sucess, FAILED otherwise.
 */
int ensure_array_capacity(array_t *arr) {
    if (!arr)
        return FAILED;

    // still have space for insertion
    if (arr->n < arr->capacity)
        return SUCCESS;
    
    // need more space
    int new_capacity = (arr->capacity) * 2;
    void **new = realloc(arr->A, sizeof(*(arr->A)) * new_capacity);
    if (!new) {
        fprintf(stderr, "array realloc\n");
        return FAILED;
    }

    arr->A = new;
    arr->capacity = new_capacity;
    return SUCCESS;
}

/* Appends the value to the end of the array. 
 * Returns SUCESS on success, FAILED otherwise. 
 */
int array_append(array_t *arr, void *value) {
    if (!arr || !value) { 
        return FAILED;
    }
    if (ensure_array_capacity(arr) == FAILED) 
        return FAILED;
    
    arr->A[(arr->n)++] = value;
    return SUCCESS;
}

/* Searches for the key in the array.
 * Returns the index of the corresponding element in the array if found, 
   FAILED otherwise.
 */
int search_array(array_t *arr, void *key) {
    if (!arr || !key) 
        return FAILED;
    for (int i = 0; i < arr->n; i++) {
        if (arr->cmp(arr->A[i], key) == 0) 
            return i;
    }
    return FAILED;
}

/* Returns the element as the given index in the array.
 * Returns NULL if the input is invalid.
 */
void *get_elem_at(array_t *arr, int idx) {
    if (!arr || !is_valid_idx(arr, idx)) {
        print_err(INVALID_INPUT);
        return NULL;
    }
    return arr->A[idx];
}

/* Returns TRUE if the index is valid for this array, FALSE otherwise.
 */
int is_valid_idx(array_t *arr, int idx) {
    if (arr == NULL)
        return FALSE;
    return (idx >= 0 && idx < arr->n) ? TRUE : FALSE;
}

/* Prints the data stored in the array.
 */
void print_array(array_t *arr, void (*print_value) (void *)) {
    if (!arr) 
        return;
    for (int i = 0; i < arr->n; i++) {
        print_value(arr->A[i]);
    }
}