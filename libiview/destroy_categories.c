#include <stdlib.h>
#include "iview.h"
#include "internal.h"

void iv_destroy_categories(struct iv_category *categories) {
    if(!categories) { return; }
    struct iv_category *current = categories;
    while(current) {
        if(current->id) { free(current->id); }
        if(current->name) { free(current->name); }
        if(current->mgmt->sub_head) {
            iv_destroy_categories(current->mgmt->sub_head);
        }
        struct iv_category *next = current->mgmt->next;
        free(current->mgmt);
        free(current);
        current = next;
    }
}
