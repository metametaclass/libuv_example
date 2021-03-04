#include <stdio.h>

#define USING_UV_SHARED

#include <uv.h>

int main(int argc, char **argv) {
    int rc;
    uv_loop_t loop = {0};

    rc = uv_loop_init(&loop);
    if (rc) {
        printf("uv_loop_init: error %d", rc);
        return rc;
    }
    rc = uv_run(&loop, UV_RUN_DEFAULT);
    if (rc) {
        printf("uv_loop_init: error %d", rc);
        return rc;
    }
    rc = uv_loop_close(&loop);
    if (rc) {
        printf("uv_loop_init: error %d", rc);
        return rc;
    }
    return 0;
}
