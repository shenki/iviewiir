#include <stdlib.h>
#include <neon/ne_uri.h>
#include "iview.h"

void iv_destroy_auth(struct iv_auth *auth) {
    ne_uri_free(&auth->server);
    free(auth->token);
}
