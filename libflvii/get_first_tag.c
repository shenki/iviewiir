#include "flvii.h"
#include "internal.h"

int flvii_get_first_tag(struct flvii_ctx *ctx, struct flvii_tag *tag) {
    if(!(UNKNOWN == ctx->state || FLV == ctx->state)) {
        return -FLVII_EMALFORMED;
    }
    int return_val = flvii_extract_tag(ctx, ctx->first_offset, tag);
    if(FLVII_OK == return_val && tag->prev_tag_size) {
        ctx->state = MALFORMED_FLV;
        return_val = -FLVII_EMALFORMED;
    }
    return return_val;
}
