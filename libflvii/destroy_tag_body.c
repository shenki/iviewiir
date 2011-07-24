#include <stdlib.h>

void flvii_destroy_tag_body(char *buf) {
    if(buf) { free(buf); }
}
