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
 * Call back creating buffer that libuv functions need. 
 */
uv_buf_t 
 alloc_buffer(uv_handle_t *handle, size_t suggested_size) 
{//printf("alloc buffer \n");
    return uv_buf_init((char*) malloc(suggested_size), suggested_size);             
}

/*
 * Call back triggered when a child process finishes and returns. Sends the client the data from the 
 * process run and the origional hook that was sent when the process was launched. 
 */
void 
child_exit(uv_process_t *req, int exit_status, int term_signal)
{//printf("child exit called \n");
    client_node             *temp_node = clients->head;
    client                  *curr_client = NULL;
    process_node            *curr_process_node = NULL, *temp_process_node = NULL;
    uv_process_t            *temp_process_watcher = NULL;
    char                    *reply_txt = NULL;
    int                     reply_len = 0;

    find_client_and_process_from_process_watcher(req, &curr_client, &curr_process_node); 

    if (curr_client == NULL) {
        fprintf(stderr, "ERROR: NO client found for returning process!!! \n");
    } 
    else {//printf("process_call: %s\n", curr_process_node->process_data->process_call);

        curr_process_node->process_data->exit_code = exit_status; 
        reply_txt = package_reply(curr_process_node->process_data, &reply_len);
        send_reply(curr_client, reply_txt, reply_len);
        free(reply_txt);
        reply_txt = NULL;
        uv_close((uv_handle_t *) &curr_process_node->process_data->out_pipe, NULL);
        uv_close((uv_handle_t *) &curr_process_node->process_data->err_pipe, NULL);
        uv_close((uv_handle_t *) req, NULL);

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
 * Call back triggered when a running child process sends data to standard out. The data is captured and saved with the process so that 
 * it can later be sent back to the client. 
 */
void 
read_out(uv_stream_t *out_pipe, ssize_t nread, uv_buf_t buf)
{//printf("read_out, nread: %d\n", (int) nread);
    if (nread < 1)
        return;

    int             buffer_size_increase = 0, i;
    process_node    *temp_node = find_process_from_pipe(out_pipe);

    if (temp_node == NULL){
        printf("Error: NULL process node returned from find process from pipe\n");
        return;
    }

    process         *current_process = temp_node->process_data;

    if (current_process == NULL) // TODO: call function to prune dead connections. 
        return;

    while (current_process->out_position + nread > current_process->out_len + buffer_size_increase)
        buffer_size_increase += data_size;

    current_process->out_output = realloc (current_process->out_output, current_process->out_len + buffer_size_increase);

    for (i = 0; i < nread; i++)
        current_process->out_output[current_process->out_position++] = buf.base[i];

    free(buf.base);
}

/*
 * Call back triggered when a running child process sends data to standard error. the data is captured and saved with the process so that 
 * it can later be sent back to the client. 
 */
void 
read_err(uv_stream_t *err_pipe, ssize_t nread, uv_buf_t buf)
{//printf("read_err, nread: %d\n", (int) nread);
    if (nread < 1)
    return;

    int             buffer_size_increase = 0, i;
    process_node    *temp_node = find_process_from_pipe(err_pipe);

    if (temp_node == NULL){
        printf("Error: NULL process node returned from find process from pipe\n");
        return;
    }

    process         *current_process = temp_node->process_data;

    if (current_process == NULL) // TODO: call function to prune dead connections. 
        return;

    while (current_process->err_position + nread > current_process->err_len + buffer_size_increase)
        buffer_size_increase += data_size;

    current_process->err_output = realloc (current_process->err_output, current_process->err_len + buffer_size_increase);

    for (i = 0; i < nread; i++){
        current_process->err_output[current_process->err_position++] = buf.base[i];
        //printf("%c", buf.base[i]);
    } //printf("\nprocess_call: %s\n", current_process->process_call);
    free(buf.base);
}

/*
 * Call back triggered when a new client connects to hooky. 
 */
void 
on_connect(uv_stream_t *listener, int status)
{//printf("on connect \n");
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
        uv_close((uv_handle_t*) connecting_client->client_connection, NULL);
        free_client(connecting_client);
    }
}

/*
 * Determines if data recived froma  client is a new or continuing client request, parses data, updates client info. 
 */
void 
process_data_from_client(client *current_client, ssize_t nread, uv_buf_t buf)
{//printf("process data: %d\n", (int) buf.base); 
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

    for (; j < current_client->data_length; j++)
        current_client->data[j] = '\0';                   
//    printf("data: ");
//
//    for (j = 0; j < current_client->data_length; j++)
//        printf("%c", current_client->data[j]);
//    printf("\n");
//    free(buf.base);
}

/*
 * Uses the parsed data the client sent and the base path from the config file to create a command to be executed in a child process. 
 */
char*
assemble_command(char *hook)
{//printf("Assemble command \n");
    char                *command = NULL; 
    int                 i, len;   

    if (hook == NULL){
        printf("Error: NULL hook passed to assemble command. NULL command returned.");
        return (NULL);
    }

    i = strlen(hook);
    len = strlen(clients->base_path) + i + 2;
    command = (char *) malloc(len); 
    strcpy(command, clients->base_path);                      
    strncat(command, hook, i);

    for (i = 0; i < len; i++)
        if (command[i] == '.')
            command[i] = '/';

    return (command);
}

/*
 * Uses the parsed data from the client, including the assembled command, and argurments passed for it to iniciate a child process executing the 
 * command. 
 */
void 
execute_request(client *current_client)  
{//printf("execute request \n");
    uv_stdio_container_t    child_stdio[3];
    int                     len, ret;
    char                    *command = NULL, *args[3], *reply, *hook = NULL, *payload = NULL;
    bool                    file_exists;
    process_node            *temp_node = NULL;
    json_object             *jobj = json_tokener_parse(current_client->data);
    json_object             *hook_jobj = json_object_object_get(jobj, "hook");
    json_object             *payload_jobj = json_object_object_get(jobj, "payload");
    uv_process_options_t    options = {0};

    len =  json_object_get_string_len(hook_jobj) + 1;
    hook = (char *) malloc(len);
    strcpy(hook, json_object_get_string(hook_jobj));
    json_object_put(hook_jobj);

    len =  json_object_get_string_len(payload_jobj) + 1;
    payload = (char *) malloc(len);
    strcpy(payload, json_object_get_string(payload_jobj));
    json_object_put(payload_jobj);
    json_object_put(jobj);

    temp_node = new_process_node(new_null_process(), NULL);
    temp_node->process_data->process_call = current_client->data; //printf("process_call: %s\n", temp_node->process_data->process_call);
    current_client->data = NULL; 
    reset_client(current_client);

    uv_pipe_init(loop, &temp_node->process_data->err_pipe, 1);
    uv_pipe_init(loop, &temp_node->process_data->out_pipe, 1);
    uv_pipe_open(&temp_node->process_data->err_pipe, 0);
    uv_pipe_open(&temp_node->process_data->out_pipe, 1);

    command = assemble_command(hook);

    file_exists = file_exist(command);
    if (file_exists){
    args[0] = command;
    args[1] = payload;
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

    if (!file_exists || ret != 0){ //printf("file exist: %d \n", (int)file_exists);
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
        //printf("process_call: %s\n", current_client->processes->process_data->process_call);
    }

    free(hook);
    hook = NULL;
    free(payload);
    payload = NULL;
    free(command); 
    command = NULL;
}

/*
 * Assembles a responce for the client, from all the data a running process returned, in json format. 
 */
char* 
package_reply(process *current_process, int *len) 
{//printf("package_reply \n"); 
    char                        *reply;
    struct json_object          *reply_json_object = json_object_new_object();
    struct json_object          *temp_int_json_object = json_object_new_int64(current_process->exit_code);
    struct json_object          *temp_stdout_json_object = json_object_new_string(current_process->out_output);
    struct json_object          *temp_stderr_json_object = json_object_new_string(current_process->err_output);
    struct json_object          *temp_hook_json_object = json_object_new_string(current_process->process_call);

    json_object_object_add  (reply_json_object, "exit_code", temp_int_json_object); 
    json_object_object_add  (reply_json_object, "stdout", temp_stdout_json_object);
    json_object_object_add  (reply_json_object, "stderr", temp_stderr_json_object);
    json_object_object_add (reply_json_object, "hook", temp_hook_json_object); 

    *len = strlen(json_object_to_json_string(reply_json_object)) + 1;
    reply = (char *) malloc(*len);
    strcpy(reply, json_object_to_json_string(reply_json_object));

    json_object_put(temp_int_json_object);
    json_object_put(temp_stdout_json_object); 
    json_object_put(temp_stderr_json_object); 
    json_object_put(temp_hook_json_object); 
    json_object_put(reply_json_object);
//printf("reply: %s\n", reply);
    return(reply);
}

/*
 * Sends to messages to the client first is the length of the second and the second is the packaged reply containing the 
 * results of the hook called by the client. 
 */
void 
send_reply (client *current_client, char *reply_txt, int reply_size)
{//printf("send reply, reply: %s\n", reply_txt);
    uv_write_t      *data_len_write = NULL;
    uv_write_t      *data_write = NULL;
    uv_buf_t        data_len_buff;
    uv_buf_t        data_write_buff;

    data_len_write = (uv_write_t *) malloc(sizeof(uv_write_t));
    data_len_write->data = (void*) &reply_size;
    data_len_buff.len = 4;
    data_len_buff.base = (char *) &reply_size;
    uv_write(data_len_write, (uv_stream_t*) current_client->client_connection, &data_len_buff, 1, on_write);

    data_write = (uv_write_t *) malloc(sizeof(uv_write_t));
    data_write->data = (void*) reply_txt;
    data_write_buff.len = reply_size;
    data_write_buff.base = reply_txt;
    uv_write(data_write, (uv_stream_t*) current_client->client_connection, &data_write_buff, 1, on_write);

}

/*
 * After starting a process, or determining that the input is invalid, this function is called to reset the client status so that a new request can be recived. 
 */
void 
reset_client(client *current_client)
{//printf("reset client\n");
    current_client->data_length = 0;
    current_client->data_position = 0;
    free(current_client->data);
    current_client->data = NULL;
}

/*
 * This function must exist since a call back is required by the function uv_write which sends data to the client. However
 * it doesn't do anything since we aren't closing any connections after writing. it is possiable some memory management stuff could end up here
 * but for now it is intentinally empty. 
 */
void
on_write (uv_write_t *req, int status)
{//printf("on write \n");

    free(req);
    req = NULL;
    // TODO: verify memory management with libuv and json-c lib

}

/*
 * Call back triggered by data being ready to read from the client. Recives the data and sends it for processing. 
 */
void 
on_read(uv_stream_t *client_conn, ssize_t nread, uv_buf_t buf)
{//printf("on read, nread: %d\n", (int) nread);
    client_node     *current_client_node = find_client_from_connection(client_conn);
    client          *current_client = NULL;

    if (current_client_node != NULL)
        current_client = current_client_node->client_data;

    if (nread < 1) { //printf("nread: %d\n", (int) nread);          // TODO: fix detect client dc
        if (nread == UV_EOF) {
            fprintf(stderr, "Read error: EOF. %p\n", current_client);

            //uv_read_stop((uv_stream_t *) current_client->client_connection);
            //uv_close((uv_handle_t *) current_client->client_connection, NULL); 
            if (clients->head = current_client_node){ 
                clients->head = free_one_client_node(current_client_node);
                current_client = NULL; //printf("HUH \n");
            }
            else {
                client_node         *temp_node = clients->head;
                while(temp_node != NULL){
                    if (temp_node->next == current_client_node)
                        temp_node->next = free_one_client_node(temp_node->next);
                }
            } 
        }
    }
    else {

        if (current_client != NULL) {
            process_data_from_client(current_client, nread, buf);

            if(current_client->data_length == current_client->data_position && current_client->data_length != 0)
                execute_request(current_client);
        }
        else {
            // TODO: call function to prune dead connections. 
            uv_close((uv_handle_t*) client_conn, NULL);
        }
    }
    free(buf.base);
}

/* 
 * catches interrupt signal and allows the program to cleanup before exiting. 
 */
void 
signal_cb (uv_signal_t *sig_event, int signum) 
{//printf("signal_cb \n");
    struct timeval      delay = { 2, 0 };

    printf("Caught an interrupt signal; exiting cleanly.\n");

    raise(SIGTERM);
}

/*
 * Takes a complete path to a file and determines if it actually exists. 
 */
bool 
file_exist(char file_path[])
{//printf("file exist %s \n", file_path);
    if (access(file_path, F_OK) == 0) 
        return(true);
    else 
        return(false);
}

/*
 * 
 */
void 
client_dc(void *ctx)          // TODO:    adapt to libuv 
{//printf("client_dc\n");
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
