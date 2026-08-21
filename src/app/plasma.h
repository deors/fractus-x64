#ifndef FRACTUS_X64_APP_PLASMA_H
#define FRACTUS_X64_APP_PLASMA_H

#include "app/app.h"
#include "app/commons.h"
#include "core/plasma.h"

typedef enum fractus_app_plasma_method {
    FRACTUS_APP_PLASMA_METHOD_NONE = -1,
    FRACTUS_APP_PLASMA_METHOD_RECTANGLES = 0,
    FRACTUS_APP_PLASMA_METHOD_CIRCLES = 1
} fractus_app_plasma_method;

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

typedef struct fractus_app_plasma_fields {
    fractus_ui_numeric_field rectangular_dispersion;
    fractus_ui_numeric_field circular_circles;
    fractus_ui_numeric_field circular_radius;
} fractus_app_plasma_fields;

void fractus_app_init_plasma_fields(
    fractus_app_plasma_fields *fields,
    int32_t dispersion,
    int32_t circle_count,
    int32_t max_radius);

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

#endif
