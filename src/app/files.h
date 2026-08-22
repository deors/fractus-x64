#ifndef FRACTUS_X64_APP_FILES_H
#define FRACTUS_X64_APP_FILES_H

#include "app/app.h"
#include "app/commons.h"

typedef enum fractus_app_palette_color_action {
    FRACTUS_APP_PALETTE_RED_DEC = 0,
    FRACTUS_APP_PALETTE_RED_INC,
    FRACTUS_APP_PALETTE_GREEN_DEC,
    FRACTUS_APP_PALETTE_GREEN_INC,
    FRACTUS_APP_PALETTE_BLUE_DEC,
    FRACTUS_APP_PALETTE_BLUE_INC,
    FRACTUS_APP_PALETTE_COLOR_ACCEPT,
    FRACTUS_APP_PALETTE_COLOR_CANCEL
} fractus_app_palette_color_action;

void fractus_app_sort_palette_files(
    fractus_app_palette_file *files,
    size_t count);

void fractus_app_sort_graphic_files(
    fractus_app_graphic_file *files,
    size_t count);

size_t fractus_app_list_graphic_files(
    const fractus_platform_context *platform,
    fractus_app_graphic_file *files,
    size_t capacity);

size_t fractus_app_list_palette_files(
    const fractus_platform_context *platform,
    fractus_app_palette_file *files,
    size_t capacity);

fractus_status fractus_app_load_palette_into_state(
    const char *palette_path,
    fractus_framebuffer *framebuffer,
    fractus_legacy_config *config,
    const char *cfg_path);

fractus_status fractus_app_load_graphic_palette_into_state(
    const char *graphic_path,
    fractus_framebuffer *framebuffer,
    fractus_legacy_config *config,
    const char *cfg_path);

fractus_status fractus_app_restore_default_palette(
    const fractus_platform_context *platform,
    fractus_framebuffer *framebuffer,
    fractus_legacy_config *config,
    const char *cfg_path);

fractus_status fractus_app_save_current_palette_file(
    const fractus_platform_context *platform,
    const fractus_framebuffer *framebuffer,
    fractus_legacy_config *config,
    const char *cfg_path);

fractus_status fractus_app_load_graphic_into_state(
    const char *graphic_path,
    fractus_framebuffer *framebuffer,
    fractus_legacy_config *config,
    const char *cfg_path);

fractus_status fractus_app_change_graphic_palette_into_state(
    const fractus_platform_context *platform,
    const char *graphic_path,
    const char *palette_path,
    fractus_framebuffer *framebuffer,
    fractus_legacy_config *config,
    const char *cfg_path);

fractus_status fractus_app_save_current_graphic_file(
    const fractus_platform_context *platform,
    const fractus_framebuffer *framebuffer);

fractus_status fractus_app_save_current_graphic_file_ex(
    const fractus_platform_context *platform,
    const fractus_framebuffer *framebuffer,
    char *out_saved_name,
    size_t out_saved_name_size);

fractus_status fractus_app_save_next_graphic_if_requested(
    const fractus_platform_context *platform,
    const fractus_framebuffer *framebuffer,
    int *save_next_graphic,
    char *error_message,
    size_t error_message_size);

fractus_status fractus_app_save_next_graphic_if_requested_ex(
    const fractus_platform_context *platform,
    const fractus_framebuffer *framebuffer,
    int *save_next_graphic,
    char *error_message,
    size_t error_message_size,
    char *out_saved_name,
    size_t out_saved_name_size);

fractus_status fractus_app_run_load_graphic_view(
    fractus_framebuffer *ui_framebuffer,
    fractus_framebuffer *drawing_framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    const fractus_app_graphic_file *graphic_files,
    size_t graphic_file_count,
    size_t *graphic_file_page,
    fractus_legacy_config *legacy_config,
    const char *cfg_path,
    fractus_app_view *view);

fractus_status fractus_app_run_load_palette_view(
    fractus_framebuffer *ui_framebuffer,
    fractus_framebuffer *drawing_framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    const fractus_app_palette_file *palette_files,
    size_t palette_file_count,
    size_t *palette_file_page,
    fractus_legacy_config *legacy_config,
    const char *cfg_path,
    fractus_app_view *view);

fractus_status fractus_app_run_load_graphic_palette_view(
    fractus_framebuffer *ui_framebuffer,
    fractus_framebuffer *drawing_framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    const fractus_app_graphic_file *graphic_files,
    size_t graphic_file_count,
    size_t *graphic_file_page,
    fractus_legacy_config *legacy_config,
    const char *cfg_path,
    fractus_app_view *view);

fractus_status fractus_app_run_change_graphic_palette_graphic_view(
    fractus_platform_context *platform,
    fractus_framebuffer *ui_framebuffer,
    fractus_framebuffer *drawing_framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    const fractus_app_graphic_file *graphic_files,
    size_t graphic_file_count,
    size_t *graphic_file_page,
    fractus_app_palette_file *palette_files,
    size_t *palette_file_count,
    size_t *palette_file_page,
    char *selected_graphic_path,
    size_t selected_graphic_path_size,
    fractus_legacy_config *legacy_config,
    const char *cfg_path,
    fractus_app_view *view);

fractus_status fractus_app_run_change_graphic_palette_palette_view(
    fractus_platform_context *platform,
    fractus_framebuffer *ui_framebuffer,
    fractus_framebuffer *drawing_framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    const fractus_app_palette_file *palette_files,
    size_t palette_file_count,
    size_t *palette_file_page,
    char *selected_graphic_path,
    fractus_legacy_config *legacy_config,
    const char *cfg_path,
    char *error_message,
    size_t error_message_size,
    fractus_app_view *view);

fractus_status fractus_app_run_palette_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_app_view *view);

fractus_status fractus_app_run_palette_edit_select_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    uint32_t *palette_selected_index,
    fractus_color_rgba8 *palette_original_color,
    fractus_color_rgba8 *palette_pending_color,
    fractus_ui_numeric_field *red_field,
    fractus_ui_numeric_field *green_field,
    fractus_ui_numeric_field *blue_field,
    fractus_app_view *view);

fractus_status fractus_app_run_palette_edit_color_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    const char *cfg_path,
    fractus_legacy_config *legacy_config,
    uint32_t palette_selected_index,
    fractus_color_rgba8 palette_original_color,
    fractus_color_rgba8 *palette_pending_color,
    fractus_ui_numeric_field *red_field,
    fractus_ui_numeric_field *green_field,
    fractus_ui_numeric_field *blue_field,
    fractus_app_view *view);

fractus_status fractus_app_run_palette_copy_source_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_color_rgba8 *palette_copy_color,
    uint32_t *palette_copy_source_index,
    fractus_app_view *view);

fractus_status fractus_app_run_palette_copy_targets_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    const char *cfg_path,
    fractus_legacy_config *legacy_config,
    uint32_t palette_copy_source_index,
    fractus_color_rgba8 palette_copy_color,
    fractus_app_view *view);

fractus_status fractus_app_run_palette_gradient_first_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    uint32_t *palette_gradient_first_index,
    fractus_app_view *view);

fractus_status fractus_app_run_palette_gradient_second_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    const char *cfg_path,
    fractus_legacy_config *legacy_config,
    uint32_t palette_gradient_first_index,
    fractus_app_view *view);

#endif
