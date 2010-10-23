#include <neon/ne_uri.h>
#include <stdlib.h>
#include <errno.h>

#ifndef LIBIVIEW_H
#define LIBIVIEW_H

#define IV_MIN(a,b) (a <= b ? a : b)
#define IV_MAX(a,b) (a >= b ? a : b)

#define IV_CONFIG_URI "http://www.abc.net.au/iview/xml/config.xml?r=359"
#define IV_SERIES_URI "http://www.abc.net.au/iview/api/series_mrss.htm"
#define IV_AUTH_URI "http://www2b.abc.net.au/iViewHandshaker/services/iviewhandshaker.asmx/isp"
#define IV_AKAMAI_PREFIX "/flash/playback/_definst_/"
#define IV_SWF_HASH "96cc76f1d5385fb5cda6e2ce5c73323a399043d0bb6c687edd807e5c73c42b37"
#define IV_SWF_SIZE "2122"
#define IV_SWF_URL "http://www.abc.net.au/iview/images/iview.jpg"

/* Return values */
#define IV_OK 0
#define IV_EURIPARSE 1
#define IV_EREQUEST 2
#define IV_ESAXPARSE 3
#define IV_ENOMEM ENOMEM

struct iv_config {
    ne_uri api;
    ne_uri auth;
    char *tray;
    char *categories;
    char *classifications;
    char *captions;
    int captions_offset;
    unsigned short live_streaming;
    ne_uri server_streaming; //rtmp uri
    char *server_fallback;
    char *highlights;
    char *home;
    char *geo;
    char *time;
    char *feedback_url;
};

struct iv_series {
    int id;
    const char *title;
};

struct iv_item {
    int id;
    char *title;
    char *url;
    char *description;
    char *thumbnail;
    char *date;
    char *rating;
    char *link; /* link to play on iView site */
    char *home; /* Program website */
};

struct iv_auth {
    ne_uri server;
    char *prefix;
    char *token;
    short free;
};

#if __STDC_VERSION__ == 199901L
#define INLINE inline
#elif __GNUC__
#define INLINE extern inline
#else
#define INLINE
#endif

ssize_t iv_get_xml_buffer(ne_uri *uri, char **buf_ptr);
#define iv_destroy_xml_buffer(buf) free(buf)
int iv_parse_config(struct iv_config *config, const char *buf, size_t len);
void iv_destroy_config(struct iv_config *config);
ssize_t iv_get_index(struct iv_config *config, char **buf_ptr);
ssize_t iv_parse_index(const char *buf, size_t len,
        struct iv_series **index_ptr);
void iv_destroy_index(struct iv_series *index, size_t len);
ssize_t iv_get_series_items(struct iv_config *config, char *uri, struct iv_series *series,
        char **buf_ptr);
ssize_t iv_parse_series_items(char *buf, size_t len, struct iv_item **items);
void iv_destroy_series_items(struct iv_item *items, size_t items_len);
int iv_parse_auth(const struct iv_config *config, const char *buf, size_t len,
        struct iv_auth *auth);
void iv_destroy_auth(struct iv_auth *auth);
char *iv_generate_video_uri(const struct iv_auth *auth, const struct iv_item *item);
#define iv_destroy_video_uri(uri) free(uri)
int iv_fetch_video(const struct iv_auth *auth, const struct iv_item *item, const char *outpath);

#endif /* LIBIVIEW_H */
