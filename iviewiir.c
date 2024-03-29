/*
 * Copyright 2011 Joel Stanley <joel@jms.id.au>
 * Copyright 2011 Andrew Jeffery <andrew@aj.id.au>
 *
 * This file is part of iviewiir.
 *
 * iviewiir is free software: you can redistribute it and/or modify it under
 * the terms of version 3 the GNU General Public License as published by the
 * Free Software Foundation.
 *
 * iviewiir is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with iviewiir.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <libgen.h>
#include <libxml/xmlstring.h>

#include "ccan/opt/opt.h"

#include "iviewiir.h"
#include "libiview/iview.h"

#define CONFIG_FILE "iview.config"
#define INDEX_FILE  "iview.index"
#define CACHE_VALID_SECONDS (30*60)
static char *cache_dir;
static int use_cache = 1;

#if defined(DEBUG)
#define debug(format, ...) \
        fprintf(stderr, "DEBUG (%s:%d): " format, \
                __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define debug(...) do {} while (0)
#endif

#define error(format, ...) \
        fprintf(stderr, "ERROR (%s:%d): " format, \
                __FILE__, __LINE__, ##__VA_ARGS__)

char* join_path(const char *dirname, const char *fname) {
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
size_t load_buf(char **buf, const char *fname) {
    if (use_cache)
        return 0;
    struct stat fstats;
    int sz = 0;
    char* fpath = join_path(cache_dir, fname);
    debug("Loading from cache: %s\n", fpath);
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

int dump_buf(const void const *buf, size_t buf_len, const char *fname) {
    /* Dump config to disk. */
    char *fpath = join_path(cache_dir, fname);
    FILE *fs = fopen(fpath, "w");
    if(fs==NULL) {
        perror(fpath);
        return -1;
    }
    if(buf_len > fwrite(buf, sizeof(char), buf_len, fs)) {
        error("Failed write to %s\n", fname);
        goto cleanup;
    }
    if(1 > fwrite("\n", sizeof(char), 1, fs)) {
        error("Failed write to %s\n", fname);
    }
cleanup:
    free(fpath);
    if(fclose(fs)) {
        perror("fclose");
        return -1;
    }
    return 0;
}

struct iv_config *iviewiir_configure() {
    struct iv_config *config;
    char *config_buf = NULL;
    ssize_t config_buf_len = load_buf(&config_buf, CONFIG_FILE);
    if(config_buf_len == 0) {
        /* Cache was stale or did not exist, so re-fetch. */
        debug("Fetching configuration\n");
        config_buf_len = iv_get_http_buffer(IV_CONFIG_URI, &config_buf);
        if(0 >= config_buf_len) {
            fprintf(stderr, "error retrieving config xml\n");
            return NULL;
        }
        if(-1 == dump_buf(config_buf, config_buf_len, CONFIG_FILE)) {
            config = NULL;
            goto config_cleanup;
        }
    }
    debug("%s\n", config_buf);
    if(IV_OK != iv_parse_config(config_buf, config_buf_len, &config)) {
        return NULL;
    }
config_cleanup:
    iv_destroy_http_buffer(config_buf);
    return config;
}

int iviewiir_index(struct iv_config *config, struct iv_series **index) {
    char *index_xml_buf = NULL;
    ssize_t index_buf_len = load_buf(&index_xml_buf, INDEX_FILE);
    if(index_buf_len == 0) {
        debug("Didn't load index from cache, fetching it\n");
        index_buf_len = iv_get_index(config, &index_xml_buf);
        dump_buf(index_xml_buf, index_buf_len, INDEX_FILE);
    }
    debug("index length %zd:\n%s\n", index_buf_len, index_xml_buf);
    int index_len = iv_parse_index(index_xml_buf, index);
    iv_destroy_http_buffer(index_xml_buf);
    return index_len;
}

void list_all(struct iv_config *config, struct iv_series *index,
        int index_len) {
    struct iv_episode *items;
    int i;
    for(i=0; i<index_len; i++) {
        int items_len = iv_easy_series(config, &index[i], &items);
        if(0 > items_len) {
            error("iv_easy_series returned %d\n", items_len);
            continue;
        }
        if(0 == items_len) {
            continue;
        }
        printf("%s\n", index[i].title);
        int j;
        for(j=0; j<items_len; j++) {
            printf("%d:%d - %s\n", index[i].id, items[j].id,
                    items[j].title);
        }
        printf("\n");
        iv_destroy_series(items, items_len);
    }
}

int list_items(struct iv_config *config, struct iv_series *index,
        const int index_len, const unsigned int sid) {
    struct iv_episode *items;
    // Fetch episode lists for the SID
    debug("sid: %d\n", sid);
    int i;
    for(i=0; i<index_len; i++) {
        if(sid == index[i].id) {
            break;
        }
    }
    debug("series index: %d\n", i);
    ssize_t items_len = iv_easy_series(config, &index[i], &items);
    if(1 > items_len) {
        fprintf(stderr, "No items in series.\n");
        return -1;
    }
    for(i=0; i<items_len; i++) {
        printf("%d:%d - %s\n",
                sid, items[i].id, items[i].title);
    }
    iv_destroy_series(items, items_len);
    return 0;
}

static int print_percentage(const struct iv_progress *progress,
        void *user_data) {
    if(0.0 == progress->percentage ||
            ((*(double *)user_data) + 1) <= progress->percentage) {
        const char *type = NULL;
        double dl_value = 0;
        if(progress->count > 1024*1024) {
            type = "MB";
            dl_value = progress->count / (1024.0 * 1024.0);
        } else if(progress->count > 1024) {
            type = "KB";
            dl_value = progress->count / 1024.0;
        } else {
            type = "B";
            dl_value = progress->count * 1.0;
        }
        printf("\rProgress: %.01f%%%8.01f%s",
                progress->percentage, dl_value, type);
        fflush(stdout);
    }
    if(((*(double *)user_data) + 1) <= progress->percentage) {
        (*(double *)user_data)++;
    }
    return 0;
}

int download_item(struct iv_config *config, struct iv_series *index,
        const unsigned int index_len, const unsigned int sid,
        const unsigned int pid) {
    int return_val = 0;
    struct iv_episode *items;
    // Fetch episode lists for the SID
    debug("sid: %d\n", sid);
    int series_index;
    if(-1 == (series_index = iv_find_series(sid, index, index_len, NULL))) {
        printf("Failed to find series");
        return -1;
    }
    ssize_t items_len =
        iv_easy_series(config, &index[series_index], &items);
    if(1 > items_len) {
        fprintf(stderr, "No items in series, exiting\n");
        return -1;
    }
    int item_index;
    if(-1 == (item_index = iv_find_episode(pid, items, items_len, NULL))) {
        printf("Failed to find specified episode\n");
        return_val = -1;
        goto done;
    }
    printf("%s : %s\n",
        items[item_index].title, basename((char *)items[item_index].url));
    char const *path = basename((char *)items[item_index].url);
    struct stat st;
    if (0 == stat(path, &st)) {
        printf("File \"%s\" exists. Aborting download.\n", path);
        return_val = -1;
        goto done;
    }
    const int fd = creat(path, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    double progress = 0;
    if(IV_OK == iv_easy_fetch_episode_async(config, &(items[item_index]), fd,
            &print_percentage, &progress)) {
        struct stat stat_buf;
        stat(path, &stat_buf);
        double size_mb = stat_buf.st_size / (1024.0 * 1024.0);
        printf("\rComplete: %s - %.01fMB\n", path, size_mb);
    } else {
        printf("\rDownload failed :(\n");
        return_val = -1;
    }
    close(fd);
done:
    iv_destroy_series(items, items_len);
    return return_val;
}

int main(int argc, char **argv) {
    static bool show_series = false, show_all = false, use_cache = true;
    static int i_sid = 0;
    static char usage_str[] = "[SID[:PID]]";
    static struct opt_table opts[] = {
        OPT_WITH_ARG("--items-list|-i", opt_set_intval, NULL, &i_sid,
                "List episodes in a series. Requires a SID as a parameter."),
        OPT_WITHOUT_ARG("--series-list|-s", opt_set_bool, &show_series,
                "List the series available. The first element is the SID."),
        OPT_WITHOUT_ARG("--all|-a", opt_set_bool, &show_all,
                "List all items in all non-empty series."),
        OPT_WITHOUT_ARG("--force|-f", opt_set_invbool, &use_cache,
                "Force bypass the cached metadata."),
        OPT_WITHOUT_ARG("--help|-h", opt_usage_and_exit,
                usage_str, "Show this message."),
        OPT_ENDTABLE
    };
    opt_register_table(opts, NULL);
    if(!opt_parse(&argc, argv, opt_log_stderr)) {
        /* opt_parse will print an error to stderr. */
        exit(1);
    }
    if (!show_all && !show_series && !i_sid && (argc == 1)) {
        opt_usage_and_exit(usage_str);
    }

    struct iv_series *index;
    struct iv_config *config;
    int return_val = 0;
    cache_dir = xdg_user_dir_lookup_with_fallback("CACHE", "/tmp");
    if(NULL == (config = iviewiir_configure())) {
        fprintf(stderr, "Couldn't configure iviewiir, exiting\n");
        return 1;
    }
    int index_len = iviewiir_index(config, &index);
    if(0 >= index_len) {
        fprintf(stderr, "No items in index, exiting\n");
        return_val = 1;
        goto config_cleanup;
    }
    /* Check if they want everything listed */
    if(show_all) {
        list_all(config, index, index_len);
        return_val = 0;
        goto index_cleanup;
    }
    /* Check if they wanted a series list. */
    if(show_series) {
        int i;
        for(i=0; i<index_len; i++) {
            /* Heuristic to trim out empty series. */
            if((int)9e6 < index[i].id) {
                continue;
            }
            printf("%d - %s\n", index[i].id, index[i].title);
        }
        return_val = 0;
        goto index_cleanup;
    }
    /* Check if they want an episode list. */
    if(i_sid) {
        return_val = list_items(config, index, index_len, i_sid);
        goto index_cleanup;
    }
    /* If we've reached here and there are no arguments, print help message. */
    if (argc == 1) {
        opt_usage_and_exit(usage_str);
    }
    /* Otherwise, if they supplied a SID or SID:PID tuple, download the PID */
    int i = 1;
    while(i < argc) {
        if(NULL != strchr(argv[i], ':')) {
            // SID:PID
            const unsigned long sid = strtoul(strtok(argv[i], ":"), NULL, 10);
            const unsigned int pid = strtoul(strtok(NULL, ":"), NULL, 10);
            return_val += download_item(config, index, index_len, sid, pid);
        } else {
            // Check if it's a valid SID
            const unsigned int sid = strtoul(argv[i], NULL, 10);
            struct iv_episode *items;
            // Fetch episode lists for the SID
            debug("sid: %d\n", sid);
            int series_index;
            if(-1 == (series_index =
                        iv_find_series(sid, index, index_len, NULL))) {
                printf("No such series");
                return_val += 1;
                continue;
            }
            // Fetch items in series
            ssize_t items_len =
                iv_easy_series(config, &index[series_index], &items);
            if(1 > items_len) {
                printf("No items in series.\n");
                return_val += 1;
                continue;
            }
            for(i=0; i<items_len; i++) {
                return_val += download_item(config, index, index_len, sid,
                        items[i].id);
            }
            iv_destroy_series(items, items_len);
        }
        i++;
    }
index_cleanup:
    iv_destroy_index(index, index_len);
config_cleanup:
    iv_destroy_config(config);
    free(cache_dir);
    return return_val;
}
