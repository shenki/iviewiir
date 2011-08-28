#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "flvii.h"
#include "internal.h"

ssize_t flvii_get_metadata(struct flvii_ctx *ctx, char **buf) {
    *buf = malloc(ctx->metadata_size);
    if(NULL == *buf) {
        return -(errno);
    }
    memcpy(*buf, ctx->metadata, ctx->metadata_size);
    return ctx->metadata_size;
}
