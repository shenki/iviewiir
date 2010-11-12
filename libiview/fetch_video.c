#include <unistd.h>
#include <stdio.h>
#include <librtmp/rtmp.h>
#include "iview.h"

int iv_fetch_video(const struct iv_auth *auth, const struct iv_item *item,
        const char *outpath) {
    int return_val = IV_OK;
#define BUF_SZ (64*1024)
    char *buf = malloc(BUF_SZ);
    if(!buf) {
        return -IV_ENOMEM;
    }
    char *rtmp_uri = iv_generate_video_uri(auth, item);
    FILE *outfile = fopen(outpath, "w");
    RTMP *rtmp = RTMP_Alloc();
    RTMP_Init(rtmp);
    RTMP_SetupURL(rtmp, rtmp_uri);
    RTMP_Connect(rtmp, NULL);
    RTMP_ConnectStream(rtmp, 0);
    RTMP_SetBufferMS(rtmp, (uint32_t) (2 * 3600 * 1000)); // 2hrs
    RTMP_UpdateBufferMS(rtmp);
    int read, wrote;
    while(0 < (read = RTMP_Read(rtmp, buf, BUF_SZ))) {
        wrote = fwrite(buf, 1, read, outfile);
        if(wrote != read) {
            return_val = -IV_ENOMEM;
            goto done;
        }
    }
done:
    RTMP_Close(rtmp);
    RTMP_Free(rtmp);
    fclose(outfile);
    free(buf);
    iv_destroy_video_uri(rtmp_uri);
    return 0;
}
