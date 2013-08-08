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
    process_node            *curr_process_node = NULL, *temp_process_node = NULL;
    client_node             *curr_client = NULL;
    watcher_node            *temp_watcher_node = NULL;
    char                    *reply_txt = NULL;
    int                     reply_len = 0;

    find_client_and_process_from_process_watcher(req, &curr_client, &curr_process_node, &temp_watcher_node); 

    free_one_watcher_node(temp_watcher_node);

    if (curr_client == NULL) {
        fprintf(stderr, "ERROR: NO client found for returning process!!! \n");
    } 
    else {

        curr_process_node->process_data->exit_code = exit_status; 
        reply_txt = package_reply(curr_process_node->process_data, &reply_len);
        send_reply(curr_client->client_data, reply_txt, reply_len);
        free(reply_txt);
        reply_txt = NULL; 
        uv_shutdown_t       *out = NULL, *err = NULL;
        uv_close((uv_handle_t *) &curr_process_node->process_data->out_pipe, NULL);
        uv_close((uv_handle_t *) &curr_process_node->process_data->err_pipe, NULL);
        curr_process_node->process_data = free_process(curr_process_node->process_data);
        uv_close((uv_handle_t *) req, NULL);
        free_one_process_node(curr_client, curr_process_node); 
    } 
//printf("leaving child exit\n");
}


/*
 * Call back triggered when a running child process sends data to standard out. The data is captured and saved with the process so that 
 * it can later be sent back to the client. 
 */
void 
read_out(uv_stream_t *out_pipe, ssize_t nread, uv_buf_t buf)
{//printf("read_out, nread: %d\n", (int) nread);
    if (nread < 1){
        free(buf.base);
        return;
    }

    int             buffer_size_increase = 0, i;
    process_node    *temp_node = find_process_from_pipe(out_pipe);

    if (temp_node == NULL){
        printf("Error: NULL process node returned from find process from pipe\n");
        free(buf.base);
        return;
    }

    process         *current_process = temp_node->process_data;

    if (current_process == NULL){ 
        free(buf.base);
        return;
    }

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
    if (nread < 1){
        free(buf.base);
    return;
    }

    int             buffer_size_increase = 0, i;
    process_node    *temp_node = find_process_from_pipe(err_pipe);

    if (temp_node == NULL){
        printf("Error: NULL process node returned from find process from pipe\n");
        free(buf.base);
        return;
    }

    process         *current_process = temp_node->process_data;

    if (current_process == NULL){
        free(buf.base);
        return;
    }

    while (current_process->err_position + nread > current_process->err_len + buffer_size_increase)
        buffer_size_increase += data_size;

    current_process->err_output = realloc (current_process->err_output, current_process->err_len + buffer_size_increase);

    for (i = 0; i < nread; i++){
        current_process->err_output[current_process->err_position++] = buf.base[i];
    }
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

    bool            operation_complete = false;

    client              *connecting_client = new_null_client();
    uv_tcp_init(loop, connecting_client->client_connection);

    if (uv_accept(listener, (uv_stream_t*) connecting_client->client_connection) == 0) {
        uv_read_start((uv_stream_t*) connecting_client->client_connection, alloc_buffer, on_read); 
        client_node         *temp_node = new_client_node(connecting_client, clients->head, NULL);
        while(operation_complete == false){
            if(clients->lock == NULL)
                clients->lock = temp_node;

            if(clients->lock == temp_node){

                if (temp_node->next != NULL)
                    temp_node->next->previous = temp_node;
                else 
                    clients->tail = temp_node;

                clients->head = temp_node;
                operation_complete = true;
                clients->lock = NULL;
            }
        }
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
{//printf("process data: \n"); 
    int             i = 0, j = 0, *int_ptr; 
    char            *temp_num = (char *) &current_client->data_length; 

    if (current_client->data_length == 0){ 
            for(i = 0; i < 4; i++)
                temp_num[i] = buf.base[i];

        current_client->data = (char *) malloc(current_client->data_length + 1);
        current_client->data[current_client->data_length] = '\0';
        i = 4;
    }

    for ( ; (current_client->data_position < current_client->data_length) && (i < nread); )
        current_client->data[current_client->data_position++] = buf.base[i++];
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
execute_request(client_node *current_client_node)  
{//printf("execute request \n");

    uv_stdio_container_t    child_stdio[3];
    client                  *current_client = current_client_node->client_data;
    int                     len, ret = 0;
    char                    *command = NULL, *args[3], *reply, *hook = NULL, *payload = NULL;
    bool                    file_exists, operation_complete = false;
    process_node            *temp_node = NULL;
    json_object             *jobj = json_tokener_parse(current_client->data);
    json_object             *hook_jobj = json_object_object_get(jobj, "hook");
    json_object             *payload_jobj = json_object_object_get(jobj, "payload");
    uv_process_options_t    options = {0};
    watcher_node *temp_watcher_node = new_watcher_node(NULL, NULL, new_null_watcher_pack());

    len =  json_object_get_string_len(hook_jobj);
    if (len > 0){
        hook = (char *) malloc(len + 1);
        hook[len] = '\0';
        strcpy(hook, json_object_get_string(hook_jobj)); 
    }
    else {
        hook = (char *) malloc(2);
        hook[0] = 'g';
        hook[1] = '\0';
    }
    json_object_put(hook_jobj);

    len =  json_object_get_string_len(payload_jobj);
    if (len > 0){
        payload = (char *) malloc(len + 1);
        payload[len] = '\0';
        strcpy(payload, json_object_get_string(payload_jobj)); 
    }
    else {
        payload = (char *) malloc(1);
        payload[0] = '\0';
    }
    json_object_put(payload_jobj);
    json_object_put(jobj);

    temp_node = new_process_node(new_null_process(), NULL, NULL);
    temp_node->process_data->process_call = current_client->data;
    current_client->data = NULL; 
    reset_client(current_client);

    command = assemble_command(hook);

    file_exists = file_exist(command);
    if (file_exists){

        uv_pipe_init(loop, &temp_node->process_data->err_pipe, 1);
        uv_pipe_init(loop, &temp_node->process_data->out_pipe, 1);
        uv_pipe_open(&temp_node->process_data->err_pipe, 0);
        uv_pipe_open(&temp_node->process_data->out_pipe, 1);

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

        temp_watcher_node->pack_data->child_req = &temp_node->process_data->child_req;
        temp_watcher_node->pack_data->containing_process_node = temp_node;

        ret = uv_spawn(loop, &temp_node->process_data->child_req, options);
    }

    if (!file_exists || ret != 0){ 
        fprintf(stderr, "%s\n", uv_strerror(ret));  
        temp_node->process_data->exit_code = 0;
        strncpy(temp_node->process_data->out_output, "", temp_node->process_data->out_len);
        strncpy(temp_node->process_data->err_output, "", temp_node->process_data->err_len);
        reply = package_reply(temp_node->process_data, &len);
        send_reply(current_client, reply, len);
        free(reply);
        free_process_nodes(temp_node);
    }
    else {
        uv_read_start((uv_stream_t*) &temp_node->process_data->out_pipe, alloc_buffer, read_out);
        uv_read_start((uv_stream_t*) &temp_node->process_data->err_pipe, alloc_buffer, read_err);

        temp_watcher_node->pack_data->owner_node = current_client_node;
        temp_node->next = current_client->processes;
        temp_node->previous = NULL;
        if (temp_node->next != NULL)
            temp_node->next->previous = temp_node;
        else
            current_client->processes_tail = temp_node;
        current_client->processes = temp_node;


        while(operation_complete == false) {
            if (watchers->lock == NULL)
                watchers->lock = temp_watcher_node;

            if (watchers->lock == temp_watcher_node){
                temp_watcher_node->next = watchers->head;
                if (temp_watcher_node->next != NULL)
                    temp_watcher_node->next->previous = temp_watcher_node;
                else 
                    watchers->tail = temp_watcher_node;
                watchers->head = temp_watcher_node;

                operation_complete = true;
                watchers->lock = NULL;
            }
        }
    }

    free(hook);
    hook = NULL;
    free(payload);
    payload = NULL;
    free(command); 
    command = NULL;
//printf("leaving execute request\n");
}


/*
 * Assembles a responce for the client, from all the data a running process returned, in json format. 
 */
char* 
package_reply(process *current_process, int *len) 
{//printf("package_reply \n"); 
    int                         i = 0, j = 0, temp_len = 0;
    char                        *reply, *prereply;
    struct json_object          *reply_json_object = json_object_new_object();
    struct json_object          *temp_int_json_object = json_object_new_int64(current_process->exit_code);
    struct json_object          *temp_stdout_json_object = json_object_new_string(current_process->out_output);
    struct json_object          *temp_stderr_json_object = json_object_new_string(current_process->err_output);
    struct json_object          *temp_hook_json_object = json_object_new_string(current_process->process_call);

    json_object_object_add (reply_json_object, "exit_code", temp_int_json_object); 
    json_object_object_add (reply_json_object, "stdout", temp_stdout_json_object);
    json_object_object_add (reply_json_object, "stderr", temp_stderr_json_object);
    json_object_object_add (reply_json_object, "hook", temp_hook_json_object); 

    temp_len = strlen(json_object_to_json_string(reply_json_object));
    prereply = (char *) malloc(temp_len + 1);
    prereply[temp_len] = '\0';
    strncpy(prereply, json_object_to_json_string(reply_json_object), temp_len); 

    reply = (char *) malloc(temp_len + 5);
    reply[temp_len + 4] = '\0';

    for (i = 3; i >= 0; i--)
        reply[i] = ((char *) &temp_len)[i];

    for (i = 4, j = 0; i < temp_len + 5; i++)
        reply[i] = prereply[j++];   
    *len = temp_len;

    free(prereply);
    json_object_put(temp_int_json_object);
    json_object_put(temp_stdout_json_object); 
    json_object_put(temp_stderr_json_object); 
    json_object_put(temp_hook_json_object); 
    json_object_put(reply_json_object);
//printf("leaving package reply\n");
    return(reply);
}

/*
 * Sends to messages to the client first is the length of the second and the second is the packaged reply containing the 
 * results of the hook called by the client. 
 */
void 
send_reply (client *current_client, char *reply_txt, int reply_size)
{//printf("send reply\n");

    uv_write_t      *data_write = (uv_write_t *) malloc(sizeof(uv_write_t));
    uv_buf_t        data_write_buff;

    data_write->data = (void*) reply_txt;
    data_write_buff.len = reply_size + 4;
    data_write_buff.base = reply_txt; 

    uv_write(data_write, (uv_stream_t*) current_client->client_connection, &data_write_buff, 1, on_write); 
//printf("leaving send reply\n");
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
 *  
 */
void
on_write (uv_write_t *req, int status)
{//printf("on write \n");
    req->data = NULL;
    free(req);
    req = NULL; 
//printf("leaving on write\n");
}

/*
 * Call back triggered by data being ready to read from the client. Recives the data and sends it for processing. 
 */
void 
on_read(uv_stream_t *client_conn, ssize_t nread, uv_buf_t buf)
{//printf("on read, nread: %d\n", (int) nread);
    client_node     *current_client_node = find_client_from_connection(client_conn);
    client          *current_client = NULL;

 //printf("clients  %p, head %p, tail %p\n", clients, clients->head, clients->tail);
   if (current_client_node != NULL)
        current_client = current_client_node->client_data;

    if (nread < 1) { 
        if (nread == UV_EOF) {
            fprintf(stderr, "Read error: EOF. \n");
            free_one_client_node(current_client_node); 
        }
        else {printf("WTF ");
            uv_read_stop((uv_stream_t *) current_client->client_connection);
            uv_close((uv_handle_t *) current_client->client_connection, NULL);
            free_one_client_node(current_client_node); 

        }
    }
    else {

        if (current_client != NULL) {
            process_data_from_client(current_client, nread, buf);

            if(current_client->data_length == current_client->data_position && current_client->data_length != 0)
                execute_request(current_client_node);
        }
        else {
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
{printf("signal_cb \n");
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
