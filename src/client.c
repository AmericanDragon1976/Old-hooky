/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2013 <hal at pagodabox dot com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "client.h"
#include "hooky.h"
#include "callback.h"

/*
 * Allocates memory for a new process, the process call is null and will simple be transfered from another source so there is no
 * version of this function that creates a new buffer for it. 
 */
process* 
new_null_process()
{
    process         *new_process = (process *) malloc(sizeof(process));

    new_process->process_call = NULL;
    new_process->out_output = (char *) malloc(data_size);
    new_process->out_output[0] = '\0';
    new_process->err_output = (char *) malloc(data_size);
    new_process->err_output[0] = '\0';
    new_process->out_len = data_size;
    new_process->err_len = data_size;
    new_process->out_position = 0;
    new_process->err_position = 0;
    new_process->exit_code = 0;
}

/*
 * Allocated memory for a new process node and sets the pointers to the values passed in. 
 */
process_node* 
new_process_node(process* input_process, process_node* input_node)
{
    process_node        *new_node = (process_node *) malloc(sizeof(process_node));

    new_node->process_data = input_process;
    new_node->next = input_node;

    return(new_node);
}

/*
 * Calls new process node passing in NULL pointers. 
 */
process_node* 
new_null_process_node()
{
    return(new_process_node(NULL, NULL));
}

/*
 * Allocates memory for a new client sets pointer members to values passes in, and set other values to 0.
 */
client* 
new_client(uv_tcp_t *input_connection, char *input_data)
{
    client          *new_client = (client *) malloc(sizeof(client));

    new_client->client_connection = input_connection;
    new_client->data_length = 0;
    new_client->data = input_data;
    new_client->data_position = 0;

    return(new_client);
}

/*
 * Calls new_client() passing in NULL pointers.
 */
client* 
new_null_client()
{
    uv_tcp_t        *new_connection = (uv_tcp_t *) malloc(sizeof(uv_tcp_t));

    return(new_client(new_connection, NULL));
}

/*
 * Allocates memory for new_client_list() and sets pointer members to values passes in.
 */
client_list* 
new_client_list(client_node *input_node, char *path)
{
    client_list         *new_client_list = (client_list *) malloc(sizeof(client_list));

    new_client_list->head = input_node;
    new_client_list->base_path = path;

    return(new_client_list);
}

/*
 *  Calles new_client_list() passing in NULL pointers.
 */
client_list* 
new_null_client_list()
{
    return(new_client_list(NULL, NULL));
}

/*
 * Allocates memory for new_client_node() and sets pointer members to values passes in.
 */
client_node* 
new_client_node(client *input_client, client_node *input_node)
{
    client_node     *new_client_node = (client_node *) malloc(sizeof(client_node));

    new_client_node->client_data = input_client;
    new_client_node->next = input_node;

    return(new_client_node);
}

/*
 *   Calles new_client_node() passing in NULL pointers.
 */
client_node* 
new_null_client_node()
{
    return(new_client_node(NULL, NULL));
}

/*
 * Frees all memory associated with a client pointer checking for NULL pointers. 
 */
client* 
free_client(client *old_client)
{
    if (old_client == NULL)
        return(old_client);

    uv_close((uv_handle_t*) old_client->client_connection, NULL);
    free(old_client->client_connection);
    free(old_client->data);
    free_process_nodes(old_client->processes);
    free(old_client);
    old_client = NULL;
    return(old_client);
}

/*
 * Frees all memory associated with a client list checking for NULL pointers. 
 */
client_list* 
free_client_list(client_list *old_list)
{
    if (old_list == NULL)
        return(old_list);

    if (old_list->head != NULL)
        old_list->head = free_client_node(old_list->head);

    free(old_list);
    old_list = NULL;
    return(old_list);
}

/*
 * Frees all memory associated with a client node checking for NULL pointers. 
 */
client_node* 
free_client_node(client_node *old_node)
{
    if (old_node == NULL)
        return (old_node);

    if (old_node->client_data != NULL)
        old_node->client_data = free_client(old_node->client_data);

    if (old_node->next != NULL)
        old_node->next = free_client_node(old_node->next);

    free(old_node);
    old_node = NULL;
    return(old_node);
}

/*
 * Frees the memmory associated with a process node passed in and recurses on all node down list from it. 
 */
process_node* 
free_process_nodes(process_node *old_node)
{
    if (old_node == NULL)
        return(old_node);

    free_process(old_node->process_data);
    free_process_nodes(old_node->next);

    free(old_node);
    old_node = NULL;
    return(old_node);
}

/*
 * Frees the node passed in and returns its "next" link;
 */
process_node* 
free_one_process_node(process_node *old_node)
{
    if (old_node == NULL)
        return(old_node);

    process_node        *temp_node = old_node->next;

    free_process(old_node->process_data);
    free(old_node);
    old_node = NULL;

    return(temp_node);

}

/*
 * Frees all memory associated with a process. 
 */
process* 
free_process(process* old_process)
{
    if (old_process == NULL)
        return (old_process); 

    free(old_process->process_call);
    free(old_process->out_output);
    free(old_process->err_output);
    free(old_process);
    old_process = NULL;
    return(NULL);
}

/*
 * Prints all client data to standard out. Used in debugging, not use in production versions. 
 */
void 
print_client(client *client_to_print)
{
    if (client_to_print == NULL){
        printf("NULL client \n");
    }
    else {
        printf("Client \n");
        printf("client_connection: %p\n", client_to_print->client_connection);
        printf("processes: %p\n", client_to_print->processes);
        printf("data_len: %d\n", client_to_print->data_length);
        printf("data: ");
        int i;
        for (i = 0; i < client_to_print->data_length; i++)
            printf("%c", client_to_print->data[i]);
        printf("\n\n");
    }
}

/*
 * Helper function to print client, prints a node. Used in debugging not used in production verdions. 
 */
void 
print_client_node(client_node *node_to_print)
{
    printf("Node:  \n");

    if (node_to_print == NULL){
        printf("NULL node \n");
    }
    else {
        print_client(node_to_print->client_data);
        print_client_node(node_to_print->next);
    }
}

/*
 * Prints all clients in the list to standard out. Used in debugging, not used in production versions. 
 */
void 
print_client_list(client_list *list_to_print)
{
    if (list_to_print == NULL){
        printf("NULL list \n");
    }
    else {
        printf("client List: \n head\n");
        print_client_node(list_to_print->head);
    }
}

/*
 * Checks rether a particular process is owned by a particular client, ie it is in one of the nodes in it list of processes. 
 * Returns true if it is owned and false if not. 
 */
bool
client_owns_process(client *input_client, process *input_process)
{
    process_node            *curr_node = input_client->processes;

    while (curr_node != NULL){
        if (curr_node->process_data == input_process)
            return (true);
        else 
            curr_node = curr_node->next;
    }

    return (false);
}

/*
 * Takes a process and searchs the client list for the client that owns the process, returnes that client. 
 */
 client*
find_client_from_process(process *input_process)
{
    client_node         *curr_node = clients->head;
    client              *curr_client = NULL;

    while (curr_node != NULL){
        curr_client = curr_node->client_data;
        if (client_owns_process(curr_client, input_process)){
            curr_node = NULL;
        }
        else {
            curr_node = curr_node->next;
            curr_client = NULL;
        }
    }

    return (curr_client);
}

/*
 * Takes a pipe connecting a child process to either standard out or standard error and searches the list of clients for the 
 * process that is connected by that pipe and returns it. 
 */
process* 
find_process_from_pipe(uv_stream_t *info_pipe)
{
    client_node         *curr_node = clients->head;
    client              *curr_client = NULL;
    process_node        *curr_process_node = NULL;
    process             *curr_process = NULL;

    while (curr_node != NULL){
        curr_client = curr_node->client_data;
        if (curr_client != NULL){
            curr_process_node = curr_client->processes;

            while (curr_process_node != NULL){
                curr_process = curr_process_node->process_data;
                if(curr_process != NULL && (&curr_process->out_pipe == (uv_pipe_t *) info_pipe || &curr_process->err_pipe == (uv_pipe_t *) info_pipe)){
                    curr_node = NULL;
                    curr_process_node = NULL;
                }
                else {
                    curr_process_node = curr_process_node->next;
                    curr_process = NULL;
                }
            } 

            if (curr_node != NULL)
                curr_node = curr_node->next;
        } 
        else {
            curr_node = curr_node->next;
        }
    }

    return(curr_process);
}

/*
 * Takes a connection and finds the client in the list connected on that connection and returns it. 
 */
client* 
find_client_from_connection(uv_stream_t *client_conn)
{
    client_node         *curr_node = clients->head;
    client              *curr_client = NULL;

    while (curr_node != NULL){
        curr_client = curr_node->client_data;
        if (curr_client->client_connection == (uv_tcp_t *) client_conn){
            curr_node = NULL;
        }
        else {
            curr_node = curr_node->next;
            curr_client = NULL;
        }
    }
    return (curr_client);
}

/*
 * Takes the uv proces, the watcher with the processes id, and searches the list of clients for the one who owns the process being
 * watched. temp client will be set to that client and temp process will be set to that process. If it is not found in the list both 
 * will be NYULL. 
 */
 void 
find_client_and_process_from_process_watcher(uv_process_t *watcher, client *temp_client, process_node *temp_process_node)
{
    client_node         *curr_client_node = clients->head;
    process             *temp_process = NULL;

    temp_client = NULL;
    temp_process_node = NULL;

    while (curr_client_node != NULL){
        temp_client = curr_client_node->client_data;

        if (temp_client != NULL)
            temp_process_node = temp_client->processes;

        while (temp_process_node != NULL){
            temp_process = temp_process_node->process_data;

            if (temp_process != NULL && watcher == &temp_process->child_req)
                return;
            else 
                temp_process_node = temp_process_node->next;
        }
        temp_client = NULL;
        curr_client_node = curr_client_node->next;
    }
}
