#include "flvii.h"
#include "internal.h"

int flvii_get_next_tag(struct flvii_ctx *ctx, const struct flvii_tag *current,
        struct flvii_tag *next) {
    if(!(UNKNOWN == ctx->state || FLV == ctx->state)) {
        return -FLVII_EMALFORMED;
    }
    const off_t offset = current->file_offset
             + sizeof(current->prev_tag_size)
             + FLVII_TAG_HEADER_SIZE
             + current->body_length;
    return flvii_extract_tag(ctx, offset, next);
}
