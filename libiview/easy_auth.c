#include "iview.h"
#include "internal.h"

int iv_easy_auth(const struct iv_config *config, struct iv_auth **auth) {
    char *auth_buf;
    const ssize_t auth_buf_len = iv_get_auth(config, &auth_buf);
    if(0 > auth_buf_len) {
        return auth_buf_len;
    }
    const int result = iv_parse_auth(auth_buf, auth_buf_len, auth);
    iv_destroy_http_buffer(auth_buf);
    return result;
}
