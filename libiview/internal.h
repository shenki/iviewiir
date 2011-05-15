#ifndef IV_INTERNAL_H
#define IV_INTERNAL_H

#include <stdlib.h>
#include <sys/types.h>
#include <libxml/xmlstring.h>

#define IV_SERIES_URI "http://www.abc.net.au/iview/api/series_mrss.htm"
#define IV_AKAMAI_PREFIX "/flash/playback/_definst_/"
#define IV_SWF_HASH "96cc76f1d5385fb5cda6e2ce5c73323a399043d0bb6c687edd807e5c73c42b37"
#define IV_SWF_SIZE "2122"
#define IV_SWF_URL "http://www.abc.net.au/iview/images/iview.jpg"

struct iv_config {
    xmlChar *api;
    xmlChar *auth;
    xmlChar *tray;
    xmlChar *categories;
    xmlChar *classifications;
    xmlChar *captions;
    int captions_offset;
    unsigned short live_streaming;
    xmlChar *server_streaming; //rtmp uri
    xmlChar *server_fallback;
    xmlChar *highlights;
    xmlChar *home;
    xmlChar *geo;
    xmlChar *time;
    xmlChar *feedback_url;
};

struct iv_auth {
    xmlChar *server;
    xmlChar *prefix;
    xmlChar *token;
    short free;
};

/* strtrim
 *
 * Creates a copy of the provided string placed in |out| with |chars| stripped
 * off the start and end of the provided string.
 *
 * @out: The container for the resulting string
 * @str: The string to trim
 * @chars: The chars to strip off the provided string
 *
 * @return The length of the string held in |out|
 */
ssize_t strtrim(char **out, const char *str, const char *chars);

/* strntrim
 *
 * Creates a copy of the provided string placed in |out| with |chars| stripped
 * off the start and end of the provided string.
 *
 * @out: The container for the resulting string
 * @str: The string to trim
 * @str_len: The length of the string to trim
 * @chars: The chars to strip off the provided string
 *
 * @return The length of the string held in |out|
 */
ssize_t strntrim(char **out, const char *str, size_t str_len, const char *chars);

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

/*
 * Currently unused inlining macros to work around the conflict between GNU89
 * inlines and C99 (which are virtually opposed in implementation).
 */
#if __STDC_VERSION__ == 199901L
#define INLINE inline
#elif __GNUC__
#define INLINE extern inline
#else
#define INLINE
#endif

#endif /* IV_INTERNAL_H */
