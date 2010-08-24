#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "iview.h"

ssize_t iv_get_index(struct iv_config *config, char **buf_ptr) {
    config->api.query = "seriesIndex";
    return iv_get_xml_buffer(&(config->api), buf_ptr);
}
