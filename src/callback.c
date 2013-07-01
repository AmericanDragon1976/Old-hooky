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
 * 
 */
void on_connect(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address, int socklen, void *c_list)
{
    struct event_base   *event_loop = evconnlistener_get_base(listener);
    client_list         *clients = (client_list *) c_list;
    client              *connecting_client = new_null_client();

    connecting_client->client_bufferevent = bufferevent_socket_new(event_loop, fd, BEV_OPT_CLOSE_ON_FREE|EV_PERSIST);
    bufferevent_setcb(connecting_client->client_bufferevent, on_read, NULL, event_cb, clients);
    bufferevent_enable(connecting_client->client_bufferevent, EV_READ|EV_WRITE);

    clients->head = new_client_node(connecting_client, clients->head); 
}

/*
 * determines if new or continuing client request, parses data, updates client
 */
void 
process_data_from_client(client *current_client, char *data_recived, int len)
{
    bool            new_request;
    int             i = 0, j = 0, size = 0;
    int             *int_ptr;
    char            str_size[4], temp_char_num[4];

    int_ptr = (int *) &temp_char_num;
    *int_ptr = 0;

    if (current_client->data_length == 0)
        new_request = true;
    else 
        new_request = false;

    if (new_request){ 
        j = 0;
        for (i = 3; i >= 0; i--)
            str_size[i] = data_recived[j++];

        i = 0;
        for (i = 0; i < 4; i++){ 
            temp_char_num[0] = str_size[i];
            size = size * 256 + (*int_ptr); 
            *int_ptr = 0;
        }

        current_client->data_length = size; 
        current_client->data = (char *) malloc(size);
        i = 4;
    }

    size = current_client->data_length;
    j = current_client->data_position;

    for ( ; j < size && i < len; )
        current_client->data[j++] = data_recived[i++];

    current_client->data_position = j;
    free(data_recived); 
}

/*
 * 
 */
int 
execute_request(client *current_client, char* path)  
{
    int         exit_code, len;
    char        *command = NULL;

    len = current_client->data_length + strlen(path) + 1;
    command = (char *) malloc(len);
    strcpy(command, path);
    strcat(command, current_client->data);
    command[len -1] = '\0';

    exit_code = system(command);
    free(command);

    return(exit_code);
}

/*
 * 
 */
void 
reset_client(client *current_client)
{
    //current_client->channel = 0;
    current_client->data_length = 0;
    current_client->data_position = 0;
    free(current_client->data);
    current_client->data = NULL;
}

/*
 * 
 */
void 
on_read(struct bufferevent *buffer_event, void *c_list)
{
    client_list     *clients = (client_list *) c_list;
    client_node     *current_node = clients->head;
    client          *current_client = NULL;
    struct evbuffer *input = bufferevent_get_input(buffer_event);
    int             len, request_exit_code, reply_size;
    char            *data_recived = NULL;

    while (current_node != NULL){
        if (current_node->client_data != NULL){
            current_client = current_node->client_data;

            if (current_client == NULL || current_client->client_bufferevent != buffer_event){
                current_node = current_node->next;
            }
            else {
                ///break;
                current_node = NULL;
            }
        }
    }

    len = evbuffer_get_length(input); 
    data_recived = (char *) malloc(len);
    bzero(data_recived, len); 
    evbuffer_remove(input, data_recived, len);

    process_data_from_client(current_client, data_recived, len);
    data_recived = NULL; 

    if(current_client->data_length == current_client->data_position && current_client->data_length != 0){
        request_exit_code = execute_request(current_client, clients->base_path);
        reply_size = sizeof(request_exit_code);
        bufferevent_write(current_client->client_bufferevent, &reply_size, 4);
        bufferevent_write(current_client->client_bufferevent, &request_exit_code, reply_size);
        bufferevent_flush(current_client->client_bufferevent, EV_WRITE, BEV_NORMAL);
        reset_client(current_client);
    }
}


/* 
 * catches interrupt signal and allows the program to cleanup before exiting. 
 */
void 
signal_cb (evutil_socket_t sig, short events, void *user_data) 
{
    struct event_base   *base = (struct event_base *) user_data;
    struct timeval      delay = { 2, 0 };

    printf("Caught an interrupt signal; exiting cleanly in two seconds.\n");


    event_base_loopexit(base, &delay);
}

void 
client_dc(struct bufferevent *buffer_event, void *ctx)
{ 
    if (ctx == NULL){
        fprintf(stderr,"Fatal error: Null list passed to client_dc()");
        exit(0);
    }

    client          *temp_client;
    client_list     *list = (client_list *) ctx;

    if (list->head == NULL){
        fprintf(stderr,"Fatal error: Empty list passed to client_dc()");
        exit(0);
    }

    client_node     *current_node = list->head, *temp_node = list->head->next;

    if (current_node->client_data->client_bufferevent == buffer_event){
        list->head = current_node->next;
        free_client_node(current_node);
    }
    else {
        while (temp_node != NULL){
            if (temp_node->client_data->client_bufferevent == buffer_event){
                current_node->next = temp_node->next;
                free_client_node(temp_node);
            }
            else {
                current_node = temp_node;
                temp_node = temp_node->next;
            }
        }
    }
}

/* 
 * triggered by all event buffer event, reports errors and successful connects. 
 */
void 
event_cb(struct bufferevent *buffer_event, short what, void *ctx)
{ 
    if (what & BEV_EVENT_ERROR) {
        unsigned long err;

        while ((err = (bufferevent_get_openssl_error(buffer_event)))) { 
            const char *msg = (const char*)
                ERR_reason_error_string(err);
            const char *lib = (const char*)
                ERR_lib_error_string(err);
            const char *func = (const char*)
                ERR_func_error_string(err);
            fprintf(stderr,
                "%s in %s %s\n", msg, lib, func);
        }

        if (errno)
            perror("connection error");
    }

    if (what & BEV_EVENT_EOF)
        client_dc(buffer_event, ctx);

    if (what & BEV_EVENT_CONNECTED)
        printf("CONNECTION SUCCESSFUL\n");

    if (what & BEV_EVENT_TIMEOUT)
        printf("TIMEOUT\n");
}

/*
 * 
 */
bool 
file_exist(char file_path[])
{
    if (access(file_path, F_OK) != -1)
        return(true);
    else
        return(false);
}
