#ifndef IV_INTERNAL_H
#define IV_INTERNAL_H

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

struct iv_auth {
    ne_uri server;
    char *prefix;
    char *token;
    short free;
};

#endif /* IV_INTERNAL_H */
