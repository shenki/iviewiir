#include <stdlib.h>
#include "iview.h"

void iv_destroy_http_buffer(char *buf) {
    if(NULL != buf) { free(buf); }
}
