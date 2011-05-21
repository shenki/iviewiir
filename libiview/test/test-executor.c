#include <stdio.h>
#include "CuTest.h"

CuSuite *strrpl_get_cusuite();

int main() {
    CuString *output = CuStringNew();
    CuSuite *suite = CuSuiteNew();
    CuSuiteAddSuite(suite, strrpl_get_cusuite());
    CuSuiteRun(suite);
    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);
    printf("%s\n", output->buffer);
    return 0;
}
