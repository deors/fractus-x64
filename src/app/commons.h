#ifndef FRACTUS_X64_APP_COMMONS_H
#define FRACTUS_X64_APP_COMMONS_H

#include "core/core.h"
#include "core/fractal.h"
#include "platform/graphics.h"
#include "platform/platform.h"
#include "ui/font.h"
#include "ui/ui.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define FRACTUS_APP_DIALOG_BUTTON_CAPACITY 32u
#define FRACTUS_APP_GRAPHIC_FILE_CAPACITY 256u
#define FRACTUS_APP_GRAPHIC_FILE_PAGE_SIZE 10u
#define FRACTUS_APP_PALETTE_FILE_CAPACITY 256u
#define FRACTUS_APP_PALETTE_FILE_PAGE_SIZE 10u
#define FRACTUS_APP_MENU_LOAD_GRAPHIC_INDEX 10
#define FRACTUS_APP_MENU_SAVE_NEXT_GRAPHIC_INDEX 11
#define FRACTUS_APP_MENU_LOAD_PALETTE_INDEX 12
#define FRACTUS_APP_MENU_SAVE_PALETTE_INDEX 13
#define FRACTUS_APP_MENU_LOAD_GRAPHIC_PALETTE_INDEX 14
#define FRACTUS_APP_MENU_CHANGE_GRAPHIC_PALETTE_INDEX 15
#define FRACTUS_APP_MENU_PALETTE_INDEX 16
#define FRACTUS_APP_MENU_EDIT_PALETTE_COLOR_INDEX 17
#define FRACTUS_APP_MENU_COPY_PALETTE_COLOR_INDEX 18
#define FRACTUS_APP_MENU_GRADIENT_PALETTE_INDEX 19
#define FRACTUS_APP_MENU_RESTORE_PALETTE_INDEX 20
#define FRACTUS_APP_MENU_VIDEO_INDEX 21
#define FRACTUS_APP_MENU_FRACTALS_CONFIG_INDEX 22
#define FRACTUS_APP_MENU_RESTORE_FRACTALS_CONFIG_INDEX 23
#define FRACTUS_APP_MENU_HELP_INDEX 24
#define FRACTUS_APP_MENU_ABOUT_INDEX 25
#define FRACTUS_APP_MENU_EXIT_INDEX 26
#define FRACTUS_APP_PALETTE_GRID_X 51
#define FRACTUS_APP_PALETTE_GRID_Y 80
#define FRACTUS_APP_PALETTE_CELL_WIDTH 28
#define FRACTUS_APP_PALETTE_CELL_HEIGHT 16
#define FRACTUS_APP_PALETTE_CELL_STEP_X 34
#define FRACTUS_APP_PALETTE_CELL_STEP_Y 20
#define FRACTUS_APP_PALETTE_GRID_COLUMNS 16
#define FRACTUS_APP_PALETTE_GRID_ROWS 15
#define FRACTUS_APP_NUMBERED_FILE_LIMIT 9999u
#define FRACTUS_APP_DRAWING_MIN_WIDTH 640u
#define FRACTUS_APP_DRAWING_MIN_HEIGHT 480u

#define FRACTUS_APP_RECT(x1, y1, x2, y2) \
    { (x1), (y1), (x2) - (x1) + 1, (y2) - (y1) + 1 }
#define FRACTUS_APP_ARRAY_COUNT(values) (sizeof(values) / sizeof((values)[0]))

typedef enum fractus_app_view {
    FRACTUS_APP_VIEW_MAIN_MENU = 0,
    FRACTUS_APP_VIEW_MANDELBROT_MENU,
    FRACTUS_APP_VIEW_MANDELBROT_CONFIG,
    FRACTUS_APP_VIEW_MANDELBROT,
    FRACTUS_APP_VIEW_MANDELBROT_DEM_CONFIG,
    FRACTUS_APP_VIEW_MANDELBROT_DEM,
    FRACTUS_APP_VIEW_JULIA_MENU,
    FRACTUS_APP_VIEW_JULIA_CONFIG,
    FRACTUS_APP_VIEW_JULIA,
    FRACTUS_APP_VIEW_JULIA_DEM_CONFIG,
    FRACTUS_APP_VIEW_JULIA_DEM,
    FRACTUS_APP_VIEW_BIOMORPH_CONFIG,
    FRACTUS_APP_VIEW_BIOMORPH,
    FRACTUS_APP_VIEW_PLASMA_MENU,
    FRACTUS_APP_VIEW_PLASMA_RECTANGULAR_CONFIG,
    FRACTUS_APP_VIEW_PLASMA_RECTANGULAR,
    FRACTUS_APP_VIEW_PLASMA_CIRCULAR_CONFIG,
    FRACTUS_APP_VIEW_PLASMA_CIRCULAR,
    FRACTUS_APP_VIEW_ATTRACTORS_MENU,
    FRACTUS_APP_VIEW_ATTRACTORS_CONFIG,
    FRACTUS_APP_VIEW_LORENZ,
    FRACTUS_APP_VIEW_VIDEO_CONFIG,
    FRACTUS_APP_VIEW_FRACTALS_DEFAULT_CONFIG,
    FRACTUS_APP_VIEW_LOAD_GRAPHIC,
    FRACTUS_APP_VIEW_LOAD_PALETTE,
    FRACTUS_APP_VIEW_LOAD_GRAPHIC_PALETTE,
    FRACTUS_APP_VIEW_CHANGE_GRAPHIC_PALETTE_GRAPHIC,
    FRACTUS_APP_VIEW_CHANGE_GRAPHIC_PALETTE_PALETTE,
    FRACTUS_APP_VIEW_GRAPHIC,
    FRACTUS_APP_VIEW_PALETTE,
    FRACTUS_APP_VIEW_PALETTE_EDIT_SELECT,
    FRACTUS_APP_VIEW_PALETTE_EDIT_COLOR,
    FRACTUS_APP_VIEW_PALETTE_COPY_SOURCE,
    FRACTUS_APP_VIEW_PALETTE_COPY_TARGETS,
    FRACTUS_APP_VIEW_PALETTE_GRADIENT_FIRST,
    FRACTUS_APP_VIEW_PALETTE_GRADIENT_SECOND,
    FRACTUS_APP_VIEW_ERROR
} fractus_app_view;

typedef fractus_ui_button fractus_app_menu_entry;

typedef struct fractus_app_palette_file {
    char label[96];
    char path[512];
} fractus_app_palette_file;

typedef struct fractus_app_graphic_file {
    char label[96];
    char path[512];
} fractus_app_graphic_file;

typedef struct fractus_app_mandelbrot_selection {
    int active;
    int has_first_corner;
    fractus_point_i32 first_corner;
} fractus_app_mandelbrot_selection;

void fractus_app_set_button(
    fractus_app_menu_entry *entry,
    int32_t x1,
    int32_t y1,
    int32_t x2,
    int32_t y2,
    uint8_t fill_color,
    uint8_t text_color,
    const char *label);

int32_t fractus_app_clamp_i32(int32_t value, int32_t minimum, int32_t maximum);
double fractus_app_clamp_f64(double value, double minimum, double maximum);
uint8_t fractus_app_scale_vga_to_rgb8(int32_t value);
int32_t fractus_app_scale_rgb8_to_vga(uint8_t value);
fractus_color_rgba8 fractus_app_rgb8(uint8_t r, uint8_t g, uint8_t b);
fractus_color_rgba8 fractus_app_vga_color(int32_t r, int32_t g, int32_t b);

void fractus_app_vga_channels_from_color(
    fractus_color_rgba8 color,
    int32_t *r,
    int32_t *g,
    int32_t *b);

void fractus_app_init_palette_color_fields(
    fractus_ui_numeric_field *red_field,
    fractus_ui_numeric_field *green_field,
    fractus_ui_numeric_field *blue_field,
    fractus_color_rgba8 color);

void fractus_app_log(const char *message);

fractus_status fractus_app_set_message(
    char *destination,
    size_t destination_size,
    const char *message);

int fractus_app_view_is_generated_drawing(fractus_app_view view);

fractus_status fractus_app_resolve_numbered_write_path(
    const fractus_platform_context *platform,
    const char *directory_name,
    const char *extension,
    char *buffer,
    size_t buffer_size);

fractus_status fractus_app_apply_palette(
    fractus_framebuffer *framebuffer,
    const fractus_palette *palette);

fractus_status fractus_app_sync_framebuffer_palette(
    fractus_framebuffer *target,
    const fractus_framebuffer *source);

fractus_status fractus_app_resolve_drawing_video_mode(
    const fractus_legacy_config *config,
    fractus_size_u32 *size,
    int *fullscreen);

fractus_status fractus_app_resize_drawing_framebuffer(
    fractus_framebuffer *framebuffer,
    const fractus_framebuffer *palette_source,
    fractus_size_u32 size);

fractus_status fractus_app_ensure_drawing_framebuffer_size(
    const fractus_legacy_config *config,
    fractus_framebuffer *framebuffer,
    const fractus_framebuffer *palette_source);

fractus_status fractus_app_configure_platform_for_present(
    fractus_platform_context *platform,
    const fractus_legacy_config *config,
    int drawing);

void fractus_app_build_options_from_entries(
    fractus_ui_menu_option *options,
    const fractus_app_menu_entry *entries,
    size_t entry_count);

size_t fractus_app_copy_control_entries(
    fractus_app_menu_entry *entries,
    size_t capacity,
    const fractus_app_menu_entry *controls,
    size_t control_count);

fractus_status fractus_app_copy_framebuffer_for_overlay(
    fractus_framebuffer *target,
    const fractus_framebuffer *source);

fractus_status fractus_app_draw_drawing_footer(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const char *text);

fractus_status fractus_app_draw_drawing_footer_ex(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const char *default_text,
    const char *saved_filename,
    int allow_selection);

fractus_status fractus_app_draw_save_feedback(
    fractus_framebuffer *framebuffer,
    uint32_t frame);

int fractus_app_map_drawing_window_point(
    const fractus_platform_context *platform,
    const fractus_framebuffer *framebuffer,
    fractus_point_i32 window_point,
    fractus_point_i32 *framebuffer_point);

#endif
