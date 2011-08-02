#include "flvii.h"
#include "internal.h"

double flvii_get_duration(const struct flvii_ctx *ctx) {
    return ctx->duration_sec;
}

