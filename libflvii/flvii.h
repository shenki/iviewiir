#ifndef LIBFLVII
#define LIBFLVII

/* flvii.h
 *
 * libflvii is a library for parsing the FLV container format, developed by
 * Adobe Systems[1]. It's original purpose was to aid resumption of downloads
 * from the ABC iView service, in an attempt to avoid rtmpdump code.
 *
 * [1] http://download.macromedia.com/f4v/video_file_format_spec_v10_1.pdf
 *
 * Example use of libflvii:

#include <stdio.h>
#include "flvii.h"

int main(void) {
    struct flvii_ctx *ctx;
    struct flvii_tag *lkf;
    int result = 0, ret_val = 0;
    uint32_t timestamp;
    result = flvii_new_ctx("test.flv", &ctx);
    if(FLVII_OK != result) {
        puts("Failed to create context!");
        return -1;
    }
    result = flvii_is_flv(ctx);
    if(1 != result) {
        printf("test.flv is not a valid flv: %d\n", result);
        ret_val = -1;
        goto ctx_cleanup;
    }
    puts("It's an FLV!");
    result = flvii_new_tag(&lkf);
    if(FLVII_OK != result) {
        ret_val = -1;
        goto ctx_cleanup;
    }
    result = flvii_find_last_keyframe(ctx, lkf);
    if(FLVII_OK != result) {
        printf("Failed to find last keyframe: %d\n", result);
        ret_val = -1;
        goto tag_cleanup;
    }
    puts("Found last keyframe!");
    timestamp = flvii_get_tag_timestamp(lkf);
    printf("Can resume from %dms\n", timestamp);
tag_cleanup:
    flvii_destroy_tag(lkf);
ctx_cleanup:
    flvii_destroy_ctx(ctx);
    return ret_val;
}

 */

#include <stdint.h>
#include <stddef.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Return codes
#define FLVII_OK (0)
#define FLVII_EMALFORMED (1000)
#define FLVII_ESHORTREAD (1001)
#define FLVII_ENOMETADATA (1002)
#define FLVII_EBADARG (1003)

// FLV header macros
#define FLVII_OFFSET (0x09)
#define FLVII_STREAM_TYPE(val) (0x05 & val)
#define FLVII_STREAM_VIDEO (0x01)
#define FLVII_STREAM_AUDIO (0x04)
#define FLVII_STREAM_AV (0x05)

// Tag header macros
#define FLVII_TAG_HEADER_SIZE (11)
#define FLVII_TAG_TYPE(val) (0x1B & val)
#define FLVII_TAG_AUDIO (0x08)
#define FLVII_TAG_VIDEO (0x09)
#define FLVII_TAG_META (0x12)

// Audio tag metadata macros
#define FLVII_AUDIO_TYPE(val) (0x01 & val)
#define FLVII_AUDIO_TYPE_MONO (0x00)
#define FLVII_AUDIO_TYPE_STEREO (0x01)

#define FLVII_AUDIO_SIZE(val) ((0x02 & val) >> 1)
#define FLVII_AUDIO_SIZE_8BIT (0x00)
#define FLVII_AUDIO_SIZE_16BIT (0x02)

#define FLVII_AUDIO_RATE(val) ((0x0c & val) >> 2)
#define FLVII_AUDIO_RATE_5_5KHZ (0x00)
#define FLVII_AUDIO_RATE_11KHZ (0x01)
#define FLVII_AUDIO_RATE_22KHZ (0x02)
#define FLVII_AUDIO_RATE_44KHZ (0x03)

#define FLVII_AUDIO_FORMAT(val) ((0xf0 & val) >> 4)
#define FLVII_AUDIO_FORMAT_UNCOMPRESSED (0x00)
#define FLVII_AUDIO_FORMAT_ADPCM (0x01)
#define FLVII_AUDIO_FORMAT_MP3 (0x02)
#define FLVII_AUDIO_FORMAT_NELLY8KM (0x03)
#define FLVII_AUDIO_FORMAT_NELLY (0x04)

// Video tag metadata macros
#define FLVII_VIDEO_CODEC(val) (0x0f & val)
#define FLVII_VIDEO_CODEC_H263 (0x02)
#define FLVII_VIDEO_CODEC_SCREENVIDEO (0x03)
#define FLVII_VIDEO_CODEC_VP6 (0x04)

#define FLVII_VIDEO_FRAME_TYPE(val) ((0xf0 & val) >> 4)
#define FLVII_VIDEO_KEYFRAME (0x01)
#define FLVII_VIDEO_INTERFRAME (0x02)
#define FLVII_VIDEO_DINTERFRAME (0x03)

/* flvii_ctx
 *
 * A context structure for operations on an FLV file. Most functions in
 * libflvii require an instance of this struct to be passed in. The fields
 * present in the struct are for internal use only, thus it is a partial
 * declaration.
 */
struct flvii_ctx;

/* flvii_tag
 *
 * Represents a single tag in the FLV tag stream. Tag data can be queried with
 * various helper functions, for example the timestamp or content of the tag.
 * The internals of the tag struct are currently determined by the FLV
 * specification and are viewed as an implementation detail; thus here it is a
 * partial declaration.
 */
struct flvii_tag;

/* flvii_new_ctx
 *
 * Creates a context structure for operations on FLV files. It is the
 * responsibility of the caller to free the memory associated with the context
 * once finished with it by calling flvii_destroy_ctx.
 *
 * Once a context has been created the following operation (if not
 * flvii_destroy_ctx()) should be a call to flvii_is_flv() to determine whether
 * the file is a valid FLV.
 *
 * @path   The path to the FLV file to inspect.
 * @ctx    A container for a context pointer. The context instances itself
 *         is created internally.
 *
 * @return FLVII_OK if a context struct was successfully created, less than zero
 *         otherwise. If the return value is not FLVII_OK then the pointer in
 *         the ctx parameter is invalid.
 */
int flvii_new_ctx(const char *path, struct flvii_ctx **ctx);

/* flvii_destroy_ctx
 *
 * Cleans up and frees memory associated with a flvii_ctx instance. Once the
 * context has been destroyed any further operations on the context are
 * invalid.
 */
void flvii_destroy_ctx(struct flvii_ctx *ctx);

/* flvii_is_flv
 *
 * Determines whether a file pointed at by ctx is a valid FLV file.
 * flvii_is_flv() should be called immediately after creating a flvii_ctx and
 * before any further operations are performed.
 *
 * If the context does not represent a valid FLV file then the following call
 * should be to flvii_destroy_ctx() as nothing further can be done. Otherwise,
 * tag iterator functions and data extraction helpers can be used to navigate
 * the FLV file as required.
 *
 * @ctx    The context to inspect.
 *
 * @return 1 if the path describes an FLV file, 0 if it is not. In the case of
 *         an error a value less than zero is returned.
 */
int flvii_is_flv(struct flvii_ctx *ctx);

/* flvii_new_tag
 *
 * Once the context has been verified as valid, stream tags are navigated by
 * the provided iterator functions. flvii_new_tag() provides the means to
 * create tag instances to be passed to the iterators/helpers.
 *
 * It is the responsibility of the caller to destroy tag objects (using
 * flvii_destroy_tag()) once finished with them.
 *
 * @tag    A container for the flvii_tag instance.
 *
 * @return FLVII_OK if the flvii_tag instance was successfully allocated, less
 *         than zero otherwise.
 */
int flvii_new_tag(struct flvii_tag **tag);

/* flvii_destroy_tag
 *
 * Cleans up and frees memory associated with a flvii_tag instance. Once the
 * instance has been destroyed any further operations on it are invalid.
 */
void flvii_destroy_tag(struct flvii_tag *tag);

/* flvii_get_tag_timestamp
 *
 * @return The tag time in milliseconds relative to the first frame.
 */
uint32_t flvii_get_tag_timestamp(const struct flvii_tag *tag);

/* flvii_find_last_keyframe
 *
 * Useful when attempting to find a resume point for a download. The timestamp
 * of the keyframe to be resumed from is required by librtmp's API; this
 * function can be used in combination with flvii_get_tag_timestamp() to
 * determine this value.
 *
 * @ctx    The FLV context to inspect.
 * @tag    A flvii_tag instance which will be populated with the metadata for the
 *         last keyframe.
 *
 * @return FLVII_OK on success, less than zero on failure.
 */
int flvii_find_last_keyframe(struct flvii_ctx *ctx, struct flvii_tag *tag);

/* flvii_get_first_tag
 *
 * Navigates to and extracts the metadata of the first tag in the stream (after
 * the stream header), populating the provided flvii_tag with the information.
 *
 * @ctx    The context to retrieve the first tag from. The context should be
 *         created with flvii_new_ctx() and tested with flvii_is_flv() prior to
 *         passing the instance to flvii_get_first_tag().
 * @tag    The flvii_tag instance to be populated.
 *
 * @return FLVII_OK if the first tag was found and extracted, less than zero otherwise.
 */
int flvii_get_first_tag(struct flvii_ctx *ctx, struct flvii_tag *tag);

/* flvii_get_last_tag
 *
 * A robust method for determining the last good tag in the stream. A
 * well-formed FLV's last 4 bytes should record the total size of the previous
 * tag, however corruption or truncation due to early exit may prevent this
 * from being written. flvii_get_last_tag() will use the O(1) last tag
 * determination method if the FLV is well-formed, falling back to an O(n)
 * search through the file if the previous tag size was determined to be
 * incorrect.
 *
 * @ctx    The context to retrieve the last tag from. The context should be
 *         created with flvii_new_ctx() and tested with flvii_is_flv() prior to
 *         passing the instance to flvii_get_last_tag().
 * @tag    The flvii_tag instance to be populated.
 *
 * @return FLVII_OK if the last tag was found and extracted, less than zero otherwise.
 */
int flvii_get_last_tag(struct flvii_ctx *ctx, struct flvii_tag *tag);

/* flvii_get_next_tag
 *
 * @ctx    The context to retrieve the last tag from. The context should be
 *         created with flvii_new_ctx() and tested with flvii_is_flv() prior to
 *         passing the instance to flvii_get_next_tag().
 *
 * @current
 *         The tag prior to the tag required. current should be populated by
 *         one of the flvii_get_*_tag() methods, particularly
 *         flvii_get_first_tag() or flvii_get_last_tag() if this is the first
 *         iterative call.
 *
 * @next   The flvii_tag instance to populate with the metadata from the tag
 *         after the one pointed at by current.
 *
 * @return FLVII_OK if the metadata is extracted successfully, less than zero
 *         otherwise.
 */
int flvii_get_next_tag(struct flvii_ctx *ctx, const struct flvii_tag *current,
        struct flvii_tag *next);

/* flvii_get_prev_tag
 *
 * @ctx    The context to retrieve the last tag from. The context should be
 *         created with flvii_new_ctx() and tested with flvii_is_flv() prior to
 *         passing the instance to flvii_get_next_tag().
 *
 * @current
 *         The tag prior to the tag required. current should be populated by
 *         one of the flvii_get_*_tag() methods, particularly
 *         flvii_get_first_tag() or flvii_get_last_tag() if this is the first
 *         iterative call.
 *
 * @prev   The flvii_tag instance to populate with the metadata from the tag
 *         before the one pointed at by current.
 *
 * @return FLVII_OK if the metadata is extracted successfully, less than zero
 *         otherwise.
 */
int flvii_get_prev_tag(struct flvii_ctx *ctx, const struct flvii_tag *current,
        struct flvii_tag *prev);

/* flvii_is_last_tag
 *
 * A means to stop iteration in a readable fashion. Note that in its current
 * form the first call will trigger a call to flvii_get_last_tag(), which may
 * have unwanted consequences if the FLV file is not well-formed.
 *
 * @ctx    The context to retrieve the last tag from. The context should be
 *         created with flvii_new_ctx() and tested with flvii_is_flv() prior to
 *         passing the instance to flvii_get_next_tag().
 *
 * @tag    The tag to test against the known last tag.
 *
 * @return 1 if tag is the last tag in the stream, 0 otherwise
 */
int flvii_is_last_tag(struct flvii_ctx *ctx, struct flvii_tag *tag);

/* flvii_extract_tag_body
 *
 * Fetches the content of the tag. The content type of the body can be
 * determined by using one of the numerous macros defined at the top of this
 * header. It is the responsibility of the caller to free the buffer provided
 * by a successful call to flvii_extract_tag_body().
 *
 * @ctx    The context to read the body from.
 *
 * @tag    The tag whose body to read.
 *
 * @buf    A container for the buffer holding the body. The buffer is allocated
 *         internally and assigned to buf. Whether the pointer held in buf is
 *         valid is determined by the return value.
 *
 * @return FLVII_OK if the tag body was extracted successfully, less than zero
 *         otherwise. If the return value is less than zero then any value held
 *         in the buf parameter is invalid.
 */
ssize_t flvii_extract_tag_body(const struct flvii_ctx *ctx,
        const struct flvii_tag *tag, char **buf);

/* flvii_destroy_tag_body
 *
 * Cleans up and frees memory associated with a tag body buffer. Once the
 * instance has been destroyed any further operations on it are invalid.
 */
void flvii_destroy_tag_body(char *buf);

#ifdef __cplusplus
};
#endif

#endif /* LIBFLVII */
