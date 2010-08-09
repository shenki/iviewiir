#include <string.h>
#include <stdlib.h>
#include <neon/ne_session.h>
#include <neon/ne_request.h>
#include "../iviewiir.h"

size_t iv_get_config(char *config_url, char **buffer) {
    int init_result = ne_sock_init();
    ne_session *config_session = ne_session_create(IVIEW_CONFIG_SCHEME,
            IVIEW_CONFIG_HOST, IVIEW_CONFIG_PORT);
    ne_request *config_request = ne_request_create(config_session,
            "GET", IVIEW_CONFIG_PATH);
    ne_session_destroy(config_session);
    ne_sock_exit();
}
