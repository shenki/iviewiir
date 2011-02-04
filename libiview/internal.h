#ifndef IV_INTERNAL_H
#define IV_INTERNAL_H

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

#endif /* IV_INTERNAL_H */
