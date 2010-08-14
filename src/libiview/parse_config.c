#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <neon/ne_xml.h>
#include "iview.h"

#define XML_CONFIG_STATE 1
#define XML_PARAM_STATE 2

static int accept_start_config(void *userdata, int parent, const char *nspace,
        const char *name, const char **attrs) {
    if(0 != strncmp("config", name, 6)) {
        return NE_XML_DECLINE;
    }
    return XML_CONFIG_STATE;
}

static int accept_cdata_config(void *userdata, int state, const char *cdata,
    size_t len) {
    return 0;
}

static int accept_end_config(void *userdata, int state, const char *nspace,
        const char *name) {
    return 0;
}

static int parse_param_attrs(struct iv_config *config,
        const char **attrs) {
    if(!strcmp("captions_offset", attrs[1])) {
        config->captions_offset = atoi(attrs[1]);
        return XML_PARAM_STATE;
    }
    if(!strcmp("live_streaming", attrs[1])) {
        config->live_streaming = 0 == strcmp("true", attrs[1]) ? 1 : 0;
        return XML_PARAM_STATE;
    }
    if(!strcmp("server_streaming", attrs[1])) {
        /* url should look like:
         * "rtmp://cp53909.edgefcs.net/ondemand"
         */
        char *tok, *dptr;
        config->server_streaming.url = strdup(attrs[3]);
        strtok_r(attrs[3], "/", &dptr); // ditch "rtmp:"
        config->server_streaming.host = strdup(strtok_r(NULL, "/", &dptr));
        config->server_streaming.app = strdup(strtok_r(NULL, "/", &dptr));
        return XML_PARAM_STATE;
    }
    char **param;
    if(!strcmp("api", attrs[1])) {
        param = &(config->api);
    } else if(!strcmp("auth", attrs[1])) {
        param = &(config->auth);
    } else if(!strcmp("tray", attrs[1])) {
        param = &(config->tray);
    } else if(!strcmp("categories", attrs[1])) {
        param = &(config->categories);
    } else if(!strcmp("classifications", attrs[1])) {
        param = &(config->classifications);
    } else if(!strcmp("captions", attrs[1])) {
        param = &(config->captions);
    } else if(!strcmp("server_fallback", attrs[1])) {
        param = &(config->server_fallback);
    } else if(!strcmp("highlights", attrs[1])) {
        param = &(config->highlights);
    } else if(!strcmp("home", attrs[1])) {
        param = &(config->home);
    } else if(!strcmp("geo", attrs[1])) {
        param = &(config->geo);
    } else if(!strcmp("time", attrs[1])) {
        param = &(config->time);
    } else if(!strcmp("feedback_url", attrs[1])) {
        param = &(config->feedback_url);
    } else {
        printf("error: parse_param_attrs found no match for %s\n", attrs[1]);
        return NE_XML_ABORT;
    }
    *param = strdup(attrs[3]);
    return XML_PARAM_STATE;
}

static int accept_start_param(void *userdata, int parent, const char *nspace,
        const char *name, const char **attrs) {
    if(0 != strncmp("param", name, 6)) {
        return NE_XML_DECLINE;
    }
    int r = parse_param_attrs((struct iv_config *)userdata, attrs);
    return r;
}

static int accept_cdata_param(void *userdata, int state, const char *cdata,
    size_t len) {
    return 0;
}

static int accept_end_param(void *userdata, int state, const char *nspace,
        const char *name) {
    return 0;
}

int iv_parse_config(struct iv_config *config, const char *buf, size_t len) {
    ne_xml_parser *config_parser = ne_xml_create();
    ne_xml_push_handler(config_parser, accept_start_config,
            accept_cdata_config, accept_end_config, (void *)config);
    ne_xml_push_handler(config_parser, accept_start_param,
            accept_cdata_param, accept_end_param, (void *)config);
    return ne_xml_parse(config_parser, buf, len);
}
