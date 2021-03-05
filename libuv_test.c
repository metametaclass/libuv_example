#include <stdio.h>

#define USING_UV_SHARED

#include <uv.h>

#include "debug.h"
#include "wmq_error.h"

int main(int argc, char **argv) {
    int rc;
    uv_loop_t loop = {0};
    uv_tty_t tty = {0};

    debug_set_level(LL_DETAIL, WMQ_LOG_OPTION_USE_ODS | WMQ_LOG_OPTION_USE_STDERR | WMQ_LOG_OPTION_SHOW_TIME);
    WMQ_LOG(LL_INFO, "starting");

    rc = uv_loop_init(&loop);
    WMQ_CHECK_ERROR_AND_RETURN_RESULT(rc, "uv_loop_init");

    rc = uv_tty_init(&loop, &tty, 0, 0);
    WMQ_CHECK_ERROR_AND_RETURN_RESULT(rc, "uv_tty_init");

    rc = uv_run(&loop, UV_RUN_DEFAULT);
    WMQ_CHECK_ERROR_AND_RETURN_RESULT(rc, "uv_run");

    rc = uv_loop_close(&loop);
    WMQ_CHECK_ERROR_AND_RETURN_RESULT(rc, "uv_loop_close");

    return 0;
}
