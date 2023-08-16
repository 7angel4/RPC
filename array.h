/*-----------------------------------------------------------------------------
 * Project 2
 * Created by Angel He (angelh1@student.unimelb.edu.au) 09/05/2023
 * array.h :
              = the interface of the module `array` of the project 
              = provides a polymorphic dynamic array
 * Note: This module is partially adapted from my COMP20003 
  (Algorithms and Data Structures) Assignment 2: PR Quadtrees, 
  submitted on 08/09/2022
 ----------------------------------------------------------------------------*/

#ifndef ARRAY_H
#define ARRAY_H

#define INIT_SIZE 20

typedef struct array array_t;

/* Creates and returns an empty array, 
   attached with the given comparison and freeing functions.
 */
array_t *create_array(int (*cmp) (void *, void *), void (*free_elem) (void *));

/* Frees the memory used by the array, including its elements.
 */
void free_array(array_t *arr);

/* Ensures that the array's capacity exceeds its number of elements
 * i.e. there is space for adding new elements.
 * Returns SUCCESS on sucess, FAILED otherwise.
 */
int ensure_array_capacity(array_t *arr);

/* Appends the value to the end of the array. 
 * Returns SUCESS on success, FAILED otherwise. 
 */
int array_append(array_t *arr, void *value);

/* Searches for the key in the array.
 * Returns the index of the corresponding element in the array if found, 
   FAILED otherwise.
 */
int search_array(array_t *arr, void *key);

/* Returns the element as the given index in the array.
 * Returns NULL if the input is invalid.
 */
void *get_elem_at(array_t *arr, int idx);

/* Returns TRUE if the index is valid for this array, FALSE otherwise.
 */
int is_valid_idx(array_t *arr, int idx);

/* Prints the data stored in the array.
 */
void print_array(array_t *arr, void (*print_value) (void *));

#endif