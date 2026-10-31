#include "core/core.h"

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
