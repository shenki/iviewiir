#include "iview.h"
#include <unistd.h>
#include <stdio.h>

void iv_destroy_series(struct iv_episode *items, int items_len) {
    int i;
    for(i=0; i<items_len; i++) {
        if(NULL != items[i].title) { free((char *)items[i].title); }
        if(NULL != items[i].url) { free((char *)items[i].url); }
        if(NULL != items[i].description) { free((char *)items[i].description); }
        if(NULL != items[i].thumbnail) { free((char *)items[i].thumbnail); }
        if(NULL != items[i].date) { free((char *)items[i].date); }
        if(NULL != items[i].rating) { free((char *)items[i].rating); }
    }
    free(items);
}
