#include "core/fractal.h"

#include <math.h>
#include <stdlib.h>

static fractus_status fractus_fractal_validate_common(
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

static uint8_t fractus_fractal_iteration_color(
    uint32_t iteration,
    uint8_t palette_offset,
    uint8_t palette_span)
{
    return (uint8_t)(palette_offset + ((iteration - 1u) % palette_span));
}

static uint8_t fractus_fractal_smooth_iteration_color(
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
        return fractus_fractal_iteration_color(iteration, palette_offset, palette_span);
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

fractus_status fractus_fractal_render_mandelbrot(
    fractus_framebuffer *framebuffer,
    const fractus_mandelbrot_params *params)
{
    uint32_t x;
    uint32_t y;
    double dx;
    double dy;

    if (params == NULL ||
        fractus_fractal_validate_common(
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
            double cr = params->xmin + (double)x * dx;
            double ci = params->ymax - (double)y * dy;
            double zr = 0.0;
            double zi = 0.0;
            uint32_t iteration;
            uint8_t color_index;

            for (iteration = 1u; iteration <= params->max_iterations; ++iteration) {
                double zr2 = zr * zr;
                double zi2 = zi * zi;

                if (zr2 + zi2 > params->escape_radius_squared) {
                    break;
                }

                zi = (2.0 * zr * zi) + ci;
                zr = zr2 - zi2 + cr;
            }

            if (iteration > params->max_iterations) {
                color_index = params->inside_color_index;
            } else if (params->color_mode == FRACTUS_MANDELBROT_COLOR_SMOOTH) {
                color_index = fractus_fractal_smooth_iteration_color(
                    iteration,
                    zr,
                    zi,
                    params->palette_offset,
                    params->palette_span);
            } else {
                color_index = fractus_fractal_iteration_color(
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

fractus_status fractus_fractal_render_julia(
    fractus_framebuffer *framebuffer,
    const fractus_julia_params *params)
{
    uint32_t x;
    uint32_t y;
    double dx;
    double dy;

    if (params == NULL ||
        fractus_fractal_validate_common(
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
                color_index = fractus_fractal_smooth_iteration_color(
                    iteration,
                    zr,
                    zi,
                    params->palette_offset,
                    params->palette_span);
            } else {
                color_index = fractus_fractal_iteration_color(
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
        fractus_fractal_validate_common(
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
                            color_index = fractus_fractal_iteration_color(
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

static inline void fractus_biomorph_step_z2(
    double zr,
    double zi,
    double cr,
    double ci,
    double *next_r,
    double *next_i)
{
    *next_r = (zr * zr) - (zi * zi) + cr;
    *next_i = (2.0 * zr * zi) + ci;
}

static inline void fractus_biomorph_step_z3(
    double zr,
    double zi,
    double cr,
    double ci,
    double *next_r,
    double *next_i)
{
    double zr2 = zr * zr;
    double zi2 = zi * zi;

    *next_r = zr * (zr2 - 3.0 * zi2) + cr;
    *next_i = zi * (3.0 * zr2 - zi2) + ci;
}

static inline void fractus_biomorph_step_z4(
    double zr,
    double zi,
    double cr,
    double ci,
    double *next_r,
    double *next_i)
{
    double zr2 = zr * zr;
    double zi2 = zi * zi;

    *next_r = (zr2 * zr2) - (6.0 * zr2 * zi2) + (zi2 * zi2) + cr;
    *next_i = (4.0 * zr * zi * (zr2 - zi2)) + ci;
}

static inline void fractus_biomorph_step_z5(
    double zr,
    double zi,
    double cr,
    double ci,
    double *next_r,
    double *next_i)
{
    double zr2 = zr * zr;
    double zi2 = zi * zi;
    double zr4 = zr2 * zr2;
    double zi4 = zi2 * zi2;

    *next_r = (zr * (zr4 - (10.0 * zr2 * zi2) + (5.0 * zi4))) + cr;
    *next_i = (zi * ((5.0 * zr4) - (10.0 * zr2 * zi2) + zi4)) + ci;
}

static inline void fractus_biomorph_step_sin_z(
    double zr,
    double zi,
    double cr,
    double ci,
    double *next_r,
    double *next_i)
{
    /* Evitar overflow y optimizar calculo hiperbolico evaluando exp una sola vez */
    double clamped_i = (zi > 50.0) ? 50.0 : ((zi < -50.0) ? -50.0 : zi);
    double exp_p = exp(clamped_i);
    double exp_m = 1.0 / exp_p;
    double cosh_i = 0.5 * (exp_p + exp_m);
    double sinh_i = 0.5 * (exp_p - exp_m);

    *next_r = (sin(zr) * cosh_i) + cr;
    *next_i = (cos(zr) * sinh_i) + ci;
}

static inline void fractus_biomorph_step_exp_z(
    double zr,
    double zi,
    double cr,
    double ci,
    double *next_r,
    double *next_i)
{
    /* Evitar overflow en exp manteniendo el argumento en rango seguro */
    double clamped_r = (zr > 100.0) ? 100.0 : ((zr < -100.0) ? -100.0 : zr);
    double exp_r = exp(clamped_r);

    *next_r = (exp_r * cos(zi)) + cr;
    *next_i = (exp_r * sin(zi)) + ci;
}

static inline int fractus_biomorph_check_trap(
    double zr,
    double zi,
    double cutoff,
    fractus_biomorph_trap_mode mode)
{
    int re_trap = isfinite(zr) && (zr > -cutoff && zr < cutoff);
    int im_trap = isfinite(zi) && (zi > -cutoff && zi < cutoff);

    switch (mode) {
    case FRACTUS_BIOMORPH_TRAP_RE_AND_IM:
        return re_trap && im_trap;
    case FRACTUS_BIOMORPH_TRAP_SOLO_RE:
        return re_trap;
    case FRACTUS_BIOMORPH_TRAP_SOLO_IM:
        return im_trap;
    case FRACTUS_BIOMORPH_TRAP_RE_OR_IM:
    default:
        return re_trap || im_trap;
    }
}

fractus_status fractus_fractal_render_biomorph(
    fractus_framebuffer *framebuffer,
    const fractus_biomorph_params *params)
{
    uint32_t x;
    uint32_t y;
    double dx;
    double dy;
    double cutoff;

    if (params == NULL ||
        fractus_fractal_validate_common(
            framebuffer,
            params->max_iterations,
            params->escape_radius_squared,
            params->palette_span) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    dx = (params->xmax - params->xmin) / (double)(framebuffer->size.width - 1u);
    dy = (params->ymax - params->ymin) / (double)(framebuffer->size.height - 1u);
    cutoff = (params->cutoff > 0.0) ? params->cutoff : 1.0;

    #pragma omp parallel for schedule(dynamic, 8) private(x)
    for (y = 0u; y < framebuffer->size.height; ++y) {
        for (x = 0u; x < framebuffer->size.width; ++x) {
            double zr = params->xmin + (double)x * dx;
            double zi = params->ymax - (double)y * dy;
            uint32_t iteration;
            uint8_t color_index = params->background_color_index;

            for (iteration = 1u; iteration <= params->max_iterations; ++iteration) {
                double r2 = (zr * zr) + (zi * zi);
                double next_r;
                double next_i;

                if (r2 > params->escape_radius_squared || !isfinite(r2)) {
                    if (fractus_biomorph_check_trap(zr, zi, cutoff, params->trap_mode)) {
                        color_index = fractus_fractal_iteration_color(
                            iteration,
                            params->palette_offset,
                            params->palette_span);
                    }
                    break;
                }

                switch (params->equation) {
                case FRACTUS_BIOMORPH_EQ_Z3:
                    fractus_biomorph_step_z3(zr, zi, params->constant_real, params->constant_imag, &next_r, &next_i);
                    break;
                case FRACTUS_BIOMORPH_EQ_Z4:
                    fractus_biomorph_step_z4(zr, zi, params->constant_real, params->constant_imag, &next_r, &next_i);
                    break;
                case FRACTUS_BIOMORPH_EQ_Z5:
                    fractus_biomorph_step_z5(zr, zi, params->constant_real, params->constant_imag, &next_r, &next_i);
                    break;
                case FRACTUS_BIOMORPH_EQ_SIN_Z:
                    fractus_biomorph_step_sin_z(zr, zi, params->constant_real, params->constant_imag, &next_r, &next_i);
                    break;
                case FRACTUS_BIOMORPH_EQ_EXP_Z:
                    fractus_biomorph_step_exp_z(zr, zi, params->constant_real, params->constant_imag, &next_r, &next_i);
                    break;
                case FRACTUS_BIOMORPH_EQ_Z2:
                default:
                    fractus_biomorph_step_z2(zr, zi, params->constant_real, params->constant_imag, &next_r, &next_i);
                    break;
                }

                zr = next_r;
                zi = next_i;
            }

            if (iteration > params->max_iterations) {
                if (fractus_biomorph_check_trap(zr, zi, cutoff, params->trap_mode)) {
                    color_index = fractus_fractal_iteration_color(
                        params->max_iterations,
                        params->palette_offset,
                        params->palette_span);
                }
            }

            framebuffer->index_pixels[(size_t)y * framebuffer->pitch_pixels + x] = color_index;
        }
    }

    framebuffer->pixels_dirty = 1;
    return FRACTUS_STATUS_OK;
}

fractus_status fractus_fractal_render_mandelbrot_dem(
    fractus_framebuffer *framebuffer,
    const fractus_mandelbrot_dem_params *params)
{
    uint32_t x;
    uint32_t y;
    double dx;
    double dy;
    double pixel_size;

    if (params == NULL ||
        fractus_fractal_validate_common(
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
        pixel_size = (dx > 0.0) ? dx : 1e-12;
    }

    #pragma omp parallel for schedule(dynamic, 8) private(x)
    for (y = 0u; y < framebuffer->size.height; ++y) {
        for (x = 0u; x < framebuffer->size.width; ++x) {
            double cr = params->xmin + (double)x * dx;
            double ci = params->ymax - (double)y * dy;
            double zr = 0.0;
            double zi = 0.0;
            double dzr = 0.0;
            double dzi = 0.0;
            uint32_t iteration;
            uint8_t color_index;

            for (iteration = 1u; iteration <= params->max_iterations; ++iteration) {
                double zr2 = zr * zr;
                double zi2 = zi * zi;

                if (zr2 + zi2 > params->escape_radius_squared) {
                    break;
                }

                {
                    double next_dzr = 2.0 * (zr * dzr - zi * dzi) + 1.0;
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

                    if (params->color_mode == FRACTUS_MANDELBROT_DEM_COLOR_GRADIENT) {
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
                            color_index = fractus_fractal_iteration_color(
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

static uint8_t fractus_plasma_random_color(const fractus_plasma_params *params)
{
    return (uint8_t)(params->palette_offset + (rand() % params->palette_span));
}

static fractus_status fractus_plasma_adjust(
    fractus_framebuffer *framebuffer,
    const fractus_plasma_params *params,
    int32_t xa,
    int32_t ya,
    int32_t x,
    int32_t y,
    int32_t xb,
    int32_t yb)
{
    uint8_t current;
    uint8_t a;
    uint8_t b;
    int32_t span;
    int32_t displacement;
    int32_t value;

    if (fractus_framebuffer_get_pixel(framebuffer, (uint32_t)x, (uint32_t)y, &current) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    if (current != 0u) {
        return FRACTUS_STATUS_OK;
    }

    if (fractus_framebuffer_get_pixel(framebuffer, (uint32_t)xa, (uint32_t)ya, &a) != FRACTUS_STATUS_OK ||
        fractus_framebuffer_get_pixel(framebuffer, (uint32_t)xb, (uint32_t)yb, &b) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    span = abs(xa - xb) + abs(ya - yb);
    displacement = (params->dispersion * ((rand() % 32768) - 16383) * span) >> 18;
    value = ((int32_t)a + (int32_t)b + 1) / 2 + displacement;

    if (value < (int32_t)params->palette_offset) {
        value = (int32_t)params->palette_offset;
    }

    if (value >= (int32_t)(params->palette_offset + params->palette_span)) {
        value = (int32_t)(params->palette_offset + params->palette_span - 1u);
    }

    return fractus_framebuffer_set_pixel(framebuffer, (uint32_t)x, (uint32_t)y, (uint8_t)value);
}

static fractus_status fractus_plasma_divide(
    fractus_framebuffer *framebuffer,
    const fractus_plasma_params *params,
    int32_t x1,
    int32_t y1,
    int32_t x2,
    int32_t y2)
{
    int32_t x;
    int32_t y;
    uint8_t center;
    uint8_t p1;
    uint8_t p2;
    uint8_t p3;
    uint8_t p4;
    uint8_t average;

    if (x2 - x1 < 2 && y2 - y1 < 2) {
        return FRACTUS_STATUS_OK;
    }

    x = (x1 + x2) >> 1;
    y = (y1 + y2) >> 1;

    if (fractus_plasma_adjust(framebuffer, params, x1, y1, x, y1, x2, y1) != FRACTUS_STATUS_OK ||
        fractus_plasma_adjust(framebuffer, params, x2, y1, x2, y, x2, y2) != FRACTUS_STATUS_OK ||
        fractus_plasma_adjust(framebuffer, params, x1, y2, x, y2, x2, y2) != FRACTUS_STATUS_OK ||
        fractus_plasma_adjust(framebuffer, params, x1, y1, x1, y, x1, y2) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    if (fractus_framebuffer_get_pixel(framebuffer, (uint32_t)x, (uint32_t)y, &center) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    if (center == 0u) {
        if (fractus_framebuffer_get_pixel(framebuffer, (uint32_t)x1, (uint32_t)y1, &p1) != FRACTUS_STATUS_OK ||
            fractus_framebuffer_get_pixel(framebuffer, (uint32_t)x2, (uint32_t)y1, &p2) != FRACTUS_STATUS_OK ||
            fractus_framebuffer_get_pixel(framebuffer, (uint32_t)x2, (uint32_t)y2, &p3) != FRACTUS_STATUS_OK ||
            fractus_framebuffer_get_pixel(framebuffer, (uint32_t)x1, (uint32_t)y2, &p4) != FRACTUS_STATUS_OK) {
            return FRACTUS_STATUS_ERROR;
        }

        average = (uint8_t)(((uint32_t)p1 + (uint32_t)p2 + (uint32_t)p3 + (uint32_t)p4 + 2u) >> 2);
        if (fractus_framebuffer_set_pixel(framebuffer, (uint32_t)x, (uint32_t)y, average) != FRACTUS_STATUS_OK) {
            return FRACTUS_STATUS_ERROR;
        }
    }

    if (fractus_plasma_divide(framebuffer, params, x1, y1, x, y) != FRACTUS_STATUS_OK ||
        fractus_plasma_divide(framebuffer, params, x, y1, x2, y) != FRACTUS_STATUS_OK ||
        fractus_plasma_divide(framebuffer, params, x, y, x2, y2) != FRACTUS_STATUS_OK ||
        fractus_plasma_divide(framebuffer, params, x1, y, x, y2) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_fractal_render_plasma(
    fractus_framebuffer *framebuffer,
    const fractus_plasma_params *params)
{
    if (framebuffer == NULL || params == NULL || !framebuffer->initialized ||
        params->palette_span == 0u || params->dispersion <= 0) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    srand(params->seed);

    if (fractus_framebuffer_clear(framebuffer, 0u) != FRACTUS_STATUS_OK ||
        fractus_framebuffer_set_pixel(framebuffer, 0u, 0u, fractus_plasma_random_color(params)) != FRACTUS_STATUS_OK ||
        fractus_framebuffer_set_pixel(framebuffer, framebuffer->size.width - 1u, 0u, fractus_plasma_random_color(params)) != FRACTUS_STATUS_OK ||
        fractus_framebuffer_set_pixel(framebuffer, framebuffer->size.width - 1u, framebuffer->size.height - 1u, fractus_plasma_random_color(params)) != FRACTUS_STATUS_OK ||
        fractus_framebuffer_set_pixel(framebuffer, 0u, framebuffer->size.height - 1u, fractus_plasma_random_color(params)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return fractus_plasma_divide(
        framebuffer,
        params,
        0,
        0,
        (int32_t)framebuffer->size.width - 1,
        (int32_t)framebuffer->size.height - 1);
}

fractus_status fractus_fractal_render_plasma_circular(
    fractus_framebuffer *framebuffer,
    const fractus_plasma_circular_params *params)
{
    int32_t x;
    int32_t y;
    int32_t n;
    int32_t *center_x;
    int32_t *center_y;
    int32_t *radius;
    uint8_t *color;

    if (framebuffer == NULL || params == NULL || !framebuffer->initialized ||
        params->palette_span == 0u || params->circle_count <= 0 || params->max_radius <= 0) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    center_x = (int32_t *)malloc((size_t)params->circle_count * sizeof(*center_x));
    center_y = (int32_t *)malloc((size_t)params->circle_count * sizeof(*center_y));
    radius = (int32_t *)malloc((size_t)params->circle_count * sizeof(*radius));
    color = (uint8_t *)malloc((size_t)params->circle_count * sizeof(*color));
    if (center_x == NULL || center_y == NULL || radius == NULL || color == NULL) {
        free(center_x);
        free(center_y);
        free(radius);
        free(color);
        return FRACTUS_STATUS_ERROR;
    }

    srand(params->seed);
    for (n = 0; n < params->circle_count; ++n) {
        center_x[n] = (int32_t)((double)framebuffer->size.width * ((double)rand() / (double)RAND_MAX));
        center_y[n] = (int32_t)((double)framebuffer->size.height * ((double)rand() / (double)RAND_MAX));
        radius[n] = (int32_t)((double)params->max_radius * ((double)rand() / (double)RAND_MAX));
        color[n] = (uint8_t)(params->palette_offset + (rand() % params->palette_span));
    }

    for (y = 0; y < (int32_t)framebuffer->size.height; ++y) {
        for (x = 0; x < (int32_t)framebuffer->size.width; ++x) {
            int32_t inside_count = 0;
            uint32_t total = 0u;

            for (n = 0; n < params->circle_count; ++n) {
                int32_t dx = x - center_x[n];
                int32_t dy = y - center_y[n];
                if ((dx * dx) + (dy * dy) <= (radius[n] * radius[n])) {
                    ++inside_count;
                    total += color[n];
                }
            }

            if (fractus_framebuffer_set_pixel(
                    framebuffer,
                    (uint32_t)x,
                    (uint32_t)y,
                    (inside_count > 0) ? (uint8_t)(total / (uint32_t)inside_count) : params->palette_offset) != FRACTUS_STATUS_OK) {
                free(center_x);
                free(center_y);
                free(radius);
                free(color);
                return FRACTUS_STATUS_ERROR;
            }
        }
    }

    free(center_x);
    free(center_y);
    free(radius);
    free(color);
    return FRACTUS_STATUS_OK;
}
