#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <json/json.h>
#include <libxml/xmlstring.h>
#include "iview.h"

/* Sample JSON structure:
[{
    "a":"3094182",
    "b":"Scrapheap Challenge Series 8",
    "e":"scrapheap challenge lifestyle abc2 reality recent last-chance",
    "f":[{"a":"694500","f":"2010-12-28 19:30:00","g":"2011-01-11 19:30:00"},
        {"a":"694499","f":"2010-12-27 19:30:00","g":"2011-01-10 19:30:00"},
        {"a":"691271","f":"2010-12-23 19:30:00","g":"2011-01-06 19:30:00"},
        {"a":"691263","f":"2010-12-22 19:30:00","g":"2011-01-05 19:30:00"},
        {"a":"691261","f":"2010-12-21 19:30:00","g":"2011-01-04 19:30:00"},
        {"a":"691257","f":"2010-12-20 19:30:00","g":"2011-01-03 19:30:00"},
        {"a":"689994","f":"2010-12-17 19:30:00","g":"2010-12-31 19:30:00"},
        {"a":"689987","f":"2010-12-16 19:30:00","g":"2010-12-30 19:30:00"},
        {"a":"689983","f":"2010-12-15 19:30:00","g":"2010-12-29 19:30:00"}]
}]
*/

int iv_parse_index(const char *buf, struct iv_series **index_ptr) {
    json_object *json_index, *json_element, *json_series;
    json_index = json_tokener_parse(buf);
    const int index_len = json_object_array_length(json_index);
    *index_ptr =
        (struct iv_series *)malloc(index_len * sizeof(struct iv_series));
    if(!*index_ptr) {
        return -IV_ENOMEM;
    }
    int i;
    for(i=0; i<index_len; i++) {
        json_element = json_object_array_get_idx(json_index, i);
        json_object *json_id = json_object_object_get(json_element, "a");
        (*index_ptr)[i].id = json_object_get_int(json_id);
        json_series = json_object_object_get(json_element, "b");
        (*index_ptr)[i].title =
            BAD_CAST(strdup(json_object_to_json_string(json_series)));
    }
    array_list_free(json_object_get_array(json_index));
    return index_len;
}
