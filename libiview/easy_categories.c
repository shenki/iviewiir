#include "iview.h"
#include "internal.h"

int iv_easy_categories(const struct iv_config *config,
        struct iv_category **categories_ptr) {
    char *categories_buf;
    const ssize_t categories_buf_len =
        iv_get_categories(config, &categories_buf);
    if(0 > categories_buf_len) {
        return categories_buf_len;
    }
    const int result =
        iv_parse_categories(categories_buf, categories_buf_len, categories_ptr);
    iv_destroy_http_buffer(categories_buf);
    return result;
}
