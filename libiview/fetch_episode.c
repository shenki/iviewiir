#define _GNU_SOURCE
#include <stdio.h>
#undef _GNU_SOURCE
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <librtmp/rtmp.h>
#include <librtmp/log.h>
#include "flvii.h"
#include "iview.h"
#include "internal.h"

/* iv_generate_video_uri
 *
 * Combines item and authentication information to generate the RTMP URI for
 * downloading the video. The resulting URI can be passed to iv_fetch_episode.
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
    if(-1 == playpath_len) { return -(errno); }
    // The extension should be .flv or .mp4, thus we have a minimum of four
    // characters.
    if(5 > playpath_len) {
        return_val = -IV_EURIPARSE;
        goto done;
    }
    // Trim the extension for RTMP URI generation
    playpath[playpath_len-4] = '\0';
    const size_t url_len = strlen(item->url);
    // Below we perform a compare against the last three characters of the item
    // path (url), thus it should be more than 3 characters for the comparison
    // to be valid.
    if(4 > url_len) {
        return_val = -IV_EURIPARSE;
        goto done;
    }
    const char *prefix =
        !strcmp("flv", &item->url[strlen(item->url)-3]) ?  "" : "mp4:";
    const int rtmp_uri_len = asprintf(&rtmp_uri,
            "%s?auth=%s playpath=%s%s swfUrl=%s swfVfy=1 swfAge=0",
            auth->server, auth->token, prefix, playpath, IV_SWF_URL);
    if(-1 == rtmp_uri_len) {
        return_val = -(errno);
        *uri = NULL;
    } else {
        return_val = rtmp_uri_len;
        *uri = rtmp_uri;
    }
    IV_DEBUG("Video URI: %s\n", rtmp_uri);
done:
    free(playpath);
    return return_val;
}

int iv_fetch_episode(const struct iv_auth *auth, const struct iv_episode *item,
        const int fd) {
    return iv_fetch_episode_async(auth, item, fd, NULL, NULL);
}

static int configure_resume(const int fd, RTMP *rtmp) {
    int result, return_val = 0;
    struct stat fd_stat;
    struct flvii_ctx *ctx;
    struct flvii_tag _lkf, *lkf=&_lkf;
    char *metadata, *tagdata;
    ssize_t metadata_size, tagdata_size;
    // Preparation for resume:
    //
    // Determine whether we're dealing with a file or some other form of
    // descriptor. If we're dealing with a file we'll attempt to resume the
    // download where it left off.
    if(-1 == fstat(fd, &fd_stat)) {
        return -(errno);
    }
    if(!(S_ISREG(fd_stat.st_mode) && fd_stat.st_size)) {
        // Early (okay) exit - we're dealing with a stream that isn't backed by
        // a file, so download from the beginning
        return 0;
    }
    // FD is backed by a file, test if it's an FLV
    result = flvii_new_ctx(fd, &ctx);
    if(0 > result) {
        return return_val;
    }
    result = flvii_is_flv(ctx);
    if (1 > result) {
        return_val = result;
        goto ctx_cleanup;
    }
    // It's an FLV, find an appropriate resume point
    result = flvii_find_last_keyframe(ctx, lkf);
    if(0 > result) {
        return_val = result;
        goto ctx_cleanup;
    }
    // Gather all the resume data before manipulating the rtmp struct
    metadata_size = flvii_get_metadata(ctx, &metadata);
    if(0 >= metadata_size) {
        return_val =
            metadata_size ? metadata_size : -FLVII_ENOMETADATA;
        goto ctx_cleanup;
    }
    // Allocate space for tag data off the stack
    tagdata_size = FLVII_TAG_HEADER_SIZE + lkf->body_length;
    tagdata = malloc(tagdata_size);
    if(!tagdata) {
        return_val = -(errno);
        goto metadata_cleanup;
    }
    // Grab the bytes composing the tag from the file, required for rtmp.
    if(-1 == lseek(fd, lkf->file_offset+sizeof(lkf->prev_tag_size), SEEK_SET)) {
        return_val = -(errno);
        goto tagdata_cleanup;
    }
    if(tagdata_size != read(fd, tagdata, tagdata_size)) {
        return_val =
            (-1 == tagdata_size) ? -(errno) : FLVII_ESHORTREAD;
        goto tagdata_cleanup;
    }
    //RTMP_LogHex(RTMP_LOGDEBUG, (uint8_t *)tagdata, tagdata_size);
    // Seek to the end of the last key frame ready for resume
    {
        const off_t file_offset =
            lkf->file_offset
            + sizeof(lkf->prev_tag_size)
            + FLVII_TAG_HEADER_SIZE
            + lkf->body_length;
        if(-1 == lseek(fd, file_offset, SEEK_SET)) {
            return_val = -(errno);
            goto metadata_cleanup;
        }
    }
    // Configure RTMP session for resume
    if(RTMP_ConnectStream(rtmp, lkf->timestamp) && 0 < tagdata_size) {
        IV_DEBUG("RTMP_ConnectStream successful, configuring resume\n");
        rtmp->m_read.flags |= RTMP_READ_RESUME;
        rtmp->m_read.timestamp = lkf->timestamp;
        rtmp->m_read.nResumeTS = lkf->timestamp;
        rtmp->m_read.metaHeader = metadata;
        rtmp->m_read.nMetaHeaderSize = metadata_size;
        rtmp->m_read.initialFrameType = lkf->tag_type;
        rtmp->m_read.initialFrame = tagdata;
        rtmp->m_read.nInitialFrameSize = tagdata_size;
        RTMP_Log(RTMP_LOGDEBUG, "Expecting packet with type: %02X, size: %d, TS: %d ms",
                rtmp->m_read.initialFrameType, rtmp->m_read.nInitialFrameSize, rtmp->m_read.timestamp);
    } else {
        IV_DEBUG("RTMP_ConnectStream failed :(\n");
    }
ctx_cleanup:
    flvii_destroy_ctx(ctx);
    return return_val;
tagdata_cleanup:
    free(tagdata);
metadata_cleanup:
    free(metadata);
    goto ctx_cleanup;
}

int iv_fetch_episode_async(const struct iv_auth *auth,
        const struct iv_episode *item,
        const int fd,
        iv_download_progress_cb *progress_cb,
        void *user_data) {
    int result, return_val = IV_OK;
    int read;
    char *rtmp_uri = NULL;
    ssize_t rtmp_uri_len;
    double tmp_duration = (2 * 3600) * 1000;
    struct iv_progress progress = { 0, tmp_duration, 0.0, 0, 0 };
    // Generate the RTMP URI
    if(0 >= (rtmp_uri_len = generate_video_uri(auth, item, &rtmp_uri))) {
        return rtmp_uri_len;
    }
    IV_DEBUG("RTMP URL: %s\n", rtmp_uri);
    // Start the RTMP session, initialising rtmp struct
#ifdef DEBUG
    RTMP_LogSetLevel(RTMP_LOGDEBUG);
#endif
    RTMP *rtmp = RTMP_Alloc();
    RTMP_Init(rtmp);
    RTMP_SetupURL(rtmp, rtmp_uri);
    RTMP_Connect(rtmp, NULL);
    RTMP_SetBufferMS(rtmp, (uint32_t)progress.duration);
    RTMP_UpdateBufferMS(rtmp);
    // Configure resume
    result = configure_resume(fd, rtmp);
    if(0 > result) {
        return_val = result;
        goto rtmp_cleanup;
    }
    // Done configuring resume, fire the progress callback to signal
    // downloading has begun.
    if(NULL != progress_cb) {
        progress_cb((const struct iv_progress *)&progress, user_data);
    }
#define BUF_SZ (64*1024)
    char *buf = malloc(BUF_SZ);
    if(!buf) { return -(errno); }
    IV_DEBUG("RTMP flags: %x\n", rtmp->m_read.flags);
    while(-1 < (read = RTMP_Read(rtmp, buf, BUF_SZ))
            && RTMP_IsConnected(rtmp)) {
        IV_DEBUG("Read from stream: %d (%d)\n", read, rtmp->m_read.status);
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
        int wrote = 0;
        int remaining = read;
        while(remaining) {
            wrote = write(fd, &buf[(read - remaining)], remaining);
            if(0 > wrote) {
                return_val = -(errno);
                goto buf_cleanup;
            }
            remaining -= wrote;
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
    IV_DEBUG("Stream complete: %d, (%d)\n", read, rtmp->m_read.status);
#undef BUF_SZ
    if(NULL != progress_cb) {
        progress.done = 1;
        progress_cb((const struct iv_progress *)&progress, user_data);
    }
buf_cleanup:
    free(buf);
rtmp_cleanup:
    RTMP_Close(rtmp);
    RTMP_Free(rtmp);
    free(rtmp_uri);
    return return_val;
}
