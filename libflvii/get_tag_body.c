#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "flvii.h"
#include "internal.h"

ssize_t flvii_extract_tag_body(const struct flvii_ctx *ctx,
        const struct flvii_tag *tag, char **buf) {
    if(!(UNKNOWN == ctx->state || FLV == ctx->state)) {
        return -FLVII_EMALFORMED;
    }
    FLVII_DEBUG("Extracting tag body (0x%x bytes)\n", tag->body_length);
    *buf = malloc(tag->body_length);
    if(!buf) {
        return -errno;
    }
    const off_t offset = tag->file_offset
        + FLVII_TAG_HEADER_SIZE
        + sizeof(tag->prev_tag_size);
    fseeko(ctx->file, offset, SEEK_SET);
    if(tag->body_length != fread(*buf, 1, tag->body_length, ctx->file)) {
        free(*buf);
        return -FLVII_ESHORTREAD;
    }
    FLVII_DEBUG("Extraction successful\n");
    return tag->body_length;
}
