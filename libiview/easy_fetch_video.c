#include "iview.h"
#include "internal.h"

int iv_easy_fetch_video(const struct iv_config *config,
        const struct iv_episode *item, const int fd) {
    struct iv_auth *auth;
    const int auth_result = iv_get_auth(config, &auth);
    if(IV_OK != auth_result) {
        return auth_result;
    }
    const int fetch_result = iv_fetch_video(auth, item, fd);
    iv_destroy_auth(auth);
    return fetch_result;
}

int iv_easy_fetch_video_async(const struct iv_config *config,
        const struct iv_episode *item, const int fd,
        iv_download_progress_cb *progress_cb, void *user_data) {
    struct iv_auth *auth;
    const int auth_result = iv_get_auth(config, &auth);
    if(IV_OK != auth_result) {
        return auth_result;
    }
    const int fetch_result = iv_fetch_video_async(auth, item, fd,
            progress_cb, user_data);
    iv_destroy_auth(auth);
    return fetch_result;
}
