#include <string.h>
#include <libxml/parser.h>
#include <libxml/xmlstring.h>
#include "iview.h"
#include "internal.h"

/* Sample auth XML data:
    <?xml version="1.0" encoding="utf-8"?>
    <iview xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns="http://www.abc.net.au/iView/Services/iViewHandshaker">
      <ip>121.45.113.44</ip>
      <isp>Internode</isp>
      <desc>Internet service provider</desc>
      <host>Hostworks</host>
      <server>rtmp://203.18.195.10/ondemand</server>

      <bwtest>rtmp://203.18.195.10/live</bwtest>
      <token>83CCCD795CD079650849</token>
      <text><![CDATA[Unmetering is available for <a href="http://www.internode.on.net/" target="_blank">Internode ADSL</a> customers. Check <a href="http://www.internode.on.net/unmetered/" target="_blank">this page</a> for details on whether iview content is metered under your plan.]]></text>
      <free>yes</free>  
    </iview>
    <!-- 0.001417875289917 -->
*/

enum parse_state { st_start, st_iview, st_token, st_server, st_free, st_end };

struct auth_parse_ctx {
    enum parse_state state;
    int return_value;
    struct iv_auth *auth;
};

static void start_element(void *_ctx, const xmlChar *name,
        const xmlChar **attrs IV_UNUSED) {
    struct auth_parse_ctx *ctx = (struct auth_parse_ctx *)_ctx;
    if(IV_OK != ctx->return_value) {
        return;
    }
    if(!xmlStrcmp(BAD_CAST("iview"), name)) {
        ctx->state = st_iview;
        return;
    }
    if(!xmlStrcmp(BAD_CAST("token"), name)) {
        ctx->state = st_token;
        return;
    }
    if(!xmlStrcmp(BAD_CAST("server"), name)) {
        ctx->state = st_server;
        return;
    }
    if(!xmlStrcmp(BAD_CAST("free"), name)) {
        ctx->state = st_free;
        return;
    }
}

static void end_element(void *_ctx, const xmlChar *name) {
    struct auth_parse_ctx *ctx = (struct auth_parse_ctx *)_ctx;
    if(IV_OK != ctx->return_value) {
        return;
    }
    if(!xmlStrcmp(BAD_CAST("iview"), name)) {
        ctx->state = st_end;
        return;
    }
    // Current XML is only one element deep, so if a tag has ended we should be
    // back inside the iview element
    ctx->state = st_iview;
    return;
}

static void content_handler(void *_ctx, const xmlChar *data, int len) {
    struct auth_parse_ctx *ctx = (struct auth_parse_ctx *)_ctx;
    if(IV_OK != ctx->return_value) {
        return;
    }
    switch(ctx->state) {
        case st_token:
            ctx->auth->token = xmlStrndup(data, len);
            if(!ctx->auth->token) {
                ctx->return_value = IV_ENOMEM;
            }
            break;
        case st_server:
            ctx->auth->server = xmlStrndup(data, len);
            if(!ctx->auth->server) {
                ctx->return_value = IV_ENOMEM;
            }
            break;
        case st_free:
#define FREE_VALUE "yes"
            ctx->auth->free =
                !xmlStrncmp(BAD_CAST(FREE_VALUE), data, strlen(FREE_VALUE));
#undef FREE_VALUE
            break;
        default:
            return;
    }
    return;
}

int iv_parse_auth(const char *buf, size_t len, struct iv_auth **auth) {
    // Initialise and fetch auth xml
    *auth = calloc(1, sizeof(struct iv_auth));
    if(NULL == *auth) {
        return -IV_ENOMEM;
    }
    // Parse auth xml
    xmlSAXHandlerPtr handler = calloc(1, sizeof(xmlSAXHandler));
    handler->initialized = XML_SAX2_MAGIC;
    handler->startElement = start_element;
    handler->endElement = end_element;
    handler->characters = content_handler;
    struct auth_parse_ctx ctx = {
        .state = st_start,
        .return_value = IV_OK,
        .auth = *auth
    };
    if(0 > xmlSAXUserParseMemory(handler, &ctx, buf, len)) {
        IV_DEBUG("Failure in parsing xml\n");
        return -IV_ESAXPARSE;
    }
    return -(ctx.return_value);
}
