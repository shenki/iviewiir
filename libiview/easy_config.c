#include "iview.h"

int iv_easy_config(struct iv_config **config) {
    char *config_buf = NULL;
    const ssize_t config_buf_len =
        iv_get_http_buffer(IV_CONFIG_URI, &config_buf);
    if(0 >= config_buf_len) {
        return -1;
    }
    const int result = iv_parse_config(config_buf, config_buf_len, config);
    iv_destroy_http_buffer(config_buf);
    return result;
}
