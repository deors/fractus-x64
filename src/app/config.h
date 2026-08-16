#ifndef FRACTUS_X64_APP_CONFIG_H
#define FRACTUS_X64_APP_CONFIG_H

#include "app/app.h"
#include "app/commons.h"

typedef enum fractus_app_simple_config_action {
    FRACTUS_APP_SIMPLE_CONFIG_DEC = 0,
    FRACTUS_APP_SIMPLE_CONFIG_INC,
    FRACTUS_APP_SIMPLE_CONFIG_ACCEPT,
    FRACTUS_APP_SIMPLE_CONFIG_CANCEL
} fractus_app_simple_config_action;

fractus_status fractus_app_apply_legacy_config(
    fractus_framebuffer *framebuffer,
    const fractus_legacy_config *config);

void fractus_app_apply_legacy_numeric_config(
    const fractus_legacy_config *config,
    fractus_mandelbrot_params *mandelbrot_params,
    fractus_julia_params *julia_params,
    fractus_biomorph_params *biomorph_params);

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
    fractus_legacy_config *legacy_config,
    char *cfg_path,
    size_t cfg_path_size);

fractus_status fractus_app_run_iterations_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_legacy_config *legacy_config,
    fractus_legacy_config *config_draft,
    fractus_mandelbrot_params *mandelbrot_params,
    fractus_julia_params *julia_params,
    fractus_biomorph_params *biomorph_params,
    const char *cfg_path,
    fractus_ui_numeric_field *iterations_field,
    fractus_app_view *view);

fractus_status fractus_app_run_escape_radius_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_legacy_config *legacy_config,
    fractus_legacy_config *config_draft,
    fractus_mandelbrot_params *mandelbrot_params,
    fractus_julia_params *julia_params,
    fractus_biomorph_params *biomorph_params,
    const char *cfg_path,
    fractus_ui_numeric_field *escape_radius_field,
    fractus_app_view *view);

fractus_status fractus_app_run_biomorph_radius_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_legacy_config *legacy_config,
    fractus_legacy_config *config_draft,
    fractus_mandelbrot_params *mandelbrot_params,
    fractus_julia_params *julia_params,
    fractus_biomorph_params *biomorph_params,
    const char *cfg_path,
    fractus_ui_numeric_field *biomorph_radius_field,
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
