#include <stdlib.h>
#include <errno.h>
#include <libxml/xmlstring.h>
#include <unistd.h>

#ifndef LIBIVIEW_H
#define LIBIVIEW_H

#if defined(DEBUG)
#define IV_DEBUG(format, ...) \
        fprintf(stderr, "DEBUG (%s:%d): " format, \
                __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define IV_DEBUG(...) do {} while (0)
#endif

/* Typesafe min/max macros from Linux: include/linux/kernel.h */
#define IV_MIN(x, y) ({        \
            typeof(x) _min1 = (x);      \
            typeof(y) _min2 = (y);      \
            (void) (&_min1 == &_min2);    \
            _min1 < _min2 ? _min1 : _min2; })

#define IV_MAX(x, y) ({        \
            typeof(x) _max1 = (x);      \
            typeof(y) _max2 = (y);      \
            (void) (&_max1 == &_max2);    \
            _max1 > _max2 ? _max1 : _max2; })

#define IV_XML_ATTR_NAME(attrs) (attrs[1])
#define IV_XML_ATTR_VALUE(attrs) (attrs[3])

/*
 * IV_UNUSED - a parameter is unused
 *
 * Some compilers (eg. gcc with -W or -Wunused) warn about unused
 * function parameters.  This suppresses such warnings and indicates
 * to the reader that it's deliberate.
 */
#define IV_UNUSED __attribute__((unused))


#define IV_CONFIG_URI "http://www.abc.net.au/iview/xml/config.xml?r=374"

/* Return values */
#define IV_OK 0
#define IV_EURIPARSE 1
#define IV_EREQUEST 2
#define IV_ESAXPARSE 3
#define IV_EXML 4
#define IV_ENOMEM ENOMEM

struct iv_config;

struct iv_series {
    int id;
    const xmlChar *title;
};

struct iv_item {
    int id;
    xmlChar *title;
    xmlChar *url;
    xmlChar *description;
    xmlChar *thumbnail;
    xmlChar *date;
    xmlChar *rating;
    xmlChar *link; /* link to play on iView site */
    xmlChar *home; /* Program website */
};

struct iv_auth;

#if __STDC_VERSION__ == 199901L
#define INLINE inline
#elif __GNUC__
#define INLINE extern inline
#else
#define INLINE
#endif

ssize_t iv_get_xml_buffer(const char *uri, char **buf_ptr);
#define iv_destroy_xml_buffer(buf) free(buf)
int iv_get_config(const char *buf, size_t len, struct iv_config **config);
void iv_destroy_config(struct iv_config *config);
ssize_t iv_get_index(struct iv_config *config, char **buf_ptr);
int iv_parse_index(const char *buf, struct iv_series **index_ptr);
void iv_destroy_index(struct iv_series *index, int len);
ssize_t iv_get_series_items(struct iv_config *config,
        struct iv_series *series, char **buf_ptr);
ssize_t iv_parse_series_items(char *buf, size_t len, struct iv_item **items);
void iv_destroy_series_items(struct iv_item *items, int items_len);
int iv_get_auth(const struct iv_config *config, struct iv_auth **auth);
void iv_destroy_auth(struct iv_auth *auth);
#define iv_destroy_video_uri(uri) free(uri)
int iv_fetch_video(const struct iv_auth *auth, const struct iv_item *item, const char *outpath);

#endif /* LIBIVIEW_H */
