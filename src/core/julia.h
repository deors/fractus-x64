#ifndef FRACTUS_X64_CORE_JULIA_H
#define FRACTUS_X64_CORE_JULIA_H

#include "platform/framebuffer.h"
#include "platform/types.h"

typedef enum fractus_julia_color_mode {
    FRACTUS_JULIA_COLOR_ESCAPE = 0,
    FRACTUS_JULIA_COLOR_SMOOTH
} fractus_julia_color_mode;

typedef struct fractus_julia_params {
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    double constant_real;
    double constant_imag;
    uint32_t max_iterations;
    double escape_radius_squared;
    uint8_t inside_color_index;
    uint8_t palette_offset;
    uint8_t palette_span;
    fractus_julia_color_mode color_mode;
} fractus_julia_params;

typedef enum fractus_julia_dem_color_mode {
    FRACTUS_JULIA_DEM_COLOR_BOUNDARY = 0,
    FRACTUS_JULIA_DEM_COLOR_GRADIENT
} fractus_julia_dem_color_mode;

typedef struct fractus_julia_dem_params {
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    double constant_real;
    double constant_imag;
    uint32_t max_iterations;
    double escape_radius_squared;
    uint8_t inside_color_index;
    uint8_t palette_offset;
    uint8_t palette_span;
    fractus_julia_dem_color_mode color_mode;
} fractus_julia_dem_params;

fractus_status fractus_fractal_render_julia(
    fractus_framebuffer *framebuffer,
    const fractus_julia_params *params);

fractus_status fractus_fractal_render_julia_dem(
    fractus_framebuffer *framebuffer,
    const fractus_julia_dem_params *params);

#endif
