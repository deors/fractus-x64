#include "ui/ui.h"

#include "platform/graphics.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

fractus_status fractus_ui_init(fractus_ui_context *ui, fractus_size_u32 logical_size)
{
    if (ui == NULL || logical_size.width == 0u || logical_size.height == 0u) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    ui->logical_size = logical_size;
    ui->pointer_position.x = 0;
    ui->pointer_position.y = 0;
    ui->buttons_down.left = 0;
    ui->buttons_down.right = 0;
    ui->press_event.position = ui->pointer_position;
    ui->press_event.buttons = ui->buttons_down;
    ui->release_event.position = ui->pointer_position;
    ui->release_event.buttons = ui->buttons_down;
    ui->key_pressed = 0u;
    ui->key_released = 0u;
    ui->text_input[0] = '\0';
    ui->active_menu_index = -1;
    ui->active_menu_valid = 0;
    ui->quit_requested = 0;
    ui->press_pending = 0;
    ui->release_pending = 0;
    ui->key_press_pending = 0;
    ui->key_release_pending = 0;
    ui->text_input_pending = 0;
    ui->initialized = 1;
    return FRACTUS_STATUS_OK;
}

void fractus_ui_shutdown(fractus_ui_context *ui)
{
    if (ui == NULL) {
        return;
    }

    ui->initialized = 0;
    ui->logical_size.width = 0u;
    ui->logical_size.height = 0u;
    ui->pointer_position.x = 0;
    ui->pointer_position.y = 0;
    ui->buttons_down.left = 0;
    ui->buttons_down.right = 0;
    ui->press_pending = 0;
    ui->release_pending = 0;
    ui->key_press_pending = 0;
    ui->key_release_pending = 0;
    ui->text_input_pending = 0;
    ui->active_menu_index = -1;
    ui->active_menu_valid = 0;
    ui->quit_requested = 0;
}

void fractus_ui_begin_frame(fractus_ui_context *ui)
{
    if (ui == NULL || !ui->initialized) {
        return;
    }

    ui->press_pending = 0;
    ui->release_pending = 0;
    ui->key_press_pending = 0;
    ui->key_release_pending = 0;
    ui->key_pressed = 0u;
    ui->key_released = 0u;
    ui->text_input_pending = 0;
    ui->text_input[0] = '\0';
}

int fractus_ui_point_in_rect(fractus_point_i32 point, fractus_rect_i32 rect)
{
    return point.x >= rect.x &&
           point.y >= rect.y &&
           point.x < rect.x + rect.width &&
           point.y < rect.y + rect.height;
}

fractus_status fractus_ui_handle_platform_event(
    fractus_ui_context *ui,
    const fractus_platform_event *event)
{
    if (ui == NULL || event == NULL || !ui->initialized) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    switch (event->type) {
    case FRACTUS_PLATFORM_EVENT_QUIT:
        ui->quit_requested = 1;
        break;
    case FRACTUS_PLATFORM_EVENT_WINDOW_RESIZED:
        ui->logical_size = event->data.window_resized.size;
        break;
    case FRACTUS_PLATFORM_EVENT_MOUSE_MOVED:
        ui->pointer_position = event->data.mouse_move.position;
        break;
    case FRACTUS_PLATFORM_EVENT_MOUSE_BUTTON_DOWN:
        ui->pointer_position = event->data.mouse_button.position;
        ui->press_event.position = ui->pointer_position;
        ui->press_event.buttons.left = (event->data.mouse_button.button == 1u);
        ui->press_event.buttons.right = (event->data.mouse_button.button == 3u);
        if (ui->press_event.buttons.left) {
            ui->buttons_down.left = 1;
        }
        if (ui->press_event.buttons.right) {
            ui->buttons_down.right = 1;
        }
        ui->press_pending = 1;
        break;
    case FRACTUS_PLATFORM_EVENT_MOUSE_BUTTON_UP:
        ui->pointer_position = event->data.mouse_button.position;
        ui->release_event.position = ui->pointer_position;
        ui->release_event.buttons.left = (event->data.mouse_button.button == 1u);
        ui->release_event.buttons.right = (event->data.mouse_button.button == 3u);
        if (ui->release_event.buttons.left) {
            ui->buttons_down.left = 0;
        }
        if (ui->release_event.buttons.right) {
            ui->buttons_down.right = 0;
        }
        ui->release_pending = 1;
        break;
    case FRACTUS_PLATFORM_EVENT_KEY_DOWN:
        ui->key_pressed = event->data.key.keycode;
        ui->key_press_pending = 1;
        break;
    case FRACTUS_PLATFORM_EVENT_KEY_UP:
        ui->key_released = event->data.key.keycode;
        ui->key_release_pending = 1;
        break;
    case FRACTUS_PLATFORM_EVENT_TEXT_INPUT:
        snprintf(ui->text_input, sizeof(ui->text_input), "%s", event->data.text_input.text);
        ui->text_input_pending = 1;
        break;
    default:
        break;
    }

    return FRACTUS_STATUS_OK;
}

int fractus_ui_wait_press(
    const fractus_ui_context *ui,
    fractus_ui_pointer_event *press_event,
    int *cancelled)
{
    if (ui == NULL || !ui->initialized) {
        return 0;
    }

    if (cancelled != NULL) {
        *cancelled = ui->quit_requested || (ui->key_press_pending && ui->key_pressed == 27u);
    }

    if (!ui->press_pending) {
        return 0;
    }

    if (press_event != NULL) {
        *press_event = ui->press_event;
    }

    return 1;
}

int fractus_ui_wait_release(
    const fractus_ui_context *ui,
    fractus_ui_pointer_event *release_event)
{
    if (ui == NULL || !ui->initialized || !ui->release_pending) {
        return 0;
    }

    if (release_event != NULL) {
        *release_event = ui->release_event;
    }

    return 1;
}

int fractus_ui_menu(
    fractus_ui_context *ui,
    const fractus_ui_menu_option *options,
    size_t option_count,
    int *selected_index,
    int *cancelled)
{
    size_t i;

    if (selected_index != NULL) {
        *selected_index = -1;
    }

    if (cancelled != NULL) {
        *cancelled = 0;
    }

    if (ui == NULL || options == NULL || option_count == 0u || !ui->initialized) {
        return 0;
    }

    if (cancelled != NULL &&
        (ui->quit_requested || (ui->key_press_pending && ui->key_pressed == 27u))) {
        *cancelled = 1;
        return 1;
    }

    if (ui->press_pending && ui->press_event.buttons.right) {
        if (cancelled != NULL) {
            *cancelled = 1;
        }
        ui->active_menu_valid = 0;
        ui->active_menu_index = -1;
        return 1;
    }

    if (ui->press_pending && ui->press_event.buttons.left) {
        ui->active_menu_valid = 0;
        ui->active_menu_index = -1;

        for (i = 0u; i < option_count; ++i) {
            if (fractus_ui_point_in_rect(ui->press_event.position, options[i].bounds)) {
                ui->active_menu_valid = 1;
                ui->active_menu_index = (int)i;
                break;
            }
        }
    }

    if (ui->release_pending && ui->release_event.buttons.left) {
        if (ui->active_menu_valid &&
            ui->active_menu_index >= 0 &&
            (size_t)ui->active_menu_index < option_count &&
            fractus_ui_point_in_rect(
                ui->release_event.position,
                options[ui->active_menu_index].bounds)) {
            if (selected_index != NULL) {
                *selected_index = ui->active_menu_index;
            }

            ui->active_menu_valid = 0;
            ui->active_menu_index = -1;
            return 1;
        }

        ui->active_menu_valid = 0;
        ui->active_menu_index = -1;
    }

    return 0;
}

int fractus_ui_menu_is_active(
    const fractus_ui_context *ui,
    size_t option_index)
{
    if (ui == NULL || !ui->initialized || !ui->active_menu_valid) {
        return 0;
    }

    return ui->active_menu_index == (int)option_index;
}

int fractus_ui_active_menu_index(
    const fractus_ui_context *ui,
    const fractus_ui_menu_option *options,
    size_t option_count)
{
    if (ui == NULL ||
        options == NULL ||
        !ui->active_menu_valid ||
        !ui->buttons_down.left ||
        ui->active_menu_index < 0 ||
        (size_t)ui->active_menu_index >= option_count) {
        return -1;
    }

    if (!fractus_ui_point_in_rect(ui->pointer_position, options[ui->active_menu_index].bounds)) {
        return -1;
    }

    return ui->active_menu_index;
}

static double fractus_ui_clamp_f64(double value, double min_value, double max_value)
{
    if (value < min_value) {
        return min_value;
    }

    if (value > max_value) {
        return max_value;
    }

    return value;
}

static int32_t fractus_ui_clamp_i32(int32_t value, int32_t min_value, int32_t max_value)
{
    if (value < min_value) {
        return min_value;
    }

    if (value > max_value) {
        return max_value;
    }

    return value;
}

static int fractus_ui_numeric_char_allowed(
    const fractus_ui_numeric_field *field,
    char value,
    size_t position)
{
    size_t i;

    if (value >= '0' && value <= '9') {
        return 1;
    }

    if ((value == '-' || value == '+') && position == 0u) {
        return field->text[0] != '-' && field->text[0] != '+';
    }

    if ((value == '.' || value == ',') && field->kind == FRACTUS_UI_NUMERIC_FLOAT) {
        for (i = 0u; field->text[i] != '\0'; ++i) {
            if (field->text[i] == '.' || field->text[i] == ',') {
                return 0;
            }
        }
        return 1;
    }

    return 0;
}

static int fractus_ui_numeric_text_fits(
    const fractus_ui_numeric_field *field,
    const fractus_font_library *fonts,
    const char *text)
{
    int32_t width;
    int32_t available_width;

    if (field == NULL || text == NULL) {
        return 0;
    }

    if (fonts == NULL || !fonts->initialized) {
        return strlen(text) < sizeof(field->text) - 1u;
    }

    if (fractus_font_measure_text(fonts, FRACTUS_FONT_SMALL, text, &width, NULL) != FRACTUS_STATUS_OK) {
        return 0;
    }

    available_width = field->bounds.width - 12;
    if (available_width < 1) {
        available_width = 1;
    }

    return width <= available_width;
}

static fractus_status fractus_ui_numeric_field_insert_char(
    fractus_ui_numeric_field *field,
    const fractus_font_library *fonts,
    char value)
{
    size_t length;
    size_t i;
    char candidate[32];

    if (field == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (value == ',') {
        value = '.';
    }

    length = strlen(field->text);
    if (length + 1u >= sizeof(field->text)) {
        return FRACTUS_STATUS_OK;
    }

    if (!fractus_ui_numeric_char_allowed(field, value, field->cursor)) {
        return FRACTUS_STATUS_OK;
    }

    memcpy(candidate, field->text, field->cursor);
    candidate[field->cursor] = value;
    memcpy(candidate + field->cursor + 1u, field->text + field->cursor, length - field->cursor + 1u);
    if (!fractus_ui_numeric_text_fits(field, fonts, candidate)) {
        return FRACTUS_STATUS_OK;
    }

    for (i = length + 1u; i > field->cursor; --i) {
        field->text[i] = field->text[i - 1u];
    }

    field->text[field->cursor] = value;
    ++field->cursor;
    return FRACTUS_STATUS_OK;
}

static void fractus_ui_numeric_field_delete_left(fractus_ui_numeric_field *field)
{
    size_t length;
    size_t i;

    if (field == NULL || field->cursor == 0u) {
        return;
    }

    length = strlen(field->text);
    for (i = field->cursor - 1u; i < length; ++i) {
        field->text[i] = field->text[i + 1u];
    }

    --field->cursor;
}

static void fractus_ui_numeric_field_delete_at(fractus_ui_numeric_field *field)
{
    size_t length;
    size_t i;

    if (field == NULL) {
        return;
    }

    length = strlen(field->text);
    if (field->cursor >= length) {
        return;
    }

    for (i = field->cursor; i < length; ++i) {
        field->text[i] = field->text[i + 1u];
    }
}

fractus_status fractus_ui_numeric_field_init_int(
    fractus_ui_numeric_field *field,
    fractus_rect_i32 bounds,
    int32_t value,
    int32_t min_value,
    int32_t max_value)
{
    int32_t clamped;

    if (field == NULL || bounds.width <= 0 || bounds.height <= 0 || min_value > max_value) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    clamped = fractus_ui_clamp_i32(value, min_value, max_value);
    memset(field, 0, sizeof(*field));
    field->bounds = bounds;
    field->kind = FRACTUS_UI_NUMERIC_INT;
    field->min_value = (double)min_value;
    field->max_value = (double)max_value;
    field->precision = 0;
    snprintf(field->text, sizeof(field->text), "%d", (int)clamped);
    snprintf(field->original_text, sizeof(field->original_text), "%s", field->text);
    field->cursor = strlen(field->text);
    return FRACTUS_STATUS_OK;
}

fractus_status fractus_ui_numeric_field_init_float(
    fractus_ui_numeric_field *field,
    fractus_rect_i32 bounds,
    double value,
    double min_value,
    double max_value,
    int precision)
{
    double clamped;

    if (field == NULL || bounds.width <= 0 || bounds.height <= 0 || min_value > max_value || precision < 0 || precision > 9) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    clamped = fractus_ui_clamp_f64(value, min_value, max_value);
    memset(field, 0, sizeof(*field));
    field->bounds = bounds;
    field->kind = FRACTUS_UI_NUMERIC_FLOAT;
    field->min_value = min_value;
    field->max_value = max_value;
    field->precision = precision;
    snprintf(field->text, sizeof(field->text), "%.*f", precision, clamped);
    snprintf(field->original_text, sizeof(field->original_text), "%s", field->text);
    field->cursor = strlen(field->text);
    return FRACTUS_STATUS_OK;
}

fractus_status fractus_ui_numeric_field_begin_edit(fractus_ui_numeric_field *field)
{
    if (field == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    snprintf(field->original_text, sizeof(field->original_text), "%s", field->text);
    field->cursor = strlen(field->text);
    field->editing = 1;
    return FRACTUS_STATUS_OK;
}

void fractus_ui_numeric_field_cancel_edit(fractus_ui_numeric_field *field)
{
    if (field == NULL) {
        return;
    }

    snprintf(field->text, sizeof(field->text), "%s", field->original_text);
    field->cursor = strlen(field->text);
    field->editing = 0;
}

fractus_status fractus_ui_numeric_field_get_int(
    const fractus_ui_numeric_field *field,
    int32_t *value)
{
    char *end;
    long parsed;

    if (field == NULL || value == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    parsed = strtol(field->text, &end, 10);
    if (end == field->text || *end != '\0') {
        return FRACTUS_STATUS_ERROR;
    }

    *value = fractus_ui_clamp_i32((int32_t)parsed, (int32_t)field->min_value, (int32_t)field->max_value);
    return FRACTUS_STATUS_OK;
}

fractus_status fractus_ui_numeric_field_get_float(
    const fractus_ui_numeric_field *field,
    double *value)
{
    char *end;
    double parsed;

    if (field == NULL || value == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    parsed = strtod(field->text, &end);
    if (end == field->text || *end != '\0') {
        return FRACTUS_STATUS_ERROR;
    }

    *value = fractus_ui_clamp_f64(parsed, field->min_value, field->max_value);
    return FRACTUS_STATUS_OK;
}

fractus_status fractus_ui_numeric_field_handle_input(
    fractus_ui_numeric_field *field,
    const fractus_ui_context *ui,
    const fractus_font_library *fonts,
    int *accepted,
    int *cancelled)
{
    size_t i;

    if (field == NULL || ui == NULL || !ui->initialized) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (accepted != NULL) {
        *accepted = 0;
    }
    if (cancelled != NULL) {
        *cancelled = 0;
    }

    if (!field->editing) {
        if (ui->release_pending &&
            ui->release_event.buttons.left &&
            fractus_ui_point_in_rect(ui->release_event.position, field->bounds)) {
            return fractus_ui_numeric_field_begin_edit(field);
        }
        return FRACTUS_STATUS_OK;
    }

    if (ui->key_press_pending) {
        switch (ui->key_pressed) {
        case FRACTUS_PLATFORM_KEY_ESCAPE:
            fractus_ui_numeric_field_cancel_edit(field);
            if (cancelled != NULL) {
                *cancelled = 1;
            }
            return FRACTUS_STATUS_OK;
        case FRACTUS_PLATFORM_KEY_RETURN:
            field->editing = 0;
            if (accepted != NULL) {
                *accepted = 1;
            }
            return FRACTUS_STATUS_OK;
        case FRACTUS_PLATFORM_KEY_BACKSPACE:
            fractus_ui_numeric_field_delete_left(field);
            return FRACTUS_STATUS_OK;
        case FRACTUS_PLATFORM_KEY_DELETE:
            fractus_ui_numeric_field_delete_at(field);
            return FRACTUS_STATUS_OK;
        case FRACTUS_PLATFORM_KEY_LEFT:
            if (field->cursor > 0u) {
                --field->cursor;
            }
            return FRACTUS_STATUS_OK;
        case FRACTUS_PLATFORM_KEY_RIGHT:
            if (field->cursor < strlen(field->text)) {
                ++field->cursor;
            }
            return FRACTUS_STATUS_OK;
        case FRACTUS_PLATFORM_KEY_HOME:
            field->cursor = 0u;
            return FRACTUS_STATUS_OK;
        case FRACTUS_PLATFORM_KEY_END:
            field->cursor = strlen(field->text);
            return FRACTUS_STATUS_OK;
        default:
            break;
        }
    }

    if (ui->text_input_pending) {
        for (i = 0u; ui->text_input[i] != '\0'; ++i) {
            if (fractus_ui_numeric_field_insert_char(field, fonts, ui->text_input[i]) != FRACTUS_STATUS_OK) {
                return FRACTUS_STATUS_ERROR;
            }
        }
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_ui_draw_text_centered(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_font_kind kind,
    int32_t center_x,
    int32_t y,
    uint8_t color_index,
    const char *text)
{
    int32_t width;

    if (fonts == NULL || !fonts->initialized) {
        return FRACTUS_STATUS_OK;
    }

    if (fractus_font_measure_text(fonts, kind, text, &width, NULL) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return fractus_font_draw_text(framebuffer, fonts, kind, center_x - width / 2, y, color_index, text);
}

fractus_status fractus_ui_draw_text_left(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_font_kind kind,
    int32_t x,
    int32_t y,
    uint8_t color_index,
    const char *text)
{
    if (fonts == NULL || !fonts->initialized) {
        return FRACTUS_STATUS_OK;
    }

    return fractus_font_draw_text(framebuffer, fonts, kind, x, y, color_index, text);
}

fractus_status fractus_ui_fill_edges(
    fractus_framebuffer *framebuffer,
    int32_t x1,
    int32_t y1,
    int32_t x2,
    int32_t y2,
    uint8_t border_color,
    uint8_t fill_color)
{
    return fractus_graphics_fill_rect(
        framebuffer,
        (fractus_rect_i32){x1, y1, x2 - x1 + 1, y2 - y1 + 1},
        fill_color) == FRACTUS_STATUS_OK &&
        fractus_graphics_rect(
            framebuffer,
            (fractus_rect_i32){x1, y1, x2 - x1 + 1, y2 - y1 + 1},
            border_color) == FRACTUS_STATUS_OK
        ? FRACTUS_STATUS_OK
        : FRACTUS_STATUS_ERROR;
}

fractus_status fractus_ui_draw_window(
    fractus_framebuffer *framebuffer,
    int32_t x1,
    int32_t y1,
    int32_t x2,
    int32_t y2)
{
    if (fractus_ui_fill_edges(framebuffer, x1, y1, x2 - 1, y2 - 1, 7u, 8u) != FRACTUS_STATUS_OK ||
        fractus_graphics_line(framebuffer, x1, y2, x2, y2, 0u) != FRACTUS_STATUS_OK ||
        fractus_graphics_line(framebuffer, x2, y2, x2, y1, 0u) != FRACTUS_STATUS_OK ||
        fractus_graphics_line(framebuffer, x1 + 1, y2 - 2, x1 + 1, y1 + 1, 15u) != FRACTUS_STATUS_OK ||
        fractus_graphics_line(framebuffer, x1 + 1, y1 + 1, x2 - 2, y1 + 1, 15u) != FRACTUS_STATUS_OK ||
        fractus_graphics_line(framebuffer, x1 + 4, y1 + 22, x1 + 4, y1 + 4, 7u) != FRACTUS_STATUS_OK ||
        fractus_graphics_line(framebuffer, x1 + 4, y1 + 4, x2 - 5, y1 + 4, 7u) != FRACTUS_STATUS_OK ||
        fractus_graphics_line(framebuffer, x1 + 4, y1 + 23, x2 - 4, y1 + 23, 15u) != FRACTUS_STATUS_OK ||
        fractus_graphics_line(framebuffer, x2 - 4, y1 + 23, x2 - 4, y1 + 4, 15u) != FRACTUS_STATUS_OK ||
        fractus_graphics_fill_rect(framebuffer, (fractus_rect_i32){x1 + 5, y1 + 5, x2 - x1 - 9, 18}, 5u) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_ui_draw_frame(
    fractus_framebuffer *framebuffer,
    int32_t x1,
    int32_t y1,
    int32_t x2,
    int32_t y2)
{
    if (fractus_ui_fill_edges(framebuffer, x1, y1, x2 - 1, y2 - 1, 7u, 15u) != FRACTUS_STATUS_OK ||
        fractus_graphics_line(framebuffer, x1 + 1, y2 - 2, x1 + 1, y1 + 1, 0u) != FRACTUS_STATUS_OK ||
        fractus_graphics_line(framebuffer, x1 + 1, y1 + 1, x2 - 2, y1 + 1, 0u) != FRACTUS_STATUS_OK ||
        fractus_graphics_line(framebuffer, x1, y2, x2, y2, 15u) != FRACTUS_STATUS_OK ||
        fractus_graphics_line(framebuffer, x2, y2, x2, y1, 15u) != FRACTUS_STATUS_OK ||
        fractus_graphics_line(framebuffer, x1 + 1, y2 - 1, x2 - 1, y2 - 1, 8u) != FRACTUS_STATUS_OK ||
        fractus_graphics_line(framebuffer, x2 - 1, y2 - 1, x2 - 1, y1 + 1, 8u) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_ui_draw_group_box(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    int32_t x1,
    int32_t y1,
    int32_t x2,
    int32_t y2,
    uint8_t frame_color,
    uint8_t text_color,
    const char *title)
{
    int32_t title_width;

    if (fractus_graphics_rect(framebuffer, (fractus_rect_i32){x1, y1, x2 - x1, y2 - y1}, 7u) != FRACTUS_STATUS_OK ||
        fractus_graphics_rect(framebuffer, (fractus_rect_i32){x1 + 1, y1 + 1, x2 - x1, y2 - y1}, 15u) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    if (fonts == NULL || !fonts->initialized) {
        return FRACTUS_STATUS_OK;
    }

    if (fractus_font_measure_text(fonts, FRACTUS_FONT_SMALL, title, &title_width, NULL) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    if (fractus_graphics_line(framebuffer, x1 + 4, y1, x1 + 6 + title_width, y1, frame_color) != FRACTUS_STATUS_OK ||
        fractus_graphics_line(framebuffer, x1 + 4, y1 + 1, x1 + 6 + title_width, y1 + 1, frame_color) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, x1 + 6, y1 - 4, text_color, title);
}

fractus_status fractus_ui_draw_button(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const fractus_ui_button *entry,
    int pressed)
{
    int32_t x1;
    int32_t y1;
    int32_t x2;
    int32_t y2;
    int32_t text_y;
    int disabled;
    uint8_t dark_edge;

    if (framebuffer == NULL || fonts == NULL || entry == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    x1 = entry->bounds.x;
    y1 = entry->bounds.y;
    x2 = entry->bounds.x + entry->bounds.width - 1;
    y2 = entry->bounds.y + entry->bounds.height - 1;
    disabled = entry->text_color == 7u;
    dark_edge = disabled ? 7u : 0u;
    if (!pressed) {
        if (fractus_graphics_line(framebuffer, x1 + 1, y1, x2 - 1, y1, dark_edge) != FRACTUS_STATUS_OK ||
            fractus_graphics_line(framebuffer, x2, y1 + 1, x2, y2 - 1, dark_edge) != FRACTUS_STATUS_OK ||
            fractus_graphics_line(framebuffer, x2 - 1, y2, x1 + 1, y2, dark_edge) != FRACTUS_STATUS_OK ||
            fractus_graphics_line(framebuffer, x1, y2 - 1, x1, y1 + 1, dark_edge) != FRACTUS_STATUS_OK ||
            fractus_graphics_line(framebuffer, x1 + 1, y2 - 2, x1 + 1, y1 + 1, 15u) != FRACTUS_STATUS_OK ||
            fractus_graphics_line(framebuffer, x1 + 1, y1 + 1, x2 - 2, y1 + 1, 15u) != FRACTUS_STATUS_OK ||
            fractus_graphics_line(framebuffer, x1 + 2, y2 - 3, x1 + 2, y1 + 2, 15u) != FRACTUS_STATUS_OK ||
            fractus_graphics_line(framebuffer, x1 + 2, y1 + 2, x2 - 3, y1 + 2, 15u) != FRACTUS_STATUS_OK ||
            fractus_graphics_line(framebuffer, x1 + 1, y2 - 1, x2 - 1, y2 - 1, 7u) != FRACTUS_STATUS_OK ||
            fractus_graphics_line(framebuffer, x2 - 1, y2 - 1, x2 - 1, y1 + 1, 7u) != FRACTUS_STATUS_OK ||
            fractus_graphics_line(framebuffer, x1 + 2, y2 - 2, x2 - 2, y2 - 2, 7u) != FRACTUS_STATUS_OK ||
            fractus_graphics_line(framebuffer, x2 - 2, y2 - 2, x2 - 2, y1 + 2, 7u) != FRACTUS_STATUS_OK ||
            fractus_graphics_fill_rect(framebuffer, (fractus_rect_i32){x1 + 3, y1 + 3, x2 - x1 - 5, y2 - y1 - 5}, entry->fill_color) != FRACTUS_STATUS_OK) {
            return FRACTUS_STATUS_ERROR;
        }
        text_y = (y1 + y2) / 2 - 5;
    } else {
        if (fractus_graphics_line(framebuffer, x1 + 1, y1, x2 - 1, y1, dark_edge) != FRACTUS_STATUS_OK ||
            fractus_graphics_line(framebuffer, x2, y1 + 1, x2, y2 - 1, dark_edge) != FRACTUS_STATUS_OK ||
            fractus_graphics_line(framebuffer, x2 - 1, y2, x1 + 1, y2, dark_edge) != FRACTUS_STATUS_OK ||
            fractus_graphics_line(framebuffer, x1, y2 - 1, x1, y1 + 1, dark_edge) != FRACTUS_STATUS_OK ||
            fractus_graphics_line(framebuffer, x1 + 1, y2 - 1, x1 + 1, y1 + 1, 7u) != FRACTUS_STATUS_OK ||
            fractus_graphics_line(framebuffer, x1 + 1, y1 + 1, x2 - 1, y1 + 1, 7u) != FRACTUS_STATUS_OK ||
            fractus_graphics_fill_rect(framebuffer, (fractus_rect_i32){x1 + 2, y1 + 2, x2 - x1 - 2, y2 - y1 - 2}, entry->fill_color) != FRACTUS_STATUS_OK) {
            return FRACTUS_STATUS_ERROR;
        }
        text_y = (y1 + y2) / 2 - 4;
    }

    return fractus_ui_draw_text_centered(
        framebuffer,
        fonts,
        FRACTUS_FONT_SMALL,
        (x1 + x2) / 2,
        text_y,
        entry->text_color,
        entry->label);
}

fractus_status fractus_ui_draw_button_list(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const fractus_ui_button *entries,
    size_t entry_count,
    int active_index)
{
    size_t i;

    if (framebuffer == NULL || fonts == NULL || entries == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    for (i = 0u; i < entry_count; ++i) {
        if (fractus_ui_draw_button(framebuffer, fonts, &entries[i], active_index == (int)i && entries[i].text_color != 7u) != FRACTUS_STATUS_OK) {
            return FRACTUS_STATUS_ERROR;
        }
    }

    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_ui_draw_radio_mark(
    fractus_framebuffer *framebuffer,
    int32_t center_x,
    int32_t center_y,
    int selected)
{
    if (fractus_graphics_line(framebuffer, center_x - 2, center_y - 4, center_x + 2, center_y - 4, 15u) != FRACTUS_STATUS_OK ||
        fractus_graphics_line(framebuffer, center_x - 4, center_y - 3, center_x + 4, center_y - 3, 15u) != FRACTUS_STATUS_OK ||
        fractus_graphics_fill_rect(framebuffer, (fractus_rect_i32){center_x - 4, center_y - 2, 9, 5}, 15u) != FRACTUS_STATUS_OK ||
        fractus_graphics_line(framebuffer, center_x - 4, center_y + 3, center_x + 4, center_y + 3, 15u) != FRACTUS_STATUS_OK ||
        fractus_graphics_line(framebuffer, center_x - 2, center_y + 4, center_x + 2, center_y + 4, 15u) != FRACTUS_STATUS_OK ||
        fractus_graphics_line(framebuffer, center_x - 2, center_y - 5, center_x + 2, center_y - 5, 0u) != FRACTUS_STATUS_OK ||
        fractus_graphics_line(framebuffer, center_x - 4, center_y - 4, center_x - 3, center_y - 4, 0u) != FRACTUS_STATUS_OK ||
        fractus_graphics_line(framebuffer, center_x + 3, center_y - 4, center_x + 4, center_y - 4, 0u) != FRACTUS_STATUS_OK ||
        fractus_graphics_line(framebuffer, center_x - 5, center_y - 3, center_x - 5, center_y + 3, 0u) != FRACTUS_STATUS_OK ||
        fractus_graphics_line(framebuffer, center_x + 5, center_y - 3, center_x + 5, center_y + 3, 0u) != FRACTUS_STATUS_OK ||
        fractus_graphics_line(framebuffer, center_x - 4, center_y + 4, center_x - 3, center_y + 4, 0u) != FRACTUS_STATUS_OK ||
        fractus_graphics_line(framebuffer, center_x + 3, center_y + 4, center_x + 4, center_y + 4, 0u) != FRACTUS_STATUS_OK ||
        fractus_graphics_line(framebuffer, center_x - 2, center_y + 5, center_x + 2, center_y + 5, 0u) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    if (selected &&
        (fractus_graphics_line(framebuffer, center_x - 1, center_y - 2, center_x + 1, center_y - 2, 0u) != FRACTUS_STATUS_OK ||
         fractus_graphics_line(framebuffer, center_x - 2, center_y - 1, center_x + 2, center_y - 1, 0u) != FRACTUS_STATUS_OK ||
         fractus_graphics_fill_rect(framebuffer, (fractus_rect_i32){center_x - 2, center_y, 5, 2}, 0u) != FRACTUS_STATUS_OK ||
         fractus_graphics_line(framebuffer, center_x - 1, center_y + 2, center_x + 1, center_y + 2, 0u) != FRACTUS_STATUS_OK)) {
        return FRACTUS_STATUS_ERROR;
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_ui_draw_radio_option(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const fractus_ui_radio_option *option,
    int selected,
    int active)
{
    int32_t center_y;
    uint8_t text_color;

    if (framebuffer == NULL || fonts == NULL || option == NULL || option->label == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    center_y = option->bounds.y + option->bounds.height / 2;
    text_color = active ? 6u : 0u;

    if (fractus_ui_draw_radio_mark(framebuffer, option->bounds.x + 10, center_y, selected) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return fractus_ui_draw_text_left(
        framebuffer,
        fonts,
        FRACTUS_FONT_SMALL,
        option->bounds.x + 28,
        center_y - 5,
        text_color,
        option->label);
}

fractus_status fractus_ui_draw_radio_list(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const fractus_ui_radio_option *options,
    size_t option_count,
    int selected_index,
    int active_index)
{
    size_t i;

    if (framebuffer == NULL || fonts == NULL || options == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    for (i = 0u; i < option_count; ++i) {
        if (fractus_ui_draw_radio_option(
                framebuffer,
                fonts,
                &options[i],
                selected_index == (int)i,
                active_index == (int)i) != FRACTUS_STATUS_OK) {
            return FRACTUS_STATUS_ERROR;
        }
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_ui_draw_value_box(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    int32_t x1,
    int32_t y1,
    int32_t x2,
    int32_t y2,
    const char *value)
{
    if (framebuffer == NULL || fonts == NULL || value == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_ui_draw_frame(framebuffer, x1, y1, x2, y2) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return fractus_ui_draw_text_centered(
        framebuffer,
        fonts,
        FRACTUS_FONT_SMALL,
        (x1 + x2) / 2,
        y1 + 5,
        0u,
        value);
}

fractus_status fractus_ui_draw_numeric_field(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const fractus_ui_numeric_field *field)
{
    char prefix[32];
    int32_t prefix_width = 0;
    int32_t text_x;
    int32_t text_y;
    size_t prefix_length;

    if (framebuffer == NULL || fonts == NULL || field == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (!field->editing) {
        return fractus_ui_draw_value_box(
            framebuffer,
            fonts,
            field->bounds.x,
            field->bounds.y,
            field->bounds.x + field->bounds.width - 1,
            field->bounds.y + field->bounds.height - 1,
            field->text);
    }

    if (fractus_ui_draw_frame(
            framebuffer,
            field->bounds.x,
            field->bounds.y,
            field->bounds.x + field->bounds.width - 1,
            field->bounds.y + field->bounds.height - 1) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    text_x = field->bounds.x + 6;
    text_y = field->bounds.y + 5;
    if (fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, text_x, text_y, 0u, field->text) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    if (fonts->initialized) {
        prefix_length = field->cursor;
        if (prefix_length >= sizeof(prefix)) {
            prefix_length = sizeof(prefix) - 1u;
        }
        memcpy(prefix, field->text, prefix_length);
        prefix[prefix_length] = '\0';

        if (fractus_font_measure_text(fonts, FRACTUS_FONT_SMALL, prefix, &prefix_width, NULL) != FRACTUS_STATUS_OK) {
            return FRACTUS_STATUS_ERROR;
        }
    }

    return fractus_graphics_line(
        framebuffer,
        text_x + prefix_width,
        field->bounds.y + 4,
        text_x + prefix_width,
        field->bounds.y + field->bounds.height - 5,
        0u);
}

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
    const char *value)
{
    if (framebuffer == NULL || fonts == NULL || label == NULL || value == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, label_x, label_y, 0u, label) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return fractus_ui_draw_value_box(framebuffer, fonts, value_x1, value_y1, value_x2, value_y2, value);
}
