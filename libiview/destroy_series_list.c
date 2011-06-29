#include <stdlib.h>
#include "iview.h"

void iv_destroy_series_list(struct iv_series_list *list) {
    struct iv_series_list *current = list, *next;
    while(current) {
        next = current->next;
        free(current);
        current = next;
    }
}
