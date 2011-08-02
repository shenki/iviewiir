#include <assert.h>
#include <librtmp/amf.h>
#include <librtmp/rtmp.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "flvii.h"
#include "internal.h"

// Const version of AVC
#define AVCC(str) {(char *)str, sizeof(str)-1}
const AVal av_onMetaData = AVCC("onMetaData");
const AVal av_duration = AVCC("duration");

/* @return: 0 if decoding was successful, le if it was not. */
static int extract_metadata(struct flvii_ctx *ctx,
        const char *buf, size_t len) {
    // Extract metadata
    AMFObject meta_obj;
    FLVII_DEBUG("Attempting AMF decode...\n");
    int nRes = AMF_Decode(&meta_obj, buf, len, FALSE);
    if (0 > nRes) {
        FLVII_DEBUG("Decoding unsuccessful :(\n");
        return 1;
    }
    // Test if we have a metadata packet
    AVal meta_string;
    FLVII_DEBUG("Extracting onMetaData property\n");
    AMFProp_GetString(AMF_GetProp(&meta_obj, NULL, 0), &meta_string);
    if (!AVMATCH(&meta_string, &av_onMetaData)) {
        FLVII_DEBUG("Match failed :(\n");
        return 1;
    }
    FLVII_DEBUG("Matched onMetaData tag, extracting data...\n");
    // Copy the packet data
    if (ctx->metadata) {
        free(ctx->metadata);
    }
    ctx->metadata = (char *) malloc(len);
    memcpy(ctx->metadata, buf, len);
    ctx->metadata_size = len;
    // Determine the duration for future reference
    ctx->duration_sec = -1;
    {
        AMFObjectProperty prop;
        if (RTMP_FindFirstMatchingProperty(&meta_obj, &av_duration, &prop))
        {
            ctx->duration_sec = AMFProp_GetNumber(&prop);
            FLVII_DEBUG("Duration: %lf\n", ctx->duration_sec);
        }
    }
    return 0;
}

/* @return: 0 if the metadata is found, 1 if it is not, less than zero if some
 * error occurred.
 */
static int find_metadata(struct flvii_ctx *ctx) {
    bool found_meta = false;
    // Initialise data structures
    struct flvii_tag _current, *current=&_current, _next, *next=&_next;
    // Find META AMF packet pair
    int result = flvii_get_first_tag(ctx, current);
    if(0 > result) {
        FLVII_DEBUG("Couldn't find first tag: %d\n", result);
        return result;
    }
    while(!(found_meta || flvii_is_last_tag(ctx, current))) {
        if(FLVII_TAG_META == FLVII_TAG_TYPE(current->tag_type)) {
            FLVII_DEBUG("Found META tag at offset 0x%lx\n",
                    current->file_offset);
            char *buf;
            const ssize_t size = flvii_extract_tag_body(ctx, current, &buf);
            if(0 > size) { return size; } // unrecoverable
            found_meta = (0 == extract_metadata(ctx, buf, size));
            flvii_destroy_tag_body(buf);
        }
        if(!found_meta) {
            result = flvii_get_next_tag(ctx, current, next);
            if(0 > result) {
                FLVII_DEBUG("Failed to find next tag: %d\n", result);
                return result;
            }
            FLVII_FLIP(current, next);
            memset(next, 0, sizeof(struct flvii_tag));
        }
    }
    return found_meta ? 0 : -FLVII_ENOMETADATA;
}

int flvii_is_flv(struct flvii_ctx *ctx) {
    int return_val = 0;
    // Check the size
    fseek(ctx->file, 0, SEEK_END);
    ctx->file_size = ftello(ctx->file);
    fseek(ctx->file, 0, SEEK_SET);
    if(0 >= ctx->file_size) {
        return 0;
    }
    // Check the headers
    {
#define HEADER_SIZE (9)
        char data[HEADER_SIZE] = { 0 };
        if(sizeof(data) != fread(&data, 1, sizeof(data), ctx->file)) {
            FLVII_DEBUG("short read\n");
            ctx->state = NOT_FLV;
            return -FLVII_ESHORTREAD;
        }
        if(!FLVII_HAS_SIGNATURE(data)) {
            FLVII_DEBUG("No FLV signature found\n");
            ctx->state = NOT_FLV;
            return 0;
        }
        if(FLVII_STREAM_AV != FLVII_STREAM_TYPE(data[4])) {
            FLVII_DEBUG("Stream type is not A/V\n");
            ctx->state = MALFORMED_FLV;
            return 0;
        }
        ctx->type = AV;
        ctx->first_offset = AMF_DecodeInt32(&data[5]);
        assert(HEADER_SIZE == ctx->first_offset);
#undef HEADER_SIZE
    }
    const int result = find_metadata(ctx);
    if(0 > result) {
        FLVII_DEBUG("Failed to find metadata: %d\n", result);
        ctx->state = MALFORMED_FLV;
        return result;
    }
    return_val = 1;
    ctx->state = FLV;
    return return_val;
}
