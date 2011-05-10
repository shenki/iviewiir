#include "iview.h"
#include <unistd.h>
#include <stdio.h>

void iv_destroy_series(struct iv_episode *items, int items_len) {
    int i;
    free(items[0].title);
    free(items[0].description);
    for(i=1; i<items_len; i++) {
        free(items[i].title);
        free(items[i].url);
        free(items[i].description);
        free(items[i].thumbnail);
        free(items[i].date);
        free(items[i].rating);
        free(items[i].link);
        free(items[i].home);
    }
    free(items);
}
