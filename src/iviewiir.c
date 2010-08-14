#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <neon/ne_uri.h>
#include "iviewiir.h"
#include "libiview/iview.h"

void wv_get_program() {
}

void iviewiir_configure(struct iv_config *config) {
    char *config_buf;
    ne_uri config_uri;
    if(ne_uri_parse(IV_CONFIG_URI, &config_uri)) {
        printf("error: uri parsing failed on %s\n", IV_CONFIG_URI);
    }
    size_t config_buf_len = iv_get_xml_buffer(&config_uri, &config_buf);
    int result = iv_parse_config(config, config_buf, config_buf_len);
    printf("info: parse result = %d\n",
            iv_parse_config(config, index_buf, index_buf_len));
}

int main(int argc, char **argv) {
    struct iv_config config;
    iviewiir_configure(&config);
    return 0;
}
