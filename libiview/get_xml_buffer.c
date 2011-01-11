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
#define BUF_CHUNK_SIZE (4096)
    *buf_ptr = (char *)malloc(BUF_CHUNK_SIZE);
    if(!*buf_ptr) {
        return_val = -IV_ENOMEM;
        goto close;
    }
    char *buf = *buf_ptr;
    unsigned short alloc_count = 1;
    ssize_t read_len = 0;
    size_t total_len = 0;
    // Read response into buffer
    while(BUF_CHUNK_SIZE ==
            (read_len = xmlNanoHTTPRead(ctx, buf, BUF_CHUNK_SIZE))) {
        if(NULL ==
                (*buf_ptr = realloc(*buf_ptr, ++alloc_count*BUF_CHUNK_SIZE))) {
            return_val = -IV_ENOMEM;
            goto close;
        }
        buf = *buf_ptr + total_len;
        total_len += read_len;
    }
    if(-1 == read_len) {
        return_val = -IV_EREQUEST;
        free(*buf_ptr);
        *buf_ptr = NULL;
        goto close;
    }
    // Trim buffer to size
    if(NULL == (*buf_ptr = realloc(*buf_ptr, total_len+1))) {
        return_val = -IV_ENOMEM;
        goto close;
    }
    (*buf_ptr)[total_len] = '\0';
    return_val = total_len;
close:
    xmlNanoHTTPClose(ctx);
done:
    xmlNanoHTTPCleanup();
    return return_val;
}
