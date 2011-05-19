#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "internal.h"

/* Preconditions:
 * - |src| is null-terminated
 * - |search| is null-terminated
 * - |replace| is null-terminated
 *
 * Postconditions:
 * - |dst| is valid if return value is > 0
 * - |dst| is null-terminated
 */
size_t strrpl(char **dst, const char *src, const char *search,
        const char *replace) {
    return strnrpl(dst, src, strlen(src), search, replace);
}

/* Preconditions:
 * - |search| is null-terminated
 * - |replace| is null-terminated
 *
 * Postconditions:
 * - |dst| is valid if return value is > 0
 * - |dst| is null-terminated
 */
size_t strnrpl(char **dst, const char *src, size_t src_len,
        const char *search, const char *replace) {
    int i = 0;
    const char *substr_start = src;
    const char *substr_end;
    char *substr_dst;
    ptrdiff_t substr_len = 0;
    const size_t search_len = strlen(search);
    const size_t replace_len = strlen(replace);
    size_t dst_len = 0;
    // Why did they bother?
    if(!substr_start) { return 0; }
    if(!(search && replace)) { goto trivial_case; }
    // Determine the number of instances of search
    for(; (substr_start = strstr(substr_start, search)); i++, substr_start++);
    // If there aren't any we don't need to sanitise
    if(0 == i) { goto trivial_case; }
    // Allocate space for the sanitised string
    dst_len = src_len - (i * search_len) + (i * replace_len);
    if(0 == dst_len) { return 0; };
    *dst = (char *)calloc(1, dst_len);
    if(!*dst) { return 0; }
    // Find each instance of search
    substr_start = src;
    substr_dst = *dst;
    while((substr_end = strstr(substr_start, search))) {
        substr_len = substr_end - substr_start;
        // Copy each substring not containing search
        strncpy(substr_dst, substr_start, substr_len);
        substr_dst += substr_len;
        // Copy in replace
        strcpy(substr_dst, replace);
        substr_dst += replace_len;
        // Jump over search string
        substr_start = substr_end + search_len;
    }
    strcpy(substr_dst, substr_start);
    return dst_len;
trivial_case:
    *dst = strndup(src, src_len);
    return src_len;
}
