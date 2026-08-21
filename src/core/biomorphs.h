#ifndef FRACTUS_X64_CORE_BIOMORPHS_H
#define FRACTUS_X64_CORE_BIOMORPHS_H

#include "platform/framebuffer.h"
#include "platform/types.h"

typedef enum fractus_biomorph_equation {
    FRACTUS_BIOMORPH_EQ_Z2 = 0,
    FRACTUS_BIOMORPH_EQ_Z3 = 1,
    FRACTUS_BIOMORPH_EQ_Z4 = 2,
    FRACTUS_BIOMORPH_EQ_Z5 = 3,
    FRACTUS_BIOMORPH_EQ_SIN_Z = 4,
    FRACTUS_BIOMORPH_EQ_EXP_Z = 5
} fractus_biomorph_equation;

typedef enum fractus_biomorph_trap_mode {
    FRACTUS_BIOMORPH_TRAP_RE_OR_IM = 0,
    FRACTUS_BIOMORPH_TRAP_RE_AND_IM = 1,
    FRACTUS_BIOMORPH_TRAP_SOLO_RE = 2,
    FRACTUS_BIOMORPH_TRAP_SOLO_IM = 3
} fractus_biomorph_trap_mode;

typedef struct fractus_biomorph_params {
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    double constant_real;
    double constant_imag;
    uint32_t max_iterations;
    double escape_radius_squared;
    double cutoff;
    uint8_t background_color_index;
    uint8_t palette_offset;
    uint8_t palette_span;
    fractus_biomorph_equation equation;
    fractus_biomorph_trap_mode trap_mode;
} fractus_biomorph_params;

fractus_status fractus_fractal_render_biomorph(
    fractus_framebuffer *framebuffer,
    const fractus_biomorph_params *params);

#endif
