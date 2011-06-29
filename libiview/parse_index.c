#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <json/json.h>
#include "iview.h"
#include "internal.h"

static int cmpseries(const void *l, const void *r) {
    const char *lt = ((struct iv_series *)l)->title;
    const char *rt = ((struct iv_series *)r)->title;
    return strcmp(lt, rt);
}

int iv_parse_index(const char *buf, struct iv_series **index_ptr) {
    json_object *json_index, *json_element, *json_series, *json_keywords;
    json_index = json_tokener_parse(buf);
    const int index_len = json_object_array_length(json_index);
    *index_ptr =
        (struct iv_series *)malloc(index_len * sizeof(struct iv_series));
    if(!*index_ptr) { return -(errno); }
    int i;
    for(i=0; i<index_len; i++) {
        json_element = json_object_array_get_idx(json_index, i);
        json_object *json_id =
            json_object_object_get(json_element, JSON_SERIES_ID);
        (*index_ptr)[i].id = json_object_get_int(json_id);
        json_series = json_object_object_get(json_element, JSON_SERIES_NAME);
        const char *title = json_object_to_json_string(json_series);
        if(-1 == strtrim((char **)&(*index_ptr)[i].title, title, "\"")) {
            (*index_ptr)[i].title = NULL;
        }
        json_keywords =
            json_object_object_get(json_element, JSON_SERIES_KEYWORDS);
        const char *keywords = json_object_to_json_string(json_keywords);
        if(-1 == strtrim((char **)&(*index_ptr)[i].keywords, keywords, "\"")) {
            (*index_ptr)[i].keywords = NULL;
        }
    }
    array_list_free(json_object_get_array(json_index));
    qsort(*index_ptr, index_len, sizeof(struct iv_series), cmpseries);
    free(json_index);
    return index_len;
}

#ifdef LIBIVIEW_TEST
#include "test/CuTest.h"

static const char *index_buf =
"[{\"a\":\"3094182\",\"b\":\"Scrapheap Challenge Series 8\",\"e\":\"scrapheap challenge lifestyle abc2 reality recent last-chance\",\"f\":[{\"a\":\"694500\",\"f\":\"2010-12-28 19:30:00\",\"g\":\"2011-01-11 19:30:00\"},{\"a\":\"694499\",\"f\":\"2010-12-27 19:30:00\",\"g\":\"2011-01-10 19:30:00\"},{\"a\":\"691271\",\"f\":\"2010-12-23 19:30:00\",\"g\":\"2011-01-06 19:30:00\"}]}]";

void test_iv_parse_index(CuTest *tc) {
    struct iv_series *series;
    const int index_len = iv_parse_index(index_buf, &series);
    CuAssertIntEquals(tc, 1, index_len);
    CuAssertIntEquals(tc, 3094182, series[0].id);
    CuAssertPtrNotNull(tc, series[0].title);
    CuAssertStrEquals(tc, "Scrapheap Challenge Series 8", series[0].title);
    CuAssertPtrNotNull(tc, series[0].keywords);
    CuAssertStrEquals(tc,
            "scrapheap challenge lifestyle abc2 reality recent last-chance",
            series[0].keywords);
    iv_destroy_index(series, index_len);
}

CuSuite *iv_parse_index_get_cusuite() {
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_iv_parse_index);
    return suite;
}
#endif /* LIBIVIEW_TEST */
