#include "core/core.h"

#include <math.h>

fractus_status fractus_core_init(fractus_core *core, fractus_size_u32 logical_size)
{
    if (core == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_framebuffer_init(&core->ui_framebuffer, logical_size) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    if (fractus_framebuffer_init(&core->drawing_framebuffer, logical_size) != FRACTUS_STATUS_OK) {
        fractus_framebuffer_shutdown(&core->ui_framebuffer);
        return FRACTUS_STATUS_ERROR;
    }

    core->initialized = 1;
    return FRACTUS_STATUS_OK;
}

void fractus_core_shutdown(fractus_core *core)
{
    if (core == NULL) {
        return;
    }

    fractus_framebuffer_shutdown(&core->drawing_framebuffer);
    fractus_framebuffer_shutdown(&core->ui_framebuffer);
    core->initialized = 0;
}

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
    double *imag_part)
{
    double dx;
    double dy;

    if (real_part == NULL || imag_part == NULL || width < 2u || height < 2u ||
        x >= width || y >= height || !(xmax > xmin) || !(ymax > ymin)) {
        return 0;
    }

    dx = (xmax - xmin) / (double)(width - 1u);
    dy = (ymax - ymin) / (double)(height - 1u);
    *real_part = xmin + (double)x * dx;
    *imag_part = ymax - (double)y * dy;
    return 1;
}

double fractus_plane_zoom_level(double reference_span, double current_span)
{
    if (!(reference_span > 0.0) || !(current_span > 0.0)) {
        return 0.0;
    }

    return reference_span / current_span;
}

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
    int32_t to_y)
{
    double dx;
    double dy;
    double shift_x;
    double shift_y;

    if (xmin == NULL || xmax == NULL || ymin == NULL || ymax == NULL ||
        width < 2u || height < 2u || !(*xmax > *xmin) || !(*ymax > *ymin) ||
        (from_x == to_x && from_y == to_y)) {
        return 0;
    }

    dx = (*xmax - *xmin) / (double)(width - 1u);
    dy = (*ymax - *ymin) / (double)(height - 1u);
    shift_x = (double)(to_x - from_x) * dx;
    shift_y = (double)(to_y - from_y) * dy;
    *xmin -= shift_x;
    *xmax -= shift_x;
    *ymin += shift_y;
    *ymax += shift_y;
    return 1;
}

int fractus_plane_zoom_at_pixel(
    double *xmin,
    double *xmax,
    double *ymin,
    double *ymax,
    uint32_t width,
    uint32_t height,
    uint32_t x,
    uint32_t y,
    double steps)
{
    double span_x;
    double span_y;
    double factor;
    double factor_min;
    double factor_max;
    double dx;
    double dy;
    double center_x;
    double center_y;
    double new_span_x;
    double new_span_y;

    if (xmin == NULL || xmax == NULL || ymin == NULL || ymax == NULL ||
        width < 2u || height < 2u || x >= width || y >= height ||
        !(*xmax > *xmin) || !(*ymax > *ymin) || !(steps > 0.0 || steps < 0.0)) {
        return 0;
    }

    if (steps > 16.0) {
        steps = 16.0;
    } else if (steps < -16.0) {
        steps = -16.0;
    }

    span_x = *xmax - *xmin;
    span_y = *ymax - *ymin;
    factor = pow(FRACTUS_PLANE_WHEEL_ZOOM_STEP, steps);
    if (!(factor > 0.0)) {
        return 0;
    }

    factor_min = span_x / FRACTUS_PLANE_MAX_SPAN;
    if (span_y / FRACTUS_PLANE_MAX_SPAN > factor_min) {
        factor_min = span_y / FRACTUS_PLANE_MAX_SPAN;
    }
    factor_max = span_x / FRACTUS_PLANE_MIN_SPAN;
    if (span_y / FRACTUS_PLANE_MIN_SPAN < factor_max) {
        factor_max = span_y / FRACTUS_PLANE_MIN_SPAN;
    }
    if (factor < factor_min) {
        factor = factor_min;
    }
    if (factor > factor_max) {
        factor = factor_max;
    }
    if (!(factor > 0.0) || factor == 1.0) {
        return 0;
    }

    dx = span_x / (double)(width - 1u);
    dy = span_y / (double)(height - 1u);
    center_x = *xmin + (double)x * dx;
    center_y = *ymax - (double)y * dy;
    new_span_x = span_x / factor;
    new_span_y = span_y / factor;
    *xmin = center_x - (double)x * (new_span_x / (double)(width - 1u));
    *xmax = *xmin + new_span_x;
    *ymax = center_y + (double)y * (new_span_y / (double)(height - 1u));
    *ymin = *ymax - new_span_y;
    return (*xmax > *xmin) && (*ymax > *ymin);
}
