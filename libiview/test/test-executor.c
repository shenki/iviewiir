#include <stdio.h>
#include "CuTest.h"

#ifndef LIBIVIEW_TEST
#error "libiview's objects must be compiled with -DLIBIVIEW_TEST to link test-executor corrctly"
#endif

CuSuite *strrpl_get_cusuite();
CuSuite *strtrim_get_cusuite();
CuSuite *iv_parse_config_get_cusuite();
CuSuite *iv_parse_index_get_cusuite();

int main() {
    CuString *output = CuStringNew();
    CuSuite *suite = CuSuiteNew();
    CuSuiteAddSuite(suite, strtrim_get_cusuite());
    CuSuiteAddSuite(suite, strrpl_get_cusuite());
    CuSuiteAddSuite(suite, iv_parse_config_get_cusuite());
    CuSuiteAddSuite(suite, iv_parse_index_get_cusuite());
    CuSuiteRun(suite);
    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);
    printf("%s\n", output->buffer);
    return 0;
}
