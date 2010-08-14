#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "iviewiir.h"
#include "libiview/iview.h"

void wv_get_program() {
}

void configure() {
    struct iv_config config;
    char *config_buf;
    size_t config_buf_len = iv_get_config(&config_buf);
    printf("info: parse result = %d\n",
            iv_parse_config(&config, config_buf, config_buf_len));
}

int main(int argc, char **argv) {
    configure();
    return 0;
}
