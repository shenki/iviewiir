#include <stdio.h>
#include <string.h>
#include <neon/ne_string.h>
#include <neon/ne_alloc.h>
#include "iview.h"
#include "internal.h"

char *iv_generate_video_uri(const struct iv_auth *auth, const struct iv_item *item) {
    char *rtmp_uri;
    char *uri;
    if(auth->free) {
        uri = ne_concat(item->url, NULL);
    } else {
        uri = ne_concat(auth->prefix, item->url, NULL);
    }
    uri[strlen(uri)-4] = '\0';
    if(!strcmp("flv", &item->url[strlen(item->url)-3])) {
        rtmp_uri = ne_concat(auth->server.scheme, "://", auth->server.host,
                auth->server.path, "?auth=", auth->token,
                " playpath=", uri,
                " swfUrl=", IV_SWF_URL,
                " swfVfy=1",
                " swfAge=0", NULL);
    } else {
        rtmp_uri = ne_concat(auth->server.scheme, "://", auth->server.host,
                auth->server.path, "?auth=", auth->token,
                " playpath=", "mp4:", uri,
                " swfUrl=", IV_SWF_URL,
                " swfVfy=1",
                " swfAge=0", NULL);
    }
    ne_free(uri);
    return rtmp_uri;
}
