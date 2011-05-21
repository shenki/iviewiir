#include "iview.h"
#include "internal.h"

ssize_t iv_get_auth(const struct iv_config *config, char **buf_ptr) {
    return iv_get_http_buffer((char *)config->auth, buf_ptr);
}
