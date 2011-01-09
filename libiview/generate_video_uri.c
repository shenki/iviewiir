#define _GNU_SOURCE
#include <stdio.h>
#undef _GNU_SOURCE
#include <string.h>
#include "iview.h"

ssize_t iv_generate_video_uri(const struct iv_auth *auth,
        const struct iv_item *item, char **uri) {
    int return_val = 0;
    char *rtmp_uri = NULL;
    char *playpath = NULL;
    const int playpath_len = asprintf(&playpath, "%s%s",
            (char *)(auth->free ? BAD_CAST("") : auth->prefix), item->url);
    if(-1 == playpath_len) {
        return -IV_ENOMEM;
    }
    // Trim the extension for RTMP URI generation
    uri[strlen(playpath)-4] = '\0';
    const char *prefix =
        !xmlStrcmp(BAD_CAST("flv"), &item->url[xmlStrlen(item->url)-3]) ?
            "" : "mp4:";
    const int rtmp_uri_len = asprintf(&rtmp_uri,
            "%s?auth=%s playpath=%s%s swfUrl=%s swfVfy=1 swfAge=0",
            item->url, auth->token, prefix, playpath, IV_SWF_URL);
    return_val = rtmp_uri_len;
    if(-1 == rtmp_uri_len) {
        return_val = -IV_ENOMEM;
    }
    free(playpath);
    *uri = rtmp_uri;
    return rtmp_uri_len;
}
