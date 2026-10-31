#ifndef FRACTUS_X64_CORE_PRECISION_H
#define FRACTUS_X64_CORE_PRECISION_H

#include <math.h>
#include <stddef.h>
#include <stdint.h>

/* Two IEEE doubles: about 30 decimal digits, without a long-double or an external library.
   Renderers stay on double until the pixel step falls below FRACTUS_HP_RELATIVE_STEP
   of the coordinate scale. The navigation floor is much smaller than the double floor. */
#define FRACTUS_HP_RELATIVE_STEP 1e-14
#define FRACTUS_HP_PLANE_MIN_SPAN 1e-28

typedef struct fractus_hp {
    double hi;
    double lo;
} fractus_hp;

typedef struct fractus_hp_window {
    fractus_hp xmin;
    fractus_hp xmax;
    fractus_hp ymin;
    fractus_hp ymax;
    int valid;
} fractus_hp_window;

/* Nested zero. A bare {0} only names xmin.hi and warns under -Wextra. */
#define FRACTUS_HP_WINDOW_ZERO {{0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, 0}

static inline fractus_hp fractus_hp_renorm(double hi, double lo)
{
    fractus_hp result;
    double sum;
    double error;

    if (!(fabs(hi) >= fabs(lo))) {
        double swap = hi;
        hi = lo;
        lo = swap;
    }

    sum = hi + lo;
    error = lo - (sum - hi);
    result.hi = sum;
    result.lo = error;
    return result;
}

static inline fractus_hp fractus_hp_from_double(double value)
{
    fractus_hp result;
    result.hi = value;
    result.lo = 0.0;
    return result;
}

static inline double fractus_hp_to_double(fractus_hp value)
{
    return value.hi + value.lo;
}

static inline fractus_hp fractus_hp_add(fractus_hp a, fractus_hp b)
{
    double sum;
    double error;
    double tail;
    double tail_error;
    double bridge;
    fractus_hp mid;

    sum = a.hi + b.hi;
    bridge = sum - a.hi;
    error = (a.hi - (sum - bridge)) + (b.hi - bridge);
    tail = a.lo + b.lo;
    bridge = tail - a.lo;
    tail_error = (a.lo - (tail - bridge)) + (b.lo - bridge);
    error += tail;
    mid = fractus_hp_renorm(sum, error);
    return fractus_hp_renorm(mid.hi, mid.lo + tail_error);
}

static inline fractus_hp fractus_hp_sub(fractus_hp a, fractus_hp b)
{
    fractus_hp negated;
    negated.hi = -b.hi;
    negated.lo = -b.lo;
    return fractus_hp_add(a, negated);
}

static inline double fractus_hp_product_error(double a, double b, double product)
{
    const double splitter = 134217729.0;
    double a_hi;
    double a_lo;
    double b_hi;
    double b_lo;
    double scaled;

    scaled = splitter * a;
    a_hi = scaled - (scaled - a);
    a_lo = a - a_hi;
    scaled = splitter * b;
    b_hi = scaled - (scaled - b);
    b_lo = b - b_hi;
    return ((a_hi * b_hi - product) + a_hi * b_lo + a_lo * b_hi) + a_lo * b_lo;
}

static inline fractus_hp fractus_hp_mul(fractus_hp a, fractus_hp b)
{
    double product = a.hi * b.hi;
    double error = fractus_hp_product_error(a.hi, b.hi, product);
    error += a.hi * b.lo;
    error += a.lo * b.hi;
    error += a.lo * b.lo;
    return fractus_hp_renorm(product, error);
}

static inline fractus_hp fractus_hp_mul_double(fractus_hp a, double b)
{
    double product = a.hi * b;
    double error = fractus_hp_product_error(a.hi, b, product);
    error += a.lo * b;
    return fractus_hp_renorm(product, error);
}

static inline fractus_hp fractus_hp_div_double(fractus_hp a, double b)
{
    double quotient = a.hi / b;
    fractus_hp remainder = fractus_hp_sub(a, fractus_hp_mul_double(fractus_hp_from_double(quotient), b));
    fractus_hp result = fractus_hp_renorm(quotient, remainder.hi / b);
    return fractus_hp_renorm(result.hi, result.lo + (remainder.lo / b));
}

static inline int fractus_hp_is_positive(fractus_hp value)
{
    return value.hi > 0.0 || (value.hi == 0.0 && value.lo > 0.0);
}

static inline int fractus_hp_greater_double(fractus_hp value, double limit)
{
    if (value.hi > limit) {
        return 1;
    }
    if (value.hi < limit) {
        return 0;
    }
    return value.lo > 0.0;
}

static inline fractus_hp fractus_hp_sin(fractus_hp value)
{
    return fractus_hp_renorm(sin(value.hi), value.lo * cos(value.hi));
}

static inline fractus_hp fractus_hp_cos(fractus_hp value)
{
    return fractus_hp_renorm(cos(value.hi), -value.lo * sin(value.hi));
}

static inline fractus_hp fractus_hp_exp(fractus_hp value)
{
    double exponent = exp(value.hi);
    return fractus_hp_renorm(exponent, exponent * value.lo);
}

static inline void fractus_hp_plane_axes(
    const fractus_hp_window *window,
    uint32_t width,
    uint32_t height,
    fractus_hp *dx,
    fractus_hp *dy)
{
    fractus_hp span_x = fractus_hp_sub(window->xmax, window->xmin);
    fractus_hp span_y = fractus_hp_sub(window->ymax, window->ymin);

    *dx = fractus_hp_div_double(span_x, (double)(width - 1u));
    *dy = fractus_hp_div_double(span_y, (double)(height - 1u));
}

static inline fractus_hp fractus_hp_plane_real(const fractus_hp_window *window, fractus_hp dx, uint32_t x)
{
    return fractus_hp_add(window->xmin, fractus_hp_mul_double(dx, (double)x));
}

static inline fractus_hp fractus_hp_plane_imag(const fractus_hp_window *window, fractus_hp dy, uint32_t y)
{
    return fractus_hp_sub(window->ymax, fractus_hp_mul_double(dy, (double)y));
}

void fractus_hp_window_adopt_doubles(
    fractus_hp_window *window,
    double xmin,
    double xmax,
    double ymin,
    double ymax);
void fractus_hp_window_publish_doubles(
    const fractus_hp_window *window,
    double *xmin,
    double *xmax,
    double *ymin,
    double *ymax);
int fractus_hp_window_spans_positive(const fractus_hp_window *window);
int fractus_hp_window_equal(const fractus_hp_window *a, const fractus_hp_window *b);
int fractus_hp_window_needs_extended(
    const fractus_hp_window *window,
    uint32_t width,
    uint32_t height);
int fractus_hp_window_use_extended(
    fractus_hp_window *window,
    const fractus_hp_window *stored,
    double xmin,
    double xmax,
    double ymin,
    double ymax,
    uint32_t width,
    uint32_t height);
int fractus_hp_plane_pixel(
    const fractus_hp_window *window,
    uint32_t width,
    uint32_t height,
    uint32_t x,
    uint32_t y,
    fractus_hp *real_part,
    fractus_hp *imag_part);
int fractus_hp_plane_pan(
    fractus_hp_window *window,
    uint32_t width,
    uint32_t height,
    int32_t from_x,
    int32_t from_y,
    int32_t to_x,
    int32_t to_y);
int fractus_hp_plane_zoom_at_pixel(
    fractus_hp_window *window,
    uint32_t width,
    uint32_t height,
    uint32_t x,
    uint32_t y,
    double steps);
int fractus_hp_plane_zoom_centered(
    fractus_hp_window *window,
    uint32_t width,
    uint32_t height,
    double steps);
int fractus_hp_plane_zoom_at_pixel_floor(
    fractus_hp_window *window,
    uint32_t width,
    uint32_t height,
    uint32_t x,
    uint32_t y,
    double steps,
    double min_span);
int fractus_hp_plane_apply_selection(
    fractus_hp_window *window,
    uint32_t width,
    uint32_t height,
    int32_t x_min,
    int32_t x_max,
    int32_t y_min,
    int32_t y_max);
int fractus_hp_format(
    fractus_hp value,
    int significant_digits,
    char *buffer,
    size_t buffer_size);

#endif
