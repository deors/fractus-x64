#ifndef FRACTUS_X64_APP_ATTRACTORS_H
#define FRACTUS_X64_APP_ATTRACTORS_H

#include "app/app.h"
#include "app/commons.h"

typedef enum fractus_app_attractor_method {
    FRACTUS_APP_ATTRACTOR_METHOD_NONE = -1,
    FRACTUS_APP_ATTRACTOR_METHOD_LORENZ = 0,
    FRACTUS_APP_ATTRACTOR_METHOD_HENON = 1,
    FRACTUS_APP_ATTRACTOR_METHOD_ROSSLER = 2,
    FRACTUS_APP_ATTRACTOR_METHOD_IKEDA = 3,
    FRACTUS_APP_ATTRACTOR_METHOD_GUMOWSKI_MIRA = 4,
    FRACTUS_APP_ATTRACTOR_METHOD_AIZAWA = 5,
    FRACTUS_APP_ATTRACTOR_METHOD_PETER_DE_JONG = 6,
    FRACTUS_APP_ATTRACTOR_METHOD_PICKOVER = 7,
    FRACTUS_APP_ATTRACTOR_METHOD_TINKERBELL = 8,
    FRACTUS_APP_ATTRACTOR_METHOD_THOMAS = 9,
    FRACTUS_APP_ATTRACTOR_METHOD_SVENSSON = 10,
    FRACTUS_APP_ATTRACTOR_METHOD_COUNT = 11
} fractus_app_attractor_method;

const char *fractus_app_attractor_method_name(fractus_app_attractor_method method);

fractus_status fractus_app_run_attractors_menu_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_app_attractor_method *selected_method,
    fractus_app_view *view);

fractus_status fractus_app_run_attractors_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_app_attractor_method *selected_method,
    fractus_app_view *view);

#endif
