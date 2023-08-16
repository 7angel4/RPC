#include "rpc.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#define IP_ADDR 'i'
#define PORT 'p'
#define NUM_ARGS 2

void read_args(int argc, char *argv[], int *ip_idx, int *port);

int main(int argc, char *argv[]) {
    int exit_code = 0;

    int ip_idx, port;
    read_args(argc, argv, &ip_idx, &port);
    rpc_client *state = rpc_init_client(argv[ip_idx], port);
    if (state == NULL) {
        exit(EXIT_FAILURE);
    }

    rpc_handle *handle_add2 = rpc_find(state, "add2");
    if (handle_add2 == NULL) {
        fprintf(stderr, "ERROR: Function add2 does not exist\n");
        exit_code = 1;
        goto cleanup;
    }

    for (int i = 0; i < 2; i++) {
        /* Prepare request */
        char left_operand = i;
        char right_operand = 100;
        rpc_data request_data = {
            .data1 = left_operand, .data2_len = 1, .data2 = &right_operand};

        /* Call and receive response */
        rpc_data *response_data = rpc_call(state, handle_add2, &request_data);
        if (response_data == NULL) {
            fprintf(stderr, "Function call of add2 failed\n");
            exit_code = 1;
            goto cleanup;
        }

        /* Interpret response */
        assert(response_data->data2_len == 0);
        assert(response_data->data2 == NULL);
        printf("Result of adding %d and %d: %d\n", left_operand, right_operand,
               response_data->data1);
        rpc_data_free(response_data);
    }

    rpc_handle *handle_subtract2 = rpc_find(state, "subtract2");
    if (handle_subtract2 == NULL) {
        fprintf(stderr, "ERROR: Function subtract2 does not exist\n");
        exit_code = 1;
        goto cleanup;
    }
    for (int i = 0; i < 2; i++) {
        /* Prepare request */
        char left_operand = i;
        char right_operand = 100;
        rpc_data request_data = {
            .data1 = left_operand, .data2_len = 1, .data2 = &right_operand};

        /* Call and receive response */
        rpc_data *response_data = rpc_call(state, handle_subtract2, &request_data);
        if (response_data == NULL) {
            fprintf(stderr, "Function call of subtract2 failed\n");
            exit_code = 1;
            goto cleanup;
        }

        /* Interpret response */
        assert(response_data->data2_len == 0);
        assert(response_data->data2 == NULL);
        printf("Result of subtracting %d and %d: %d\n", left_operand, right_operand,
               response_data->data1);
        rpc_data_free(response_data);
    }

cleanup:
    if (handle_add2 != NULL) {
        free(handle_add2);
    }

    if (handle_subtract2 != NULL) {
        free(handle_subtract2);
    }

    rpc_close_client(state);
    state = NULL;

    return exit_code;
}

/* Extracts the command line arguments, and stores the associated index
   in the corresponding pointers.
 */
void read_args(int argc, char *argv[], int *ip_idx, int *port) {
    int c;
    int values_read = 0;
    
    while ((c = getopt(argc, argv, "i:p:")) != -1) {
        switch (c) {
            case IP_ADDR:
                *ip_idx = optind-1;
                values_read++;
                break;
            case PORT:
                *port = atoi(optarg);
                values_read++;
                break;
            default:
                exit(0);
        }
    }
    
    if (values_read != NUM_ARGS) {
        perror("Invalid number of arguments\n");
        exit(0);
    }
}