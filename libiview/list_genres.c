#include "iview.h"
#include "internal.h"

int iv_list_genres(const struct iv_category *categories,
        struct iv_category_list **list) {
    struct iv_category *cat_current = categories->mgmt->sub_head;
    while((cat_current) && !(cat_current->mgmt->genre)) {
        cat_current = cat_current->mgmt->next;
    }
    if(!cat_current) { return -IV_EEMPTY; }
    // Allocate first entry in the list
    *list = calloc(1, sizeof(struct iv_category_list));
    if(!*list) { return -errno; }
    struct iv_category_list *list_current = *list;
    // Populate the list
    while(cat_current) {
        list_current->category = cat_current;
        cat_current = cat_current->mgmt->next;
        // Only populate the next field if its actually required
        while((cat_current) && !(cat_current->mgmt->genre)) {
            cat_current = cat_current->mgmt->next;
        }
        if(!cat_current) { break; }
        list_current->next = calloc(1, sizeof(struct iv_category_list));
        if(!list_current->next) {
            int _errno = errno;
            iv_destroy_category_list(*list);
            return -_errno;
        }
        list_current = list_current->next;
    }
    return IV_OK;
}
