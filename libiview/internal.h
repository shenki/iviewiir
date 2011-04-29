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

#endif /* IV_INTERNAL_H */
