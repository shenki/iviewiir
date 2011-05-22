#include <string.h>
#include "internal.h"

ssize_t strtrim(char **out, const char *str, const char *chars) {
    if(NULL == str) { *out = NULL; return 0; }
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

    const size_t out_len = end - start + 1;
    *out = strndup(&str[start], out_len);
    if(!*out) { return -1; }
    return out_len;
}

#ifdef LIBIVIEW_TEST
#include "test/CuTest.h"
void test_strtrim_null_src(CuTest *tc) {
    const char *src = NULL;
    char *dst;
    const char *trim_chars = " ";
    const ssize_t result = strtrim(&dst, src, trim_chars);
    CuAssertPtrEquals(tc, NULL, dst);
    CuAssertTrue(tc, 0 == result);
}

void test_strtrim_null_chars(CuTest *tc) {
    const char *src = "abc";
    char *dst;
    const char *trim_chars = NULL;
    const ssize_t result = strtrim(&dst, src, trim_chars);
    CuAssertPtrNotNull(tc, dst);
    CuAssertTrue(tc, 0 <= result);
    CuAssertTrue(tc, strlen(src) == (size_t)result);
    CuAssertStrEquals(tc, src, dst);
    free(dst);
}

void test_strtrim_empty_src(CuTest *tc) {
    const char *src = "";
    char *dst;
    const char *trim_chars = NULL;
    const ssize_t result = strtrim(&dst, src, trim_chars);
    CuAssertTrue(tc, NULL == dst);
    CuAssertTrue(tc, 0 == result);
}

void test_strtrim_empty_chars(CuTest *tc) {
    const char *src = "abc";
    char *dst;
    const char *trim_chars = "";
    const ssize_t result = strtrim(&dst, src, trim_chars);
    CuAssertPtrNotNull(tc, dst);
    CuAssertTrue(tc, 0 <= result);
    CuAssertTrue(tc, strlen(src) == (size_t)result);
    CuAssertStrEquals(tc, src, dst);
    free(dst);
}

void test_strtrim_no_trim(CuTest *tc) {
    const char *src = "abc";
    char *dst;
    const char *trim_chars = "A";
    const ssize_t result = strtrim(&dst, src, trim_chars);
    CuAssertPtrNotNull(tc, dst);
    CuAssertTrue(tc, 0 <= result);
    CuAssertTrue(tc, strlen(src) == (size_t)result);
    CuAssertStrEquals(tc, src, dst);
    free(dst);
}

void test_strtrim_only_trim(CuTest *tc) {
    const char *src = "A";
    char *dst;
    const char *trim_chars = "A";
    const ssize_t result = strtrim(&dst, src, trim_chars);
    CuAssertTrue(tc, NULL == dst);
    CuAssertTrue(tc, 0 == result);
}

void test_strtrim_left_trim(CuTest *tc) {
    const char *src = "abc";
    char *dst;
    const char *trim_chars = "a";
    const char *expected = "bc";
    const ssize_t result = strtrim(&dst, src, trim_chars);
    CuAssertPtrNotNull(tc, dst);
    CuAssertTrue(tc, 0 <= result);
    CuAssertTrue(tc, strlen(expected) == (size_t)result);
    CuAssertStrEquals(tc, expected, dst);
    free(dst);
}

void test_strtrim_multi_left_trim(CuTest *tc) {
    const char *src = "aabc";
    char *dst;
    const char *trim_chars = "a";
    const char *expected = "bc";
    const ssize_t result = strtrim(&dst, src, trim_chars);
    CuAssertPtrNotNull(tc, dst);
    CuAssertTrue(tc, 0 <= result);
    CuAssertTrue(tc, strlen(expected) == (size_t)result);
    CuAssertStrEquals(tc, expected, dst);
    free(dst);
}

void test_strtrim_right_trim(CuTest *tc) {
    const char *src = "cba";
    char *dst;
    const char *trim_chars = "a";
    const char *expected = "cb";
    const ssize_t result = strtrim(&dst, src, trim_chars);
    CuAssertPtrNotNull(tc, dst);
    CuAssertTrue(tc, 0 <= result);
    CuAssertTrue(tc, strlen(expected) == (size_t)result);
    CuAssertStrEquals(tc, expected, dst);
    free(dst);
}

void test_strtrim_multi_right_trim(CuTest *tc) {
    const char *src = "cbaa";
    char *dst;
    const char *trim_chars = "a";
    const char *expected = "cb";
    const ssize_t result = strtrim(&dst, src, trim_chars);
    CuAssertPtrNotNull(tc, dst);
    CuAssertTrue(tc, 0 <= result);
    CuAssertTrue(tc, strlen(expected) == (size_t)result);
    CuAssertStrEquals(tc, expected, dst);
    free(dst);
}

void test_strtrim_both_trim(CuTest *tc) {
    const char *src = "aba";
    char *dst;
    const char *trim_chars = "a";
    const char *expected = "b";
    const ssize_t result = strtrim(&dst, src, trim_chars);
    CuAssertPtrNotNull(tc, dst);
    CuAssertTrue(tc, 0 <= result);
    CuAssertTrue(tc, strlen(expected) == (size_t)result);
    CuAssertStrEquals(tc, expected, dst);
    free(dst);
}

void test_strtrim_multi_both_trim(CuTest *tc) {
    const char *src = "aabaa";
    char *dst;
    const char *trim_chars = "a";
    const char *expected = "b";
    const ssize_t result = strtrim(&dst, src, trim_chars);
    CuAssertPtrNotNull(tc, dst);
    CuAssertTrue(tc, 0 <= result);
    CuAssertTrue(tc, strlen(expected) == (size_t)result);
    CuAssertStrEquals(tc, expected, dst);
    free(dst);
}

void test_strtrim_control_char_trim(CuTest *tc) {
    const char *src = "\tabc\n";
    char *dst;
    const char *trim_chars = "\t\n";
    const char *expected = "abc";
    const ssize_t result = strtrim(&dst, src, trim_chars);
    CuAssertPtrNotNull(tc, dst);
    CuAssertTrue(tc, 0 <= result);
    CuAssertTrue(tc, strlen(expected) == (size_t)result);
    CuAssertStrEquals(tc, expected, dst);
    free(dst);
}

void test_strntrim_right_trim(CuTest *tc) {
    const char *src = "abcda";
    char *dst;
    const char *trim_chars = "a";
    const char *expected = "bc";
    const ssize_t result = strntrim(&dst, src, 3, trim_chars);
    CuAssertPtrNotNull(tc, dst);
    CuAssertTrue(tc, 0 <= result);
    CuAssertTrue(tc, strlen(expected) == (size_t)result);
    CuAssertStrEquals(tc, expected, dst);
    free(dst);
}

CuSuite *strtrim_get_cusuite() {
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_strtrim_null_src);
    SUITE_ADD_TEST(suite, test_strtrim_null_chars);
    SUITE_ADD_TEST(suite, test_strtrim_empty_src);
    SUITE_ADD_TEST(suite, test_strtrim_empty_chars);
    SUITE_ADD_TEST(suite, test_strtrim_no_trim);
    SUITE_ADD_TEST(suite, test_strtrim_only_trim);
    SUITE_ADD_TEST(suite, test_strtrim_left_trim);
    SUITE_ADD_TEST(suite, test_strtrim_multi_left_trim);
    SUITE_ADD_TEST(suite, test_strtrim_right_trim);
    SUITE_ADD_TEST(suite, test_strtrim_multi_right_trim);
    SUITE_ADD_TEST(suite, test_strtrim_both_trim);
    SUITE_ADD_TEST(suite, test_strtrim_multi_both_trim);
    SUITE_ADD_TEST(suite, test_strtrim_null_chars);
    SUITE_ADD_TEST(suite, test_strtrim_control_char_trim);
    SUITE_ADD_TEST(suite, test_strntrim_right_trim);
    return suite;
}
#endif /* LIBIVIEW_TEST */
