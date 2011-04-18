#include "iview.h"

int iv_find_series(const unsigned int series_id,
        const struct iv_series *series_list,
        const unsigned int series_len,
        const struct iv_series **series_ptr) {
    // We're not going to do anything fancy here, just scan linearly through
    // the list and see if we find the required SID
    unsigned int i;
    for(i=0; i<series_len; i++) {
        if(series_id == series_list[i].id) {
            if(NULL != series_ptr) {
                *series_ptr = &series_list[i];
            }
            return i;
        }
    }
    return -1;
}
