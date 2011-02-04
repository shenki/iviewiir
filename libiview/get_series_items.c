#define _GNU_SOURCE
#include <stdio.h>
#undef _GNU_SOURCE
#include <string.h>
#include "iview.h"

ssize_t iv_get_series_items(struct iv_config *config IV_UNUSED, const char *uri,
        struct iv_series *series, char **buf_ptr) {
    char *series_uri;
    // FIXME: GNU magic sprintf, not portable.
    if(-1 == asprintf(&series_uri, "%s?id=%d", uri, series->id)) {
        *buf_ptr = NULL;
        return -IV_ENOMEM;
    }
    int result = iv_get_xml_buffer(series_uri, buf_ptr);
    free(series_uri);
    return result;
}
