#include <errno.h>
#include <librtmp/amf.h>
#include <string.h>
#include "flvii.h"
#include "internal.h"

static int find_last_tag(struct flvii_ctx *ctx, struct flvii_tag *tag) {
    struct flvii_tag _current, *current=&_current, _next, *next=&_next;
    int result = flvii_get_first_tag(ctx, next);
    if(0 > result) { return result; }
    do {
        FLVII_FLIP(current, next);
        result = flvii_get_next_tag(ctx, current, next);
    } while(FLVII_OK == result && ctx->file_size > current->file_offset);
    memcpy(tag, current, sizeof(struct flvii_tag));
    return FLVII_OK;
}

int flvii_get_last_tag(struct flvii_ctx *ctx, struct flvii_tag *tag) {
    if(!(UNKNOWN == ctx->state || FLV == ctx->state)) {
        return -FLVII_EMALFORMED;
    }
    if(ctx->last_offset) {
        return flvii_extract_tag(ctx, ctx->last_offset, tag);
    }
    char buffer[16];
    uint32_t prev_tag_size = 0;
    // Find the the last prevTagSize value
    fseeko(ctx->file, -sizeof(prev_tag_size), SEEK_END);
    int read = fread(buffer, 1, sizeof(prev_tag_size), ctx->file);
    if(sizeof(prev_tag_size) != read) {
        return -FLVII_ESHORTREAD;
    }
    prev_tag_size = AMF_DecodeInt32(buffer);
    FLVII_DEBUG("Final prevTagSize: 0x%x\n", prev_tag_size);
    if(0 == prev_tag_size) {
        return find_last_tag(ctx, tag);
    } else if(prev_tag_size > (ctx->file_size - sizeof(prev_tag_size) - 13)) {
        FLVII_DEBUG("prevTagSize (%d) is larger than file size (%ld)\n",
                prev_tag_size, ctx->file_size - sizeof(prev_tag_size) - 13);
        FLVII_DEBUG("Seeking forwards to find last frame\n");
        return find_last_tag(ctx, tag);
    }
    // Wind back to start of previous frame
    const int offset = ctx->file_size
        - prev_tag_size
        - 2 * sizeof(prev_tag_size);
    return flvii_extract_tag(ctx, offset, tag);
}
