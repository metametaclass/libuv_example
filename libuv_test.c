#include <stdio.h>

#define USING_UV_SHARED

#include <stdlib.h>
#include <uv.h>

#include "debug.h"
#include "wmq_error.h"

static void on_alloc_buffer(uv_handle_t* handle, size_t size, uv_buf_t* buf) {
    buf->base = malloc(size);
    buf->len = size;
}

static void tty_read(uv_stream_t* tty_in, ssize_t nread, const uv_buf_t* buf) {
    if (nread <= 0) {
        WMQ_LOG(LL_INFO, "closing");
        uv_close((uv_handle_t*)tty_in, NULL);
    } else {
        WMQ_LOG(LL_INFO, "%zu: %.*s", nread, nread, buf->base);
    }
}

int main(int argc, char** argv) {
    int rc;
    uv_loop_t loop = {0};
    uv_tty_t tty = {0};

    debug_set_level(LL_DETAIL, WMQ_LOG_OPTION_USE_ODS | WMQ_LOG_OPTION_USE_STDERR | WMQ_LOG_OPTION_SHOW_TIME | WMQ_LOG_OPTION_SHOW_PID | WMQ_LOG_OPTION_SHOW_TID);
    WMQ_LOG(LL_INFO, "starting, sizeof(long unsigned int):%zu sizeof(int):%zu", sizeof(long unsigned int), sizeof(int));

    rc = uv_loop_init(&loop);
    WMQ_CHECK_ERROR_AND_RETURN_RESULT(rc, "uv_loop_init");

    //_open_osfhandle()
    rc = uv_tty_init(&loop, &tty, 0, 1);
    WMQ_CHECK_ERROR_AND_RETURN_RESULT(rc, "uv_tty_init");

    rc = uv_read_start((uv_stream_t*)&tty, on_alloc_buffer, tty_read);
    WMQ_CHECK_ERROR_AND_RETURN_RESULT(rc, "uv_tty_init");

    rc = uv_run(&loop, UV_RUN_DEFAULT);
    WMQ_CHECK_ERROR_AND_RETURN_RESULT(rc, "uv_run");

    rc = uv_loop_close(&loop);
    WMQ_CHECK_ERROR_AND_RETURN_RESULT(rc, "uv_loop_close");

    return 0;
}
