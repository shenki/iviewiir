#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <neon/ne_xml.h>
#include "iview.h"
#include "internal.h"

#define XML_CONFIG_STATE 1
#define XML_PARAM_STATE 2

static int accept_start_config(void *userdata IV_UNUSED, int parent IV_UNUSED,
        const char *nspace IV_UNUSED, const char *name,
        const char **attrs IV_UNUSED) {
    if(0 != strncmp("config", name, 6)) {
        return NE_XML_DECLINE;
    }
    return XML_CONFIG_STATE;
}

static int parse_param_attrs(struct iv_config *config,
        const char **attrs) {
    int r;
    if(!strcmp("captions_offset", attrs[1])) {
        config->captions_offset = atoi(attrs[1]);
        return XML_PARAM_STATE;
    }
    if(!strcmp("live_streaming", attrs[1])) {
        config->live_streaming = 0 == strcmp("true", attrs[1]) ? 1 : 0;
        return XML_PARAM_STATE;
    }
    if(!strcmp("server_streaming", attrs[1])) {
        if(0 != (r = ne_uri_parse(attrs[3], &(config->server_streaming)))) {
            IV_DEBUG("failed to parse 'server_streaming' URI: %d - %s\n",
                    r, attrs[3]);
            return NE_XML_ABORT;
        }
        return XML_PARAM_STATE;
    }
    if(!strcmp("api", attrs[1])) {
        if(0 != (r = ne_uri_parse(attrs[3], &(config->api)))) {
            IV_DEBUG("failed to parse 'api' URI: %d - %s\n", r, attrs[3]);
            return NE_XML_ABORT;
        }
        return XML_PARAM_STATE;
    }
    if(!strcmp("auth", attrs[1])) {
        if(0 != (r = ne_uri_parse(attrs[3], &(config->auth)))) {
            IV_DEBUG("failed to parse 'auth' URI: %d - %s\n", r, attrs[3]);
            return NE_XML_ABORT;
        }
        return XML_PARAM_STATE;
    }
    char **param;
    if(!strcmp("tray", attrs[1])) {
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
        IV_DEBUG("unhandled parameter: %s\n", attrs[1]);
        return XML_PARAM_STATE;
    }
    *param = strdup(attrs[3]);
    return XML_PARAM_STATE;
}

static int accept_start_param(void *userdata, int parent IV_UNUSED,
        const char *nspace IV_UNUSED, const char *name, const char **attrs) {
    if(0 != strncmp("param", name, 6)) {
        return NE_XML_DECLINE;
    }
    int r = parse_param_attrs((struct iv_config *)userdata, attrs);
    return r;
}

static int iv_parse_config(struct iv_config *config, const char *buf, size_t len) {
    memset(config, 0, sizeof(struct iv_config));
    ne_xml_parser *config_parser = ne_xml_create();
    ne_xml_push_handler(config_parser, accept_start_config,
            NULL, NULL, (void *)config);
    ne_xml_push_handler(config_parser, accept_start_param,
            NULL, NULL, (void *)config);
    int result = ne_xml_parse(config_parser, buf, len);
    IV_DEBUG("parse result: %d - %s\n",
            result, ne_xml_get_error(config_parser));
    ne_xml_parse(config_parser, buf, 0);
    ne_xml_destroy(config_parser);
    return result ? -IV_ESAXPARSE : IV_OK;
}

struct iv_config *iv_get_config(const char *buf, size_t len) {
    struct iv_config *config = malloc(sizeof(struct iv_config));
    if(NULL == config) {
        return NULL;
    }
    int result;
    if(0 != (result = iv_parse_config(config, buf, len))) {
        return NULL;
    }
    return config;
}
