#define _GNU_SOURCE
#include <stdio.h>
#undef _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <libxml/xmlstring.h>
#include "iview.h"
#include "internal.h"

ssize_t iv_get_index(struct iv_config *config, char **buf_ptr) {
    char *api_query;
    // FIXME: GNU magic sprintf, not portable.
    if(-1 == asprintf(&api_query, "%s%s%s", config->api,
                '?' == config->api[xmlStrlen(config->api)-1] ? "" : "?",
                "seriesIndex")) {
        return -IV_ENOMEM;
    }
    int result = iv_get_http_buffer(api_query, buf_ptr);
    free(api_query);
    return result;
}
