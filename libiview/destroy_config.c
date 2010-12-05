#include <unistd.h>
#include <stdio.h>
#include <neon/ne_alloc.h>
#include <neon/ne_uri.h>
#include "iview.h"
#include "internal.h"

void iv_destroy_config(struct iv_config *config) {
    ne_uri_free(&(config->api));
    ne_uri_free(&(config->auth));
    free(config->tray);
    free(config->categories);
    free(config->classifications);
    free(config->captions);
    ne_uri_free(&(config->server_streaming));
    free(config->server_fallback);
    free(config->highlights);
    free(config->home);
    free(config->geo);
    free(config->time);
    free(config->feedback_url);
    free(config);
}
