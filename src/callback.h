/*struct bufferevent *buffer_event, void *svc_package
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

#ifndef CALLBACK_H
#define CALLBACK_H

 // functions 

uv_buf_t alloc_buffer(uv_handle_t *handle, size_t suggested_size);
void child_exit(uv_process_t *req, int exit_status, int term_signal);
void read_out(uv_stream_t *out_pipe, ssize_t nread, uv_buf_t buf);
void read_err(uv_stream_t *err_pipe, ssize_t nread, uv_buf_t buf);
void on_connect(uv_stream_t *server, int status);
void process_data_from_client(client *current_client, ssize_t nread, uv_buf_t buf);
char* assemble_command(char *hook);
void execute_request(client *current_client);
char* package_reply(process *current_process, int *len);
void send_reply (client *current_client, char *reply, int len);
void reset_client(client *current_client);
void on_write (uv_write_t *req, int status);
void on_read(uv_stream_t *client, ssize_t nread, uv_buf_t buf);
void signal_cb (uv_signal_t *sig_event, int signum);
bool file_exist(char file_path[]);
void client_dc(void *ctx);

#endif 