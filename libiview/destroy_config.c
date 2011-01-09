#include <unistd.h>
#include <stdio.h>
#include "iview.h"

void iv_destroy_config(struct iv_config *config) {
    free(config->api);
    free(config->auth);
    free(config->tray);
    free(config->categories);
    free(config->classifications);
    free(config->captions);
    free(config->server_streaming);
    free(config->server_fallback);
    free(config->highlights);
    free(config->home);
    free(config->geo);
    free(config->time);
    free(config->feedback_url);
}
