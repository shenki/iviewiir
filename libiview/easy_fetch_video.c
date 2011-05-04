#include "iview.h"
#include "internal.h"

int iv_easy_fetch_video(const struct iv_config *config,
        const struct iv_item *item, const char *outpath) {
    struct iv_auth *auth;
    const int auth_result = iv_get_auth(config, &auth);
    if(IV_OK != auth_result) {
        return auth_result;
    }
    const int fetch_result = iv_fetch_video(auth, item, outpath);
    iv_destroy_auth(auth);
    return fetch_result;
}
