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

void on_connect(uv_stream_t *server, int status);
void process_data_from_client(client *current_client, ssize_t nread, uv_buf_t buf);
int execute_request(client *current_client, char *path);
void reset_client(client *current_client);
void on_read(uv_stream_t *client, ssize_t nread, uv_buf_t buf);
void signal_cb (uv_signal_t *sig_event, int signum);
void client_dc(/*struct bufferevent *buffer_event,*/ void *ctx);
void event_cb(/*struct bufferevent *buffer_event,*/ short what, void *ctx);
bool file_exist(char *file_path);

#endif 