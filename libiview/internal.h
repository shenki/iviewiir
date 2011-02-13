#ifndef IV_INTERNAL_H
#define IV_INTERNAL_H

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

#endif /* IV_INTERNAL_H */
