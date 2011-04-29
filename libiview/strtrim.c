#include <string.h>
#include "internal.h"

ssize_t strtrim(char **out, const char *str, const char *chars) {
    const size_t len = strlen(str);
    return strntrim(out, str, len, chars);
}

ssize_t
strntrim(char **out, const char *str, size_t str_len, const char *chars) {
    unsigned int start, end;
    char *_chars = (char *)chars;

    // Bail early if there's nothing to do
    if(0 == str_len) {
        *out = NULL;
        return 0;
    }

    // Initialise chars if nothing was set
    if(NULL == _chars) {
        _chars = (char *)" \t";
    }

    // Find the trimmed start
    start = 0;
    while(start<str_len && NULL != strchr(_chars, str[start])) { start++; }
    if(str_len == start) {
        // Everything was trimmed
        *out = NULL;
        return 0;
    }

    // Find the trimmed end
    end = str_len - 1;
    while(end && NULL != strchr(_chars, str[end])) { end--; }

    *out = strndup(&str[start], end - start + 1);
    if(!*out) { return -1; }
    return end - start;
}
