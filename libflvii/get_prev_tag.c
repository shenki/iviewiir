#include "flvii.h"
#include "internal.h"

int flvii_get_prev_tag(struct flvii_ctx *ctx,
        const struct flvii_tag *current, struct flvii_tag *prev) {
    if(!(UNKNOWN == ctx->state || FLV == ctx->state)) {
        return -FLVII_EMALFORMED;
    }
    const off_t offset = current->file_offset
            - current->prev_tag_size
            - sizeof(current->prev_tag_size);
    return flvii_extract_tag(ctx, offset, prev);
}
