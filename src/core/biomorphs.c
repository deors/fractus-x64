#include "core/biomorphs.h"

#include <math.h>
#include <stdlib.h>

static fractus_status fractus_biomorph_validate_common(
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

static uint8_t fractus_biomorph_iteration_color(
    uint32_t iteration,
    uint8_t palette_offset,
    uint8_t palette_span)
{
    return (uint8_t)(palette_offset + ((iteration - 1u) % palette_span));
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
        fractus_biomorph_validate_common(
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
                        color_index = fractus_biomorph_iteration_color(
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
                    color_index = fractus_biomorph_iteration_color(
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
