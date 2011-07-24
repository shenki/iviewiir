#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "flvii.h"
#include "internal.h"

void flvii_destroy_ctx(struct flvii_ctx *ctx) {
    if(!ctx) { return; }
    if(ctx->metadata) { free(ctx->metadata); }
    fclose(ctx->file);
    free(ctx);
}
