#include "core/julia.h"

#include <math.h>
#include <stdlib.h>

#define FRACTUS_COMPLEX_DEM
#include "core/complex_step.h"

FRACTUS_COMPLEX_DEFINE_Z2(fractus_d, double, FRACTUS_D_MUL, FRACTUS_D_ADD, FRACTUS_D_SUB, FRACTUS_D_LEFT)
FRACTUS_COMPLEX_DEFINE_DZ(fractus_d, double, FRACTUS_D_MUL, FRACTUS_D_ADD, FRACTUS_D_SUB, FRACTUS_D_LEFT)
FRACTUS_COMPLEX_DEFINE_Z2(fractus_hp, fractus_hp, FRACTUS_HP_MUL, FRACTUS_HP_ADD, FRACTUS_HP_SUB, FRACTUS_HP_LEFT)
FRACTUS_COMPLEX_DEFINE_DZ(fractus_hp, fractus_hp, FRACTUS_HP_MUL, FRACTUS_HP_ADD, FRACTUS_HP_SUB, FRACTUS_HP_LEFT)

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

static fractus_status fractus_julia_render_extended(
    fractus_framebuffer *framebuffer,
    const fractus_julia_params *params,
    const fractus_hp_window *window,
    const fractus_render_progress *progress)
{
    uint32_t x;
    uint32_t y;
    fractus_hp dx;
    fractus_hp dy;
    fractus_hp cr = fractus_hp_from_double(params->constant_real);
    fractus_hp ci = fractus_hp_from_double(params->constant_imag);

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
            uint8_t color_index;

            for (iteration = 1u; iteration <= params->max_iterations; ++iteration) {
                fractus_hp zr2 = fractus_hp_mul(zr, zr);
                fractus_hp zi2 = fractus_hp_mul(zi, zi);
                fractus_hp mag = fractus_hp_add(zr2, zi2);
                fractus_hp next_zi;
                fractus_hp next_zr;

                if (fractus_hp_greater_double(mag, params->escape_radius_squared)) {
                    break;
                }

                fractus_hp_z2(zr, zi, cr, ci, &next_zr, &next_zi);
                zi = next_zi;
                zr = next_zr;
            }

            if (iteration > params->max_iterations) {
                color_index = params->inside_color_index;
            } else if (params->color_mode == FRACTUS_JULIA_COLOR_SMOOTH) {
                color_index = fractus_julia_smooth_iteration_color(
                    iteration,
                    zr.hi,
                    zi.hi,
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

static fractus_status fractus_julia_dem_render_extended(
    fractus_framebuffer *framebuffer,
    const fractus_julia_dem_params *params,
    const fractus_hp_window *window,
    const fractus_render_progress *progress)
{
    uint32_t x;
    uint32_t y;
    fractus_hp dx;
    fractus_hp dy;
    double pixel_size;
    fractus_hp cr = fractus_hp_from_double(params->constant_real);
    fractus_hp ci = fractus_hp_from_double(params->constant_imag);
    fractus_hp one = fractus_hp_from_double(1.0);

    fractus_hp_plane_axes(window, framebuffer->size.width, framebuffer->size.height, &dx, &dy);
    pixel_size = fractus_dem_pixel_size(fractus_hp_to_double(dx), fractus_hp_to_double(dy));

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
            fractus_hp dzr = fractus_hp_from_double(1.0);
            fractus_hp dzi = fractus_hp_from_double(0.0);
            uint32_t iteration;
            uint8_t color_index;

            for (iteration = 1u; iteration <= params->max_iterations; ++iteration) {
                fractus_hp zr2 = fractus_hp_mul(zr, zr);
                fractus_hp zi2 = fractus_hp_mul(zi, zi);
                fractus_hp mag = fractus_hp_add(zr2, zi2);
                fractus_hp next_dzr;
                fractus_hp next_dzi;
                fractus_hp next_zi;
                fractus_hp next_zr;

                if (fractus_hp_greater_double(mag, params->escape_radius_squared)) {
                    break;
                }

                fractus_hp_dz(zr, zi, dzr, dzi, one, 0, &next_dzr, &next_dzi);
                dzr = next_dzr;
                dzi = next_dzi;
                fractus_hp_z2(zr, zi, cr, ci, &next_zr, &next_zi);
                zr = next_zr;
                zi = next_zi;
            }

            if (iteration > params->max_iterations) {
                color_index = params->inside_color_index;
            } else {
                color_index = fractus_dem_distance_color(
                    params->color_mode == FRACTUS_JULIA_DEM_COLOR_GRADIENT,
                    zr.hi,
                    zi.hi,
                    dzr.hi,
                    dzi.hi,
                    pixel_size,
                    iteration,
                    params->inside_color_index,
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

fractus_status fractus_fractal_render_julia_with_progress(
    fractus_framebuffer *framebuffer,
    const fractus_julia_params *params,
    const fractus_render_progress *progress)
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
            return fractus_julia_render_extended(framebuffer, params, &window, progress);
        }
    }

    dx = (params->xmax - params->xmin) / (double)(framebuffer->size.width - 1u);
    dy = (params->ymax - params->ymin) / (double)(framebuffer->size.height - 1u);

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
            uint8_t color_index;

            for (iteration = 1u; iteration <= params->max_iterations; ++iteration) {
                double zr2 = zr * zr;
                double zi2 = zi * zi;

                double next_zr;
                double next_zi;

                if (zr2 + zi2 > params->escape_radius_squared) {
                    break;
                }

                fractus_d_z2(zr, zi, params->constant_real, params->constant_imag, &next_zr, &next_zi);
                zr = next_zr;
                zi = next_zi;
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

fractus_status fractus_fractal_render_julia(
    fractus_framebuffer *framebuffer,
    const fractus_julia_params *params)
{
    return fractus_fractal_render_julia_with_progress(framebuffer, params, NULL);
}

fractus_status fractus_fractal_render_julia_dem_with_progress(
    fractus_framebuffer *framebuffer,
    const fractus_julia_dem_params *params,
    const fractus_render_progress *progress)
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
            return fractus_julia_dem_render_extended(framebuffer, params, &window, progress);
        }
    }

    dx = (params->xmax - params->xmin) / (double)(framebuffer->size.width - 1u);
    dy = (params->ymax - params->ymin) / (double)(framebuffer->size.height - 1u);
    pixel_size = fractus_dem_pixel_size(dx, dy);

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
            double dzr = 1.0;
            double dzi = 0.0;
            double cr = params->constant_real;
            double ci = params->constant_imag;
            uint32_t iteration;
            uint8_t color_index;

            for (iteration = 1u; iteration <= params->max_iterations; ++iteration) {
                double zr2 = zr * zr;
                double zi2 = zi * zi;

                double next_dzr;
                double next_dzi;
                double next_zr;
                double next_zi;

                if (zr2 + zi2 > params->escape_radius_squared) {
                    break;
                }

                fractus_d_dz(zr, zi, dzr, dzi, 1.0, 0, &next_dzr, &next_dzi);
                dzr = next_dzr;
                dzi = next_dzi;
                fractus_d_z2(zr, zi, cr, ci, &next_zr, &next_zi);
                zr = next_zr;
                zi = next_zi;
            }

            if (iteration > params->max_iterations) {
                color_index = params->inside_color_index;
            } else {
                color_index = fractus_dem_distance_color(
                    params->color_mode == FRACTUS_JULIA_DEM_COLOR_GRADIENT,
                    zr,
                    zi,
                    dzr,
                    dzi,
                    pixel_size,
                    iteration,
                    params->inside_color_index,
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

fractus_status fractus_fractal_render_julia_dem(
    fractus_framebuffer *framebuffer,
    const fractus_julia_dem_params *params)
{
    return fractus_fractal_render_julia_dem_with_progress(framebuffer, params, NULL);
}

static int fractus_julia_buffers_match(
    fractus_framebuffer *fast,
    fractus_framebuffer *extended,
    const fractus_julia_params *params)
{
    fractus_hp_window window = {0};

    if (fractus_fractal_render_julia(fast, params) != FRACTUS_STATUS_OK) {
        return 0;
    }
    fractus_hp_window_adopt_doubles(&window, params->xmin, params->xmax, params->ymin, params->ymax);
    if (fractus_julia_render_extended(extended, params, &window, NULL) != FRACTUS_STATUS_OK) {
        return 0;
    }
    return fractus_precision_indices_match(fast, extended);
}

static int fractus_julia_dem_buffers_match(
    fractus_framebuffer *fast,
    fractus_framebuffer *extended,
    const fractus_julia_dem_params *params)
{
    fractus_hp_window window = {0};

    if (fractus_fractal_render_julia_dem(fast, params) != FRACTUS_STATUS_OK) {
        return 0;
    }
    fractus_hp_window_adopt_doubles(&window, params->xmin, params->xmax, params->ymin, params->ymax);
    if (fractus_julia_dem_render_extended(extended, params, &window, NULL) != FRACTUS_STATUS_OK) {
        return 0;
    }
    return fractus_precision_indices_match(fast, extended);
}

int fractus_julia_precision_paths_match(void)
{
    fractus_framebuffer fast;
    fractus_framebuffer extended;
    fractus_size_u32 size = {48u, 36u};
    fractus_julia_params julia = {
        -1.5, 1.5, -1.2, 1.2,
        -0.7, 0.27015,
        80u, 4.0, 0u, FRACTUS_PALETTE_OFFSET, FRACTUS_PALETTE_SPAN,
        FRACTUS_JULIA_COLOR_ESCAPE,
        FRACTUS_HP_WINDOW_ZERO
    };
    fractus_julia_dem_params dem = {
        -1.5, 1.5, -1.2, 1.2,
        -0.7, 0.27015,
        80u, 100.0, 0u, FRACTUS_PALETTE_OFFSET, FRACTUS_PALETTE_SPAN,
        FRACTUS_JULIA_DEM_COLOR_BOUNDARY,
        FRACTUS_HP_WINDOW_ZERO
    };
    int match;

    if (fractus_framebuffer_init(&fast, size) != FRACTUS_STATUS_OK) {
        return 0;
    }
    if (fractus_framebuffer_init(&extended, size) != FRACTUS_STATUS_OK) {
        fractus_framebuffer_shutdown(&fast);
        return 0;
    }

    match = fractus_julia_buffers_match(&fast, &extended, &julia);
    julia.color_mode = FRACTUS_JULIA_COLOR_SMOOTH;
    match = match && fractus_julia_buffers_match(&fast, &extended, &julia);
    match = match && fractus_julia_dem_buffers_match(&fast, &extended, &dem);
    dem.color_mode = FRACTUS_JULIA_DEM_COLOR_GRADIENT;
    match = match && fractus_julia_dem_buffers_match(&fast, &extended, &dem);

    fractus_framebuffer_shutdown(&extended);
    fractus_framebuffer_shutdown(&fast);
    return match;
}
