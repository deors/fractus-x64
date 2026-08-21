#include "core/plasma.h"

#include <math.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
    double *center_x;
    double *center_y;
    double *inv_wavelength;
    double *phase;
    double *weight;
    double *sum_buffer;
    double base_radius;
    double s_min = 1e30;
    double s_max = -1e30;
    double range;
    size_t total_pixels;

    if (framebuffer == NULL || params == NULL || !framebuffer->initialized ||
        params->palette_span == 0u || params->circle_count <= 0 || params->max_radius <= 0) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    total_pixels = (size_t)framebuffer->size.width * (size_t)framebuffer->size.height;
    center_x = (double *)malloc((size_t)params->circle_count * sizeof(*center_x));
    center_y = (double *)malloc((size_t)params->circle_count * sizeof(*center_y));
    inv_wavelength = (double *)malloc((size_t)params->circle_count * sizeof(*inv_wavelength));
    phase = (double *)malloc((size_t)params->circle_count * sizeof(*phase));
    weight = (double *)malloc((size_t)params->circle_count * sizeof(*weight));
    sum_buffer = (double *)malloc(total_pixels * sizeof(*sum_buffer));

    if (center_x == NULL || center_y == NULL || inv_wavelength == NULL || phase == NULL || weight == NULL || sum_buffer == NULL) {
        free(center_x);
        free(center_y);
        free(inv_wavelength);
        free(phase);
        free(weight);
        free(sum_buffer);
        return FRACTUS_STATUS_ERROR;
    }

    srand(params->seed);
    base_radius = (double)params->max_radius;
    if (base_radius < 2.0) {
        base_radius = 2.0;
    }

    for (n = 0; n < params->circle_count; ++n) {
        double radius_factor;
        double wavelength;

        center_x[n] = ((double)rand() / (double)RAND_MAX) * (double)framebuffer->size.width;
        center_y[n] = ((double)rand() / (double)RAND_MAX) * (double)framebuffer->size.height;

        radius_factor = 0.4 + (((double)rand() / (double)RAND_MAX) * 1.2);
        wavelength = base_radius * radius_factor;
        if (wavelength < 2.0) {
            wavelength = 2.0;
        }

        inv_wavelength[n] = (2.0 * M_PI) / wavelength;
        phase[n] = ((double)rand() / (double)RAND_MAX) * 2.0 * M_PI;
        weight[n] = 0.5 + (((double)rand() / (double)RAND_MAX) * 0.5);
    }

    #pragma omp parallel for schedule(dynamic, 8) private(x, n) reduction(min:s_min) reduction(max:s_max)
    for (y = 0; y < (int32_t)framebuffer->size.height; ++y) {
        for (x = 0; x < (int32_t)framebuffer->size.width; ++x) {
            double sum = 0.0;
            size_t idx = (size_t)y * (size_t)framebuffer->size.width + (size_t)x;

            for (n = 0; n < params->circle_count; ++n) {
                double dx = (double)x - center_x[n];
                double dy = (double)y - center_y[n];
                double dist = sqrt((dx * dx) + (dy * dy));
                sum += weight[n] * sin((dist * inv_wavelength[n]) + phase[n]);
            }

            sum_buffer[idx] = sum;
            if (sum < s_min) {
                s_min = sum;
            }
            if (sum > s_max) {
                s_max = sum;
            }
        }
    }

    range = s_max - s_min;
    if (range <= 0.0) {
        range = 1.0;
    }

    #pragma omp parallel for schedule(dynamic, 8) private(x)
    for (y = 0; y < (int32_t)framebuffer->size.height; ++y) {
        for (x = 0; x < (int32_t)framebuffer->size.width; ++x) {
            size_t idx = (size_t)y * (size_t)framebuffer->size.width + (size_t)x;
            double normalized = (sum_buffer[idx] - s_min) / range;
            uint8_t color_index;

            if (normalized < 0.0) {
                normalized = 0.0;
            } else if (normalized > 1.0) {
                normalized = 1.0;
            }

            color_index = (uint8_t)(params->palette_offset + (uint32_t)(normalized * (double)(params->palette_span - 1u)));
            framebuffer->index_pixels[(size_t)y * framebuffer->pitch_pixels + (size_t)x] = color_index;
        }
    }

    free(center_x);
    free(center_y);
    free(inv_wavelength);
    free(phase);
    free(weight);
    free(sum_buffer);

    framebuffer->pixels_dirty = 1;
    return FRACTUS_STATUS_OK;
}

fractus_status fractus_fractal_render_plasma_circular_legacy(
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
