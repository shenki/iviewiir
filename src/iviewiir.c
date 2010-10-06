#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <getopt.h>
#include <neon/ne_uri.h>
#include <libgen.h>
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
    /* Two extra characters: one for the null, one for the path seperator. */
    char *fpath = malloc(strlen(dirname) + strlen(fname) + 2);
    strcpy(fpath, dirname);
    if(fpath[strlen(fpath)-1] != '/') {
        strncat(fpath, "/", 1);
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
    if (fs == NULL) {
      perror(fpath);
      goto end;
    }
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

int iviewiir_configure(struct iv_config *config) {
    char *config_buf = NULL;
    ne_uri config_uri;
    size_t config_buf_len = load_buf(&config_buf, CONFIG_FILE);
    if(config_buf_len == 0) {
        /* Cache was stale or did not exist, so re-fetch. */
        if(ne_uri_parse(IV_CONFIG_URI, &config_uri)) {
            error("uri parsing failed on %s\n", IV_CONFIG_URI);
            return -IV_EURIPARSE;
        }
        config_buf_len = iv_get_xml_buffer(&config_uri, &config_buf);
        ne_uri_free(&config_uri);
        if(0 >= config_buf_len) {
            error("error retrieving config xml\n");
            return config_buf_len;
        }
        dump_buf(config_buf, config_buf_len, CONFIG_FILE);
    }
    debug("%s\n", config_buf);
    int result = iv_parse_config(config, config_buf, config_buf_len);
    debug("iv_parse_config: %d\n", result);
    iv_destroy_xml_buffer(config_buf);
    return result;
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

ssize_t iviewiir_series(struct iv_config *config, struct iv_series *series,
        struct iv_item **items) {
    char *series_buf;
    const ssize_t len =
        iv_get_series_items(config, IV_SERIES_URI, series, &series_buf);
    if(0 >= len) {
        iv_destroy_xml_buffer(series_buf);
        return len;
    }
    debug("series:\n%s\n", series_buf);
    const ssize_t items_len = iv_parse_series_items(series_buf, len, items);
    if(0 > items_len) {
        iv_destroy_xml_buffer(series_buf);
        return items_len;
    }
    iv_destroy_xml_buffer(series_buf);
    return items_len;
}

void iviewiir_download(const struct iv_config *config, const struct iv_item *item) {
    char *auth_xml_buf;
    struct iv_auth auth;
    memset(&auth, 0, sizeof(auth));
    ne_uri auth_uri;
    if(ne_uri_parse(IV_AUTH_URI, &auth_uri)) {
        error("uri parsing failed on %s\n", IV_AUTH_URI);
    }
    ssize_t auth_buf_len = iv_get_xml_buffer(&auth_uri, &auth_xml_buf);
    debug("%s\n", auth_xml_buf);
    if(iv_parse_auth(config, auth_xml_buf, auth_buf_len, &auth)) {
        error("iv_parse_auth failed\n");
    }
    char *path = strdup(item->url);
    char *flvname = basename(path);
    iv_fetch_video(&auth, item, flvname);
    free(path);
    iv_destroy_auth(&auth);
    iv_destroy_xml_buffer(auth_xml_buf);
}

void usage(void) {
    printf("Usage: iviewiir [-ihs] [SID:PID]\n\n"
           "\t-i --items-list=SID: List episodes in a series. "
           "Requires a SID as a parameter. "
           "The first element on each output line is a SID:PID tuple\n"
           "\t-s --series-list: List the series available. "
           "The first element on each output line is the SID\n"
           "\t-h --help: Show this help\n\n"
           "Without any parameters a SID:PID tuple should be supplied, "
           "which will download the associated video\n");
}

int main(int argc, char **argv) {
    int return_val = 0;
    long bsopts = 0;
#define OPT_SERIES_LIST (1 << 1)
#define OPT_ITEMS_LIST (1 << 2)
#define OPT_HELP (1 << 3)
#define OPT_ALL (1 << 4)
#define OPT_h(opts) (opts & OPT_HELP)
#define OPT_s(opts) (opts & OPT_SERIES_LIST)
#define OPT_i(opts) (opts & OPT_ITEMS_LIST)
#define OPT_a(opts) (opts & OPT_ALL)
    char *opts = "ai:sh";
    int sid, pid;
    int lflag;
    struct option lopts[] = {
        {"items-list", 1, NULL, 'i'},
        {"series-list", 0, NULL, 's'},
        {"all", 0, NULL, 'a'},
        {"help", 0, NULL, 'h'},
        {0, 0, 0, 0}
    };
    int lindex;
    char opt;
    while(-1 != (opt = getopt_long(argc, argv, opts, lopts, &lindex))) {
        switch(opt) {
            case 'a':
                bsopts |= OPT_ALL;
                break;
            case 'i':
                bsopts |= OPT_ITEMS_LIST;
                sid = atoi(optarg);
                break;
            case 's':
                bsopts |= OPT_SERIES_LIST;
                break;
            case 'h':
                bsopts |= OPT_HELP;
                break;
        }
    }
    if(0 == bsopts) {
        if(NULL == argv[optind] || NULL == strchr(argv[optind], ':')) {
            error("please supply SID:PID parameter\n\n");
            usage();
            return 1;
        }
        char *sp_tuple = strdup(argv[optind]);
        sid = atoi(strtok(sp_tuple, ":"));
        pid = atoi(strtok(NULL, ":"));
    }
    if(OPT_h(bsopts)) {
        usage();
        return 0;
    }
    struct iv_series *index;
    struct iv_item *items;
    struct iv_config config;
    cache_dir = xdg_user_dir_lookup_with_fallback("CACHE", "/tmp");
    if(IV_OK != iviewiir_configure(&config)) {
        error("Couldn't configure iviewiir, exiting\n");
        return 1;
    }
    ssize_t index_len = iviewiir_index(&config, &index);
    if(0 == index_len) {
        error("No items in index, exiting\n");
        return_val = 1;
        goto config_cleanup;
    }
    // Check if they want everything listed
    int i;
    if(OPT_a(bsopts)) {
        for(i=0; i<index_len; i++) {
            ssize_t items_len = iviewiir_series(&config, &index[i], &items);
            if(0 > items_len) {
                error("iviewiir_series returned %zd\n", items_len);
                continue;
            }
            if(0 == items_len) {
                continue;
            }
            if(1 < items_len) {
                printf("%s\n", index[i].title);
                int j;
                for(j=1; j<items_len; j++) {
                    printf("%d:%d - %s\n", index[i].id, items[j].id, items[j].title);
                }
                printf("\n");
            }
            iv_destroy_series_items(items, items_len);
        }
        return_val = 0;
        goto index_cleanup;
    }
    // Check if they wanted a series list
    if(OPT_s(bsopts)) {
        int i;
        for(i=0; i<index_len; i++) {
            printf("%d : %s\n", index[i].id, index[i].title);
        }
        return_val = 0;
        goto index_cleanup;
    }
    // Fetch episode lists for the SID
    debug("sid: %d\n", sid);
    for(i=0; i<index_len; i++) {
        if(sid == index[i].id) {
            break;
        }
    }
    debug("series index: %d\n", i);
    ssize_t items_len = iviewiir_series(&config, &index[i], &items);
    if(1 > items_len) {
        printf("No items in series.\n");
        return_val = 1;
        goto index_cleanup;
    }
    // Check if they want an episode list
    if(OPT_i(bsopts)) {
        for(i=1; i<items_len; i++) {
            printf("%d:%d - %s\n",
                    sid, items[i].id, items[i].title);
        }
        return_val = 0;
        goto items_cleanup;
    }
    // Otherwise, if they supplied a SID:PID tuple, download the PID
    if(NULL != argv[optind] && NULL != strchr(argv[optind], ':')) {
        debug("series index: %d\n", i);
        ssize_t items_len = iviewiir_series(&config, &index[i], &items);
        if(1 > items_len) {
            error("No items in series, exiting\n");
            return_val = 1;
            goto index_cleanup;
        }
        for(i=0; i<items_len; i++) {
            if(pid == items[i].id) {
                break;
            }
        }
        debug("downloading %s\n", items[i].title);
        iviewiir_download(&config, &(items[i]));
        debug("download complete\n");
    }
items_cleanup:
    iv_destroy_series_items(items, items_len);
index_cleanup:
    iv_destroy_index(index, index_len);
config_cleanup:
    iv_destroy_config(&config);
    free(cache_dir);
    return return_val;
}
