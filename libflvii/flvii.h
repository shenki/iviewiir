#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

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

struct flvii_ctx;

struct flvii_tag;

/* flvii_new_ctx
 *
 * @return FLVII_OK if a context struct was successfully created, less than zero
 * otherwise.
 */
int flvii_new_ctx(const char *path, struct flvii_ctx **ctx);

void flvii_destroy_ctx(struct flvii_ctx *ctx);

/* flvii_is_flv
 *
 * @return 1 if the path describes an FLV file, 0 if it is not. In the case of
 * an error a value less than zero is returned.
 */
int flvii_is_flv(struct flvii_ctx *ctx);

int flvii_new_tag(struct flvii_tag **tag);

void flvii_destroy_tag(struct flvii_tag *tag);

uint32_t flvii_get_tag_timestamp(const struct flvii_tag *tag);

int flvii_find_last_keyframe(struct flvii_ctx *ctx, struct flvii_tag *tag);

int flvii_get_first_tag(struct flvii_ctx *ctx, struct flvii_tag *tag);

int flvii_get_last_tag(struct flvii_ctx *ctx, struct flvii_tag *tag);

int flvii_get_next_tag(struct flvii_ctx *ctx, const struct flvii_tag *current,
        struct flvii_tag *next);

int flvii_get_prev_tag(struct flvii_ctx *ctx, const struct flvii_tag *current,
        struct flvii_tag *prev);

int flvii_is_last_tag(struct flvii_ctx *ctx, struct flvii_tag *tag);
