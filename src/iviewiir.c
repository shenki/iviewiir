#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <neon/ne_uri.h>
#include "iviewiir.h"
#include "libiview/iview.h"

#define CONFIG_FILE "iview.config"
#define INDEX_FILE  "iview.index"
static char *cache_dir;

#if defined(DEBUG)
#define debug(format, ...) \
        fprintf(stderr, "DEBUG (L%d): " format, __LINE__, ##__VA_ARGS__)
#else
#define debug(...) do {} while (0)
#endif

#define error(format, ...) fprintf(stderr, "ERROR: " format, ##__VA_ARGS__)

void dump_buff(void *buf, size_t buf_len, char *fname) {
    /* Dump config to disk. */
    char *fpath = malloc(strlen(cache_dir) + strlen(fname) + 1);
    strcpy(fpath, cache_dir);
    if(fpath[strlen(fpath)-1] != '/') {
        strcat(fpath, "/");
    }
    strncat(fpath, fname, 14);
    FILE *fs = fopen(fpath, "w");
    if(fs==NULL) {
        perror(fpath);
    } else {
        fwrite(buf, sizeof(char), buf_len, fs);
    }
    fwrite("\n", sizeof(char), 1, fs);
    free(fpath);
    fclose(fs);
}

void iviewiir_configure(struct iv_config *config) {
    char *config_buf;
    ne_uri config_uri;
    if(ne_uri_parse(IV_CONFIG_URI, &config_uri)) {
        printf("error: uri parsing failed on %s\n", IV_CONFIG_URI);
    }
    size_t config_buf_len = iv_get_xml_buffer(&config_uri, &config_buf);
    ne_uri_free(&config_uri);
    printf("%s\n", config_buf);
    int result = iv_parse_config(config, config_buf, config_buf_len);
    dump_buff(config_buf, config_buf_len, CONFIG_FILE);
    iv_destroy_xml_buffer(config_buf);
}

ssize_t iviewiir_index(struct iv_config *config, struct iv_series **index) {
    char *index_xml_buf;
    ssize_t index_buf_len = iv_get_index(config, &index_xml_buf);
    printf("index:\n%s\n", index_xml_buf);
    ssize_t index_len = iv_parse_index(index_xml_buf, index_buf_len, index);
    dump_buff(index_xml_buf, index_buf_len, INDEX_FILE);
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
    cache_dir = xdg_user_dir_lookup_with_fallback("CACHE", "/tmp");
    iviewiir_configure(&config);
    ssize_t index_len = iviewiir_index(&config, &index);
    iviewiir_series(&config, index);
    iv_destroy_index(index, index_len);
    iv_destroy_config(&config);
    free(cache_dir);
    return 0;
}
