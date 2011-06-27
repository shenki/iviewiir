#include <stdlib.h>
#include "iview.h"

void iv_destroy_category_list(struct iv_category_list *list) {
    struct iv_category_list *current = list, *next;
    while(current) {
        next = current->next;
        free(current);
        current = next;
    }
}
