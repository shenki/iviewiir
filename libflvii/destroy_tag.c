#include <stdlib.h>
#include "flvii.h"
#include "internal.h"

void flvii_destroy_tag(struct flvii_tag *tag) {
    if(tag) { free(tag); }
}
