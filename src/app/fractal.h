#ifndef FRACTUS_X64_APP_FRACTAL_H
#define FRACTUS_X64_APP_FRACTAL_H

#include "app/app.h"
#include "app/commons.h"

typedef enum fractus_app_plasma_method {
    FRACTUS_APP_PLASMA_METHOD_NONE = -1,
    FRACTUS_APP_PLASMA_METHOD_RECTANGLES = 0,
    FRACTUS_APP_PLASMA_METHOD_CIRCLES = 1
} fractus_app_plasma_method;

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

typedef enum fractus_app_plasma_rectangular_dialog_action {
    FRACTUS_APP_PLASMA_RECTANGULAR_DISPERSION_DEC = 0,
    FRACTUS_APP_PLASMA_RECTANGULAR_DISPERSION_INC,
    FRACTUS_APP_PLASMA_RECTANGULAR_DRAW,
    FRACTUS_APP_PLASMA_RECTANGULAR_CANCEL
} fractus_app_plasma_rectangular_dialog_action;

typedef enum fractus_app_plasma_circular_dialog_action {
    FRACTUS_APP_PLASMA_CIRCLES_DEC = 0,
    FRACTUS_APP_PLASMA_CIRCLES_INC,
    FRACTUS_APP_PLASMA_RADIUS_DEC,
    FRACTUS_APP_PLASMA_RADIUS_INC,
    FRACTUS_APP_PLASMA_CIRCULAR_DRAW,
    FRACTUS_APP_PLASMA_CIRCULAR_CANCEL
} fractus_app_plasma_circular_dialog_action;

typedef struct fractus_app_mandelbrot_fields {
    fractus_ui_numeric_field xmin;
    fractus_ui_numeric_field xmax;
    fractus_ui_numeric_field ymin;
    fractus_ui_numeric_field ymax;
    fractus_ui_numeric_field max_iterations;
    fractus_ui_numeric_field escape_radius_squared;
} fractus_app_mandelbrot_fields;

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

typedef struct fractus_app_plasma_fields {
    fractus_ui_numeric_field rectangular_dispersion;
    fractus_ui_numeric_field circular_circles;
    fractus_ui_numeric_field circular_radius;
} fractus_app_plasma_fields;

void fractus_app_init_mandelbrot_fields(
    fractus_app_mandelbrot_fields *fields,
    double xmin,
    double xmax,
    double ymin,
    double ymax,
    uint32_t max_iterations,
    double escape_radius_squared);

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

void fractus_app_init_plasma_fields(
    fractus_app_plasma_fields *fields,
    int32_t dispersion,
    int32_t circle_count,
    int32_t max_radius);

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

int fractus_app_handle_mandelbrot_selection_input(
    const fractus_platform_context *platform,
    const fractus_framebuffer *framebuffer,
    const fractus_ui_context *ui,
    fractus_mandelbrot_params *params,
    fractus_app_mandelbrot_selection *selection);

int fractus_app_handle_mandelbrot_dem_selection_input(
    const fractus_platform_context *platform,
    const fractus_framebuffer *framebuffer,
    const fractus_ui_context *ui,
    fractus_mandelbrot_dem_params *params,
    fractus_app_mandelbrot_selection *selection);

fractus_status fractus_app_copy_framebuffer_for_overlay(
    fractus_framebuffer *target,
    const fractus_framebuffer *source);

fractus_status fractus_app_draw_mandelbrot_selection_overlay(
    const fractus_platform_context *platform,
    fractus_framebuffer *framebuffer,
    const fractus_ui_context *ui,
    const fractus_app_mandelbrot_selection *selection);

fractus_status fractus_app_render_mandelbrot(
    const fractus_platform_context *platform,
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const fractus_mandelbrot_params *params,
    int *save_next_graphic,
    char *error_message,
    size_t error_message_size);

fractus_status fractus_app_render_mandelbrot_dem(
    const fractus_platform_context *platform,
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const fractus_mandelbrot_dem_params *params,
    int *save_next_graphic,
    char *error_message,
    size_t error_message_size);

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

fractus_status fractus_app_run_plasma_menu_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_plasma_params *plasma_rectangular_params,
    fractus_plasma_params *plasma_rectangular_pending,
    fractus_plasma_circular_params *plasma_circular_params,
    fractus_plasma_circular_params *plasma_circular_pending,
    fractus_app_plasma_fields *plasma_fields,
    fractus_app_view *view);

fractus_status fractus_app_run_plasma_rectangular_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_plasma_params *params,
    fractus_plasma_params *pending,
    fractus_app_plasma_fields *fields,
    fractus_app_view *view);

fractus_status fractus_app_render_plasma_rectangular(
    const fractus_platform_context *platform,
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const fractus_plasma_params *params,
    int *save_next_graphic,
    char *error_message,
    size_t error_message_size);

fractus_status fractus_app_run_plasma_circular_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_plasma_circular_params *params,
    fractus_plasma_circular_params *pending,
    fractus_app_plasma_fields *fields,
    fractus_app_view *view);

fractus_status fractus_app_render_plasma_circular(
    const fractus_platform_context *platform,
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const fractus_plasma_circular_params *params,
    int *save_next_graphic,
    char *error_message,
    size_t error_message_size);

fractus_status fractus_app_draw_drawing_footer(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const char *text);

fractus_status fractus_app_draw_save_feedback(
    fractus_framebuffer *framebuffer,
    uint32_t frame);

#endif
