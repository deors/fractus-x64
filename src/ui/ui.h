#ifndef FRACTUS_X64_UI_H
#define FRACTUS_X64_UI_H

#include "platform/platform.h"
#include "platform/types.h"
#include "ui/font.h"

typedef struct fractus_ui_buttons {
    int left;
    int right;
} fractus_ui_buttons;

typedef struct fractus_ui_pointer_event {
    fractus_point_i32 position;
    fractus_ui_buttons buttons;
} fractus_ui_pointer_event;

typedef struct fractus_ui_menu_option {
    fractus_rect_i32 bounds;
} fractus_ui_menu_option;

typedef struct fractus_ui_button {
    fractus_rect_i32 bounds;
    uint8_t fill_color;
    uint8_t text_color;
    const char *label;
} fractus_ui_button;

typedef struct fractus_ui_radio_option {
    fractus_rect_i32 bounds;
    const char *label;
} fractus_ui_radio_option;

typedef enum fractus_ui_numeric_kind {
    FRACTUS_UI_NUMERIC_INT = 0,
    FRACTUS_UI_NUMERIC_FLOAT
} fractus_ui_numeric_kind;

typedef struct fractus_ui_numeric_field {
    fractus_rect_i32 bounds;
    fractus_ui_numeric_kind kind;
    char text[32];
    char original_text[32];
    size_t cursor;
    double min_value;
    double max_value;
    int precision;
    int editing;
} fractus_ui_numeric_field;

typedef struct fractus_ui_context {
    fractus_size_u32 logical_size;
    fractus_point_i32 pointer_position;
    fractus_ui_buttons buttons_down;
    fractus_ui_pointer_event press_event;
    fractus_ui_pointer_event release_event;
    uint32_t key_pressed;
    uint32_t key_released;
    char text_input[32];
    int active_menu_index;
    int active_menu_valid;
    int quit_requested;
    int press_pending;
    int release_pending;
    int key_press_pending;
    int key_release_pending;
    int text_input_pending;
    int initialized;
} fractus_ui_context;

fractus_status fractus_ui_init(fractus_ui_context *ui, fractus_size_u32 logical_size);
void fractus_ui_shutdown(fractus_ui_context *ui);

void fractus_ui_begin_frame(fractus_ui_context *ui);
fractus_status fractus_ui_handle_platform_event(
    fractus_ui_context *ui,
    const fractus_platform_event *event);

int fractus_ui_wait_press(
    const fractus_ui_context *ui,
    fractus_ui_pointer_event *press_event,
    int *cancelled);
int fractus_ui_wait_release(
    const fractus_ui_context *ui,
    fractus_ui_pointer_event *release_event);
int fractus_ui_menu(
    fractus_ui_context *ui,
    const fractus_ui_menu_option *options,
    size_t option_count,
    int *selected_index,
    int *cancelled);
int fractus_ui_menu_is_active(
    const fractus_ui_context *ui,
    size_t option_index);
int fractus_ui_point_in_rect(fractus_point_i32 point, fractus_rect_i32 rect);
int fractus_ui_active_menu_index(
    const fractus_ui_context *ui,
    const fractus_ui_menu_option *options,
    size_t option_count);
fractus_status fractus_ui_numeric_field_init_int(
    fractus_ui_numeric_field *field,
    fractus_rect_i32 bounds,
    int32_t value,
    int32_t min_value,
    int32_t max_value);
fractus_status fractus_ui_numeric_field_init_float(
    fractus_ui_numeric_field *field,
    fractus_rect_i32 bounds,
    double value,
    double min_value,
    double max_value,
    int precision);
fractus_status fractus_ui_numeric_field_begin_edit(fractus_ui_numeric_field *field);
void fractus_ui_numeric_field_cancel_edit(fractus_ui_numeric_field *field);
fractus_status fractus_ui_numeric_field_handle_input(
    fractus_ui_numeric_field *field,
    const fractus_ui_context *ui,
    const fractus_font_library *fonts,
    int *accepted,
    int *cancelled);
fractus_status fractus_ui_numeric_field_get_int(
    const fractus_ui_numeric_field *field,
    int32_t *value);
fractus_status fractus_ui_numeric_field_get_float(
    const fractus_ui_numeric_field *field,
    double *value);

fractus_status fractus_ui_draw_text_centered(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_font_kind kind,
    int32_t center_x,
    int32_t y,
    uint8_t color_index,
    const char *text);
fractus_status fractus_ui_draw_text_left(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_font_kind kind,
    int32_t x,
    int32_t y,
    uint8_t color_index,
    const char *text);
fractus_status fractus_ui_fill_edges(
    fractus_framebuffer *framebuffer,
    int32_t x1,
    int32_t y1,
    int32_t x2,
    int32_t y2,
    uint8_t border_color,
    uint8_t fill_color);
fractus_status fractus_ui_draw_window(
    fractus_framebuffer *framebuffer,
    int32_t x1,
    int32_t y1,
    int32_t x2,
    int32_t y2);
fractus_status fractus_ui_draw_frame(
    fractus_framebuffer *framebuffer,
    int32_t x1,
    int32_t y1,
    int32_t x2,
    int32_t y2);
fractus_status fractus_ui_draw_group_box(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    int32_t x1,
    int32_t y1,
    int32_t x2,
    int32_t y2,
    uint8_t frame_color,
    uint8_t text_color,
    const char *title);
fractus_status fractus_ui_draw_button(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const fractus_ui_button *entry,
    int pressed);
fractus_status fractus_ui_draw_button_list(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const fractus_ui_button *entries,
    size_t entry_count,
    int active_index);
fractus_status fractus_ui_draw_radio_option(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const fractus_ui_radio_option *option,
    int selected,
    int active);
fractus_status fractus_ui_draw_radio_list(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const fractus_ui_radio_option *options,
    size_t option_count,
    int selected_index,
    int active_index);
fractus_status fractus_ui_draw_value_box(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    int32_t x1,
    int32_t y1,
    int32_t x2,
    int32_t y2,
    const char *value);
fractus_status fractus_ui_draw_numeric_field(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const fractus_ui_numeric_field *field);
fractus_status fractus_ui_draw_numeric_row(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    int32_t label_x,
    int32_t label_y,
    const char *label,
    int32_t value_x1,
    int32_t value_y1,
    int32_t value_x2,
    int32_t value_y2,
    const char *value);

#endif
