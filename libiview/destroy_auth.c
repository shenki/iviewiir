#include <stdlib.h>
#include "iview.h"
#include "internal.h"

void iv_destroy_auth(struct iv_auth *auth) {
    free(auth->server);
    free(auth->prefix);
    free(auth->token);
    free(auth);
}
