#define _GNU_SOURCE
#include <stdio.h>
#undef _GNU_SOURCE
#include <string.h>
#include <unistd.h>
#include <librtmp/rtmp.h>
#include "iview.h"
#include "internal.h"

/* iv_generate_video_uri
 *
 * Combines item and authentication information to generate the RTMP URI for
 * downloading the video. The resulting URI can be passed to iv_fetch_video.
 *
 * @auth: The struct iv_auth instance as provided by iv_get_auth()
 * @item: The series item (episode) to download
 * @uri: A container for the RTMP URI
 *
 * @return: Greater than or equal to zero on success, less than zero on
 * failure. If the call is successful the result represents the length of the
 * string held in uri. On failure, the value represents the negated error code.
 */
static ssize_t generate_video_uri(const struct iv_auth *auth,
        const struct iv_episode *item, char **uri) {
    int return_val = 0;
    char *rtmp_uri = NULL;
    char *playpath = NULL;
    const int playpath_len = asprintf(&playpath, "%s%s",
            (char *)(auth->free ? BAD_CAST("") : auth->prefix), item->url);
    if(-1 == playpath_len) {
        return -IV_ENOMEM;
    }
    // Trim the extension for RTMP URI generation
    playpath[strlen(playpath)-4] = '\0';
    const char *prefix =
        !xmlStrcmp(BAD_CAST("flv"), &item->url[xmlStrlen(item->url)-3]) ?
            "" : "mp4:";
    const int rtmp_uri_len = asprintf(&rtmp_uri,
            "%s?auth=%s playpath=%s%s swfUrl=%s swfVfy=1 swfAge=0",
            auth->server, auth->token, prefix, playpath, IV_SWF_URL);
    IV_DEBUG("Video URI: %s\n", rtmp_uri);
    return_val = rtmp_uri_len;
    if(-1 == rtmp_uri_len) {
        return_val = -IV_ENOMEM;
    }
    free(playpath);
    *uri = rtmp_uri;
    return rtmp_uri_len;
}

int iv_fetch_video(const struct iv_auth *auth, const struct iv_episode *item,
        const int fd) {
    return iv_fetch_video_async(auth, item, fd, NULL, NULL);
}

int iv_fetch_video_async(const struct iv_auth *auth, const struct iv_episode *item,
        const int fd, iv_download_progress_cb *progress_cb, void *user_data) {
    int return_val = IV_OK;
#define BUF_SZ (64*1024)
    char *buf = malloc(BUF_SZ);
    if(!buf) {
        return -IV_ENOMEM;
    }
    char *rtmp_uri;
    ssize_t rtmp_uri_len;
    if(0 >= (rtmp_uri_len = generate_video_uri(auth, item, &rtmp_uri))) {
        return rtmp_uri_len;
    }
    // Start the RTMP session
    RTMP *rtmp = RTMP_Alloc();
    RTMP_Init(rtmp);
    RTMP_SetupURL(rtmp, rtmp_uri);
    RTMP_Connect(rtmp, NULL);
    RTMP_ConnectStream(rtmp, 0);
    // Determine duration if one exists - otherwise set a default
    struct iv_progress progress = {0, 0.0, 0.0, 0, 0};
#define DEFAULT_DURATION_SEC (2 * 3600)
    double tmp_duration = DEFAULT_DURATION_SEC * 1000; // convert to ms
    progress.duration = tmp_duration;
    progress.valid = 0;
    RTMP_SetBufferMS(rtmp, (uint32_t)progress.duration);
    RTMP_UpdateBufferMS(rtmp);
    int read, wrote;
    // Determine whether we should fire the progress callback
    if(NULL != progress_cb) {
        progress_cb((const struct iv_progress *)&progress, user_data);
    }
    while(0 < (read = RTMP_Read(rtmp, buf, BUF_SZ))) {
        if(!progress.valid && 0 < (tmp_duration = RTMP_GetDuration(rtmp))) {
            // Now that we have a valid duration, report we have an extra few
            // seconds of buffer space to ensure we download the entire video
            progress.duration = (tmp_duration + 5) * 1000;
            progress.valid = 1;
            RTMP_SetBufferMS(rtmp, (uint32_t)progress.duration);
            RTMP_UpdateBufferMS(rtmp);
            IV_DEBUG("Duration: %fms, valid: %hd\n",
                    progress.duration, progress.valid);
        }
        wrote = write(fd, buf, read);
        if(wrote != read) {
            return_val = -IV_ENOMEM;
            goto done;
        }
        if(NULL != progress_cb) {
            progress.count += read;
            // Calculate percentage
            progress.percentage =
                ((double) rtmp->m_read.timestamp) / progress.duration * 100.0;
            // Trim to single digit precision
            progress.percentage =
                ((double) (int) (progress.percentage * 10.0)) / 10.0;
            progress_cb((const struct iv_progress *)&progress, user_data);
        }
    }
done:
    RTMP_Close(rtmp);
    RTMP_Free(rtmp);
    free(buf);
    iv_destroy_video_uri(rtmp_uri);
    return return_val;
}
