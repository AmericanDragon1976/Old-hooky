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
    new_client->out_len = data_size;
    new_client->out_output = (char *) malloc(new_client->out_len);
    new_client->out_position = 0;
    new_client->out_output[0] = '\0';
    new_client->err_len = data_size;
    new_client->err_output = (char *) malloc(new_client->err_len);
    new_client->err_position = 0;
    new_client->err_output[0] = '\0';
    new_client->process_running = false;

    return(new_client);
}

/*
 * Calles new_client() passing in NULL pointers.
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
    free(old_client->out_output);
    free(old_client->err_output);
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
 * 
 */
void 
print_client(client *client_to_print)
{
    if (client_to_print == NULL){
        printf("NULL client \n");
    }
    else {
        printf("Client \n");
        printf("client_connection: %p\n",client_to_print->client_connection);
        printf("data_len: %d\n", client_to_print->data_length);
        printf("data: ");
        int i;
        for (i = 0; i < client_to_print->data_length; i++)
            printf("%c", client_to_print->data[i]);
        printf("\n\n");
    }
}

/*
 * 
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
 * 
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
 *
 */
client* 
find_client_from_pipe(uv_stream_t *info_pipe)
{
    client_node         *curr_node = clients->head;
    client              *curr_client = NULL;

    while (curr_node != NULL){
        curr_client = curr_node->client_data;
        if (&curr_client->out_pipe == (uv_pipe_t *) info_pipe || &curr_client->err_pipe == (uv_pipe_t *) info_pipe)
            curr_node = NULL;
        else
            curr_node = curr_node->next;
    }

    return(curr_client);
}

/*
 *
 */
client* 
find_client_from_connection(uv_stream_t *client_conn)
{
    client_node         *curr_node = clients->head;
    client              *curr_client = NULL;

    while (curr_node != NULL){
        curr_client = curr_node->client_data;
        if (curr_client->client_connection == (uv_tcp_t *) client_conn)
            curr_node = NULL;
        else
            curr_node = curr_node->next;
    }
    return (curr_client);
}