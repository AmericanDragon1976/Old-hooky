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

/*
 * Proxy provides the proxying services for the portal proxy service. Accepts 
 * with Monitor to know where to direct clients, listens for direction to 
 * change from Monitor and passes traffic back and forth from clients to services.
 */

#ifndef CLIENT_H
#define CLIENT_H

 // structures 

 typedef struct client{
 	struct bufferevernt 	*client_bufferevent;
 	char 					channel;
 	unsigned int 			data_length;
 	char 					*data
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
void free_client(client *old_client);
void free_client_list(client_list *old_list);
void free_client_node(client_node *old_node);

#endif 