#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <neon/ne_uri.h>
#include "iviewiir.h"
#include "libiview/iview.h"

#define CONFIG_FILE "iview.config"
#define INDEX_FILE  "iview.index"
#define CACHE_VALID_SECONDS (30*60)
static char *cache_dir;

#if defined(DEBUG)
#define debug(format, ...) \
        fprintf(stderr, "DEBUG (L%d): " format, __LINE__, ##__VA_ARGS__)
#else
#define debug(...) do {} while (0)
#endif

#define error(format, ...) fprintf(stderr, "ERROR: " format, ##__VA_ARGS__)

char* join_path(char *dirname, char *fname) {
    char *fpath = malloc(strlen(dirname) + strlen(fname) + 1);
    strcpy(fpath, dirname);
    if(fpath[strlen(fpath)-1] != '/') {
        strcat(fpath, "/");
    }
    strncat(fpath, fname, 14);
    return fpath;
}

/* Loads a file |fname| from disk into |buf| if the access time is within
 * CACHE_VALID_SECONDS.  |buf| is malloced here and the caller must free if
 * size is non-zero.
 * Return value is the size of file read, or zero if it was not read. */
size_t load_buf(char **buf, char *fname) {
    struct stat fstats;
    struct timeval curtime;
    size_t sz = 0;
    char* fpath = join_path(cache_dir, fname);
    if (stat(fpath, &fstats) == -1) {
        goto end;
    }
    if((time(NULL) - fstats.st_mtime) > CACHE_VALID_SECONDS) {
        goto end;
    }
    *buf = malloc(fstats.st_size);
    FILE *fs = fopen(fpath, "r");
    sz = fread(*buf, sizeof(char), fstats.st_size, fs);
    fclose(fs);
    if (sz == fstats.st_size) {
        sz = fstats.st_size;
    } else {
        sz = 0;
        free(*buf);
    }
end:
    free(fpath);
    return sz;
}

void dump_buf(void *buf, size_t buf_len, char *fname) {
    /* Dump config to disk. */
    char *fpath = join_path(cache_dir, fname);
    FILE *fs = fopen(fpath, "w");
    if(fs==NULL) {
        perror(fpath);
    } else {
        fwrite(buf, sizeof(char), buf_len, fs);
    }
    fwrite("\n", sizeof(char), 1, fs);
    fclose(fs);
}

void iviewiir_configure(struct iv_config *config) {
    char *config_buf = NULL;
    ne_uri config_uri;
    size_t config_buf_len = load_buf(&config_buf, CONFIG_FILE);
    if(config_buf_len == 0) {
        /* Cache was stale or did not exist, so re-fetch. */
        if(ne_uri_parse(IV_CONFIG_URI, &config_uri)) {
            error("uri parsing failed on %s\n", IV_CONFIG_URI);
        }
        config_buf_len = iv_get_xml_buffer(&config_uri, &config_buf);
        ne_uri_free(&config_uri);
        dump_buf(config_buf, config_buf_len, CONFIG_FILE);
    }
    debug("%s\n", config_buf);
    int result = iv_parse_config(config, config_buf, config_buf_len);
    iv_destroy_xml_buffer(config_buf);
}

ssize_t iviewiir_index(struct iv_config *config, struct iv_series **index) {
    char *index_xml_buf;
    ssize_t index_buf_len = load_buf(&index_xml_buf, INDEX_FILE);
    if(index_buf_len == 0) {
        index_buf_len = iv_get_index(config, &index_xml_buf);
        dump_buf(index_xml_buf, index_buf_len, INDEX_FILE);
    }
    debug("index:\n%s\n", index_xml_buf);
    ssize_t index_len = iv_parse_index(index_xml_buf, index_buf_len, index);
    iv_destroy_xml_buffer(index_xml_buf);
    return index_len;
}

void iviewiir_series(struct iv_config *config, struct iv_series *series) {
    char *series_buf;
    ssize_t len =
        iv_get_series_items(config, IV_SERIES_URI, series, &series_buf);
    debug("series:\n%s\n", series_buf);
    struct iv_item *items;
    const ssize_t items_len = iv_parse_series_items(series_buf, len, &items);
    int i;
    debug("items_len = %zd\n", items_len);
    for(i=0; i<items_len; i++) {
        debug("items[%d].title: %s\n"
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
