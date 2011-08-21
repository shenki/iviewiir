#include "iview.h"
#include "internal.h"

int iv_easy_fetch_episode(const struct iv_config *config,
        const struct iv_episode *item, const int fd, const uint32_t offset) {
    struct iv_auth *auth;
    const int auth_result = iv_easy_auth(config, &auth);
    if(IV_OK != auth_result) {
        return auth_result;
    }
    const int fetch_result = iv_fetch_episode(auth, item, fd, 0, offset);
    iv_destroy_auth(auth);
    return fetch_result;
}

int iv_easy_fetch_episode_async(const struct iv_config *config,
        const struct iv_episode *item, const int fd, const uint32_t offset,
        iv_download_progress_cb *progress_cb, void *user_data) {
    struct iv_auth *auth;
    const int auth_result = iv_easy_auth(config, &auth);
    if(IV_OK != auth_result) {
        return auth_result;
    }
    const int fetch_result = iv_fetch_episode_async(auth, item, fd, 0, offset,
            progress_cb, user_data);
    iv_destroy_auth(auth);
    return fetch_result;
}
