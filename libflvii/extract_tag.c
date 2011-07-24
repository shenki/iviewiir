#include <librtmp/amf.h>
#include "flvii.h"
#include "internal.h"

int flvii_extract_tag(struct flvii_ctx *ctx, off_t file_offset,
        struct flvii_tag *tag) {
    if(!(UNKNOWN == ctx->state || FLV == ctx->state)) {
        return -FLVII_EMALFORMED;
    }
    if(!tag || ctx->file_size < file_offset) {
        return -FLVII_EBADARG;
    }
    int return_val = FLVII_OK;
    // Set offset
    tag->file_offset = file_offset;
    // Read tag data from file
    FLVII_DEBUG("Seeking to offset: 0x%lx\n", tag->file_offset);
    fseeko(ctx->file, tag->file_offset, SEEK_SET);
    char data[16] = { 0 };
    if(sizeof(data) != fread(data, 1, sizeof(data), ctx->file)) {
        FLVII_DEBUG("Short read :(\n");
        return -FLVII_ESHORTREAD;
    }
    // Set previous tag size
    tag->prev_tag_size = AMF_DecodeInt32(data);
    FLVII_DEBUG("tag->prev_tag_size: 0x%x\n", tag->prev_tag_size);
    // Set tag type
    tag->tag_type = data[4];
    switch(tag->tag_type) {
        case FLVII_TAG_VIDEO:
            FLVII_DEBUG("Found VIDEO tag\n");
            break;
        case FLVII_TAG_AUDIO:
            FLVII_DEBUG("Found AUDIO tag\n");
            break;
        case FLVII_TAG_META:
            FLVII_DEBUG("Found META tag\n");
            break;
        default:
            FLVII_DEBUG("Invalid tag type found: 0x%x\n", tag->tag_type);
            return -FLVII_EMALFORMED;
    }
    // Set body length
    tag->body_length = AMF_DecodeInt24(&data[5]);
    FLVII_DEBUG("tag->body_length: 0x%x\n", tag->body_length);
    // Set timestamp
    tag->timestamp = AMF_DecodeInt24(&data[8]);
    FLVII_DEBUG("tag->timestamp: 0x%x\n", tag->timestamp);
    tag->payload_type = data[15];
    FLVII_DEBUG("tag->payload_type: 0x%x\n", tag->payload_type);
    return return_val;
}
