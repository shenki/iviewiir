#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include "flvii.h"
#include "internal.h"

int flvii_new_ctx(const char *path, struct flvii_ctx **ctx) {
    struct flvii_ctx *_ctx;
    *ctx = NULL;
    _ctx = calloc(1, sizeof(struct flvii_ctx));
    if(!_ctx) { return -errno; }
    _ctx->path = path;
    _ctx->file = fopen(_ctx->path, "rb");
    if(!_ctx->file) {
        int _errno = errno;
        free(_ctx);
        return -_errno;
    }
    _ctx->state = UNKNOWN;
    *ctx = _ctx;
    return 0;
}
