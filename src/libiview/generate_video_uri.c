#include <stdio.h>

#include "iview.h"
#include <string.h>

char *iv_generate_video_uri(const struct iv_auth *auth, const struct iv_item *item) {
    if(auth->free) {
        return item->url;
    }
    char *prefixed_video_uri = malloc(strlen(item->url)+strlen(auth->prefix)+1);
    if(NULL == prefixed_video_uri) {
        return NULL;
    }
    prefixed_video_uri = strcpy(prefixed_video_uri, auth->prefix);
    prefixed_video_uri = strcat(prefixed_video_uri, item->url);
    if(strcmp("flv", &prefixed_video_uri[strlen(prefixed_video_uri)-3])) {
        prefixed_video_uri[strlen(prefixed_video_uri)-3] = '\0';
    }
    return prefixed_video_uri;
}
