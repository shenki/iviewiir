#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libxml/nanohttp.h>

#include "iview.h"

ssize_t iv_get_xml_buffer(const char *uri, char **buf_ptr) {
    int return_val;
    char *contentType;
    xmlNanoHTTPInit();
    void *ctx = xmlNanoHTTPOpen(uri, &contentType);
    if(200 != xmlNanoHTTPReturnCode(ctx)) {
        return_val = -1;
        goto done;
    }
    // Allocate buffer for response
    const int content_len = xmlNanoHTTPContentLength(ctx);
    *buf_ptr = (char *)malloc(content_len+1);
    if(!*buf_ptr) {
        return_val = -IV_ENOMEM;
        goto done;
    }
    char *buf = *buf_ptr;
    ssize_t read_len = 0;
    size_t total_len = 0;
    // Read response into buffer
    while(0 < (read_len = xmlNanoHTTPRead(ctx, buf, content_len-total_len))) {
        buf += read_len;
        total_len += read_len;
    }
    if(-1 == read_len) {
        return_val = -IV_EREQUEST;
        free(*buf_ptr);
        *buf_ptr = NULL;
        goto done;
    }
    (*buf_ptr)[content_len] = '\0';
    return_val = content_len;
done:
    xmlNanoHTTPClose(ctx);
    xmlNanoHTTPCleanup();
    return return_val;
}
