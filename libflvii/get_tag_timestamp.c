#include "flvii.h"
#include "internal.h"

uint32_t flvii_get_tag_timestamp(const struct flvii_tag *tag) {
    return tag->timestamp;
}
