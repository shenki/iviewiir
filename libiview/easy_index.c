#include "iview.h"
#include "internal.h"

int iv_easy_index(struct iv_config *config, struct iv_series **index_ptr) {
    char *index_xml_buf = NULL;
    const ssize_t index_buf_len = iv_get_index(config, &index_xml_buf);
    if(0 >= index_buf_len) {
        return -1;
    }
    const int index_len = iv_parse_index(index_xml_buf, index_ptr);
    iv_destroy_xml_buffer(index_xml_buf);
    return index_len;
}
