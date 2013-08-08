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
{//printf("new null process\n");
    process         *new_process = (process *) malloc(sizeof(process));

    new_process->process_call = NULL;
    new_process->out_output = (char *) malloc(data_size);
    strncpy(new_process->out_output, "", data_size - 1);
    new_process->err_output = (char *) malloc(data_size);
    strncpy(new_process->err_output, "", data_size - 1);
    new_process->out_len = data_size;
    new_process->err_len = data_size;
    new_process->out_position = 0;
    new_process->err_position = 0;
    new_process->exit_code = 0;

    return (new_process);
}

/*
 * Allocated memory for a new process node and sets the pointers to the values passed in. 
 */
process_node* 
new_process_node(process* input_process, process_node* next, process_node* prev)
{//printf("new process node\n");
    process_node        *new_node = (process_node *) malloc(sizeof(process_node));

    new_node->process_data = input_process;
    new_node->next = next;
    new_node->previous = prev;

    return(new_node);
}

/*
 * Calls new process node passing in NULL pointers. 
 */
process_node* 
new_null_process_node()
{//printf("new null process node\n");
    return(new_process_node(NULL, NULL, NULL));
}

/*
 * Allocates memory for a new client sets pointer members to values passes in, and set other values to 0.
 */
client* 
new_client(uv_tcp_t *input_connection, process_node *first_process_node, process_node *last_process_node)
{//printf("new client\n");
    client          *new_client = (client *) malloc(sizeof(client));

    new_client->client_connection = input_connection;
    new_client->data_length = 0;
    new_client->data = NULL;
    new_client->data_position = 0;
    new_client->processes = first_process_node;
    new_client->processes_tail = last_process_node;

    return(new_client);
}

/*
 * Calls new_client() passing in NULL pointers.
 */
client* 
new_null_client()
{//printf("new null client\n");
    uv_tcp_t        *new_connection = (uv_tcp_t *) malloc(sizeof(uv_tcp_t));

    return(new_client(new_connection, NULL, NULL));
}

/*
 * Allocates memory for new_client_list() and sets pointer members to values passes in.
 */
client_list* 
new_client_list(client_node *head_node, client_node *tail_node, char *path)
{//printf("new client list\n");
    client_list         *new_client_list = (client_list *) malloc(sizeof(client_list));

    new_client_list->head = head_node;
    new_client_list->tail = tail_node;
    new_client_list->base_path = path;
    new_client_list->lock = NULL;

    return(new_client_list);
}

/*
 *  Calles new_client_list() passing in NULL pointers.
 */
client_list* 
new_null_client_list()
{//printf("new null client list\n");
    return(new_client_list(NULL, NULL, NULL));
}

/*
 * Allocates memory for new_client_node() and sets pointer members to values passes in.
 */
client_node* 
new_client_node(client *input_client, client_node *next_node, client_node *prev_node)
{printf("new client node\n");
    client_node     *new_client_node = (client_node *) malloc(sizeof(client_node));

    new_client_node->client_data = input_client;
    new_client_node->next = next_node;
    new_client_node->previous = prev_node;

    return(new_client_node);
}

/*
 *   Calles new_client_node() passing in NULL pointers.
 */
client_node* 
new_null_client_node()
{//printf("new null client node\n");
    return(new_client_node(NULL, NULL, NULL));
}

watcher_pack* 
new_watcher_pack(uv_process_t *child_req, process_node *containing_process_node, client_node *owner_node)
{//printf("new watcher pack \n");
    watcher_pack            *new_watcher_pack = (watcher_pack *) malloc(sizeof(watcher_pack));

    new_watcher_pack->child_req = child_req;
    new_watcher_pack->containing_process_node = containing_process_node;
    new_watcher_pack->owner_node = owner_node;

    return(new_watcher_pack);
}

watcher_pack* 
new_null_watcher_pack()
{//printf("new null watcher pack \n");
    return(new_watcher_pack(NULL, NULL, NULL));
}

watcher_node* 
new_watcher_node(watcher_node *next, watcher_node *prev, watcher_pack *pack_data)
{//printf("new watcher node \n");
    watcher_node        *new_watcher_node = (watcher_node *) malloc(sizeof(watcher_node));
    
    new_watcher_node->next = next;
    new_watcher_node->previous = prev;
    new_watcher_node->pack_data = pack_data;

    return(new_watcher_node);
}

watcher_node* 
new_null_watcher_node()
{//printf("new null watcher node \n");
    return (new_watcher_node(NULL, NULL, NULL));
}

watcher_list* 
new_watcher_list(watcher_node *head, watcher_node *tail)
{//printf("new watcher list \n");
    watcher_list            *new_list = (watcher_list *) malloc(sizeof(watcher_list));

    new_list->head = head;
    new_list->tail = tail;
    new_list->lock = NULL;

    return(new_list);
}

watcher_list* 
new_null_watcher_list()
{//printf("new null watcher list \n");
    return (new_watcher_list(NULL, NULL));
}

void 
free_one_watcher_node(watcher_node *old_node)
{//printf("free one watcher node \n");
    if (old_node == NULL)
        return;

    bool       operation_complete = false;

    free(old_node->pack_data);
    old_node->pack_data = NULL;

    while(operation_complete == false){
        if(watchers->lock == NULL)
            watchers->lock = (void *) old_node;

        if(watchers->lock == old_node){
            if (old_node->next != NULL)
                old_node->next->previous = old_node->previous;
            else
                watchers->tail = old_node->previous;

            if (old_node->previous != NULL)
                old_node->previous->next = old_node->next;
            else
                watchers->head = old_node->next;
            operation_complete = true;
            watchers->lock = NULL;
        }
    }

    old_node->next = NULL;
    old_node->previous = NULL;
    free(old_node);
    old_node = NULL;
}

watcher_node* 
free_watcher_nodes(watcher_node *old_node)
{
    if (old_node == NULL)
        return(old_node);

    old_node->previous = NULL;
    free(old_node->pack_data);
    old_node->pack_data = NULL;
    free_watcher_nodes(old_node->next);
    old_node->next = NULL;

    return(old_node);
}

watcher_list* 
free_watcher_list(watcher_list *old_list)
{
    if (old_list == NULL)
        return(old_list);

    old_list->tail == NULL;
    old_list->head = free_watcher_nodes(old_list->head);
    free(old_list);
    old_list = NULL;

    return(old_list);
}

/*
 * Frees all memory associated with a client pointer checking for NULL pointers. 
 * If client_connection needs closing it must be closed before calling this funtion.
 */
client* 
free_client(client *old_client)
{//printf("free client\n");
    if (old_client == NULL)
        return(old_client);

    free(old_client->data); 
    old_client->data = NULL;

    old_client->processes = free_process_nodes(old_client->processes); 
    old_client->processes_tail = NULL;

    free(old_client->client_connection); 
    old_client->client_connection = NULL;   

    free(old_client); 
    old_client = NULL;

    return(old_client);
}

/*
 * Frees all memory associated with a client list checking for NULL pointers. 
 */
client_list* 
free_client_list(client_list *old_list)
{//printf("free client list\n");
    if (old_list == NULL)
        return(old_list);

    old_list->head = free_client_nodes(old_list->head);

    free(old_list->base_path);
    old_list->base_path = NULL;

    free(old_list);
    old_list = NULL;
    
    return(old_list);
}

/*
 * Frees all memory associated with a client node checking for NULL pointers. 
 */
client_node* 
free_client_nodes(client_node *old_node)
{//printf("free client nodes\n");
    if (old_node == NULL)
        return (old_node);

    old_node->client_data = free_client(old_node->client_data);

    old_node->next = free_client_nodes(old_node->next);
    old_node->previous = NULL;

    watcher_node    *temp_node = watchers->head, *node_ptr = NULL;
    while(temp_node != NULL){
        if (temp_node->pack_data != NULL)
            if(temp_node->pack_data->owner_node == old_node){
                node_ptr = temp_node;
                temp_node = temp_node->next;
                free_one_watcher_node(node_ptr);
                node_ptr = NULL;
            }
        else {
            temp_node = temp_node->next;
        }
    }

    free(old_node);
    old_node = NULL;
    return(old_node);
}

void
free_one_client_node(client_node *old_node)
{//printf("free one client node \n");
    if (old_node == NULL)
        return;

    bool        operation_complete = false;

    while(operation_complete == false){
        if(clients->lock == NULL)
            clients->lock = old_node;

        if (clients->lock == old_node){
            if (old_node->next != NULL)
                old_node->next->previous = old_node->previous;
            else 
                clients->tail = old_node->previous;

            if (old_node->previous != NULL)
                old_node->previous->next = old_node->next;
            else 
                clients->head = old_node->next;

            operation_complete = true;
            clients->lock = NULL;
        }
    }

    old_node->client_data = free_client(old_node->client_data);
    free(old_node);
    old_node = NULL;
//printf("leaving free one node \n");
}

/*
 * Frees the memmory associated with a process node passed in and recurses on all node down list from it. 
 */
process_node* 
free_process_nodes(process_node *old_node)
{//printf("free process nodes \n");
    if (old_node == NULL)
        return(old_node);

    watcher_node        *temp_node = watchers->tail;

    while(temp_node !=NULL){
        if ((temp_node->pack_data != NULL) && (temp_node->pack_data->containing_process_node == old_node)){
            free_one_watcher_node(temp_node);
            temp_node = NULL;
        }
        else{ 
            temp_node = temp_node->previous;
        }
    }

    old_node->process_data = free_process(old_node->process_data);
    old_node->next = free_process_nodes(old_node->next);
    old_node->previous = NULL;

    free(old_node);
    old_node = NULL;
    return(old_node);
}

/*
 * Frees the node passed in and returns its "next" link;
 */
void 
free_one_process_node(client_node *owner_node, process_node *old_node)
{//printf("free one process node\n");
    if (old_node == NULL)
    return;

    if (owner_node == NULL){
        printf("ERROR: null client sent to free one process node\n");
        free_process_nodes(old_node);
    }

    watcher_node        *temp_node = watchers->tail;
    bool                operation_complete = false;

    while(operation_complete == false){ 
        if (watchers->lock == NULL)
            watchers->lock = old_node;

        if (watchers->lock == old_node){
            if (old_node->next == NULL)
                owner_node->client_data->processes_tail = old_node->previous;
            else 
                old_node->next->previous = old_node->previous;

            if (old_node->previous == NULL)
                owner_node->client_data->processes = old_node->next;
            else 
                old_node->previous->next = old_node->next;

            operation_complete = true;
            watchers->lock = NULL;
        }
    }
    old_node->next = NULL;
    old_node->previous = NULL;
    old_node->process_data = free_process(old_node->process_data);

    while(temp_node !=NULL){
        if (temp_node->pack_data != NULL && temp_node->pack_data->containing_process_node == old_node){
            free_one_watcher_node(temp_node);
            temp_node = NULL;
        }
        else{ 
            temp_node = temp_node->previous;
        }
    }
    free(old_node);
    old_node = NULL;
}

/*
 * Frees all memory associated with a process. 
 */
process* 
free_process(process* old_process)
{//printf("free process\n");
    if (old_process == NULL)
        return (old_process); 

    free(old_process->process_call);
    old_process->process_call = NULL;

    free(old_process->out_output);
    old_process->out_output = NULL;

    free(old_process->err_output);
    old_process->err_output = NULL;

    free(old_process);
    old_process = NULL;

    return(old_process);
}

/*
 * Prints all client data to standard out. Used in debugging, not use in production versions. 
 */
void 
print_client(client *client_to_print)
{printf("print client %p\n", client_to_print);
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
{//printf("print client node\n");
    printf("Node: %p\n", node_to_print);

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
{printf("print client list\n");
    if (list_to_print == NULL){
        printf("NULL list \n");
    }
    else {
        printf("client List: %p\n head %p\n", list_to_print, list_to_print->head);
        print_client_node(list_to_print->head);
    }
}

/*
 * Checks rether a particular process is owned by a particular client, ie it is in one of the nodes in it list of processes. 
 * Returns true if it is owned and false if not. 
 */
bool
client_owns_process(client *input_client, process *input_process)
{//printf("client owns process\n");
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
client_node*
find_client_from_process(process_node *input_process)
{//printf("find client from process\n");
    if (clients == NULL)
        return(NULL);

    client_node         *return_node = NULL;
    watcher_node        *watcher_node_ptr = watchers->head;

    while (watcher_node_ptr != NULL){
        if (watcher_node_ptr->pack_data != NULL)
            if (watcher_node_ptr->pack_data->containing_process_node == input_process)
                return_node = watcher_node_ptr->pack_data->owner_node;
    }

    return (return_node);
}

/*
 * Takes a pipe connecting a child process to either standard out or standard error and searches the list of clients for the 
 * process that is connected by that pipe and returns it. 
 */
process_node* 
find_process_from_pipe(uv_stream_t *info_pipe)
{//printf("find process from pipe\n");
    if (clients == NULL || watchers == NULL)
        return(NULL);

    watcher_node        *curr_node = watchers->tail;
    process_node        *curr_proc_node == NULL;
    process             *curr_proc = NULL;

    while(curr_node != NULL){
        curr_proc_node = curr_node->containing_process_node;
        if (curr_proc_node != NULL)
            curr_proc = curr_proc_node->process_data;
        if (curr_proc != NULL)
            if(curr_proc->out_pipe == info_pipe || curr_proc->err_pipe == info_pipe)
                return (curr_proc_node);

        curr_proc_node == NULL;
        curr_proc == NULL;
        curr_node = curr_node->previous;
    }
    return (NULL);
}

/*
 * Takes a connection and finds the client in the list connected on that connection and returns it. 
 */
client_node* 
find_client_from_connection(uv_stream_t *client_conn)
{//printf("find client from connection\n");
    if (clients == NULL)
        return(NULL);

    client_node         *curr_node = clients->tail;
    client_node         *return_client_node = NULL;
    client              *curr_client = NULL;

    while (curr_node != NULL){
        curr_client = curr_node->client_data;
        if (curr_client != NULL)
            if (curr_client->client_connection == (uv_tcp_t *) client_conn)
                return (curr_node);
        curr_node = curr_node->previous;
        curr_client = NULL;
    }
    return (NULL);
}


/*
 * Takes the uv proces, the watcher with the processes id, and searches the list of clients for the one who owns the process being
 * watched. temp client will be set to that client and temp process will be set to that process. If it is not found in the list both 
 * will be NYULL. 
 */
void 
find_client_and_process_from_process_watcher(uv_process_t *watcher, client_node **return_client, process_node **return_process, watcher_node **return_watcher)
{//printf("find client and process from watcher\n");
    if (clients == NULL || watchers == NULL){
        fprintf(stderr, "ERROR: client and/or watchers list corrupted. \n");
        exit(1);
        return;
    }

    watcher_node    *temp_watcher_node = watchers->tail;
    watcher_pack    *temp_pack = NULL;
    process_node    *temp_proc_node = NULL;
    process         *temp_proc = NULL;

    while(temp_watcher_node != NULL){ 
        temp_pack = temp_watcher_node->pack_data;
        
        if (temp_pack != NULL)
            temp_proc_node = temp_pack->containing_process_node;

        if (temp_proc_node != NULL)
            temp_proc = temp_proc_node->process_data;

        if(temp_proc != NULL)
            if (watcher == temp_proc->child_req){
                *return_client = temp_pack->owner_node;
                *return_process = temp_proc_node;
                *return_watcher = temp_watcher_node;
                return;
            }
        temp_pack = NULL;
        temp_proc_node = NULL;
        temp_proc = NULL;
        temp_watcher_node = temp_watcher_node->previous;
    }//printf("leaving find client etc from watcher %p\n", *return_watcher);
}
