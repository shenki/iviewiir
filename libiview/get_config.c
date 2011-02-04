#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/parser.h>
#include "iview.h"
#include "internal.h"

#define XML_CONFIG_STATE 1
#define XML_PARAM_STATE 2

/* Sample XML config
<?xml version="1.0" encoding="utf-8"?>
    <config>        
            <param name="api"                                       value="http://tviview.abc.net.au/iview/api2/?"/>
            <param name="auth"                                      value="http://tviview.abc.net.au/iview/auth/?v2"/>
            <param name="tray"                                      value="xml/tray.xml"/>
            <param name="categories"                        value="xml/categories.xml"/>
            <param name="classifications"           value="xml/classifications.xml" />
            <param name="captions"                          value="http://www.abc.net.au/iview/captions/"/>
            <param name="captions_offset"           value="0"/>
            <param name="captions_live_offset"      value="-2"/>

            <param name="live_streaming"            value="true" />
            <param name="server_streaming"          value="rtmp://cp53909.edgefcs.net/ondemand" />
            <param name="server_fallback"           value="rtmp://cp44823.edgefcs.net/ondemand" />          
            <param name="highlights"                        value="http://www.abc.net.au/iview/api/highlights.htm" />       
            <param name="home"                                      value="http://www.abc.net.au/iview/xml/home.xml" />     
            <param name="geo"                                       value="http://www.abc.net.au/tv/geo/iview/geotest.xml" />       
            <param name="time"                                      value="http://www.abc.net.au/iview/api/time.htm" />     
            <param name="feedback_url"                      value="http://www2b.abc.net.au/tmb/Client/Board.aspx?b=98"/>
    </config>
*/

enum parse_state { ps_start, ps_config, ps_param, ps_end };

struct config_parse_ctx {
    enum parse_state state;
    int return_value;
    struct iv_config *config;
};

static void param_handler(struct config_parse_ctx *ctx, const xmlChar **attrs) {
    const xmlChar *n = IV_XML_ATTR_NAME(attrs);
    xmlChar *v = xmlStrdup(IV_XML_ATTR_VALUE(attrs));
    struct iv_config *c = ctx->config;
    if(NULL == v) {
        ctx->return_value = IV_ENOMEM;
    } else if(!xmlStrcmp(BAD_CAST("api"), n)) {
        c->api = v;
    } else if(!xmlStrcmp(BAD_CAST("auth"), n)) {
        c->auth = v;
    } else if(!xmlStrcmp(BAD_CAST("tray"), n)) {
        c->tray = v;
    } else if(!xmlStrcmp(BAD_CAST("categories"), n)) {
        c->categories = v;
    } else if(!xmlStrcmp(BAD_CAST("classifications"), n)) {
        c->classifications = v;
    } else if(!xmlStrcmp(BAD_CAST("captions"), n)) {
        c->captions = v;
    } else if(!xmlStrcmp(BAD_CAST("captions_offset"), n)) {
        c->captions_offset = atoi((char *)v);
        free(v);
    } else if(!xmlStrcmp(BAD_CAST("live_streaming"), n)) {
        c->live_streaming = (short)atoi((char *)v);
        free(v);
    } else if(!xmlStrcmp(BAD_CAST("server_streaming"), n)) {
        c->server_streaming = v;
    } else if(!xmlStrcmp(BAD_CAST("server_fallback"), n)) {
        c->server_fallback = v;
    } else if(!xmlStrcmp(BAD_CAST("highlights"), n)) {
        c->highlights = v;
    } else if(!xmlStrcmp(BAD_CAST("home"), n)) {
        c->home = v;
    } else if(!xmlStrcmp(BAD_CAST("geo"), n)) {
        c->geo = v;
    } else if(!xmlStrcmp(BAD_CAST("time"), n)) {
        c->time = v;
    } else if(!xmlStrcmp(BAD_CAST("feedback_url"), n)) {
        c->feedback_url = v;
    } else {
        IV_DEBUG("unhandled parameter: %s\n", n);
    }
}

static void start_element(void *_ctx, const xmlChar *name,
        const xmlChar **attrs) {
    struct config_parse_ctx *ctx = (struct config_parse_ctx *)_ctx;
    if(IV_OK != ctx->return_value) {
        return;
    }
    if(!xmlStrcmp(BAD_CAST("config"), name)) {
        ctx->state = ps_config;
        return;
    }
    if(!xmlStrcmp(BAD_CAST("param"), name)) {
        ctx->state = ps_param;
        param_handler(ctx, attrs);
        return;
    }
    ctx->return_value = IV_EXML;
}

static void end_element(void *_ctx, const xmlChar *name) {
    struct config_parse_ctx *ctx = (struct config_parse_ctx *)_ctx;
    if(IV_OK != ctx->return_value) {
        return;
    }
    if(!xmlStrcmp(BAD_CAST("param"), name)) {
        if(ps_param != ctx->state) {
            // Must be in the param state to exit it...
            ctx->return_value = IV_EXML;
            return;
        }
        ctx->state = ps_config;
        return;
    }
    // If we're not exiting a param element then we must be exiting the root
    // element, config. In this case we've finished parsing.
    ctx->state = ps_end;
}

int iv_get_config(const char *buf, size_t len, struct iv_config **config) {
    *config = malloc(sizeof(struct iv_config));
    // Instantiate SAX parser
    xmlSAXHandlerPtr handler = calloc(1, sizeof(xmlSAXHandler));
    handler->startElement = start_element;
    handler->endElement = end_element;
    // Initialise parser context
    struct config_parse_ctx ctx = {
        .state = ps_start,
        .return_value = IV_OK,
        .config = *config
    };
    // Parse document
    if(0 > xmlSAXUserParseMemory(handler, &ctx, buf, len)) {
        free(handler);
        return -IV_ESAXPARSE;
    }
    free(handler);
    return ctx.return_value;
}
