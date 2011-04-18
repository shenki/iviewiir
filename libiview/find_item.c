#include "iview.h"

int iv_find_item(const unsigned int item_id,
        const struct iv_item *items_list,
        const unsigned int items_len,
        const struct iv_item **item_ptr) {
    // We're not going to do anything fancy here, just scan linearly through
    // the list and see if we find the required PID
    unsigned int i;
    for(i=0; i<items_len; i++) {
        if(item_id == items_list[i].id) {
            if(NULL != item_ptr) {
                *item_ptr = &items_list[i];
            }
            return i;
        }
    }
    return -1;
}
