#ifndef FRACTUS_X64_CORE_H
#define FRACTUS_X64_CORE_H

#include "core/fractal.h"
#include "core/formats.h"
#include "platform/framebuffer.h"
#include "platform/types.h"

typedef struct fractus_core {
    fractus_framebuffer ui_framebuffer;
    fractus_framebuffer drawing_framebuffer;
    int initialized;
} fractus_core;

fractus_status fractus_core_init(fractus_core *core, fractus_size_u32 logical_size);
void fractus_core_shutdown(fractus_core *core);

/* Same pixel mapping as the complex-plane renderers: x grows right, y grows down. */
#define FRACTUS_PLANE_MANDELBROT_REFERENCE_SPAN (1.2 - (-2.4))
#define FRACTUS_PLANE_JULIA_REFERENCE_SPAN (1.8 - (-1.8))
#define FRACTUS_PLANE_BIOMORPH_REFERENCE_SPAN (2.0 - (-2.0))
#define FRACTUS_PLANE_WHEEL_ZOOM_STEP 1.25
#define FRACTUS_PLANE_MIN_SPAN 1e-12
#define FRACTUS_PLANE_MAX_SPAN 1e4

int fractus_plane_point_from_pixel(
    double xmin,
    double xmax,
    double ymin,
    double ymax,
    uint32_t width,
    uint32_t height,
    uint32_t x,
    uint32_t y,
    double *real_part,
    double *imag_part);
double fractus_plane_zoom_level(double reference_span, double current_span);
int fractus_plane_pan(
    double *xmin,
    double *xmax,
    double *ymin,
    double *ymax,
    uint32_t width,
    uint32_t height,
    int32_t from_x,
    int32_t from_y,
    int32_t to_x,
    int32_t to_y);
int fractus_plane_zoom_at_pixel(
    double *xmin,
    double *xmax,
    double *ymin,
    double *ymax,
    uint32_t width,
    uint32_t height,
    uint32_t x,
    uint32_t y,
    double steps);

#endif
