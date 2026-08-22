#ifndef FRACTUS_X64_APP_MANDELBROT_H
#define FRACTUS_X64_APP_MANDELBROT_H

#include "app/app.h"
#include "app/commons.h"
#include "core/mandelbrot.h"

typedef enum fractus_app_mandel_method {
    FRACTUS_APP_MANDEL_METHOD_NONE = -1,
    FRACTUS_APP_MANDEL_METHOD_ESCAPE = 0,
    FRACTUS_APP_MANDEL_METHOD_DEM = 1
} fractus_app_mandel_method;

typedef enum fractus_app_mandel_dialog_action {
    FRACTUS_APP_MANDEL_XMIN_DEC = 0,
    FRACTUS_APP_MANDEL_XMIN_INC,
    FRACTUS_APP_MANDEL_XMAX_DEC,
    FRACTUS_APP_MANDEL_XMAX_INC,
    FRACTUS_APP_MANDEL_YMIN_DEC,
    FRACTUS_APP_MANDEL_YMIN_INC,
    FRACTUS_APP_MANDEL_YMAX_DEC,
    FRACTUS_APP_MANDEL_YMAX_INC,
    FRACTUS_APP_MANDEL_ITER_DEC,
    FRACTUS_APP_MANDEL_ITER_INC,
    FRACTUS_APP_MANDEL_RADIUS_DEC,
    FRACTUS_APP_MANDEL_RADIUS_INC,
    FRACTUS_APP_MANDEL_COLOR_MODE_0,
    FRACTUS_APP_MANDEL_COLOR_MODE_1,
    FRACTUS_APP_MANDEL_DRAW,
    FRACTUS_APP_MANDEL_CANCEL
} fractus_app_mandel_dialog_action;

typedef struct fractus_app_mandelbrot_fields {
    fractus_ui_numeric_field xmin;
    fractus_ui_numeric_field xmax;
    fractus_ui_numeric_field ymin;
    fractus_ui_numeric_field ymax;
    fractus_ui_numeric_field max_iterations;
    fractus_ui_numeric_field escape_radius_squared;
} fractus_app_mandelbrot_fields;

void fractus_app_init_mandelbrot_fields(
    fractus_app_mandelbrot_fields *fields,
    double xmin,
    double xmax,
    double ymin,
    double ymax,
    uint32_t max_iterations,
    double escape_radius_squared);

fractus_status fractus_app_run_mandelbrot_menu_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_mandelbrot_params *mandelbrot_params,
    fractus_mandelbrot_params *mandelbrot_pending,
    fractus_app_mandelbrot_fields *mandelbrot_fields,
    fractus_mandelbrot_dem_params *mandelbrot_dem_params,
    fractus_mandelbrot_dem_params *mandelbrot_dem_pending,
    fractus_app_mandelbrot_fields *mandelbrot_dem_fields,
    fractus_app_view *view);

fractus_status fractus_app_run_mandelbrot_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_mandelbrot_params *params,
    fractus_mandelbrot_params *pending,
    fractus_app_mandelbrot_fields *fields,
    fractus_app_view *view);

fractus_status fractus_app_run_mandelbrot_dem_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_mandelbrot_dem_params *params,
    fractus_mandelbrot_dem_params *pending,
    fractus_app_mandelbrot_fields *fields,
    fractus_app_view *view);

fractus_status fractus_app_render_mandelbrot(
    const fractus_platform_context *platform,
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const fractus_mandelbrot_params *params,
    int *save_next_graphic,
    char *error_message,
    size_t error_message_size,
    char *saved_filename,
    size_t saved_filename_size);

fractus_status fractus_app_render_mandelbrot_dem(
    const fractus_platform_context *platform,
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const fractus_mandelbrot_dem_params *params,
    int *save_next_graphic,
    char *error_message,
    size_t error_message_size,
    char *saved_filename,
    size_t saved_filename_size);

#endif
