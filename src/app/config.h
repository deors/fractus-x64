#ifndef FRACTUS_X64_APP_CONFIG_H
#define FRACTUS_X64_APP_CONFIG_H

#include "app/app.h"
#include "app/commons.h"

typedef enum fractus_app_fractals_config_action {
    FRACTUS_APP_FRACTALS_CONFIG_ITER_DEC = 0,
    FRACTUS_APP_FRACTALS_CONFIG_ITER_INC,
    FRACTUS_APP_FRACTALS_CONFIG_ESCAPE_DEC,
    FRACTUS_APP_FRACTALS_CONFIG_ESCAPE_INC,
    FRACTUS_APP_FRACTALS_CONFIG_BIO_ITER_DEC,
    FRACTUS_APP_FRACTALS_CONFIG_BIO_ITER_INC,
    FRACTUS_APP_FRACTALS_CONFIG_BIO_RADIUS_DEC,
    FRACTUS_APP_FRACTALS_CONFIG_BIO_RADIUS_INC,
    FRACTUS_APP_FRACTALS_CONFIG_BIO_CUTOFF_DEC,
    FRACTUS_APP_FRACTALS_CONFIG_BIO_CUTOFF_INC,
    FRACTUS_APP_FRACTALS_CONFIG_PLASMA_RECT_SEED_DEC,
    FRACTUS_APP_FRACTALS_CONFIG_PLASMA_RECT_SEED_INC,
    FRACTUS_APP_FRACTALS_CONFIG_PLASMA_CIRC_SEED_DEC,
    FRACTUS_APP_FRACTALS_CONFIG_PLASMA_CIRC_SEED_INC,
    FRACTUS_APP_FRACTALS_CONFIG_SAVE,
    FRACTUS_APP_FRACTALS_CONFIG_CANCEL
} fractus_app_fractals_config_action;

fractus_status fractus_app_apply_legacy_config(
    fractus_framebuffer *framebuffer,
    const fractus_legacy_config *config);

void fractus_app_apply_legacy_numeric_config(
    const fractus_legacy_config *config,
    fractus_mandelbrot_params *mandelbrot_params,
    fractus_julia_params *julia_params,
    fractus_biomorph_params *biomorph_params,
    fractus_plasma_params *plasma_rectangular_params,
    fractus_plasma_circular_params *plasma_circular_params);

void fractus_app_reset_fractal_parameters(
    fractus_mandelbrot_params *mandelbrot_params,
    fractus_mandelbrot_dem_params *mandelbrot_dem_params,
    fractus_julia_params *julia_params,
    fractus_julia_dem_params *julia_dem_params,
    fractus_biomorph_params *biomorph_params,
    fractus_plasma_params *plasma_rectangular_params,
    fractus_plasma_circular_params *plasma_circular_params,
    const fractus_legacy_config *legacy_config);

void fractus_app_capture_palette_to_config(
    const fractus_framebuffer *framebuffer,
    fractus_legacy_config *config);

fractus_status fractus_app_save_legacy_config(
    const char *path,
    const fractus_framebuffer *framebuffer,
    fractus_legacy_config *config);

fractus_status fractus_app_persist_current_palette(
    const char *cfg_path,
    const fractus_framebuffer *framebuffer,
    fractus_legacy_config *config);

fractus_status fractus_app_load_legacy_assets(
    const fractus_platform_context *platform,
    fractus_framebuffer *framebuffer,
    fractus_mandelbrot_params *mandelbrot_params,
    fractus_julia_params *julia_params,
    fractus_biomorph_params *biomorph_params,
    fractus_plasma_params *plasma_rectangular_params,
    fractus_plasma_circular_params *plasma_circular_params,
    fractus_legacy_config *legacy_config,
    char *cfg_path,
    size_t cfg_path_size);

fractus_status fractus_app_run_fractals_default_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_legacy_config *legacy_config,
    fractus_legacy_config *config_draft,
    fractus_mandelbrot_params *mandelbrot_params,
    fractus_mandelbrot_dem_params *mandelbrot_dem_params,
    fractus_julia_params *julia_params,
    fractus_julia_dem_params *julia_dem_params,
    fractus_biomorph_params *biomorph_params,
    fractus_plasma_params *plasma_rectangular_params,
    fractus_plasma_circular_params *plasma_circular_params,
    const char *cfg_path,
    fractus_ui_numeric_field *iterations_field,
    fractus_ui_numeric_field *escape_radius_field,
    fractus_ui_numeric_field *biomorph_iterations_field,
    fractus_ui_numeric_field *biomorph_radius_field,
    fractus_ui_numeric_field *biomorph_cutoff_field,
    fractus_ui_numeric_field *plasma_rect_seed_field,
    fractus_ui_numeric_field *plasma_circ_seed_field,
    fractus_app_view *view);

fractus_status fractus_app_run_video_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_legacy_config *legacy_config,
    fractus_legacy_config *config_draft,
    fractus_mandelbrot_params *mandelbrot_params,
    fractus_julia_params *julia_params,
    fractus_biomorph_params *biomorph_params,
    const char *cfg_path,
    fractus_app_view *view);

#endif
