#include "flvii.h"
#include "internal.h"

int flvii_is_last_tag(struct flvii_ctx *ctx, struct flvii_tag *tag) {
    if(!(UNKNOWN == ctx->state || FLV == ctx->state)) {
        return -FLVII_EMALFORMED;
    }
    if(!ctx->last_offset) {
        struct flvii_tag last;
        const int result = flvii_get_last_tag(ctx, &last);
        if(FLVII_OK != result) { return result; }
        ctx->last_offset = last.file_offset;
    }
    return ctx->last_offset == tag->file_offset;
}
