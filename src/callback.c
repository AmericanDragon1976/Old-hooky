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

 uv_buf_t 
 alloc_buffer(uv_handle_t *handle, size_t suggested_size) {
    return uv_buf_init((char*) malloc(suggested_size), suggested_size);             //FREE for this Malloc
}

/*
 * 
 */
void 
on_connect(uv_stream_t *listener, int status)
{    
    if (status == -1) {
        fprintf(stderr, "Error on client connect. \n");
        return;
    }

    client              *connecting_client = new_null_client();
    uv_tcp_init(loop, connecting_client->client_connection);

    if (uv_accept(listener, (uv_stream_t*) connecting_client->client_connection) == 0) { //printf("client accepted \n");
        uv_read_start((uv_stream_t*) connecting_client->client_connection, alloc_buffer, on_read); 
        client_node         *temp_node = new_client_node(connecting_client, clients->head);
        clients->head = temp_node;
    }
    else {
        free_client(connecting_client);
    }
}

/*
 * determines if new or continuing client request, parses data, updates client
 */
void 
process_data_from_client(client *current_client, ssize_t nread, uv_buf_t buf)
{//printf("processing: %d\n", (int) buf.base); 
    bool            new_request; 
    int             i = 0, j = 0, size = 0; 
    int             *int_ptr; 
    char            str_size[4], temp_char_num[4], *data_recived = buf.base; 
//printf("nread %d\n", nread);
    int_ptr = (int *) &temp_char_num;
    *int_ptr = 0;
//printf("DL: %d\n", current_client->data_length);
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
        } //printf("Size: %d\n", size);

        current_client->data_length = size; //printf("after assign DL: %d\n", current_client->data_length);
        current_client->data = (char *) malloc(size);
        i = 4;
    }

    size = current_client->data_length;
    j = current_client->data_position;

    for ( ; j < size && i < nread; )
        current_client->data[j++] = data_recived[i++];

    current_client->data_position = j;
    free(data_recived); 
}

/*
 * 
 */
int 
execute_request(client *current_client, char* path)  
{ printf("execute hook \n");
    int         exit_code, len, i;
    char        *command = NULL;
    json_object *jobj = json_tokener_parse(current_client->data);
    json_object *hook = json_object_object_get(jobj, "hook");
    json_object *payload = json_object_object_get(jobj, "payload");
// TODO: add validation to prevent malformed requests from crashing the program.
    len = strlen(path) + json_object_get_string_len(hook) + json_object_get_string_len(payload) + 2;
    command = (char *) malloc(len); printf("path: %s\n", path);
    strcpy(command, path); printf("hook: %s\n", json_object_get_string(hook));
//    strcat(command, json_object_get_string(hook)); printf("payload: %s\n", json_object_get_string(payload));
    strcat(command, " ");
//    for (i = 0; i < len; i++)
//        if (command[i] == '.')
//            command[i] = '/';

//    strcat(command, json_object_get_string(payload));
//    command[len -1] = '\0';
//printf("command: %s\n", command);
//    if (file_exist(command))
//        exit_code = system(command);
//    else 
        exit_code = 0;

    free(command);

    return(exit_code);
}

/*
 * 
 */
char* 
package_reply(int request_exit_code, int *len, char *reply)
{
    char                        *std_out[] = {standard_output};
    struct json_object          *reply_json_object = json_object_new_object();
    struct json_object          *temp_int_json_object = json_object_new_int64(request_exit_code);
    struct json_object          *temp_string_json_object = json_object_new_string(standard_output);

    json_object_object_add  (reply_json_object, "exit_code", temp_int_json_object);
    json_object_object_add  (reply_json_object, "stdout", temp_string_json_object);
    temp_string_json_object = json_object_new_string(standard_error);                   // FREE?????
    json_object_object_add  (reply_json_object, "stderr", temp_string_json_object);

    reply = json_object_to_json_string(reply_json_object);
    *len = strlen(reply);
// printf("reply: %s len: %d\n", reply, *len);
    return(reply);
}

/*
 * 
 */
void 
reset_client(client *current_client)
{//printf("reset client\n");
    current_client->data_length = 0;
    current_client->data_position = 0;
    free(current_client->data);
    current_client->data = NULL;
}

void
on_write (uv_write_t *req, int status)
{
    // TODO: verify memory man agement with libuv and json-c lib

    free(req);
}

/*
 * 
 */
void 
on_read(uv_stream_t *client_conn, ssize_t nread, uv_buf_t buf)
{//printf("on read: \n");
    if (nread == -1) {
        if (uv_last_error(loop).code != UV_EOF)
            fprintf(stderr, "Read error %s\n", uv_err_name(uv_last_error(loop)));
        uv_close((uv_handle_t*) client_conn, NULL);
        return;
    }

    client_node     *current_node = clients->head;
    client          *current_client = NULL;
    int             request_exit_code, reply_size;
    char            *data_recived = NULL, *reply_txt = NULL;
    uv_write_t      *data_len_write = NULL;
    uv_write_t      *data_write = NULL;
    uv_buf_t        data_len_buff;
    uv_buf_t        data_write_buff;

    while (current_node != NULL){
        if (current_node->client_data != NULL){
            current_client = current_node->client_data;

            if (current_client == NULL || current_client->client_connection != client_conn){
                current_node = current_node->next; //printf("client found \n");
            }
            else {
                //printf("client found \n");
                current_node = NULL;
            }
        }
    }
printf("Data recived: %s\n", buf.base);
//    if (nread > 0)
        process_data_from_client(current_client, nread, buf);
    data_recived = NULL; 
printf("processed data: %s data pos: %d data len: %d\n", current_client->data, current_client->data_position, current_client->data_length);
    if(current_client->data_length == current_client->data_position && current_client->data_length != 0){
        request_exit_code = execute_request(current_client, clients->base_path);
        reply_txt = package_reply(request_exit_code, &reply_size, reply_txt); //printf("reply size: %d reply txt: %s\n", reply_size, reply_txt);
        data_len_write = (uv_write_t *) malloc(sizeof(uv_write_t));
        data_write = (uv_write_t *) malloc(sizeof(uv_write_t));
        data_len_write->data = (void*) &reply_size;
        data_write->data = (void*) reply_txt;
        data_len_buff.len = 4;
        data_len_buff.base = (char *) &reply_size;
        data_write_buff.len = reply_size;
        data_write_buff.base = reply_txt;
        uv_write(data_len_write, client_conn, &data_len_buff, 1, on_write);
        uv_write(data_write, client_conn, &data_write_buff, 1, on_write);
        reset_client(current_client); //printf("client reset \n");
    }
}


/* 
 * catches interrupt signal and allows the program to cleanup before exiting. 
 */
void 
signal_cb (uv_signal_t *sig_event, int signum) 
{
//    struct event_base   *base = (struct event_base *) user_data;
    struct timeval      delay = { 2, 0 };

    printf("Caught an interrupt signal; exiting cleanly.\n");

    raise(SIGTERM);
//    abort();
//    event_base_loopexit(base, &delay);
}

void 
client_dc(/*struct bufferevent *buffer_event,*/ void *ctx)
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

//    if (current_node->client_data->client_bufferevent == buffer_event){
//        list->head = current_node->next;
//        free_client_node(current_node);
//    }
//    else {
//        while (temp_node != NULL){
//            if (temp_node->client_data->client_bufferevent == buffer_event){
//                current_node->next = temp_node->next;
//                free_client_node(temp_node);
//            }
//            else {
//               current_node = temp_node;
//                temp_node = temp_node->next;
//            }
//        }
//    }
}

/* 
 * triggered by all event buffer event, reports errors and successful connects. 
 */
void 
event_cb(/*struct bufferevent *buffer_event,*/ short what, void *ctx)
{ 
    if (what /*& BEV_EVENT_ERROR*/) {
        unsigned long err;

//       while ((err = (bufferevent_get_openssl_error(buffer_event)))) { 
//            const char *msg = (const char*)
//                ERR_reason_error_string(err);
//            const char *lib = (const char*)
//                ERR_lib_error_string(err);
//            const char *func = (const char*)
//                ERR_func_error_string(err);
//            fprintf(stderr,
//                "%s in %s %s\n", msg, lib, func);
//        }

        if (errno)
            perror("connection error");
    }

//    if (what & BEV_EVENT_EOF)
//        client_dc(buffer_event, ctx);

//    if (what & BEV_EVENT_CONNECTED)
//        printf("CONNECTION SUCCESSFUL\n");

//    if (what & BEV_EVENT_TIMEOUT)
//        printf("TIMEOUT\n");
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
