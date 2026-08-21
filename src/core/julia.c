#include "core/julia.h"

#include <math.h>
#include <stdlib.h>

static fractus_status fractus_julia_validate_common(
    const fractus_framebuffer *framebuffer,
    uint32_t max_iterations,
    double escape_radius_squared,
    uint8_t palette_span)
{
    if (framebuffer == NULL || !framebuffer->initialized) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (framebuffer->size.width < 2u || framebuffer->size.height < 2u ||
        max_iterations == 0u || palette_span == 0u || escape_radius_squared <= 0.0) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    return FRACTUS_STATUS_OK;
}

static uint8_t fractus_julia_iteration_color(
    uint32_t iteration,
    uint8_t palette_offset,
    uint8_t palette_span)
{
    return (uint8_t)(palette_offset + ((iteration - 1u) % palette_span));
}

static uint8_t fractus_julia_smooth_iteration_color(
    uint32_t iteration,
    double zr,
    double zi,
    uint8_t palette_offset,
    uint8_t palette_span)
{
    const double color_scale = 4.0;
    const double modulus = sqrt(zr * zr + zi * zi);
    double mu;
    int color_step;

    if (modulus <= 1.0) {
        return fractus_julia_iteration_color(iteration, palette_offset, palette_span);
    }

    mu = (double)iteration + 1.0 - (log(log(modulus)) / log(2.0));
    if (mu < 0.0) {
        mu = 0.0;
    }

    color_step = (int)floor(mu * color_scale);
    if (color_step < 0) {
        color_step = 0;
    }

    return (uint8_t)(palette_offset + ((uint32_t)color_step % palette_span));
}

fractus_status fractus_fractal_render_julia(
    fractus_framebuffer *framebuffer,
    const fractus_julia_params *params)
{
    uint32_t x;
    uint32_t y;
    double dx;
    double dy;

    if (params == NULL ||
        fractus_julia_validate_common(
            framebuffer,
            params->max_iterations,
            params->escape_radius_squared,
            params->palette_span) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    dx = (params->xmax - params->xmin) / (double)(framebuffer->size.width - 1u);
    dy = (params->ymax - params->ymin) / (double)(framebuffer->size.height - 1u);

    #pragma omp parallel for schedule(dynamic, 8) private(x)
    for (y = 0u; y < framebuffer->size.height; ++y) {
        for (x = 0u; x < framebuffer->size.width; ++x) {
            double zr = params->xmin + (double)x * dx;
            double zi = params->ymax - (double)y * dy;
            uint32_t iteration;
            uint8_t color_index;

            for (iteration = 1u; iteration <= params->max_iterations; ++iteration) {
                double zr2 = zr * zr;
                double zi2 = zi * zi;

                if (zr2 + zi2 > params->escape_radius_squared) {
                    break;
                }

                zi = (2.0 * zr * zi) + params->constant_imag;
                zr = zr2 - zi2 + params->constant_real;
            }

            if (iteration > params->max_iterations) {
                color_index = params->inside_color_index;
            } else if (params->color_mode == FRACTUS_JULIA_COLOR_SMOOTH) {
                color_index = fractus_julia_smooth_iteration_color(
                    iteration,
                    zr,
                    zi,
                    params->palette_offset,
                    params->palette_span);
            } else {
                color_index = fractus_julia_iteration_color(
                    iteration,
                    params->palette_offset,
                    params->palette_span);
            }

            framebuffer->index_pixels[(size_t)y * framebuffer->pitch_pixels + x] = color_index;
        }
    }

    framebuffer->pixels_dirty = 1;
    return FRACTUS_STATUS_OK;
}

fractus_status fractus_fractal_render_julia_dem(
    fractus_framebuffer *framebuffer,
    const fractus_julia_dem_params *params)
{
    uint32_t x;
    uint32_t y;
    double dx;
    double dy;
    double pixel_size;

    if (params == NULL ||
        fractus_julia_validate_common(
            framebuffer,
            params->max_iterations,
            params->escape_radius_squared,
            params->palette_span) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    dx = (params->xmax - params->xmin) / (double)(framebuffer->size.width - 1u);
    dy = (params->ymax - params->ymin) / (double)(framebuffer->size.height - 1u);
    pixel_size = (dx < dy) ? dx : dy;
    if (pixel_size <= 0.0) {
        pixel_size = 1e-9;
    }

    #pragma omp parallel for schedule(dynamic, 8) private(x)
    for (y = 0u; y < framebuffer->size.height; ++y) {
        for (x = 0u; x < framebuffer->size.width; ++x) {
            double zr = params->xmin + (double)x * dx;
            double zi = params->ymax - (double)y * dy;
            double dzr = 1.0;
            double dzi = 0.0;
            double cr = params->constant_real;
            double ci = params->constant_imag;
            uint32_t iteration;
            uint8_t color_index;

            for (iteration = 1u; iteration <= params->max_iterations; ++iteration) {
                double zr2 = zr * zr;
                double zi2 = zi * zi;

                if (zr2 + zi2 > params->escape_radius_squared) {
                    break;
                }

                {
                    double next_dzr = 2.0 * (zr * dzr - zi * dzi);
                    double next_dzi = 2.0 * (zr * dzi + zi * dzr);
                    dzr = next_dzr;
                    dzi = next_dzi;
                }

                {
                    double next_zi = (2.0 * zr * zi) + ci;
                    double next_zr = zr2 - zi2 + cr;
                    zr = next_zr;
                    zi = next_zi;
                }
            }

            if (iteration > params->max_iterations) {
                color_index = params->inside_color_index;
            } else {
                double r2 = zr * zr + zi * zi;
                double dz2 = dzr * dzr + dzi * dzi;

                if (dz2 <= 1e-24 || r2 <= 1.0) {
                    color_index = params->inside_color_index;
                } else {
                    double r = sqrt(r2);
                    double dz = sqrt(dz2);
                    double distance = 2.0 * r * log(r) / dz;
                    double pixel_dist = distance / pixel_size;

                    if (params->color_mode == FRACTUS_JULIA_DEM_COLOR_GRADIENT) {
                        double v = (pixel_dist > 0.0) ? (log(1.0 + pixel_dist) * 8.0) : 0.0;
                        int color_step = (int)floor(v);
                        if (color_step < 0) {
                            color_step = 0;
                        }
                        color_index = (uint8_t)(params->palette_offset + ((uint32_t)color_step % params->palette_span));
                    } else {
                        if (pixel_dist < 1.0) {
                            color_index = params->inside_color_index;
                        } else {
                            color_index = fractus_julia_iteration_color(
                                iteration,
                                params->palette_offset,
                                params->palette_span);
                        }
                    }
                }
            }

            framebuffer->index_pixels[(size_t)y * framebuffer->pitch_pixels + x] = color_index;
        }
    }

    framebuffer->pixels_dirty = 1;
    return FRACTUS_STATUS_OK;
}
