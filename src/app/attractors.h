#ifndef FRACTUS_X64_APP_ATTRACTORS_H
#define FRACTUS_X64_APP_ATTRACTORS_H

#include "app/app.h"
#include "app/commons.h"
#include "core/attractors.h"

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

typedef enum fractus_app_lorenz_dialog_action {
    FRACTUS_APP_LORENZ_SIGMA_DEC = 0,
    FRACTUS_APP_LORENZ_SIGMA_INC,
    FRACTUS_APP_LORENZ_RHO_DEC,
    FRACTUS_APP_LORENZ_RHO_INC,
    FRACTUS_APP_LORENZ_BETA_DEC,
    FRACTUS_APP_LORENZ_BETA_INC,
    FRACTUS_APP_LORENZ_DT_DEC,
    FRACTUS_APP_LORENZ_DT_INC,
    FRACTUS_APP_LORENZ_ITERATIONS_DEC,
    FRACTUS_APP_LORENZ_ITERATIONS_INC,
    FRACTUS_APP_LORENZ_PROJ_XZ,
    FRACTUS_APP_LORENZ_PROJ_XY,
    FRACTUS_APP_LORENZ_PROJ_YZ,
    FRACTUS_APP_LORENZ_DRAW,
    FRACTUS_APP_LORENZ_CANCEL
} fractus_app_lorenz_dialog_action;

typedef struct fractus_app_lorenz_fields {
    fractus_ui_numeric_field sigma;
    fractus_ui_numeric_field rho;
    fractus_ui_numeric_field beta;
    fractus_ui_numeric_field dt;
    fractus_ui_numeric_field iterations;
} fractus_app_lorenz_fields;

typedef struct fractus_app_attractor_fields {
    fractus_app_lorenz_fields lorenz;
} fractus_app_attractor_fields;

void fractus_app_init_lorenz_fields(
    fractus_app_lorenz_fields *fields,
    double sigma,
    double rho,
    double beta,
    double dt,
    uint32_t iterations);

const char *fractus_app_attractor_method_name(fractus_app_attractor_method method);

fractus_status fractus_app_run_attractors_menu_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_lorenz_params *lorenz_params,
    fractus_lorenz_params *lorenz_pending,
    fractus_app_attractor_fields *attractor_fields,
    fractus_app_attractor_method *selected_method,
    fractus_app_view *view);

fractus_status fractus_app_run_attractors_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_lorenz_params *lorenz_params,
    fractus_lorenz_params *lorenz_pending,
    fractus_app_attractor_fields *attractor_fields,
    fractus_app_attractor_method *selected_method,
    fractus_app_view *view);

fractus_status fractus_app_render_lorenz(
    const fractus_platform_context *platform,
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const fractus_lorenz_params *params,
    int *save_next_graphic,
    char *error_message,
    size_t error_message_size,
    char *saved_filename,
    size_t saved_filename_size);

#endif
