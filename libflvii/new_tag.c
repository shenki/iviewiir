#include <errno.h>
#include <stdlib.h>
#include "flvii.h"
#include "internal.h"

int flvii_new_tag(struct flvii_tag **tag) {
    *tag = calloc(1, sizeof(struct flvii_tag));
    return (*tag) ? 0 : -errno;
}
