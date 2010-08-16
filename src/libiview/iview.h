#include <neon/ne_uri.h>

#ifndef LIBIVIEW_H
#define LIBIVIEW_H

#define IV_MIN(a,b) (a <= b ? a : b)
#define IV_MAX(a,b) (a >= b ? a : b)

#define IV_CONFIG_URI "http://www.abc.net.au/iview/xml/config.xml?r=359"

struct iv_config {
    ne_uri api;
    char *auth;
    char *tray;
    char *categories;
    char *classifications;
    char *captions;
    int captions_offset;
    unsigned short live_streaming;
    ne_uri server_streaming;
    char *server_fallback;
    char *highlights;
    char *home;
    char *geo;
    char *time;
    char *feedback_url;
};

struct iv_index {
    int id;
    const char *title;
};

#if __STDC_VERSION__ == 199901L
#define INLINE inline
#elif __GNUC__
#define INLINE extern inline
#else
#define INLINE
#endif

ssize_t iv_get_xml_buffer(ne_uri *uri, char **buf_ptr);
INLINE void iv_destroy_xml_buffer(char *buf) {
    free(buf);
}
int iv_parse_config(struct iv_config *config, const char *buf, size_t len);
ssize_t iv_get_index(struct iv_config *config, char **buf_ptr);
ssize_t iv_parse_index(const char *buf, size_t len,
        struct iv_index **index_ptr);
INLINE void iv_destroy_index(struct iv_index *index) {
    free(index);
}

#endif /* LIBIVIEW_H */
