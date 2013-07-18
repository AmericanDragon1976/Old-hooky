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
uv_buf_t 
 alloc_buffer(uv_handle_t *handle, size_t suggested_size) 
{printf("alloc buffer \n");
    return uv_buf_init((char*) malloc(suggested_size), suggested_size);             //FREE for this Malloc ?
}

/*
 * 
 */
void 
child_exit(uv_process_t *req, int exit_status, int term_signal)
{ printf("child exit called \n");
fprintf(stderr, "Process exited with status %d, signal %d\n", exit_status, term_signal);

    client_node             *temp_node = clients->head;
    client                  *curr_client = NULL;
    process_node            *curr_process_node = NULL, *temp_process_node = NULL;
    uv_process_t            *temp_process_watcher = NULL;
    char                    *reply_txt = NULL;
    int                     *reply_len = (int *) malloc(sizeof(int));

    find_client_and_process_from_process_watcher(req, curr_client, curr_process_node);

    if (curr_client == NULL) {
        fprintf(stderr, "ERROR: NO client found for returning process!!! \n");
    } 
    else {
        curr_process_node->process_data->exit_code = exit_status; 
        reply_txt = package_reply(curr_process_node->process_data, reply_len);
        send_reply(curr_client, reply_txt, *reply_len);
        free(reply_len);
        reply_len = NULL;
        uv_close((uv_handle_t *) &curr_process_node->process_data->child_req, NULL);
        // DO I NEED TO CALL UV_CLOSE ON THE PIPES TOO??   

        if (curr_client->processes == curr_process_node){
            curr_client->processes = free_one_process_node(curr_process_node);
        }
        else {
            for( temp_process_node = curr_client->processes; temp_process_node->next != curr_process_node; temp_process_node = temp_process_node->next)
                ;
            temp_process_node->next = free_one_process_node(curr_process_node);
        }
    }

}

/*
 * 
 */
void 
read_out(uv_stream_t *out_pipe, ssize_t nread, uv_buf_t buf)
{printf("read_out\n");

    int             buffer_size_increase = 0, i;
    process         *current_process = find_process_from_pipe(out_pipe);

    while (current_process->out_position + nread > current_process->out_len + buffer_size_increase)
        buffer_size_increase += data_size;

    current_process->out_output = realloc (current_process->out_output, current_process->out_len + buffer_size_increase);

    for (i = 0; i < nread; i++)
        current_process->out_output[current_process->out_position++] = buf.base[i];
}

/*
 * 
 */
void 
read_err(uv_stream_t *err_pipe, ssize_t nread, uv_buf_t buf)
{printf("read_err\n");

    int             buffer_size_increase = 0, i;
    process         *current_process = find_process_from_pipe(err_pipe);

    while (current_process->err_position + nread > current_process->err_len + buffer_size_increase)
        buffer_size_increase += data_size;

    current_process->err_output = realloc (current_process->err_output, current_process->err_len + buffer_size_increase);

    for (i = 0; i < nread; i++)
        current_process->err_output[current_process->err_position++] = buf.base[i];
}

/*
 * 
 */
void 
on_connect(uv_stream_t *listener, int status)
{ printf("on connect \n");
    if (status == -1) {
        fprintf(stderr, "Error on client connect. \n");
        return;
    }

    client              *connecting_client = new_null_client();
    uv_tcp_init(loop, connecting_client->client_connection);

    if (uv_accept(listener, (uv_stream_t*) connecting_client->client_connection) == 0) {
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
{printf("processing: %d\n", (int) buf.base); 
    int             i = 0, j = 0, size = 0, *int_ptr; 
    char            str_size[4], temp_char_num[4], *data_recived = buf.base; 

    int_ptr = (int *) &temp_char_num;
    *int_ptr = 0;

    if (current_client->data_length == 0){ 
        j = 0;
        for (i = 3; i >= 0; i--)
            str_size[i] = data_recived[j++];

        for (i = 0; i < 4; i++){ 
            temp_char_num[0] = str_size[i];
            size = size * 256 + (*int_ptr); 
            *int_ptr = 0;
        }

        current_client->data_length = size;
        current_client->data = (char *) malloc(size + 1);
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
char*
assemble_command(char *path, json_object *jobj)
{
    char                *command = NULL;
    int                 i, len;
    json_object         *hook = json_object_object_get(jobj, "hook");

    i = json_object_get_string_len(hook);
    len = strlen(path) + i + 2;
    command = (char *) malloc(len); 
    strcpy(command, path);
    strncat(command, json_object_get_string(hook), i);

    for (i = 0; i < len; i++)
        if (command[i] == '.')
            command[i] = '/';

    json_object_put(hook);
    return (command);
}

/*
 * 
 */
void 
execute_request(client *current_client, char* path)  
{ printf("execute hook \n");
    uv_stdio_container_t    child_stdio[3];
    int                     len, ret;
    char                    *command = NULL, *args[3], *reply;
    bool                    file_exists;
    process_node            *temp_node;
    json_object             *jobj = json_tokener_parse(current_client->data);
    json_object             *payload = json_object_object_get(jobj, "payload");
    uv_process_options_t    options = {0};
    
// TODO: add validation to prevent malformed requests from crashing the program.
    temp_node = new_process_node(new_null_process(), NULL);
    temp_node->process_data->process_call = current_client->data;
    current_client->data = NULL;
    uv_pipe_init(loop, &temp_node->process_data->err_pipe, 1);
    uv_pipe_init(loop, &temp_node->process_data->out_pipe, 1);
    uv_pipe_open(&temp_node->process_data->err_pipe, 0);
    uv_pipe_open(&temp_node->process_data->out_pipe, 1);

    reset_client(current_client);
    command = assemble_command(path, jobj);

    file_exists = file_exist(command);
    if (file_exists){
    args[0] = command;
    args[1] = json_object_get_string(payload);
    args[2] = NULL;

    options.exit_cb = child_exit;
    options.file = args[0];
    options.args = args;
 
    options.stdio_count = 3;
    child_stdio[0].flags = UV_IGNORE;
    child_stdio[1].flags = UV_CREATE_PIPE | UV_WRITABLE_PIPE;
    child_stdio[1].data.stream = (uv_stream_t*) &temp_node->process_data->out_pipe; 
    child_stdio[2].flags = UV_CREATE_PIPE | UV_WRITABLE_PIPE;
    child_stdio[2].data.stream = (uv_stream_t*) &temp_node->process_data->err_pipe;
    options.stdio = child_stdio;


    ret = uv_spawn(loop, &temp_node->process_data->child_req, options); 
    }

    if (!file_exists || ret != 0){
        fprintf(stderr, "%s\n", uv_strerror(ret));  
        temp_node->process_data->exit_code = 0;
        strncpy(temp_node->process_data->out_output, "", temp_node->process_data->out_len);
        strncpy(temp_node->process_data->err_output, "", temp_node->process_data->err_len);
        reply = package_reply(temp_node->process_data, &len);
        send_reply(current_client, reply, len);
        free_process_nodes(temp_node);
    }
    else {
        uv_read_start((uv_stream_t*) &temp_node->process_data->out_pipe, alloc_buffer, read_out);
        uv_read_start((uv_stream_t*) &temp_node->process_data->err_pipe, alloc_buffer, read_err);
        temp_node->next = current_client->processes;
        current_client->processes = temp_node;
    }

    free(command); 
    json_object_put(jobj);
    json_object_put(payload);
}

/*
 * 
 */
char* 
package_reply(process *current_process, int *len)    // TODO: add associated hook and payload data to reply
{printf("package_reply \n");
    char                        *reply;
    struct json_object          *reply_json_object = json_object_new_object();
    struct json_object          *temp_int_json_object = json_object_new_int64(current_process->exit_code);
    struct json_object          *temp_string_json_object = json_object_new_string(current_process->out_output);

    json_object_object_add  (reply_json_object, "exit_code", temp_int_json_object);
    json_object_object_add  (reply_json_object, "stdout", temp_string_json_object);
    json_object_put(temp_string_json_object);
    temp_string_json_object = json_object_new_string(current_process->err_output);                 
    json_object_object_add  (reply_json_object, "stderr", temp_string_json_object);
    json_object_put(temp_string_json_object);
    temp_string_json_object = json_object_new_string(current_process->process_call);
    json_object_object_add (reply_json_object, "Hook", temp_string_json_object);

    reply = json_object_to_json_string(reply_json_object);
    for (*len = 0; reply[++(*len)] != '}';);

    json_object_put(reply_json_object);
    json_object_put(temp_int_json_object);
    json_object_put(temp_string_json_object);

    return(reply);
}

/*
 * 
 */
void 
send_reply (client *current_client, char *reply_txt, int reply_size)
{ printf("send reply \n");
    uv_write_t      *data_len_write = NULL;
    uv_write_t      *data_write = NULL;
    uv_buf_t        data_len_buff;
    uv_buf_t        data_write_buff;

    data_len_write = (uv_write_t *) malloc(sizeof(uv_write_t));
    data_write = (uv_write_t *) malloc(sizeof(uv_write_t));
    data_len_write->data = (void*) &reply_size;
    data_write->data = (void*) reply_txt;
    data_len_buff.len = 4;
    data_len_buff.base = (char *) &reply_size;
    data_write_buff.len = reply_size;
    data_write_buff.base = reply_txt;
    uv_write(data_len_write, (uv_stream_t*) current_client->client_connection, &data_len_buff, 1, on_write);
    uv_write(data_write, (uv_stream_t*) current_client->client_connection, &data_write_buff, 1, on_write);
    free(data_len_write);
    free(data_write);
}

/*
 * 
 */
void 
reset_client(client *current_client)
{printf("reset client\n");
    current_client->data_length = 0;
    current_client->data_position = 0;
    free(current_client->data);
    current_client->data = NULL;
}

void
on_write (uv_write_t *req, int status)
{ printf("on write \n");
    // TODO: verify memory management with libuv and json-c lib

}

/*
 * 
 */
void 
on_read(uv_stream_t *client_conn, ssize_t nread, uv_buf_t buf)
{printf("on read, nread: %d\n", (int) nread);
    if (nread == -1) { printf("nread: %d\n", (int) nread);          // TODO: verify detect client dc is working and fix if not
        if (nread == UV_EOF)
            fprintf(stderr, "Read error: EOF.\n");
        uv_close((uv_handle_t*) client_conn, NULL);
        return;
    }

    client          *current_client = NULL;
    current_client = find_client_from_connection(client_conn);

    if (nread > 0)
        process_data_from_client(current_client, nread, buf);

    if(current_client->data_length == current_client->data_position && current_client->data_length != 0)
        execute_request(current_client, clients->base_path);
}


/* 
 * catches interrupt signal and allows the program to cleanup before exiting. 
 */
void 
signal_cb (uv_signal_t *sig_event, int signum) 
{
    struct timeval      delay = { 2, 0 };

    printf("Caught an interrupt signal; exiting cleanly.\n");

    raise(SIGTERM);
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

/*
 * 
 */
void 
client_dc(void *ctx)          // TODO:    adapt to libuv 
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
