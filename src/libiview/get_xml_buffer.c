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

ssize_t iv_get_xml_buffer(ne_uri *uri, char **buf_ptr) {
    int init_result = ne_sock_init();
    ne_session *config_session = ne_session_create(uri->scheme, uri->host,
            0 == uri->port ? ne_uri_defaultport(uri->scheme) : uri->port);
    char *path;
    path = ne_concat(uri->path, "?", uri->query, NULL);
    ne_request *config_request = ne_request_create(config_session,
            "GET", path);
    printf("info: requesting data from %s\n", path);
    free(path);
    unsigned int i = 1;
    size_t total_len = 0;
    size_t new_len = getpagesize();
    do {
        *buf_ptr = (char *)malloc(new_len);
        if(!*buf_ptr) {
            perror("malloc");
            goto done;
        }
        size_t read_len = 0;
        char *index = *buf_ptr;
        char *realloc_result;
        if(NE_OK != ne_begin_request(config_request)) {
            printf("error: failed to begin request\n");
            goto done;
        }
        if(200 != ne_get_status(config_request)->code) {
            printf("error: Response status code was %d\n",
                ne_get_status(config_request)->code);
            if(NE_RETRY == ne_end_request(config_request)) {
            printf("warning: ne_end_request returned NE_RETRY\n");
            }
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
                    perror("realloc");
                    goto done;
                }
                *buf_ptr = realloc_result;
                new_len = getpagesize();
            }
            index = &((*buf_ptr)[total_len]);
        }
    } while(0 < ne_end_request(config_request));
done:
    ne_session_destroy(config_session);
    ne_sock_exit();
    return total_len;
}
