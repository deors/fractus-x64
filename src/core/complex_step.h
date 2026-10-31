#ifndef FRACTUS_X64_CORE_COMPLEX_STEP_H
#define FRACTUS_X64_CORE_COMPLEX_STEP_H

#include <math.h>
#include <stddef.h>

#include "core/precision.h"
#include "platform/framebuffer.h"

/* One algebraic step, instantiated for double and for fractus_hp.
   The macros keep the double parenthesization. Both copies inline into the pixel loop. */

#define FRACTUS_D_MUL(a, b) ((a) * (b))
#define FRACTUS_D_ADD(a, b) ((a) + (b))
#define FRACTUS_D_SUB(a, b) ((a) - (b))
#define FRACTUS_D_LEFT(s, a) ((s) * (a))
#define FRACTUS_D_SCALE(a, s) ((a) * (s))
#define FRACTUS_D_HI(v) (v)
#define FRACTUS_D_TAIL(v) (0.0)

#define FRACTUS_HP_MUL(a, b) (fractus_hp_mul((a), (b)))
#define FRACTUS_HP_ADD(a, b) (fractus_hp_add((a), (b)))
#define FRACTUS_HP_SUB(a, b) (fractus_hp_sub((a), (b)))
#define FRACTUS_HP_LEFT(s, a) (fractus_hp_mul_double((a), (s)))
#define FRACTUS_HP_SCALE(a, s) (fractus_hp_mul_double((a), (s)))
#define FRACTUS_HP_HI(v) ((v).hi)
#define FRACTUS_HP_TAIL(v) ((v).lo)

#define FRACTUS_COMPLEX_DEFINE_Z2(PREFIX, T, MUL, ADD, SUB, LEFT) \
    static inline void PREFIX##_z2(T zr, T zi, T cr, T ci, T *next_r, T *next_i) \
    { \
        *next_r = ADD(SUB(MUL(zr, zr), MUL(zi, zi)), cr); \
        *next_i = ADD(MUL(LEFT(2.0, zr), zi), ci); \
    }

#define FRACTUS_COMPLEX_DEFINE_DZ(PREFIX, T, MUL, ADD, SUB, LEFT) \
    static inline void PREFIX##_dz(T zr, T zi, T dzr, T dzi, T one, int add_one, T *next_dzr, T *next_dzi) \
    { \
        *next_dzr = LEFT(2.0, SUB(MUL(zr, dzr), MUL(zi, dzi))); \
        *next_dzi = LEFT(2.0, ADD(MUL(zr, dzi), MUL(zi, dzr))); \
        if (add_one) { \
            *next_dzr = ADD(*next_dzr, one); \
        } \
    }

#define FRACTUS_COMPLEX_DEFINE_Z3(PREFIX, T, MUL, ADD, SUB, LEFT) \
    static inline void PREFIX##_z3(T zr, T zi, T cr, T ci, T *next_r, T *next_i) \
    { \
        T zr2 = MUL(zr, zr); \
        T zi2 = MUL(zi, zi); \
        *next_r = ADD(MUL(zr, SUB(zr2, LEFT(3.0, zi2))), cr); \
        *next_i = ADD(MUL(zi, SUB(LEFT(3.0, zr2), zi2)), ci); \
    }

#define FRACTUS_COMPLEX_DEFINE_Z4(PREFIX, T, MUL, ADD, SUB, LEFT) \
    static inline void PREFIX##_z4(T zr, T zi, T cr, T ci, T *next_r, T *next_i) \
    { \
        T zr2 = MUL(zr, zr); \
        T zi2 = MUL(zi, zi); \
        *next_r = ADD(ADD(SUB(MUL(zr2, zr2), MUL(LEFT(6.0, zr2), zi2)), MUL(zi2, zi2)), cr); \
        *next_i = ADD(MUL(MUL(LEFT(4.0, zr), zi), SUB(zr2, zi2)), ci); \
    }

#define FRACTUS_COMPLEX_DEFINE_Z5(PREFIX, T, MUL, ADD, SUB, LEFT) \
    static inline void PREFIX##_z5(T zr, T zi, T cr, T ci, T *next_r, T *next_i) \
    { \
        T zr2 = MUL(zr, zr); \
        T zi2 = MUL(zi, zi); \
        T zr4 = MUL(zr2, zr2); \
        T zi4 = MUL(zi2, zi2); \
        T ten = MUL(LEFT(10.0, zr2), zi2); \
        *next_r = ADD(MUL(zr, ADD(SUB(zr4, ten), LEFT(5.0, zi4))), cr); \
        *next_i = ADD(MUL(zi, ADD(SUB(LEFT(5.0, zr4), ten), zi4)), ci); \
    }

#ifdef FRACTUS_COMPLEX_BIOMORPH

static inline double fractus_clamp_signed(double value, double limit)
{
    if (value > limit) {
        return limit;
    }
    if (value < -limit) {
        return -limit;
    }
    return value;
}

static inline fractus_hp fractus_hp_clamp_signed(fractus_hp value, double limit)
{
    if (value.hi > limit) {
        return fractus_hp_from_double(limit);
    }
    if (value.hi < -limit) {
        return fractus_hp_from_double(-limit);
    }
    return value;
}

/* One exp for sinh and cosh. tail is the low word; double passes 0. */
static inline void fractus_hyperbolic_pair(double clamped, double tail, double *cosh_v, double *sinh_v)
{
    double exp_p = exp(clamped);
    double exp_m = 1.0 / exp_p;
    double cosh_i = 0.5 * (exp_p + exp_m);
    double sinh_i = 0.5 * (exp_p - exp_m);

    *cosh_v = cosh_i + (tail * sinh_i);
    *sinh_v = sinh_i + (tail * cosh_i);
}

#define FRACTUS_COMPLEX_DEFINE_SIN(PREFIX, T, ADD, SCALE, SIN, COS, HI, TAIL, CLAMP) \
    static inline void PREFIX##_sin_z(T zr, T zi, T cr, T ci, T *next_r, T *next_i) \
    { \
        T clamped = CLAMP(zi, 50.0); \
        double cosh_v; \
        double sinh_v; \
        fractus_hyperbolic_pair(HI(clamped), TAIL(clamped), &cosh_v, &sinh_v); \
        *next_r = ADD(SCALE(SIN(zr), cosh_v), cr); \
        *next_i = ADD(SCALE(COS(zr), sinh_v), ci); \
    }

#define FRACTUS_COMPLEX_DEFINE_EXP(PREFIX, T, ADD, SCALE, SIN, COS, HI, TAIL, CLAMP) \
    static inline void PREFIX##_exp_z(T zr, T zi, T cr, T ci, T *next_r, T *next_i) \
    { \
        T clamped = CLAMP(zr, 100.0); \
        double exp_r = exp(HI(clamped)); \
        exp_r += exp_r * TAIL(clamped); \
        *next_r = ADD(SCALE(COS(zi), exp_r), cr); \
        *next_i = ADD(SCALE(SIN(zi), exp_r), ci); \
    }

#define FRACTUS_COMPLEX_DEFINE_BIOMORPH(PREFIX, T) \
    static inline void PREFIX##_biomorph_step( \
        fractus_biomorph_equation equation, \
        T zr, \
        T zi, \
        T cr, \
        T ci, \
        T *next_r, \
        T *next_i) \
    { \
        switch (equation) { \
        case FRACTUS_BIOMORPH_EQ_Z3: \
            PREFIX##_z3(zr, zi, cr, ci, next_r, next_i); \
            break; \
        case FRACTUS_BIOMORPH_EQ_Z4: \
            PREFIX##_z4(zr, zi, cr, ci, next_r, next_i); \
            break; \
        case FRACTUS_BIOMORPH_EQ_Z5: \
            PREFIX##_z5(zr, zi, cr, ci, next_r, next_i); \
            break; \
        case FRACTUS_BIOMORPH_EQ_SIN_Z: \
            PREFIX##_sin_z(zr, zi, cr, ci, next_r, next_i); \
            break; \
        case FRACTUS_BIOMORPH_EQ_EXP_Z: \
            PREFIX##_exp_z(zr, zi, cr, ci, next_r, next_i); \
            break; \
        case FRACTUS_BIOMORPH_EQ_Z2: \
        default: \
            PREFIX##_z2(zr, zi, cr, ci, next_r, next_i); \
            break; \
        } \
    }

#endif

#ifdef FRACTUS_COMPLEX_DEM

static inline double fractus_dem_pixel_size(double dx, double dy)
{
    double pixel_size = (dx < dy) ? dx : dy;

    if (!(pixel_size > 0.0)) {
        pixel_size = (dx > 0.0) ? dx : dy;
        if (!(pixel_size > 0.0)) {
            pixel_size = 1e-30;
        }
    }
    return pixel_size;
}

static inline uint8_t fractus_dem_distance_color(
    int gradient,
    double zr,
    double zi,
    double dzr,
    double dzi,
    double pixel_size,
    uint32_t iteration,
    uint8_t inside_color_index,
    uint8_t palette_offset,
    uint8_t palette_span)
{
    double r2 = (zr * zr) + (zi * zi);
    double dz2 = (dzr * dzr) + (dzi * dzi);
    double r;
    double dz;
    double distance;
    double pixel_dist;

    if (dz2 <= 1e-24 || r2 <= 1.0) {
        return inside_color_index;
    }

    r = sqrt(r2);
    dz = sqrt(dz2);
    distance = (2.0 * r * log(r)) / dz;
    pixel_dist = distance / pixel_size;
    if (gradient) {
        double v = (pixel_dist > 0.0) ? (log(1.0 + pixel_dist) * 8.0) : 0.0;
        int color_step = (int)floor(v);

        if (color_step < 0) {
            color_step = 0;
        }
        return (uint8_t)(palette_offset + ((uint32_t)color_step % palette_span));
    }
    if (pixel_dist < 1.0) {
        return inside_color_index;
    }
    return (uint8_t)(palette_offset + ((iteration - 1u) % palette_span));
}

#endif

static inline int fractus_precision_indices_match(
    const fractus_framebuffer *fast,
    const fractus_framebuffer *extended)
{
    uint32_t y;
    uint32_t x;

    if (fast == NULL || extended == NULL ||
        fast->size.width != extended->size.width ||
        fast->size.height != extended->size.height) {
        return 0;
    }

    for (y = 0u; y < fast->size.height; ++y) {
        for (x = 0u; x < fast->size.width; ++x) {
            size_t fast_index = ((size_t)y * fast->pitch_pixels) + x;
            size_t extended_index = ((size_t)y * extended->pitch_pixels) + x;

            if (fast->index_pixels[fast_index] != extended->index_pixels[extended_index]) {
                return 0;
            }
        }
    }
    return 1;
}

#endif
