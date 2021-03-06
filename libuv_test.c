#include <stdio.h>

#define USING_UV_SHARED

#include <stdlib.h>
#include <uv.h>

#include "debug.h"
#include "libuv_compat.h"
#include "wmq_error.h"

//buffer allocation callback
static void on_alloc_buffer(uv_handle_t* handle, size_t size, uv_buf_t* buf) {
    buf->base = malloc(size);
    buf->len = size;
}

//TCP client connection close callback
static void on_close_client_handle(uv_handle_t* handle) {
    WMQ_LOG(LL_INFO, "");
    free(handle);
}

//TCP client connection read callback
static void on_incoming_tcp_read(uv_stream_t* client_handle, ssize_t nread, const uv_buf_t* buf) {
    if (nread <= 0) {
        WMQ_LOG(LL_INFO, "close client_handle");
        uv_close((uv_handle_t*)client_handle, on_close_client_handle);
    } else {
        debug_print_hex(LL_INFO, "", buf->base, nread, 0);
        //uv_write()
    }
}

//tcp server client connection callback
void on_accept_cb(uv_stream_t* server_handle, int status) {
    uv_loop_t* loop = server_handle->loop;
    uv_tcp_t* client;
    int rc;

    if (status != 0) {
        WMQ_LOG(LL_ERROR, "error %d \"%s\"", status, uv_strerror(status));
        return;
    } else {
        WMQ_LOG(LL_TRACE, "");
    }

    //allocate new server
    client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
    if (client == NULL) {
        WMQ_LOG_NOMEMORY("malloc uv_tcp_t error");
        //force_stop(loop);
        return;
    }

    //initialize libuv handle
    rc = uv_tcp_init(loop, client);
    WMQ_CHECK_ERROR(rc, "on_accept_cb: uv_tcp_init");
    if (rc) {
        free(client);
        return;
    }

    //accept connection
    rc = uv_accept(server_handle, (uv_stream_t*)client);
    WMQ_CHECK_ERROR_AND_RETURN_VOID(rc, "uv_accept");

    //start reading data
    uv_read_start((uv_stream_t*)client, on_alloc_buffer, on_incoming_tcp_read);
}

//stdin read callback
static void on_tty_read(uv_stream_t* tty_in, ssize_t nread, const uv_buf_t* buf) {
    if (nread <= 0) {
        WMQ_LOG(LL_INFO, "close tty_in");
        uv_close((uv_handle_t*)tty_in, NULL);
    } else {
        //debug_print_hex(LL_INFO, __func__, buf->base, nread, 0);
        //WMQ_LOG(LL_INFO, "%zu: %.*s", nread, nread, buf->base);
        debug_print_hex(LL_INFO, "", buf->base, nread, 0);
        if (strncmp(buf->base, "exit", 4) == 0) {
            WMQ_LOG(LL_INFO, "exit command received");
            uv_close((uv_handle_t*)tty_in, NULL);
        }
    }
}

//initialize stdin reader
int init_stdin(uv_loop_t* loop, uv_tty_t* tty) {
    int rc;
    //_open_osfhandle()

    rc = uv_tty_init(loop, tty, 0, 1);
    WMQ_CHECK_ERROR_AND_RETURN_RESULT(rc, "uv_tty_init");

    rc = uv_read_start((uv_stream_t*)tty, on_alloc_buffer, on_tty_read);
    WMQ_CHECK_ERROR_AND_RETURN_RESULT(rc, "uv_read_start");
    return 0;
}

//handle enumerator/visitor callback
void on_walk_handles(uv_handle_t* handle, void* arg) {
    WMQ_LOG(LL_INFO, "%s", uv_handle_type_name(uv_handle_get_type(handle)));
    uv_close(handle, NULL);
}

#define LISTEN_PORT 5761

int main(int argc, char** argv) {
    int rc;
    uv_loop_t loop = {0};
    uv_tty_t tty = {0};
    uv_tcp_t server = {0};

    struct sockaddr addr = {0};
    //uv_getaddrinfo_t dns_request = {0};
    //struct addrinfo hints;

    debug_set_level(LL_DETAIL, WMQ_LOG_OPTION_USE_ODS | WMQ_LOG_OPTION_USE_STDERR | WMQ_LOG_OPTION_SHOW_TIME | WMQ_LOG_OPTION_SHOW_PID | WMQ_LOG_OPTION_SHOW_TID);
    WMQ_LOG(LL_INFO, "starting, sizeof(long unsigned int):%zu sizeof(int):%zu", sizeof(long unsigned int), sizeof(int));

    //init libuv message loop
    rc = uv_loop_init(&loop);
    WMQ_CHECK_ERROR_AND_RETURN_RESULT(rc, "uv_loop_init");

    //init TCP server on LISTEN_PORT
    rc = uv_tcp_init(&loop, &server);
    WMQ_CHECK_ERROR_AND_RETURN_RESULT(rc, "uv_tcp_init");

    addr.sa_family = AF_INET;
    rc = uv_ip4_addr("0.0.0.0", LISTEN_PORT, (struct sockaddr_in*)&addr);

    //dns_request.rc = uv_getaddrinfo(&loop, &dns_request, get_addrinfo_cb, resolver->host, port, &hints);

    rc = uv_tcp_bind(&server, &addr, 0);
    WMQ_CHECK_ERROR_AND_RETURN_RESULT(rc, "uv_tcp_bind");

    rc = uv_listen((uv_stream_t*)&server, SOMAXCONN, on_accept_cb);
    WMQ_CHECK_ERROR_AND_RETURN_RESULT(rc, "uv_listen");

    //unreference handle, close loop after tty closing
    uv_unref((uv_handle_t*)&server);

    //init STDIN reader
    rc = init_stdin(&loop, &tty);
    WMQ_CHECK_ERROR_AND_RETURN_RESULT(rc, "init_stdin");

    rc = uv_run(&loop, UV_RUN_DEFAULT);
    WMQ_CHECK_ERROR_AND_RETURN_RESULT(rc, "uv_run");

    uv_walk(&loop, on_walk_handles, NULL);

    //close message loop
    rc = uv_loop_close(&loop);
    WMQ_CHECK_ERROR_AND_RETURN_RESULT(rc, "uv_loop_close");

    return 0;
}
