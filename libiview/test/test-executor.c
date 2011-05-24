#include <stdio.h>
#include <stdlib.h>
#include "CuTest.h"

#ifndef LIBIVIEW_TEST
#error "libiview's objects must be compiled with -DLIBIVIEW_TEST to link test-executor corrctly"
#endif

CuSuite *strrpl_get_cusuite();
CuSuite *strtrim_get_cusuite();
CuSuite *iv_parse_config_get_cusuite();
CuSuite *iv_parse_index_get_cusuite();
CuSuite *iv_parse_series_get_cusuite();

#define SUITE_COUNT(s) (sizeof(s)/sizeof(CuSuite *))

int main() {
    CuSuite *suites[] = {
        strtrim_get_cusuite(),
        strrpl_get_cusuite(),
        iv_parse_config_get_cusuite(),
        iv_parse_index_get_cusuite(),
        iv_parse_series_get_cusuite(),
    };
    CuString *output = CuStringNew();
    CuSuite *suite = CuSuiteNew();
    unsigned int i;
    for(i=0; i<SUITE_COUNT(suites); i++) {
        if(suites[i]) { CuSuiteAddSuite(suite, suites[i]); }
        /* Hack for CuTest odditiy - CuSuiteAddSuite() implementation copies
         * tests attached to suites[i] into suite and doesn't internally free
         * suites[i]. Documentation doesn't mention anything about this. */
        free(suites[i]);
    }
    CuSuiteRun(suite);
    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);
    printf("%s\n", output->buffer);
    CuSuiteDelete(suite);
    CuStringDelete(output);
    return 0;
}
