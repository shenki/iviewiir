#define _GNU_SOURCE
#include <stdio.h>
#undef _GNU_SOURCE
#include "iview.h"
#include "internal.h"

ssize_t iv_get_categories(const struct iv_config *config, char **buf_ptr) {
    char *url;
    if(-1 == asprintf(&url, "%s/%s", IV_IVIEW_URI,
                (const char *)config->categories)) {
        return -1;
    }
    const ssize_t result = iv_get_http_buffer(url, buf_ptr);
    free(url);
    return result;
}
