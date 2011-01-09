#include <stdlib.h>
#include "iview.h"

void iv_destroy_auth(struct iv_auth *auth) {
    free(auth->server);
    free(auth->prefix);
    free(auth->token);
}
