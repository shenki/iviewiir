#include <string.h>
#include <neon/ne_xml.h>
#include <neon/ne_uri.h>
#include "iview.h"

#define XML_IVIEW_STATE 1
#define XML_TOKEN_STATE 2
#define XML_SERVER_STATE 3
#define XML_FREE_STATE 4

static int accept_start_iview(void *userdata IV_UNUSED, int parent IV_UNUSED,
        const char *nspace IV_UNUSED, const char *name,
        const char **attrs IV_UNUSED) {
    if(0 != strcmp("iview", name)) {
        return NE_XML_DECLINE;
    }
    return XML_IVIEW_STATE;
}

static int accept_start_token(void *userdata IV_UNUSED, int parent IV_UNUSED,
        const char *nspace IV_UNUSED, const char *name,
        const char **attrs IV_UNUSED) {
    if(0 != strcmp("token", name)) {
        return NE_XML_DECLINE;
    }
    return XML_TOKEN_STATE;
}

static int accept_cdata_token(void *userdata, int state IV_UNUSED,
        const char *cdata, size_t len) {
    (*(struct iv_auth *)userdata).token = strndup(cdata, len);
    return 0;
}

static int accept_start_server(void *userdata IV_UNUSED, int parent IV_UNUSED,
        const char *nspace IV_UNUSED, const char *name,
        const char **attrs IV_UNUSED) {
    if(0 != strcmp("server", name)) {
        return NE_XML_DECLINE;
    }
    return XML_SERVER_STATE;
}

static int accept_cdata_server(void *userdata, int state IV_UNUSED,
        const char *cdata, size_t len) {
    char *server_uri = strndup(cdata, len);
    if(ne_uri_parse(server_uri, &(*(struct iv_auth *)userdata).server)) {
        return NE_XML_ABORT;
    }
    free(server_uri);
    return 0;
}

static int accept_start_free(void *userdata IV_UNUSED, int parent IV_UNUSED,
        const char *nspace IV_UNUSED, const char *name,
        const char **attrs IV_UNUSED) {
    if(0 != strcmp("free", name)) {
        return NE_XML_DECLINE;
    }
    return XML_FREE_STATE;
}

static int accept_cdata_free(void *userdata, int state IV_UNUSED,
        const char *cdata, size_t len) {
    char *free_str = strndup(cdata, len);
    (*(struct iv_auth *)userdata).free = !strcmp("yes", free_str);
    free(free_str);
    return 0;
}

int iv_parse_auth(const struct iv_config *config, const char *buf, size_t len,
        struct iv_auth *auth) {
    ne_xml_parser *auth_parser = ne_xml_create();
    ne_xml_push_handler(auth_parser, accept_start_iview,
            NULL, NULL, NULL);
    ne_xml_push_handler(auth_parser, accept_start_token,
            accept_cdata_token, NULL, (void *)auth);
    ne_xml_push_handler(auth_parser, accept_start_server,
            accept_cdata_server, NULL, (void *)auth);
    ne_xml_push_handler(auth_parser, accept_start_free,
            accept_cdata_free, NULL, (void *)auth);
    int result = ne_xml_parse(auth_parser, buf, len);
    ne_xml_parse(auth_parser, buf, 0);
    ne_xml_destroy(auth_parser);
    if(result) {
        return -IV_ESAXPARSE;
    }
    auth->prefix = IV_AKAMAI_PREFIX;
    if(0 == auth->server.host) {
        if(ne_uri_copy(&auth->server, &config->server_streaming)) {
            return -IV_EURIPARSE;
        }
    }
    return 0;
}
