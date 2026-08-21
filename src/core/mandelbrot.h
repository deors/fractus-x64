#ifndef FRACTUS_X64_CORE_MANDELBROT_H
#define FRACTUS_X64_CORE_MANDELBROT_H

#include "platform/framebuffer.h"
#include "platform/types.h"

typedef enum fractus_mandelbrot_color_mode {
    FRACTUS_MANDELBROT_COLOR_ESCAPE = 0,
    FRACTUS_MANDELBROT_COLOR_SMOOTH
} fractus_mandelbrot_color_mode;

typedef struct fractus_mandelbrot_params {
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    uint32_t max_iterations;
    double escape_radius_squared;
    uint8_t inside_color_index;
    uint8_t palette_offset;
    uint8_t palette_span;
    fractus_mandelbrot_color_mode color_mode;
} fractus_mandelbrot_params;

typedef enum fractus_mandelbrot_dem_color_mode {
    FRACTUS_MANDELBROT_DEM_COLOR_BOUNDARY = 0,
    FRACTUS_MANDELBROT_DEM_COLOR_GRADIENT
} fractus_mandelbrot_dem_color_mode;

typedef struct fractus_mandelbrot_dem_params {
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    uint32_t max_iterations;
    double escape_radius_squared;
    uint8_t inside_color_index;
    uint8_t palette_offset;
    uint8_t palette_span;
    fractus_mandelbrot_dem_color_mode color_mode;
} fractus_mandelbrot_dem_params;

fractus_status fractus_fractal_render_mandelbrot(
    fractus_framebuffer *framebuffer,
    const fractus_mandelbrot_params *params);

fractus_status fractus_fractal_render_mandelbrot_dem(
    fractus_framebuffer *framebuffer,
    const fractus_mandelbrot_dem_params *params);

#endif
