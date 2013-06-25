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

#include "hooky.h"
#include "client.h"
#include "callback.h"

/*
 * Allocates memory for a new client sets pointer members to values passes in, and set other values to 0.
 */
client* 
new_client(struct bufferevent *input_bev, char *input_data)
{
	client 			*new_client = (client *) malloc(sizeof(client));

	new_client->client_bufferevent = input_bev;
	new_client->channel = 0;
	new_client->length = 0;
	new_client->data = input_data;

	return(new_client);
}

/*
 * Calles new_client() passing in NULL pointers.
 */
client* 
new_null_client()
{
	return(new_client(NULL, NULL));
}

/*
 * Allocates memory for new_client_list() and sets pointer members to values passes in.
 */
client_list* 
new_client_list(client_node *input_node)
{
	client_list 		*new_client_list = (client_list *) malloc(sizeof(client_list));

	new_client_list->head = input_node;

	return(new_client_list);
}

/*
 *  Calles new_client_list() passing in NULL pointers.
 */
client_list* 
new_null_client_list()
{
	return(new_client_list(NULL));
}

/*
 * Allocates memory for new_client_node() and sets pointer members to values passes in.
 */
client_node* 
new_client_node(client *input_client, client_node *input_node)
{
	client_node 	*new_client_node = (client_node *) malloc(sizeof(client_node));

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
void 
free_client(client *old_client)
{
	if (old_client == NULL)
		return;

	if (old_client->client_bufferevent != NULL)
		bufferevent_free(old_client->client_bufferevent);

	if (old_client->data != NULL)
		free(old_client->data);

	free(old_client);
}

/*
 * Frees all memory associated with a client list checking for NULL pointers. 
 */
void 
free_client_list(client_list *old_list)
{
	if (old_list == NULL)
		return;

	if (old_list->head != NULL)
		free_client_node(old_list->head);

	free(old_list);
}

/*
 * Frees all memory associated with a client node checking for NULL pointers. 
 */
void 
free_client_node(client_node *old_node)
{
	if (old_node == NULL)
		return;

	if (old_node->client_data != NULL)
		free_client(old_node->client_data);

	if (old_node->next != NULL)
		free_client_node(old_node->next);

	free(old_node);
}
