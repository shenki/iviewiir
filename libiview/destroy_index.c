#include <unistd.h>
#include "iview.h"

void iv_destroy_index(struct iv_series *index, int len) {
    int i;
    for(i=0; i<len; i++) {
        free((void *)(index[i].title));
    }
    free(index);
}
