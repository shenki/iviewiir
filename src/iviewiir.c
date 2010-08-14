#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "iviewiir.h"

void wv_get_program() {
}

void configure() {
    char *config_buf;
    size_t config_buf_len = iv_get_config(API_VERSION, &config_buf);
    iv_parse_config(config_buf, config_buf_len);
}

int main(int argc, char *argv) {
    configure();
    return 0;
}
