#ifndef FRACTUS_X64_CORE_PLASMA_H
#define FRACTUS_X64_CORE_PLASMA_H

#include "platform/framebuffer.h"
#include "platform/types.h"

typedef struct fractus_plasma_params {
    uint32_t seed;
    int32_t dispersion;
    uint8_t palette_offset;
    uint8_t palette_span;
} fractus_plasma_params;

typedef struct fractus_plasma_circular_params {
    uint32_t seed;
    int32_t circle_count;
    int32_t max_radius;
    uint8_t palette_offset;
    uint8_t palette_span;
} fractus_plasma_circular_params;

fractus_status fractus_fractal_render_plasma(
    fractus_framebuffer *framebuffer,
    const fractus_plasma_params *params);

fractus_status fractus_fractal_render_plasma_circular(
    fractus_framebuffer *framebuffer,
    const fractus_plasma_circular_params *params);

fractus_status fractus_fractal_render_plasma_circular_legacy(
    fractus_framebuffer *framebuffer,
    const fractus_plasma_circular_params *params);

#endif
