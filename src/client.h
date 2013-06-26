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

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>	

#include <sys/socket.h>
#include <netinet/in.h>

#include <event2/bufferevent_ssl.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>

#include <signal.h>
#include <ctype.h>

// constants defined 

#define complete_address_len    23      // length of char arrays holding address a.d.c.e:portnum
#define file_name_len           100     // length of file names
#define ip_len                  16      // length of ip portion of address
#define port_len                6       // length of port number portion of address
#define listen_address			"127.0.0.1:4000" // default address to listen on for clients 

 // structures 

 typedef struct client{
 	struct bufferevent 		*client_bufferevent;
 	//char 					channel;
 	unsigned int 			data_length;
 	char 					*data;
 	int 					data_position;
 } client;

 typedef struct client_node{
 	client 				*client_data;
 	struct client_node 	*next;
 } client_node;

 typedef struct client_list{
 	client_node		*head;
 } client_list;

// functions

client* new_client(struct bufferevent *input_bev, char *input_data);
client* new_null_client();
client_list* new_client_list(client_node *input_node);
client_list* new_null_client_list();
client_node* new_client_node(client *input_client, client_node *input_node);
client_node* new_null_client_node();
client* free_client(client *old_client);
client_list* free_client_list(client_list *old_list);
client_node* free_client_node(client_node *old_node);
void print_client(client *client_to_print);
void print_client_node(client_node *node_to_print);
void print_client_list(client_list *list_to_print);

#endif 