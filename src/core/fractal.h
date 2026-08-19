#ifndef FRACTUS_X64_FRACTAL_H
#define FRACTUS_X64_FRACTAL_H

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

typedef enum fractus_biomorph_equation {
    FRACTUS_BIOMORPH_EQ_Z2 = 0,
    FRACTUS_BIOMORPH_EQ_Z3 = 1,
    FRACTUS_BIOMORPH_EQ_Z4 = 2,
    FRACTUS_BIOMORPH_EQ_Z5 = 3,
    FRACTUS_BIOMORPH_EQ_SIN_Z = 4,
    FRACTUS_BIOMORPH_EQ_EXP_Z = 5
} fractus_biomorph_equation;

typedef enum fractus_biomorph_trap_mode {
    FRACTUS_BIOMORPH_TRAP_RE_OR_IM = 0,
    FRACTUS_BIOMORPH_TRAP_RE_AND_IM = 1,
    FRACTUS_BIOMORPH_TRAP_SOLO_RE = 2,
    FRACTUS_BIOMORPH_TRAP_SOLO_IM = 3
} fractus_biomorph_trap_mode;

typedef struct fractus_biomorph_params {
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    double constant_real;
    double constant_imag;
    uint32_t max_iterations;
    double escape_radius_squared;
    double cutoff;
    uint8_t background_color_index;
    uint8_t palette_offset;
    uint8_t palette_span;
    fractus_biomorph_equation equation;
    fractus_biomorph_trap_mode trap_mode;
} fractus_biomorph_params;

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

fractus_status fractus_fractal_render_mandelbrot(
    fractus_framebuffer *framebuffer,
    const fractus_mandelbrot_params *params);
fractus_status fractus_fractal_render_mandelbrot_dem(
    fractus_framebuffer *framebuffer,
    const fractus_mandelbrot_dem_params *params);
fractus_status fractus_fractal_render_julia(
    fractus_framebuffer *framebuffer,
    const fractus_julia_params *params);
fractus_status fractus_fractal_render_julia_dem(
    fractus_framebuffer *framebuffer,
    const fractus_julia_dem_params *params);
fractus_status fractus_fractal_render_biomorph(
    fractus_framebuffer *framebuffer,
    const fractus_biomorph_params *params);
fractus_status fractus_fractal_render_plasma(
    fractus_framebuffer *framebuffer,
    const fractus_plasma_params *params);
fractus_status fractus_fractal_render_plasma_circular(
    fractus_framebuffer *framebuffer,
    const fractus_plasma_circular_params *params);

#endif
