#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "flvii.h"
#include "internal.h"

int flvii_new_ctx(const int fd, struct flvii_ctx **ctx) {
    struct flvii_ctx *_ctx;
    *ctx = NULL;
    _ctx = calloc(1, sizeof(struct flvii_ctx));
    if(!_ctx) { return -errno; }
    int dfd = dup(fd);
    _ctx->file = fdopen(dfd, "rb");
    if(!_ctx->file) {
        int _errno = errno;
        free(_ctx);
        return -_errno;
    }
    struct stat dfd_stat;
    if(-1 == fstat(dfd, &dfd_stat)) {
        int _errno = errno;
        free(_ctx);
        return -_errno;
    }
    _ctx->file_size = dfd_stat.st_size;
    _ctx->state = UNKNOWN;
    *ctx = _ctx;
    return 0;
}
