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

#ifndef CLIENT_H
#define CLIENT_H

#include <json/json.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h> 

#include <sys/socket.h>
#include <netinet/in.h>

#include <uv.h>

#include <signal.h>
#include <ctype.h>

// constants defined 

#define complete_address_len    23      // length of char arrays holding address a.d.c.e:portnum
#define file_name_len           100     // length of file names
#define ip_len                  16      // length of ip portion of address
#define port_len                6       // length of port number portion of address
#define handle_len				30 		// max length of handles in config file.
#define data_size				1000 	// starting size of strings to hold standrd out and standard error being reuturned, also increse inremen size.
#define listen_address          "127.0.0.1:4000" // default address to listen on for clients 
#define standard_output			"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Standard out, auctor eu nisi vitae, rutrum dictum erat. Donec tempor, nulla nec venenatis porttitor, magna orci tincidunt nulla, et lacinia augue magna vel lacus. Nulla facilisi. Aliquam erat volutpat. Vivamus vestibulum scelerisque nisl. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Ut euismod nisi vitae est vehicula elementum. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos."
#define standard_error			"Curabitur semper justo arcu, quis tincidunt justo tempor in. Standard error nunc eu viverra venenatis. Nunc sed orci eu purus varius varius non eget."

 // structures 

typedef struct process{
    char                    *process_call, *out_output, *err_output;
    uv_process_t            child_req;
    uv_pipe_t               out_pipe, err_pipe;
    unsigned int            out_len, err_len, out_position, err_position;
    int                     exit_code;
} process;

 typedef struct process_node {
    process                 *process_data;
    struct process_node     *next, *previous;
} process_node;

 typedef struct client{
    uv_tcp_t                *client_connection;
    unsigned int            data_length;
    unsigned int            data_position;
    char                    *data;
    process_node            *processes, *processes_tail;
} client;

 typedef struct client_node{
    client              *client_data;
    struct client_node  *next, *previous;
} client_node;

 typedef struct client_list{
    client_node     *head, *tail;
    char            *base_path;
    uv_tcp_t        listener;
    void            *lock;
} client_list;

typedef struct watcher_package{
    uv_process_t            *child_req;
    process_node            *containing_process_node;
    client_node             *owner_node;
 }watcher_pack;

 typedef struct watcher_node{
    struct watcher_node         *next;
    struct watcher_node         *previous;
    watcher_pack                *pack_data;
 }watcher_node;

 typedef struct watcher_list{
    watcher_node        *head, *tail;
    void                *lock;
 }watcher_list;

 extern client_list     *clients;
 extern uv_loop_t       *loop;
 extern watcher_list    *watchers;
 extern int             number;

// functions

process* new_null_process();
process_node* new_process_node(process *input_process, process_node *next, process_node *prev);
process_node* new_null_process_node();

client* new_client(uv_tcp_t *input_connection, process_node *first_process_node, process_node *last_process_node);
client* new_null_client();
client_list* new_client_list(client_node *head_node, client_node *tail_node, char *path);
client_list* new_null_client_list();
client_node* new_client_node(client *input_client, client_node *next_node, client_node *prev_node);
client_node* new_null_client_node();

watcher_pack* new_watcher_pack(uv_process_t *child_req, process_node *containing_process_node, client_node *owner_node);
watcher_pack* new_null_watcher_pack();
watcher_node* new_watcher_node(watcher_node *next, watcher_node *prev, watcher_pack *pack_data);
watcher_node* new_null_watcher_node();
watcher_list* new_watcher_list(watcher_node *head, watcher_node *tail);
watcher_list* new_null_watcher_list();

void free_one_watcher_node(watcher_node *old_node);
watcher_node* free_watcher_nodes(watcher_node *old_node);
watcher_list* free_watcher_list(watcher_list *old_list);
client* free_client(client *old_client);
client_list* free_client_list(client_list *old_list);
client_node* free_client_nodes(client_node *old_node);
void free_one_client_node(client_node *old_node);
process_node* free_process_nodes(process_node *old_node);
void free_one_process_node(client_node *owner_node, process_node *old_node);
process* free_process(process* old_process);

void print_client(client *client_to_print);
void print_client_node(client_node *node_to_print);
void print_client_list(client_list *list_to_print);

bool client_owns_process(client *input_client, process *input_process);
client_node* find_client_from_process(process_node *input_process);
process_node* find_process_from_pipe(uv_stream_t *info_pipe);
client_node* find_client_from_connection(uv_stream_t *client_conn);
void find_client_and_process_from_process_watcher(uv_process_t *watcher, client_node **return_client, process_node **return_process, watcher_node **return_watcher);

#endif 
