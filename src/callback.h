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

void on_connect(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address, int socklen, void *c_list);
void process_data_from_client(client *current_client, char *data_recived, int len);
int execute_request(client *current_client);
int package_reply(int request_exit_code, char *reply, client *current_client);
void reset_client(client *current_client);
void on_read(struct bufferevent *buffer_event, void *c_list);
void signal_cb (evutil_socket_t sig, short events, void *user_data);
void client_dc(struct bufferevent *buffer_event, void *ctx);
void event_cb(struct bufferevent *buffer_event, short what, void *ctx);
bool file_exist(char *file_path);

#endif 