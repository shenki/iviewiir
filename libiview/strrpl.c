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
    if(NULL == src) { *dst = NULL; return 0; }
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
    size_t dst_len = 0;
    // Why did they bother?
    if(!substr_start) { goto trivial_case0; }
    if(!search) { goto trivial_case1; }
    const size_t search_len = strlen(search);
    const size_t replace_len = NULL == replace ? 0 : strlen(replace);
    // Determine the number of instances of search
    for(; (substr_start = strstr(substr_start, search)); i++, substr_start++);
    // If there aren't any we don't need to sanitise
    if(0 == i) { goto trivial_case1; }
    // Allocate space for the sanitised string
    dst_len = src_len - (i * search_len) + (i * replace_len);
    if(0 == dst_len) { goto trivial_case0; };
    *dst = (char *)calloc(1, dst_len);
    if(!*dst) { goto trivial_case0; }
    // Find each instance of search
    substr_start = src;
    substr_dst = *dst;
    while((substr_end = strstr(substr_start, search))
            && ((substr_end - src) < (long)src_len)) {
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
    if((long)src_len >= (substr_end - src)) {
        strcpy(substr_dst, substr_start);
    }
    return dst_len;
trivial_case0:
    *dst = NULL;
    return 0;
trivial_case1:
    *dst = strndup(src, src_len);
    return src_len;
}

#ifdef LIBIVIEW_TEST
#include "test/CuTest.h"
void test_strrpl_null_src(CuTest *tc) {
    const char *src = NULL;
    char *dst;
    const char *search = "abc";
    const char *replace = "ABC";
    size_t result = strrpl(&dst, src, search, replace);
    CuAssertTrue(tc, NULL == dst);
    CuAssertTrue(tc, 0 == result);
}

void test_strrpl_null_search(CuTest *tc) {
    const char *src = "abc";
    char *dst;
    const char *search = NULL;
    const char *replace = "ABC";
    size_t result = strrpl(&dst, src, search, replace);
    CuAssertPtrNotNull(tc, dst);
    CuAssertTrue(tc, 0 != result);
    CuAssertTrue(tc, strlen(src) == result);
    CuAssertTrue(tc, strlen(dst) == result);
    CuAssertStrEquals(tc, src, dst);
    free(dst);
}

void test_strrpl_null_replace(CuTest *tc) {
    const char *src = "abc";
    char *dst;
    const char *search = "abc";
    const char *replace = NULL;
    size_t result = strrpl(&dst, src, search, replace);
    CuAssertTrue(tc, NULL == dst);
    CuAssertIntEquals(tc, 0, result);
}

void test_strrpl_no_search(CuTest *tc) {
    const char *src = "abc";
    char *dst;
    const char *search = "ABC";
    const char *replace = "abc";
    size_t result = strrpl(&dst, src, search, replace);
    CuAssertPtrNotNull(tc, dst);
    CuAssertTrue(tc, strlen(dst) == strlen(src));
    CuAssertTrue(tc, strlen(dst) == result);
    CuAssertStrEquals(tc, src, dst);
    free(dst);
}

void test_strrpl_only_search(CuTest *tc) {
    const char *src = "abc";
    char *dst;
    const char *search = "abc";
    const char *replace = "ABC";
    size_t result = strrpl(&dst, src, search, replace);
    CuAssertPtrNotNull(tc, dst);
    CuAssertTrue(tc, strlen(dst) == result);
    CuAssertStrEquals(tc, replace, dst);
    free(dst);
}

void test_strrpl_starts_search(CuTest *tc) {
    const char *src = "abc foobar";
    char *dst;
    const char *search = "abc";
    const char *replace = "ABC";
    const char *expected = "ABC foobar";
    size_t result = strrpl(&dst, src, search, replace);
    CuAssertPtrNotNull(tc, dst);
    CuAssertTrue(tc, strlen(expected) == result);
    CuAssertStrEquals(tc, expected, dst);
    free(dst);
}

void test_strrpl_ends_search(CuTest *tc) {
    const char *src = "foobar abc";
    char *dst;
    const char *search = "abc";
    const char *replace = "ABC";
    const char *expected = "foobar ABC";
    size_t result = strrpl(&dst, src, search, replace);
    CuAssertPtrNotNull(tc, dst);
    CuAssertTrue(tc, strlen(expected) == result);
    CuAssertStrEquals(tc, expected, dst);
    free(dst);
}

void test_strrpl_surrounds_search(CuTest *tc) {
    const char *src = "foo abc bar";
    char *dst;
    const char *search = "abc";
    const char *replace = "ABC";
    const char *expected = "foo ABC bar";
    size_t result = strrpl(&dst, src, search, replace);
    CuAssertPtrNotNull(tc, dst);
    CuAssertTrue(tc, strlen(expected) == result);
    CuAssertStrEquals(tc, expected, dst);
    free(dst);
}

void test_strrpl_bookend_search(CuTest *tc) {
    const char *src = "abc foo abc";
    char *dst;
    const char *search = "abc";
    const char *replace = "ABC";
    const char *expected = "ABC foo ABC";
    size_t result = strrpl(&dst, src, search, replace);
    CuAssertPtrNotNull(tc, dst);
    CuAssertTrue(tc, strlen(expected) == result);
    CuAssertStrEquals(tc, expected, dst);
    free(dst);
}

void test_strrpl_control_char_search(CuTest *tc) {
    const char *src = "abc\nfoo\nabc";
    char *dst;
    const char *search = "\n";
    const char *replace = " ";
    const char *expected = "abc foo abc";
    size_t result = strrpl(&dst, src, search, replace);
    CuAssertPtrNotNull(tc, dst);
    CuAssertTrue(tc, strlen(expected) == result);
    CuAssertStrEquals(tc, expected, dst);
    free(dst);
}

void test_strrpl_control_char_replace(CuTest *tc) {
    const char *src = "abc foo abc";
    char *dst;
    const char *search = " ";
    const char *replace = "\n";
    const char *expected = "abc\nfoo\nabc";
    size_t result = strrpl(&dst, src, search, replace);
    CuAssertPtrNotNull(tc, dst);
    CuAssertTrue(tc, strlen(expected) == result);
    CuAssertStrEquals(tc, expected, dst);
    free(dst);
}

void test_strnrpl_first_instance(CuTest *tc) {
    const char *src = "abc abc";
    char *dst;
    const char *search = "abc";
    const char *replace = "ABC";
    const char *expected = "ABC";
    size_t result = strnrpl(&dst, src, 3, search, replace);
    CuAssertPtrNotNull(tc, dst);
    CuAssertTrue(tc, strlen(expected) == result);
    CuAssertStrEquals(tc, expected, dst);
    free(dst);
}

CuSuite *strrpl_get_cusuite() {
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_strrpl_null_src);
    SUITE_ADD_TEST(suite, test_strrpl_null_search);
    SUITE_ADD_TEST(suite, test_strrpl_null_replace);
    SUITE_ADD_TEST(suite, test_strrpl_no_search);
    SUITE_ADD_TEST(suite, test_strrpl_only_search);
    SUITE_ADD_TEST(suite, test_strrpl_starts_search);
    SUITE_ADD_TEST(suite, test_strrpl_ends_search);
    SUITE_ADD_TEST(suite, test_strrpl_bookend_search);
    SUITE_ADD_TEST(suite, test_strrpl_control_char_search);
    SUITE_ADD_TEST(suite, test_strrpl_control_char_replace);
    SUITE_ADD_TEST(suite, test_strnrpl_first_instance);
    return suite;
}
#endif /* LIBIVIEW_TEST */
