#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <json/json.h>
#include "iview.h"

ssize_t iv_parse_index(const char *buf, size_t len,
        struct iv_series **index_ptr) {
    json_object *json_index, *json_element, *json_series;
    json_index = json_tokener_parse(buf);
    const size_t index_len = json_object_array_length(json_index);
    *index_ptr = (struct iv_series *)malloc(index_len * sizeof(struct iv_series));
    if(!*index_ptr) {
        return -IV_ENOMEM;
    }
    int i;
    for(i=0; i<index_len; i++) {
        json_element = json_object_array_get_idx(json_index, i);
        json_object *json_id = json_object_object_get(json_element, "a");
        (*index_ptr)[i].id = json_object_get_int(json_id);
        json_series = json_object_object_get(json_element, "b");
        (*index_ptr)[i].title = strdup(json_object_to_json_string(json_series));
    }
    array_list_free(json_object_get_array(json_index));
    return index_len;
}
