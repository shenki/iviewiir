#ifndef FLVII_INTERNAL_H
#define FLVII_INTERNAL_H

#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

#define FLVII_HAS_SIGNATURE(buf) (\
        4 <= sizeof(buf)/sizeof(buf[0]) \
        && 'F' == buf[0] \
        && 'L' == buf[1] \
        && 'V' == buf[2] \
        && 0x01 == buf[3] \
        )

#if defined(DEBUG)
#define FLVII_DEBUG(format, ...) \
        fprintf(stderr, "DEBUG (%s:%d): " format, \
                __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define FLVII_DEBUG(...) do {} while (0)
#endif

enum FLVII_STATE { UNKNOWN, FLV, NOT_FLV, MALFORMED_FLV };
enum FLVII_TYPE { AUDIO, VIDEO, AV };

struct flvii_ctx {
    FILE *file;
    off_t file_size;
    enum FLVII_STATE state;
    enum FLVII_TYPE type;
    off_t first_offset;
    off_t last_offset;
    char *metadata;
    size_t metadata_size;
    double duration_sec;
};

int flvii_extract_tag(struct flvii_ctx *ctx, off_t file_offset,
        struct flvii_tag *tag);

#endif /* FLVII_INTERNAL_H */
