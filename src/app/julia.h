#ifndef FRACTUS_X64_APP_JULIA_H
#define FRACTUS_X64_APP_JULIA_H

#include "app/app.h"
#include "app/commons.h"
#include "core/julia.h"

typedef enum fractus_app_julia_method {
    FRACTUS_APP_JULIA_METHOD_NONE = -1,
    FRACTUS_APP_JULIA_METHOD_ESCAPE = 0,
    FRACTUS_APP_JULIA_METHOD_DEM = 1
} fractus_app_julia_method;

typedef enum fractus_app_julia_dialog_action {
    FRACTUS_APP_JULIA_XMIN_DEC = 0,
    FRACTUS_APP_JULIA_XMIN_INC,
    FRACTUS_APP_JULIA_XMAX_DEC,
    FRACTUS_APP_JULIA_XMAX_INC,
    FRACTUS_APP_JULIA_YMIN_DEC,
    FRACTUS_APP_JULIA_YMIN_INC,
    FRACTUS_APP_JULIA_YMAX_DEC,
    FRACTUS_APP_JULIA_YMAX_INC,
    FRACTUS_APP_JULIA_CREAL_DEC,
    FRACTUS_APP_JULIA_CREAL_INC,
    FRACTUS_APP_JULIA_CIMAG_DEC,
    FRACTUS_APP_JULIA_CIMAG_INC,
    FRACTUS_APP_JULIA_ITER_DEC,
    FRACTUS_APP_JULIA_ITER_INC,
    FRACTUS_APP_JULIA_RADIUS_DEC,
    FRACTUS_APP_JULIA_RADIUS_INC,
    FRACTUS_APP_JULIA_COLOR_MODE_0,
    FRACTUS_APP_JULIA_COLOR_MODE_1,
    FRACTUS_APP_JULIA_DRAW,
    FRACTUS_APP_JULIA_CANCEL
} fractus_app_julia_dialog_action;

typedef struct fractus_app_julia_fields {
    fractus_ui_numeric_field xmin;
    fractus_ui_numeric_field xmax;
    fractus_ui_numeric_field ymin;
    fractus_ui_numeric_field ymax;
    fractus_ui_numeric_field constant_real;
    fractus_ui_numeric_field constant_imag;
    fractus_ui_numeric_field max_iterations;
    fractus_ui_numeric_field escape_radius_squared;
} fractus_app_julia_fields;

void fractus_app_init_julia_fields(
    fractus_app_julia_fields *fields,
    double xmin,
    double xmax,
    double ymin,
    double ymax,
    double constant_real,
    double constant_imag,
    uint32_t max_iterations,
    double escape_radius_squared);

fractus_status fractus_app_run_julia_menu_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_julia_params *julia_params,
    fractus_julia_params *julia_pending,
    fractus_app_julia_fields *julia_fields,
    fractus_julia_dem_params *julia_dem_params,
    fractus_julia_dem_params *julia_dem_pending,
    fractus_app_julia_fields *julia_dem_fields,
    fractus_app_view *view);

fractus_status fractus_app_run_julia_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_julia_params *params,
    fractus_julia_params *pending,
    fractus_app_julia_fields *fields,
    fractus_app_view *view);

fractus_status fractus_app_run_julia_dem_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_julia_dem_params *params,
    fractus_julia_dem_params *pending,
    fractus_app_julia_fields *fields,
    fractus_app_view *view);

fractus_status fractus_app_render_julia(
    const fractus_platform_context *platform,
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const fractus_julia_params *params,
    int *save_next_graphic,
    char *error_message,
    size_t error_message_size);

fractus_status fractus_app_render_julia_dem(
    const fractus_platform_context *platform,
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const fractus_julia_dem_params *params,
    int *save_next_graphic,
    char *error_message,
    size_t error_message_size);

#endif
