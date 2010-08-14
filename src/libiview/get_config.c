#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <neon/ne_session.h>
#include <neon/ne_request.h>
#include "../iviewiir.h"

ssize_t iv_get_config(char **buf_ptr) {
    int init_result = ne_sock_init();
    ne_session *config_session = ne_session_create(IVIEW_CONFIG_SCHEME,
            IVIEW_CONFIG_HOST, IVIEW_CONFIG_PORT);
    ne_request *config_request = ne_request_create(config_session,
            "GET", IVIEW_CONFIG_PATH);
    printf("info: requesting data from %s\n", IVIEW_CONFIG_PATH);
    if(ne_request_dispatch(config_request)) {
        printf("fetch failed\n");
    } else {
       printf("info: response status code was %d\n",
               ne_get_status(config_request)->code);
    }
    unsigned int i = 1;
    size_t new_len = i*getpagesize();
    do {
        if(NE_OK != ne_begin_request(config_request)) {
           printf("error: Response status code was %d\n",
                   ne_get_status(config_request)->code);
           if(NE_RETRY == ne_end_request(config_request)) {
               printf("warning: ne_end_request returned NE_RETRY\n");
           }
           goto done;
        }
        *buf_ptr = (char *)malloc(new_len);
        size_t read_len = 0;
        char *index = *buf_ptr;
        char *realloc_result;
        while(0 < (read_len = ne_read_response_block(config_request,
                        index, new_len)) && read_len == getpagesize()) {
            printf("info: reallocating space for response\n");
            index = &(*buf_ptr[new_len]);
            new_len += read_len;
            realloc_result = realloc(*buf_ptr, ++i*getpagesize());
            if(!realloc_result) {
                perror("realloc");
                goto done;
            }
            *buf_ptr = realloc_result;
            *buf_ptr[new_len-1] = '\0';
        }
        printf("info: read %zd bytes\n", read_len);
    } while(ne_end_request(config_request));
    printf("%s\n", *buf_ptr);
done:
    ne_session_destroy(config_session);
    ne_sock_exit();
    return new_len;
}
