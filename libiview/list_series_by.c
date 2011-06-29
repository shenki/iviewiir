#include <string.h>
#include <errno.h>
#include "iview.h"

int iv_list_series_by(const char *id, struct iv_series *series,
        const int num_series, struct iv_series_list **list) {
    int i;
    struct iv_series_list *head = NULL, *tail = NULL;
    *list = NULL;
    if(0 >= num_series) { return -IV_EEMPTY; }
    for(i=0; i<num_series; i++) {
        if(strstr(series[i].keywords, id)) {
            struct iv_series_list **next =
                (NULL == tail) ? &tail : &tail->next;
            *next = calloc(1, sizeof(struct iv_series_list));
            if(!*next) {
                int _errno = errno;
                iv_destroy_series_list(head);
                *list = NULL;
                return -_errno;
            }
            if(!head) { head = tail; }
            tail = *next;
            tail->series = &series[i];
        }
    }
    if(!head) { return -IV_EEMPTY; }
    *list = head;
    return IV_OK;
}
