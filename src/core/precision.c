#include "core/precision.h"

#include "core/core.h"

#include <stdio.h>
#include <string.h>

void fractus_hp_window_adopt_doubles(
    fractus_hp_window *window,
    double xmin,
    double xmax,
    double ymin,
    double ymax)
{
    if (window == NULL) {
        return;
    }

    if (window->valid &&
        window->xmin.hi == xmin &&
        window->xmax.hi == xmax &&
        window->ymin.hi == ymin &&
        window->ymax.hi == ymax) {
        return;
    }

    window->xmin = fractus_hp_from_double(xmin);
    window->xmax = fractus_hp_from_double(xmax);
    window->ymin = fractus_hp_from_double(ymin);
    window->ymax = fractus_hp_from_double(ymax);
    window->valid = (xmax > xmin && ymax > ymin) ? 1 : 0;
}

void fractus_hp_window_publish_doubles(
    const fractus_hp_window *window,
    double *xmin,
    double *xmax,
    double *ymin,
    double *ymax)
{
    if (window == NULL || xmin == NULL || xmax == NULL || ymin == NULL || ymax == NULL || !window->valid) {
        return;
    }

    *xmin = window->xmin.hi;
    *xmax = window->xmax.hi;
    *ymin = window->ymin.hi;
    *ymax = window->ymax.hi;
}

int fractus_hp_window_spans_positive(const fractus_hp_window *window)
{
    fractus_hp span_x;
    fractus_hp span_y;

    if (window == NULL || !window->valid) {
        return 0;
    }

    span_x = fractus_hp_sub(window->xmax, window->xmin);
    span_y = fractus_hp_sub(window->ymax, window->ymin);
    return fractus_hp_is_positive(span_x) && fractus_hp_is_positive(span_y);
}

int fractus_hp_window_equal(const fractus_hp_window *a, const fractus_hp_window *b)
{
    if (a == NULL || b == NULL) {
        return 0;
    }

    if (!a->valid && !b->valid) {
        return 1;
    }

    if (!a->valid || !b->valid) {
        return 0;
    }

    return a->xmin.hi == b->xmin.hi && a->xmin.lo == b->xmin.lo &&
        a->xmax.hi == b->xmax.hi && a->xmax.lo == b->xmax.lo &&
        a->ymin.hi == b->ymin.hi && a->ymin.lo == b->ymin.lo &&
        a->ymax.hi == b->ymax.hi && a->ymax.lo == b->ymax.lo;
}

int fractus_hp_window_needs_extended(
    const fractus_hp_window *window,
    uint32_t width,
    uint32_t height)
{
    fractus_hp span_x;
    fractus_hp span_y;
    double step_x;
    double step_y;
    double scale_x;
    double scale_y;

    if (window == NULL || !window->valid || width < 2u || height < 2u ||
        !fractus_hp_window_spans_positive(window)) {
        return 0;
    }

    span_x = fractus_hp_sub(window->xmax, window->xmin);
    span_y = fractus_hp_sub(window->ymax, window->ymin);
    step_x = fractus_hp_to_double(span_x) / (double)(width - 1u);
    step_y = fractus_hp_to_double(span_y) / (double)(height - 1u);
    scale_x = fabs(window->xmin.hi);
    if (fabs(window->xmax.hi) > scale_x) {
        scale_x = fabs(window->xmax.hi);
    }
    if (scale_x < 1.0) {
        scale_x = 1.0;
    }
    scale_y = fabs(window->ymin.hi);
    if (fabs(window->ymax.hi) > scale_y) {
        scale_y = fabs(window->ymax.hi);
    }
    if (scale_y < 1.0) {
        scale_y = 1.0;
    }

    if (!(step_x > 0.0) || step_x < scale_x * FRACTUS_HP_RELATIVE_STEP) {
        return 1;
    }
    if (!(step_y > 0.0) || step_y < scale_y * FRACTUS_HP_RELATIVE_STEP) {
        return 1;
    }
    return 0;
}

int fractus_hp_window_use_extended(
    fractus_hp_window *window,
    const fractus_hp_window *stored,
    double xmin,
    double xmax,
    double ymin,
    double ymax,
    uint32_t width,
    uint32_t height)
{
    if (window == NULL) {
        return 0;
    }

    if (stored != NULL) {
        *window = *stored;
    } else {
        *window = (fractus_hp_window){0};
    }
    fractus_hp_window_adopt_doubles(window, xmin, xmax, ymin, ymax);
    return fractus_hp_window_needs_extended(window, width, height);
}

int fractus_hp_plane_pixel(
    const fractus_hp_window *window,
    uint32_t width,
    uint32_t height,
    uint32_t x,
    uint32_t y,
    fractus_hp *real_part,
    fractus_hp *imag_part)
{
    fractus_hp span_x;
    fractus_hp span_y;
    fractus_hp dx;
    fractus_hp dy;

    if (window == NULL || real_part == NULL || imag_part == NULL ||
        width < 2u || height < 2u || x >= width || y >= height ||
        !fractus_hp_window_spans_positive(window)) {
        return 0;
    }

    span_x = fractus_hp_sub(window->xmax, window->xmin);
    span_y = fractus_hp_sub(window->ymax, window->ymin);
    dx = fractus_hp_div_double(span_x, (double)(width - 1u));
    dy = fractus_hp_div_double(span_y, (double)(height - 1u));
    *real_part = fractus_hp_add(window->xmin, fractus_hp_mul_double(dx, (double)x));
    *imag_part = fractus_hp_sub(window->ymax, fractus_hp_mul_double(dy, (double)y));
    return 1;
}

int fractus_hp_plane_pan(
    fractus_hp_window *window,
    uint32_t width,
    uint32_t height,
    int32_t from_x,
    int32_t from_y,
    int32_t to_x,
    int32_t to_y)
{
    fractus_hp span_x;
    fractus_hp span_y;
    fractus_hp dx;
    fractus_hp dy;
    fractus_hp shift_x;
    fractus_hp shift_y;
    fractus_hp_window next;

    if (window == NULL || width < 2u || height < 2u ||
        (from_x == to_x && from_y == to_y) ||
        !fractus_hp_window_spans_positive(window)) {
        return 0;
    }

    span_x = fractus_hp_sub(window->xmax, window->xmin);
    span_y = fractus_hp_sub(window->ymax, window->ymin);
    dx = fractus_hp_div_double(span_x, (double)(width - 1u));
    dy = fractus_hp_div_double(span_y, (double)(height - 1u));
    shift_x = fractus_hp_mul_double(dx, (double)(to_x - from_x));
    shift_y = fractus_hp_mul_double(dy, (double)(to_y - from_y));
    if (shift_x.hi == 0.0 && shift_x.lo == 0.0 && shift_y.hi == 0.0 && shift_y.lo == 0.0) {
        return 0;
    }

    next = *window;
    next.xmin = fractus_hp_sub(window->xmin, shift_x);
    next.xmax = fractus_hp_sub(window->xmax, shift_x);
    next.ymin = fractus_hp_add(window->ymin, shift_y);
    next.ymax = fractus_hp_add(window->ymax, shift_y);
    next.valid = 1;
    if (!fractus_hp_window_spans_positive(&next)) {
        return 0;
    }

    *window = next;
    return 1;
}

int fractus_hp_plane_zoom_at_pixel(
    fractus_hp_window *window,
    uint32_t width,
    uint32_t height,
    uint32_t x,
    uint32_t y,
    double steps)
{
    return fractus_hp_plane_zoom_at_pixel_floor(
        window,
        width,
        height,
        x,
        y,
        steps,
        FRACTUS_HP_PLANE_MIN_SPAN);
}

int fractus_hp_plane_zoom_at_pixel_floor(
    fractus_hp_window *window,
    uint32_t width,
    uint32_t height,
    uint32_t x,
    uint32_t y,
    double steps,
    double min_span)
{
    fractus_hp span_x;
    fractus_hp span_y;
    double span_x_d;
    double span_y_d;
    double factor;
    double factor_min;
    double factor_max;
    fractus_hp dx;
    fractus_hp dy;
    fractus_hp center_x;
    fractus_hp center_y;
    fractus_hp new_span_x;
    fractus_hp new_span_y;
    fractus_hp new_dx;
    fractus_hp new_dy;
    fractus_hp_window next;

    if (window == NULL || width < 2u || height < 2u || x >= width || y >= height ||
        !(steps > 0.0 || steps < 0.0) || !(min_span > 0.0) ||
        !fractus_hp_window_spans_positive(window)) {
        return 0;
    }

    if (steps > 16.0) {
        steps = 16.0;
    } else if (steps < -16.0) {
        steps = -16.0;
    }

    span_x = fractus_hp_sub(window->xmax, window->xmin);
    span_y = fractus_hp_sub(window->ymax, window->ymin);
    span_x_d = fractus_hp_to_double(span_x);
    span_y_d = fractus_hp_to_double(span_y);
    if (!(span_x_d > 0.0) || !(span_y_d > 0.0)) {
        return 0;
    }

    factor = pow(FRACTUS_PLANE_WHEEL_ZOOM_STEP, steps);
    if (!(factor > 0.0)) {
        return 0;
    }

    factor_min = span_x_d / FRACTUS_PLANE_MAX_SPAN;
    if (span_y_d / FRACTUS_PLANE_MAX_SPAN > factor_min) {
        factor_min = span_y_d / FRACTUS_PLANE_MAX_SPAN;
    }
    factor_max = span_x_d / min_span;
    if (span_y_d / min_span < factor_max) {
        factor_max = span_y_d / min_span;
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

    dx = fractus_hp_div_double(span_x, (double)(width - 1u));
    dy = fractus_hp_div_double(span_y, (double)(height - 1u));
    center_x = fractus_hp_add(window->xmin, fractus_hp_mul_double(dx, (double)x));
    center_y = fractus_hp_sub(window->ymax, fractus_hp_mul_double(dy, (double)y));
    new_span_x = fractus_hp_div_double(span_x, factor);
    new_span_y = fractus_hp_div_double(span_y, factor);
    new_dx = fractus_hp_div_double(new_span_x, (double)(width - 1u));
    new_dy = fractus_hp_div_double(new_span_y, (double)(height - 1u));

    next = *window;
    next.xmin = fractus_hp_sub(center_x, fractus_hp_mul_double(new_dx, (double)x));
    next.xmax = fractus_hp_add(next.xmin, new_span_x);
    next.ymax = fractus_hp_add(center_y, fractus_hp_mul_double(new_dy, (double)y));
    next.ymin = fractus_hp_sub(next.ymax, new_span_y);
    next.valid = 1;
    if (!fractus_hp_window_spans_positive(&next)) {
        return 0;
    }

    *window = next;
    return 1;
}

int fractus_hp_plane_zoom_centered(
    fractus_hp_window *window,
    uint32_t width,
    uint32_t height,
    double steps)
{
    fractus_hp mid_x;
    fractus_hp mid_y;
    fractus_hp shifted_mid_x;
    fractus_hp shifted_mid_y;
    fractus_hp shift_x;
    fractus_hp shift_y;
    uint32_t x;
    uint32_t y;

    if (window == NULL || width < 2u || height < 2u || !fractus_hp_window_spans_positive(window)) {
        return 0;
    }

    x = (width - 1u) / 2u;
    y = (height - 1u) / 2u;
    mid_x = fractus_hp_div_double(fractus_hp_add(window->xmin, window->xmax), 2.0);
    mid_y = fractus_hp_div_double(fractus_hp_add(window->ymin, window->ymax), 2.0);
    if (!fractus_hp_plane_zoom_at_pixel(window, width, height, x, y, steps)) {
        return 0;
    }

    shifted_mid_x = fractus_hp_div_double(fractus_hp_add(window->xmin, window->xmax), 2.0);
    shifted_mid_y = fractus_hp_div_double(fractus_hp_add(window->ymin, window->ymax), 2.0);
    shift_x = fractus_hp_sub(shifted_mid_x, mid_x);
    shift_y = fractus_hp_sub(shifted_mid_y, mid_y);
    window->xmin = fractus_hp_sub(window->xmin, shift_x);
    window->xmax = fractus_hp_sub(window->xmax, shift_x);
    window->ymin = fractus_hp_sub(window->ymin, shift_y);
    window->ymax = fractus_hp_sub(window->ymax, shift_y);
    window->valid = 1;
    return fractus_hp_window_spans_positive(window);
}

int fractus_hp_plane_apply_selection(
    fractus_hp_window *window,
    uint32_t width,
    uint32_t height,
    int32_t x_min,
    int32_t x_max,
    int32_t y_min,
    int32_t y_max)
{
    fractus_hp span_x;
    fractus_hp span_y;
    fractus_hp dx;
    fractus_hp dy;
    fractus_hp_window next;

    if (window == NULL || width < 2u || height < 2u || x_min == x_max || y_min == y_max ||
        !fractus_hp_window_spans_positive(window)) {
        return 0;
    }

    span_x = fractus_hp_sub(window->xmax, window->xmin);
    span_y = fractus_hp_sub(window->ymax, window->ymin);
    dx = fractus_hp_div_double(span_x, (double)(width - 1u));
    dy = fractus_hp_div_double(span_y, (double)(height - 1u));
    next = *window;
    next.xmin = fractus_hp_add(window->xmin, fractus_hp_mul_double(dx, (double)x_min));
    next.xmax = fractus_hp_add(window->xmin, fractus_hp_mul_double(dx, (double)x_max));
    next.ymax = fractus_hp_sub(window->ymax, fractus_hp_mul_double(dy, (double)y_min));
    next.ymin = fractus_hp_sub(window->ymax, fractus_hp_mul_double(dy, (double)y_max));
    next.valid = 1;
    if (!fractus_hp_window_spans_positive(&next)) {
        return 0;
    }

    *window = next;
    return 1;
}

static void fractus_hp_format_push(char *text, size_t capacity, size_t *length, char character)
{
    if (*length + 1u < capacity) {
        text[*length] = character;
    }
    *length += 1u;
}

int fractus_hp_format(fractus_hp value, int significant_digits, char *buffer, size_t buffer_size)
{
    fractus_hp magnitude;
    int digits[32];
    int exponent = 0;
    int negative;
    int index;
    int round_up;
    int guard;
    char text[96];
    size_t length = 0u;

    if (buffer == NULL || buffer_size == 0u) {
        return -1;
    }

    buffer[0] = '\0';
    if (significant_digits < 1) {
        significant_digits = 1;
    }
    if (significant_digits > 32) {
        significant_digits = 32;
    }

    negative = value.hi < 0.0 || (value.hi == 0.0 && value.lo < 0.0);
    magnitude.hi = negative ? -value.hi : value.hi;
    magnitude.lo = negative ? -value.lo : value.lo;
    if (magnitude.hi == 0.0 && magnitude.lo == 0.0) {
        if (buffer_size < 2u) {
            return -1;
        }
        buffer[0] = '0';
        buffer[1] = '\0';
        return 0;
    }

    for (guard = 0; guard < 400 && !(magnitude.hi >= 1.0); ++guard) {
        magnitude = fractus_hp_mul_double(magnitude, 10.0);
        exponent -= 1;
        if (magnitude.hi == 0.0 && magnitude.lo == 0.0) {
            if (buffer_size < 2u) {
                return -1;
            }
            buffer[0] = '0';
            buffer[1] = '\0';
            return 0;
        }
    }
    for (guard = 0; guard < 400 && magnitude.hi >= 10.0; ++guard) {
        magnitude = fractus_hp_div_double(magnitude, 10.0);
        exponent += 1;
    }

    for (index = 0; index < significant_digits; ++index) {
        int digit;

        if (!(magnitude.hi >= 0.0)) {
            magnitude = fractus_hp_from_double(0.0);
        }
        digit = (int)magnitude.hi;
        if (digit < 0) {
            digit = 0;
        }
        if (digit > 9) {
            digit = 9;
        }
        digits[index] = digit;
        magnitude = fractus_hp_sub(magnitude, fractus_hp_from_double((double)digit));
        magnitude = fractus_hp_mul_double(magnitude, 10.0);
    }

    round_up = fractus_hp_greater_double(magnitude, 5.0) || (magnitude.hi == 5.0 && magnitude.lo == 0.0);
    if (round_up) {
        for (index = significant_digits - 1; index >= 0; --index) {
            if (digits[index] < 9) {
                digits[index] += 1;
                break;
            }
            digits[index] = 0;
        }
        if (index < 0) {
            digits[0] = 1;
            for (index = 1; index < significant_digits; ++index) {
                digits[index] = 0;
            }
            exponent += 1;
        }
    }

    if (negative) {
        fractus_hp_format_push(text, sizeof(text), &length, '-');
    }

    if (exponent >= -6 && exponent < 8 && exponent < significant_digits) {
        if (exponent >= 0) {
            for (index = 0; index <= exponent; ++index) {
                fractus_hp_format_push(text, sizeof(text), &length, (char)('0' + digits[index]));
            }
            if (exponent + 1 < significant_digits) {
                fractus_hp_format_push(text, sizeof(text), &length, '.');
                for (index = exponent + 1; index < significant_digits; ++index) {
                    fractus_hp_format_push(text, sizeof(text), &length, (char)('0' + digits[index]));
                }
            }
        } else {
            fractus_hp_format_push(text, sizeof(text), &length, '0');
            fractus_hp_format_push(text, sizeof(text), &length, '.');
            for (index = 0; index < -exponent - 1; ++index) {
                fractus_hp_format_push(text, sizeof(text), &length, '0');
            }
            for (index = 0; index < significant_digits; ++index) {
                fractus_hp_format_push(text, sizeof(text), &length, (char)('0' + digits[index]));
            }
        }
    } else {
        char exponent_text[16];
        int exponent_length;

        fractus_hp_format_push(text, sizeof(text), &length, (char)('0' + digits[0]));
        if (significant_digits > 1) {
            fractus_hp_format_push(text, sizeof(text), &length, '.');
            for (index = 1; index < significant_digits; ++index) {
                fractus_hp_format_push(text, sizeof(text), &length, (char)('0' + digits[index]));
            }
        }
        fractus_hp_format_push(text, sizeof(text), &length, 'e');
        exponent_length = snprintf(exponent_text, sizeof(exponent_text), "%d", exponent);
        if (exponent_length < 0) {
            return -1;
        }
        for (index = 0; index < exponent_length; ++index) {
            fractus_hp_format_push(text, sizeof(text), &length, exponent_text[index]);
        }
    }

    if (length + 1u > buffer_size || length + 1u > sizeof(text)) {
        return -1;
    }
    text[length] = '\0';
    memcpy(buffer, text, length + 1u);
    return 0;
}
