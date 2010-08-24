#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <neon/ne_uri.h>
#include "iviewiir.h"
#include "libiview/iview.h"

void iviewiir_configure(struct iv_config *config) {
    char *config_buf;
    ne_uri config_uri;
    if(ne_uri_parse(IV_CONFIG_URI, &config_uri)) {
        printf("error: uri parsing failed on %s\n", IV_CONFIG_URI);
    }
    size_t config_buf_len = iv_get_xml_buffer(&config_uri, &config_buf);
    printf("%s\n", config_buf);
    int result = iv_parse_config(config, config_buf, config_buf_len);
    iv_destroy_xml_buffer(config_buf);
}

ssize_t iviewiir_index(struct iv_config *config, struct iv_series **index) {
    char *index_xml_buf;
    ssize_t index_buf_len = iv_get_index(config, &index_xml_buf);
    printf("index:\n%s\n", index_xml_buf);
    ssize_t index_len = iv_parse_index(index_xml_buf, index_buf_len, index);
    iv_destroy_xml_buffer(index_xml_buf);
    return index_len;
}

void iviewiir_series(struct iv_config *config, struct iv_series *series) {
    char *series_buf;
    ssize_t len =
        iv_get_series_items(config, IV_SERIES_URI, series, &series_buf);
    printf("series:\n%s\n", series_buf);
    struct iv_item *items;
    const ssize_t items_len = iv_parse_series_items(series_buf, len, &items);
    int i;
    printf("items_len = %zd\n", items_len);
    for(i=0; i<items_len; i++) {
        printf("items[%d].title: %s\n"
               "items[%d].url: %s\n"
               "items[%d].description: %s\n"
               "items[%d].thumbnail: %s\n"
               "items[%d].date: %s\n"
               "items[%d].rating: %s\n"
               "items[%d].link: %s\n"
               "items[%d].home: %s\n\n",
               i, items[i].title,
               i, items[i].url,
               i, items[i].description,
               i, items[i].thumbnail,
               i, items[i].date,
               i, items[i].rating,
               i, items[i].link,
               i, items[i].home);
    }
    iv_destroy_series_items(items);
    iv_destroy_xml_buffer(series_buf);
}

int main(int argc, char **argv) {
    struct iv_series *index;
    struct iv_config config;
    iviewiir_configure(&config);
    iviewiir_index(&config, &index);
    iviewiir_series(&config, index);
    iv_destroy_index(index);
    return 0;
}
