#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libxml/nanohttp.h>
#include <stdio.h>

#include "iview.h"

ssize_t iv_get_xml_buffer(const char *uri, char **buf_ptr) {
    int return_val;
    xmlNanoHTTPInit();
    IV_DEBUG("Initialised libxml2 nanohttp\n");
    void *ctx = xmlNanoHTTPOpen(uri, NULL);
    if(!ctx) {
        IV_DEBUG("Failed to open %s\n", uri);
        return_val = -IV_EREQUEST;
        goto done;
    }
    int http_code = -1;
    if(200 != (http_code = xmlNanoHTTPReturnCode(ctx))) {
        IV_DEBUG("Bad return code: %d\n", http_code);
        return_val = -IV_EREQUEST;
        goto close;
    }
    // Allocate buffer for response
    const int content_len = xmlNanoHTTPContentLength(ctx);
    *buf_ptr = (char *)malloc(content_len+1);
    if(!*buf_ptr) {
        return_val = -IV_ENOMEM;
        goto close;
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
        goto close;
    }
    (*buf_ptr)[content_len] = '\0';
    return_val = content_len;
close:
    xmlNanoHTTPClose(ctx);
done:
    xmlNanoHTTPCleanup();
    return return_val;
}
