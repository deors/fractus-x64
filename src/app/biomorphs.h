#ifndef FRACTUS_X64_APP_BIOMORPHS_H
#define FRACTUS_X64_APP_BIOMORPHS_H

#include "app/app.h"
#include "app/commons.h"
#include "core/biomorphs.h"

typedef enum fractus_app_biomorph_dialog_action {
    FRACTUS_APP_BIOMORPH_XMIN_DEC = 0,
    FRACTUS_APP_BIOMORPH_XMIN_INC,
    FRACTUS_APP_BIOMORPH_XMAX_DEC,
    FRACTUS_APP_BIOMORPH_XMAX_INC,
    FRACTUS_APP_BIOMORPH_YMIN_DEC,
    FRACTUS_APP_BIOMORPH_YMIN_INC,
    FRACTUS_APP_BIOMORPH_YMAX_DEC,
    FRACTUS_APP_BIOMORPH_YMAX_INC,
    FRACTUS_APP_BIOMORPH_CREAL_DEC,
    FRACTUS_APP_BIOMORPH_CREAL_INC,
    FRACTUS_APP_BIOMORPH_CIMAG_DEC,
    FRACTUS_APP_BIOMORPH_CIMAG_INC,
    FRACTUS_APP_BIOMORPH_RADIUS_DEC,
    FRACTUS_APP_BIOMORPH_RADIUS_INC,
    FRACTUS_APP_BIOMORPH_CUTOFF_DEC,
    FRACTUS_APP_BIOMORPH_CUTOFF_INC,
    FRACTUS_APP_BIOMORPH_EQ_0,
    FRACTUS_APP_BIOMORPH_EQ_1,
    FRACTUS_APP_BIOMORPH_EQ_2,
    FRACTUS_APP_BIOMORPH_EQ_3,
    FRACTUS_APP_BIOMORPH_EQ_4,
    FRACTUS_APP_BIOMORPH_EQ_5,
    FRACTUS_APP_BIOMORPH_TRAP_0,
    FRACTUS_APP_BIOMORPH_TRAP_1,
    FRACTUS_APP_BIOMORPH_TRAP_2,
    FRACTUS_APP_BIOMORPH_TRAP_3,
    FRACTUS_APP_BIOMORPH_DRAW,
    FRACTUS_APP_BIOMORPH_CANCEL
} fractus_app_biomorph_dialog_action;

typedef struct fractus_app_biomorph_fields {
    fractus_ui_numeric_field xmin;
    fractus_ui_numeric_field xmax;
    fractus_ui_numeric_field ymin;
    fractus_ui_numeric_field ymax;
    fractus_ui_numeric_field constant_real;
    fractus_ui_numeric_field constant_imag;
    fractus_ui_numeric_field escape_radius_squared;
    fractus_ui_numeric_field cutoff;
} fractus_app_biomorph_fields;

void fractus_app_init_biomorph_fields(
    fractus_app_biomorph_fields *fields,
    double xmin,
    double xmax,
    double ymin,
    double ymax,
    double constant_real,
    double constant_imag,
    double escape_radius_squared,
    double cutoff);

fractus_status fractus_app_run_biomorph_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_biomorph_params *params,
    fractus_biomorph_params *pending,
    fractus_app_biomorph_fields *fields,
    fractus_app_view *view);

fractus_status fractus_app_render_biomorph(
    const fractus_platform_context *platform,
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const fractus_biomorph_params *params,
    int *save_next_graphic,
    char *error_message,
    size_t error_message_size);

#endif
