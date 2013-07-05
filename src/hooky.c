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

 client_list    *clients;
 uv_loop_t       *loop;

void 
usage()
{
    printf("usage: ./hooky path/to/config/file\n");
    printf("example: \n ./hooky ../deps/config.txt\n");
    exit(0);
}

/*
 * 
 */
bool 
verify_args(int num_args, char **argv)
{
   if (num_args < 2)
       return (false);
   else
       return (true);
}

/* 
 * Takes an address in the form a.b.c.d:port_number and parses it storing the ip
 * address and port number in the approiate char arrays, address_to_parse[22], ip_address[16] 
 * and port_number[6], returns true if successful otherwise returns false 
 */
bool 
parse_address(char *address_to_parse, char *ip_address, char* port_number) 
{
    int     i, j;
    bool    port_now = false;

    j = 0;

    if ( address_to_parse == NULL)
        return port_now;

    for (i = 0; i < complete_address_len; ){
        if (address_to_parse[i] == ':') {
            i++;
            port_now = true; 
            j = 0;
        }

        if (port_now == false) {
            if (j >= ip_len)
                return (port_now);
            ip_address[j++] = address_to_parse[i++];
        }
        else {
            if (j >= port_len)
                return (port_now);
            port_number[j++] = address_to_parse[i++];
        }
    }
    return port_now;
}

/*
 * 
 */
char*
parse_config(char argv[], char *addr)          
{
    int     len, i, j;
    char    *path = NULL;
    char    handle[handle_len];
    char    *buffer = NULL;
    path = (char *) malloc(file_name_len);
    FILE    *file_pointer = NULL;

    file_pointer = fopen(argv, "r");
    if (file_pointer == NULL) {
        fprintf(stderr, "Unable to open config file. Check file name and path!\n");
        exit(0);
    }

    fseek(file_pointer, 0, SEEK_END);
    len = ftell(file_pointer);
    rewind(file_pointer);

    buffer = (char *) malloc(len);
    if (buffer == NULL){
        fprintf(stderr, "Failed to creat buffer.");
        exit(0);
    }

    if (fread(buffer, 1, len, file_pointer) != len) {
        fprintf(stderr, "Error reading file.\n");
        exit(0);
    }
    fclose(file_pointer);

    for (i = 0; i < len; i++){
        j = 0;
        bzero(handle, handle_len);
        for (; i < len && buffer[i] != '='; i++)
            handle[j++] = buffer[i]; 

        handle[j++] = '\0'; 
        i++;
        if (strcmp(handle, "path") == 0){
            for (j = 0; i < len && buffer[i] != '\n' && j < file_name_len - 1; i++)
                path[j++] = buffer[i];
            path[j] = '\0'; 
        } else if (strcmp(handle, "address") == 0){
            for (j = 0; i < len && buffer[i] != '\n' && j < complete_address_len - 1; i++)
                addr[j++] = buffer[i];
            addr[j] = '\0'; 
        } else if (strcmp(handle, "") != 0){
            fprintf(stderr, "Handle %s unknown. Check config file. \n", handle);
            exit(0);
        }
    }
    free(buffer);
    return(path);
}

/*
 * 
 */
void 
init_accept_clients(uv_loop_t *loop, uv_tcp_t *client_listener, char address[])
{
    char                ip_address[ip_len], port_number[port_len];
    struct sockaddr_in  svc_address;
    struct in_addr      *ip_bytes = (struct in_addr *) malloc (sizeof(struct in_addr));

    if (parse_address(address, ip_address, port_number)){               //uv_ip4_addr("0.0.0.0", 7000)
        inet_aton(ip_address, ip_bytes); 
        memset(&svc_address, 0, sizeof(svc_address));
        svc_address.sin_family = AF_INET;
        svc_address.sin_addr.s_addr = (*ip_bytes).s_addr; 
        svc_address.sin_port = htons(atoi(port_number));

    uv_tcp_init(loop, &listener);
    uv_tcp_bind(&server, svc_address);

    int r = uv_listen((uv_stream_t*) client_listener, 128, on_connect);
    if (r) {
        fprintf(stderr, "Listen error %s\n", uv_err_name(uv_last_error(loop)));
        return 1;
    }
}


/*
 * Recieved the event_loop (event base) and adds an event to it that will trigger 
 * when a signal is recived. It handles those signals, for the kill signal it frees
 * all memory and shuts down the event loop instead of simply letting it crash.
 */
void 
init_signals(struct event_base *event_loop)
{    
    struct event *signal_event = NULL;

    signal_event = evsignal_new(event_loop, SIGINT, signal_cb, (void *) event_loop);
    if (!signal_event || event_add(signal_event, NULL) < 0) {
        fprintf(stderr, "Could not create/add signal event.\n");
        exit(0);
    }
}

int
main(int argc, char **argv)
{
    uv_tcp_t                listener;
    char                    client_address[complete_address_len] = listen_address;

    if(!verify_args(argc, argv))
        usage();

    clients = new_null_client_list();
    loop = uv_loop_new();

    clients->base_path = parse_config(argv[1], client_address); 
    init_accept_clients(loop, &listener, client_address);

    //init_signals(event_loop);
    uv_run(loop, UV_RUN_DEFAULT);
    //event_base_free(event_loop);
}
