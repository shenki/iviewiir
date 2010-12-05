#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "iview.h"
#include "internal.h"

ssize_t iv_get_index(struct iv_config *config, char **buf_ptr) {
    /* must strdup so ne_uri_free() can be used */
    config->api.query = strdup("seriesIndex");
    return iv_get_xml_buffer(&(config->api), buf_ptr);
}
