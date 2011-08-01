#include <librtmp/amf.h>
#include <stdbool.h>
#include <string.h>

#include "flvii.h"
#include "internal.h"

int flvii_find_last_keyframe(struct flvii_ctx *ctx, struct flvii_tag *tag) {
    if(!(UNKNOWN == ctx->state || FLV == ctx->state)) {
        return -FLVII_EMALFORMED;
    }
    memset(tag, 0, sizeof(struct flvii_tag));
    struct flvii_tag _current, *current=&_current, _prev, *prev=&_prev;
    int result = flvii_get_last_tag(ctx, prev);
    if(0 > result) {
        return result;
    }
    bool is_videoframe, is_audioframe, is_keyframe;
    do {
        FLVII_FLIP(current, prev);
        memset(prev, 0, sizeof(struct flvii_tag));
        is_videoframe = ((AV == ctx->type || VIDEO == ctx->type)
                && FLVII_TAG_VIDEO == current->tag_type);
        is_audioframe =
            (AUDIO == ctx->type && FLVII_TAG_AUDIO == current->tag_type);
        is_keyframe = (FLVII_VIDEO_FRAME_TYPE(current->payload_type)
                != FLVII_VIDEO_KEYFRAME);
        result = flvii_get_prev_tag(ctx, current, prev);
        if(0 > result) {
            return result;
        }
    } while(!(is_keyframe || is_videoframe || is_audioframe));
    memcpy(tag, current, sizeof(struct flvii_tag));
    return FLVII_OK;
}
