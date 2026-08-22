#ifndef FRACTUS_X64_CORE_ATTRACTORS_H
#define FRACTUS_X64_CORE_ATTRACTORS_H

#include "platform/framebuffer.h"
#include "platform/types.h"

typedef enum fractus_lorenz_projection {
    FRACTUS_LORENZ_PROJECTION_XZ = 0,
    FRACTUS_LORENZ_PROJECTION_XY,
    FRACTUS_LORENZ_PROJECTION_YZ
} fractus_lorenz_projection;

typedef struct fractus_lorenz_params {
    double sigma;
    double rho;
    double beta;
    double dt;
    uint32_t iterations;
    fractus_lorenz_projection projection;
    uint8_t palette_offset;
    uint8_t palette_span;
} fractus_lorenz_params;

fractus_status fractus_fractal_render_lorenz(
    fractus_framebuffer *framebuffer,
    const fractus_lorenz_params *params);

#endif
