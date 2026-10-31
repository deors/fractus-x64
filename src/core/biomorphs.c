#include "core/biomorphs.h"

#include <math.h>
#include <stdlib.h>

#define FRACTUS_COMPLEX_BIOMORPH
#include "core/complex_step.h"

FRACTUS_COMPLEX_DEFINE_Z2(fractus_d, double, FRACTUS_D_MUL, FRACTUS_D_ADD, FRACTUS_D_SUB, FRACTUS_D_LEFT)
FRACTUS_COMPLEX_DEFINE_Z3(fractus_d, double, FRACTUS_D_MUL, FRACTUS_D_ADD, FRACTUS_D_SUB, FRACTUS_D_LEFT)
FRACTUS_COMPLEX_DEFINE_Z4(fractus_d, double, FRACTUS_D_MUL, FRACTUS_D_ADD, FRACTUS_D_SUB, FRACTUS_D_LEFT)
FRACTUS_COMPLEX_DEFINE_Z5(fractus_d, double, FRACTUS_D_MUL, FRACTUS_D_ADD, FRACTUS_D_SUB, FRACTUS_D_LEFT)
FRACTUS_COMPLEX_DEFINE_SIN(fractus_d, double, FRACTUS_D_ADD, FRACTUS_D_SCALE, sin, cos, FRACTUS_D_HI, FRACTUS_D_TAIL, fractus_clamp_signed)
FRACTUS_COMPLEX_DEFINE_EXP(fractus_d, double, FRACTUS_D_ADD, FRACTUS_D_SCALE, sin, cos, FRACTUS_D_HI, FRACTUS_D_TAIL, fractus_clamp_signed)
FRACTUS_COMPLEX_DEFINE_BIOMORPH(fractus_d, double)

FRACTUS_COMPLEX_DEFINE_Z2(fractus_hp, fractus_hp, FRACTUS_HP_MUL, FRACTUS_HP_ADD, FRACTUS_HP_SUB, FRACTUS_HP_LEFT)
FRACTUS_COMPLEX_DEFINE_Z3(fractus_hp, fractus_hp, FRACTUS_HP_MUL, FRACTUS_HP_ADD, FRACTUS_HP_SUB, FRACTUS_HP_LEFT)
FRACTUS_COMPLEX_DEFINE_Z4(fractus_hp, fractus_hp, FRACTUS_HP_MUL, FRACTUS_HP_ADD, FRACTUS_HP_SUB, FRACTUS_HP_LEFT)
FRACTUS_COMPLEX_DEFINE_Z5(fractus_hp, fractus_hp, FRACTUS_HP_MUL, FRACTUS_HP_ADD, FRACTUS_HP_SUB, FRACTUS_HP_LEFT)
FRACTUS_COMPLEX_DEFINE_SIN(fractus_hp, fractus_hp, FRACTUS_HP_ADD, FRACTUS_HP_SCALE, fractus_hp_sin, fractus_hp_cos, FRACTUS_HP_HI, FRACTUS_HP_TAIL, fractus_hp_clamp_signed)
FRACTUS_COMPLEX_DEFINE_EXP(fractus_hp, fractus_hp, FRACTUS_HP_ADD, FRACTUS_HP_SCALE, fractus_hp_sin, fractus_hp_cos, FRACTUS_HP_HI, FRACTUS_HP_TAIL, fractus_hp_clamp_signed)
FRACTUS_COMPLEX_DEFINE_BIOMORPH(fractus_hp, fractus_hp)

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

static inline int fractus_biomorph_check_trap(
    double zr,
    double zi,
    double cutoff,
    fractus_biomorph_trap_mode mode);

static fractus_status fractus_biomorph_render_extended(
    fractus_framebuffer *framebuffer,
    const fractus_biomorph_params *params,
    const fractus_hp_window *window,
    const fractus_render_progress *progress)
{
    uint32_t x;
    uint32_t y;
    fractus_hp dx;
    fractus_hp dy;
    fractus_hp cr = fractus_hp_from_double(params->constant_real);
    fractus_hp ci = fractus_hp_from_double(params->constant_imag);
    double cutoff = (params->cutoff > 0.0) ? params->cutoff : 1.0;

    fractus_hp_plane_axes(window, framebuffer->size.width, framebuffer->size.height, &dx, &dy);

    {
        const uint32_t fractus_row_count = framebuffer->size.height;
        const uint32_t fractus_row_band = fractus_render_band_rows(progress, fractus_row_count);
        const int fractus_row_chunk = (progress == NULL || progress->report == NULL) ? 8 : 1;
        uint32_t fractus_row_begin = 0u;

        (void)fractus_row_chunk;
        while (fractus_row_begin < fractus_row_count) {
            uint32_t fractus_row_end = fractus_row_begin + fractus_row_band;
            if (fractus_row_end > fractus_row_count) {
                fractus_row_end = fractus_row_count;
            }

            #pragma omp parallel for schedule(dynamic, fractus_row_chunk) private(x)
            for (y = fractus_row_begin; y < fractus_row_end; ++y) {
        fractus_hp zi_row = fractus_hp_plane_imag(window, dy, y);
        for (x = 0u; x < framebuffer->size.width; ++x) {
            fractus_hp zr = fractus_hp_plane_real(window, dx, x);
            fractus_hp zi = zi_row;
            uint32_t iteration;
            uint8_t color_index = params->background_color_index;

            for (iteration = 1u; iteration <= params->max_iterations; ++iteration) {
                fractus_hp mag = fractus_hp_add(fractus_hp_mul(zr, zr), fractus_hp_mul(zi, zi));
                fractus_hp next_r;
                fractus_hp next_i;

                if (fractus_hp_greater_double(mag, params->escape_radius_squared) || !isfinite(mag.hi)) {
                    if (fractus_biomorph_check_trap(zr.hi, zi.hi, cutoff, params->trap_mode)) {
                        color_index = fractus_biomorph_iteration_color(
                            iteration,
                            params->palette_offset,
                            params->palette_span);
                    }
                    break;
                }

                fractus_hp_biomorph_step(params->equation, zr, zi, cr, ci, &next_r, &next_i);
                zr = next_r;
                zi = next_i;
            }

            if (iteration > params->max_iterations &&
                fractus_biomorph_check_trap(zr.hi, zi.hi, cutoff, params->trap_mode)) {
                color_index = fractus_biomorph_iteration_color(
                    params->max_iterations,
                    params->palette_offset,
                    params->palette_span);
            }

            framebuffer->index_pixels[(size_t)y * framebuffer->pitch_pixels + x] = color_index;
        }
    }
            if (fractus_render_progress_report(progress, fractus_row_end, fractus_row_count) != 0) {
                framebuffer->pixels_dirty = 1;
                return FRACTUS_STATUS_CANCELLED;
            }
            fractus_row_begin = fractus_row_end;
        }
    }

    framebuffer->pixels_dirty = 1;
    return FRACTUS_STATUS_OK;
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

fractus_status fractus_fractal_render_biomorph_with_progress(
    fractus_framebuffer *framebuffer,
    const fractus_biomorph_params *params,
    const fractus_render_progress *progress)
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

    {
        fractus_hp_window window;
        if (fractus_hp_window_use_extended(
                &window,
                &params->precise,
                params->xmin,
                params->xmax,
                params->ymin,
                params->ymax,
                framebuffer->size.width,
                framebuffer->size.height)) {
            return fractus_biomorph_render_extended(framebuffer, params, &window, progress);
        }
    }

    dx = (params->xmax - params->xmin) / (double)(framebuffer->size.width - 1u);
    dy = (params->ymax - params->ymin) / (double)(framebuffer->size.height - 1u);
    cutoff = (params->cutoff > 0.0) ? params->cutoff : 1.0;

    {
        const uint32_t fractus_row_count = framebuffer->size.height;
        const uint32_t fractus_row_band = fractus_render_band_rows(progress, fractus_row_count);
        const int fractus_row_chunk = (progress == NULL || progress->report == NULL) ? 8 : 1;
        uint32_t fractus_row_begin = 0u;

        (void)fractus_row_chunk;
        while (fractus_row_begin < fractus_row_count) {
            uint32_t fractus_row_end = fractus_row_begin + fractus_row_band;
            if (fractus_row_end > fractus_row_count) {
                fractus_row_end = fractus_row_count;
            }

            #pragma omp parallel for schedule(dynamic, fractus_row_chunk) private(x)
            for (y = fractus_row_begin; y < fractus_row_end; ++y) {
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

                fractus_d_biomorph_step(
                    params->equation,
                    zr,
                    zi,
                    params->constant_real,
                    params->constant_imag,
                    &next_r,
                    &next_i);

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
            if (fractus_render_progress_report(progress, fractus_row_end, fractus_row_count) != 0) {
                framebuffer->pixels_dirty = 1;
                return FRACTUS_STATUS_CANCELLED;
            }
            fractus_row_begin = fractus_row_end;
        }
    }

    framebuffer->pixels_dirty = 1;
    return FRACTUS_STATUS_OK;
}

fractus_status fractus_fractal_render_biomorph(
    fractus_framebuffer *framebuffer,
    const fractus_biomorph_params *params)
{
    return fractus_fractal_render_biomorph_with_progress(framebuffer, params, NULL);
}

int fractus_biomorph_precision_paths_match(void)
{
    fractus_framebuffer fast;
    fractus_framebuffer extended;
    fractus_size_u32 size = {40u, 30u};
    fractus_biomorph_params params = {
        -2.0, 2.0, -1.5, 1.5,
        -0.6, 0.55,
        40u, 1000.0, 1.0,
        0u, FRACTUS_PALETTE_OFFSET, FRACTUS_PALETTE_SPAN,
        FRACTUS_BIOMORPH_EQ_Z2,
        FRACTUS_BIOMORPH_TRAP_RE_OR_IM,
        FRACTUS_HP_WINDOW_ZERO
    };
    int equation;
    int match = 1;

    if (fractus_framebuffer_init(&fast, size) != FRACTUS_STATUS_OK) {
        return 0;
    }
    if (fractus_framebuffer_init(&extended, size) != FRACTUS_STATUS_OK) {
        fractus_framebuffer_shutdown(&fast);
        return 0;
    }

    for (equation = (int)FRACTUS_BIOMORPH_EQ_Z2; equation <= (int)FRACTUS_BIOMORPH_EQ_EXP_Z; ++equation) {
        fractus_hp_window window = {0};

        params.equation = (fractus_biomorph_equation)equation;
        if (fractus_fractal_render_biomorph(&fast, &params) != FRACTUS_STATUS_OK) {
            match = 0;
            break;
        }
        fractus_hp_window_adopt_doubles(&window, params.xmin, params.xmax, params.ymin, params.ymax);
        if (fractus_biomorph_render_extended(&extended, &params, &window, NULL) != FRACTUS_STATUS_OK ||
            !fractus_precision_indices_match(&fast, &extended)) {
            match = 0;
            break;
        }
    }

    fractus_framebuffer_shutdown(&extended);
    fractus_framebuffer_shutdown(&fast);
    return match;
}
