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

#ifndef HOOKY_H
#define HOOKY_H

// functions

void usage();
bool verify_args(int num_args, char **argv);
bool parse_address(char *address_to_parse, char *ip_address, char* port_number);
char* parse_config(char argv[], char *addr);
void init_accept_clients(uv_loop_t *loop, uv_tcp_t *client_listener, char address[]);
void init_signals(uv_signal_t *signal_event);

#endif

/*
	TODO: Consider how to handle a process that hangs. Some processes could run for long time when they are functioning correctly. 
		* -Kill all of a clients processes if the client disconnects? 
		*  or try to make identifying them and reconnecting them to their currently running processes possiable, future version?
		
*/