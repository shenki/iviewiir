#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <neon/ne_session.h>
#include <neon/ne_request.h>
#include <neon/ne_uri.h>
#include <neon/ne_string.h>
#include "../iviewiir.h"
#include "iview.h"

ssize_t iv_get_xml_buffer(const ne_uri *uri, char **buf_ptr) {
    int return_val;
    int init_result = ne_sock_init();
    ne_session *config_session = ne_session_create(uri->scheme, uri->host,
            0 == uri->port ? ne_uri_defaultport(uri->scheme) : uri->port);
    char *path;
    path = ne_concat(uri->path, "?", uri->query, NULL);
    ne_request *config_request = ne_request_create(config_session,
            "GET", path);
    free(path);
    unsigned int i = 1;
    size_t total_len = 0;
    size_t new_len = getpagesize();
    char *realloc_result;
    do {
        *buf_ptr = (char *)malloc(new_len);
        if(!*buf_ptr) {
            return_val = -IV_ENOMEM;
            goto done;
        }
        size_t read_len = 0;
        char *index = *buf_ptr;
        if(NE_OK != ne_begin_request(config_request)) {
            free(*buf_ptr);
            return_val = -IV_EREQUEST;
            goto done;
        }
        if(200 != ne_get_status(config_request)->code) {
            free(*buf_ptr);
            return_val = -IV_EREQUEST;
            goto done;
        }
        // Maybe use ne_buffer here instead?
        while(0 < (read_len = ne_read_response_block(config_request, index,
                        new_len))) {
            total_len += read_len;
            new_len = i*getpagesize()-total_len;
            if(0 == new_len) {
                realloc_result = realloc(*buf_ptr, ++i*getpagesize());
                if(!realloc_result) {
                    return_val = -IV_ENOMEM;
                    goto done;
                }
                *buf_ptr = realloc_result;
                new_len = getpagesize();
            }
            index = &((*buf_ptr)[total_len]);
        }
    } while(0 < ne_end_request(config_request));
    /* Trim to size and add NULL terminator */
    realloc_result = realloc(*buf_ptr, total_len+1);
    if(!realloc_result) {
        return_val = -IV_ENOMEM;
        goto done;
    }
    *buf_ptr = realloc_result;
    (*buf_ptr)[total_len] = '\0';
    ne_strclean(*buf_ptr);
    return_val = total_len;
done:
    ne_request_destroy(config_request);
    ne_session_destroy(config_session);
    ne_sock_exit();
    return return_val;
}
