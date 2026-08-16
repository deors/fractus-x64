#include "app/fractal.h"
#include "app/files.h"
#include "core/fractal.h"

#include <stdio.h>
#include <string.h>

static size_t fractus_app_build_mandel_config_entries(
    fractus_app_menu_entry *entries,
    size_t capacity)
{
    static const fractus_app_menu_entry controls[] = {
        {FRACTUS_APP_RECT(379, 152, 419, 172), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 152, 464, 172), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 182, 419, 202), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 182, 464, 202), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 212, 419, 232), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 212, 464, 232), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 242, 419, 262), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 242, 464, 262), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 272, 419, 292), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 272, 464, 292), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 302, 419, 322), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 302, 464, 322), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(299, 332, 371, 352), 8u, 0u, "Clasico"},
        {FRACTUS_APP_RECT(379, 332, 464, 352), 8u, 0u, "Suave"},
        {FRACTUS_APP_RECT(210, 387, 310, 407), 8u, 0u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 387, 430, 407), 0u, 15u, "Cancelar"}
    };

    return fractus_app_copy_control_entries(
        entries,
        capacity,
        controls,
        FRACTUS_APP_ARRAY_COUNT(controls));
}

fractus_status fractus_app_run_mandelbrot_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_mandelbrot_params *params,
    fractus_mandelbrot_params *pending,
    fractus_app_view *view)
{
    char buffer[32];
    const fractus_ui_radio_option color_options[] = {
        {FRACTUS_APP_RECT(299, 332, 371, 352), "Clasico"},
        {FRACTUS_APP_RECT(379, 332, 464, 352), "Suave"}
    };
    fractus_app_menu_entry dialog_entries[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    fractus_ui_menu_option dialog_options[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    size_t dialog_entry_count;
    int active_index;
    int selected_menu = -1;
    int cancelled = 0;

    if (framebuffer == NULL || fonts == NULL || ui == NULL ||
        params == NULL || pending == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    dialog_entry_count = fractus_app_build_mandel_config_entries(dialog_entries, FRACTUS_APP_ARRAY_COUNT(dialog_entries));
    fractus_app_build_options_from_entries(dialog_options, dialog_entries, dialog_entry_count);
    active_index = fractus_ui_active_menu_index(ui, dialog_options, dialog_entry_count);

    /* 1. Contenedor exterior. */
    if (fractus_app_render_main_menu(framebuffer, fonts, -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_window(framebuffer, 160, 112, 479, 417) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, 320, 116, 15u, "Conjunto de Benoit B. Mandelbrot") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 2. Textos y controles. */
    if (fractus_ui_draw_group_box(framebuffer, fonts, 165, 142, 474, 361, 8u, 0u, "Parametros del conjunto") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    snprintf(buffer, sizeof(buffer), "%.3f", pending->xmin);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 157, "Minimo valor real", 299, 152, 371, 172, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", pending->xmax);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 187, "Maximo valor real", 299, 182, 371, 202, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", pending->ymin);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 217, "Minimo valor imaginario", 299, 212, 371, 232, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", pending->ymax);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 247, "Maximo valor imaginario", 299, 242, 371, 262, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%u", pending->max_iterations);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 277, "Iteraciones", 299, 272, 371, 292, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.0f", pending->escape_radius_squared);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 307, "Radio de escape", 299, 302, 371, 322, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    if (fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 175, 337, 0u, "Color") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_radio_list(
            framebuffer,
            fonts,
            color_options,
            FRACTUS_APP_ARRAY_COUNT(color_options),
            pending->color_mode == FRACTUS_MANDELBROT_COLOR_SMOOTH ? 1 : 0,
            (active_index == FRACTUS_APP_MANDEL_COLOR_CLASSIC || active_index == FRACTUS_APP_MANDEL_COLOR_SMOOTH) ?
                active_index - FRACTUS_APP_MANDEL_COLOR_CLASSIC :
                -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, 320, 367, 0u, "Estos valores solo afectan al dibujo actual y no cambian la cfg global.") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_button_list(framebuffer, fonts, dialog_entries, FRACTUS_APP_MANDEL_COLOR_CLASSIC, active_index) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_button_list(
            framebuffer,
            fonts,
            &dialog_entries[FRACTUS_APP_MANDEL_DRAW],
            2u,
            (active_index >= FRACTUS_APP_MANDEL_DRAW) ? active_index - FRACTUS_APP_MANDEL_DRAW : -1) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones de botones. */
    if (fractus_ui_menu(ui, dialog_options, dialog_entry_count, &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == FRACTUS_APP_MANDEL_CANCEL) {
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (selected_menu == FRACTUS_APP_MANDEL_DRAW) {
            *params = *pending;
            *view = FRACTUS_APP_VIEW_MANDELBROT;
        } else if (selected_menu == FRACTUS_APP_MANDEL_XMIN_DEC) {
            pending->xmin = fractus_app_clamp_f64(pending->xmin - 0.1, -5.0, pending->xmax - 0.1);
        } else if (selected_menu == FRACTUS_APP_MANDEL_XMIN_INC) {
            pending->xmin = fractus_app_clamp_f64(pending->xmin + 0.1, -5.0, pending->xmax - 0.1);
        } else if (selected_menu == FRACTUS_APP_MANDEL_XMAX_DEC) {
            pending->xmax = fractus_app_clamp_f64(pending->xmax - 0.1, pending->xmin + 0.1, 5.0);
        } else if (selected_menu == FRACTUS_APP_MANDEL_XMAX_INC) {
            pending->xmax = fractus_app_clamp_f64(pending->xmax + 0.1, pending->xmin + 0.1, 5.0);
        } else if (selected_menu == FRACTUS_APP_MANDEL_YMIN_DEC) {
            pending->ymin = fractus_app_clamp_f64(pending->ymin - 0.1, -5.0, pending->ymax - 0.1);
        } else if (selected_menu == FRACTUS_APP_MANDEL_YMIN_INC) {
            pending->ymin = fractus_app_clamp_f64(pending->ymin + 0.1, -5.0, pending->ymax - 0.1);
        } else if (selected_menu == FRACTUS_APP_MANDEL_YMAX_DEC) {
            pending->ymax = fractus_app_clamp_f64(pending->ymax - 0.1, pending->ymin + 0.1, 5.0);
        } else if (selected_menu == FRACTUS_APP_MANDEL_YMAX_INC) {
            pending->ymax = fractus_app_clamp_f64(pending->ymax + 0.1, pending->ymin + 0.1, 5.0);
        } else if (selected_menu == FRACTUS_APP_MANDEL_ITER_DEC) {
            pending->max_iterations = (uint32_t)fractus_app_clamp_i32((int32_t)pending->max_iterations - 16, 16, 1024);
        } else if (selected_menu == FRACTUS_APP_MANDEL_ITER_INC) {
            pending->max_iterations = (uint32_t)fractus_app_clamp_i32((int32_t)pending->max_iterations + 16, 16, 1024);
        } else if (selected_menu == FRACTUS_APP_MANDEL_RADIUS_DEC) {
            pending->escape_radius_squared = (double)fractus_app_clamp_i32((int32_t)pending->escape_radius_squared - 1, 1, 1000);
        } else if (selected_menu == FRACTUS_APP_MANDEL_RADIUS_INC) {
            pending->escape_radius_squared = (double)fractus_app_clamp_i32((int32_t)pending->escape_radius_squared + 1, 1, 1000);
        } else if (selected_menu == FRACTUS_APP_MANDEL_COLOR_CLASSIC) {
            pending->color_mode = FRACTUS_MANDELBROT_COLOR_ESCAPE;
        } else if (selected_menu == FRACTUS_APP_MANDEL_COLOR_SMOOTH) {
            pending->color_mode = FRACTUS_MANDELBROT_COLOR_SMOOTH;
        }
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_app_draw_drawing_footer(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const char *text)
{
    uint32_t scale;
    int32_t text_width;
    int32_t text_height;
    int32_t footer_height;
    int32_t footer_y;
    int32_t text_y;
    int32_t text_x;

    if (framebuffer == NULL || fonts == NULL || text == NULL || framebuffer->size.height < 24u) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    scale = 1u;
    if (framebuffer->size.height >= 2000u) {
        scale = 4u;
    } else if (framebuffer->size.height >= 1200u) {
        scale = 3u;
    } else if (framebuffer->size.height >= 720u) {
        scale = 2u;
    }

    if (fractus_font_measure_text(fonts, FRACTUS_FONT_ARIAL, text, &text_width, &text_height) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    text_width *= (int32_t)scale;
    text_height *= (int32_t)scale;
    footer_height = text_height + (int32_t)(12u * scale);
    if (footer_height < 24) {
        footer_height = 24;
    }
    if ((uint32_t)footer_height > framebuffer->size.height) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    footer_y = (int32_t)framebuffer->size.height - footer_height;
    text_y = footer_y + (footer_height - text_height) / 2;
    text_x = ((int32_t)framebuffer->size.width - text_width) / 2;
    if (text_x < 0) {
        text_x = 0;
    }

    if (fractus_graphics_fill_rect(
            framebuffer,
            (fractus_rect_i32){0, footer_y, (int32_t)framebuffer->size.width, footer_height},
            7u) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return fractus_font_draw_text_scaled(
        framebuffer,
        fonts,
        FRACTUS_FONT_ARIAL,
        text_x,
        text_y,
        15u,
        text,
        scale);
}

fractus_status fractus_app_draw_save_feedback(
    fractus_framebuffer *framebuffer,
    uint32_t frame)
{
    int32_t thickness;
    int32_t i;

    if (framebuffer == NULL || !framebuffer->initialized) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (frame >= 3u) {
        return fractus_graphics_fill_rect(
            framebuffer,
            (fractus_rect_i32){0, 0, (int32_t)framebuffer->size.width, (int32_t)framebuffer->size.height},
            15u);
    }

    thickness = (int32_t)(framebuffer->size.height / 120u);
    if (thickness < 4) {
        thickness = 4;
    }

    for (i = 0; i < thickness; ++i) {
        if (fractus_graphics_rect(
                framebuffer,
                (fractus_rect_i32){
                    i,
                    i,
                    (int32_t)framebuffer->size.width - i * 2,
                    (int32_t)framebuffer->size.height - i * 2},
                15u) != FRACTUS_STATUS_OK) {
            return FRACTUS_STATUS_ERROR;
        }
    }

    return FRACTUS_STATUS_OK;
}

static int fractus_app_map_drawing_window_point(
    const fractus_platform_context *platform,
    const fractus_framebuffer *framebuffer,
    fractus_point_i32 window_point,
    fractus_point_i32 *framebuffer_point)
{
    fractus_size_u32 output_size;
    double scale_x;
    double scale_y;
    double scale;
    int32_t destination_x;
    int32_t destination_y;
    int32_t destination_width;
    int32_t destination_height;

    if (platform == NULL || framebuffer == NULL || framebuffer_point == NULL ||
        !framebuffer->initialized || framebuffer->size.width == 0u || framebuffer->size.height == 0u) {
        return 0;
    }

    if (fractus_platform_get_output_size(platform, &output_size) != FRACTUS_STATUS_OK ||
        output_size.width == 0u || output_size.height == 0u) {
        return 0;
    }

    scale_x = (double)output_size.width / (double)framebuffer->size.width;
    scale_y = (double)output_size.height / (double)framebuffer->size.height;
    scale = (scale_x < scale_y) ? scale_x : scale_y;
    if (scale <= 0.0) {
        return 0;
    }

    destination_width = (int32_t)((double)framebuffer->size.width * scale);
    destination_height = (int32_t)((double)framebuffer->size.height * scale);
    destination_x = ((int32_t)output_size.width - destination_width) / 2;
    destination_y = ((int32_t)output_size.height - destination_height) / 2;

    if (window_point.x < destination_x ||
        window_point.y < destination_y ||
        window_point.x >= destination_x + destination_width ||
        window_point.y >= destination_y + destination_height) {
        return 0;
    }

    framebuffer_point->x = (int32_t)((double)(window_point.x - destination_x) / scale);
    framebuffer_point->y = (int32_t)((double)(window_point.y - destination_y) / scale);
    framebuffer_point->x = fractus_app_clamp_i32(framebuffer_point->x, 0, (int32_t)framebuffer->size.width - 1);
    framebuffer_point->y = fractus_app_clamp_i32(framebuffer_point->y, 0, (int32_t)framebuffer->size.height - 1);
    return 1;
}

static int fractus_app_apply_mandelbrot_selection(
    const fractus_framebuffer *framebuffer,
    fractus_mandelbrot_params *params,
    fractus_point_i32 first,
    fractus_point_i32 second)
{
    int32_t x_min;
    int32_t x_max;
    int32_t y_min;
    int32_t y_max;
    double old_xmin;
    double old_xmax;
    double old_ymin;
    double old_ymax;
    double dx;
    double dy;

    if (framebuffer == NULL || params == NULL ||
        !framebuffer->initialized || framebuffer->size.width < 2u || framebuffer->size.height < 2u) {
        return 0;
    }

    x_min = (first.x < second.x) ? first.x : second.x;
    x_max = (first.x > second.x) ? first.x : second.x;
    y_min = (first.y < second.y) ? first.y : second.y;
    y_max = (first.y > second.y) ? first.y : second.y;
    if (x_min == x_max || y_min == y_max) {
        return 0;
    }

    old_xmin = params->xmin;
    old_xmax = params->xmax;
    old_ymin = params->ymin;
    old_ymax = params->ymax;
    dx = (old_xmax - old_xmin) / (double)(framebuffer->size.width - 1u);
    dy = (old_ymax - old_ymin) / (double)(framebuffer->size.height - 1u);

    params->xmin = old_xmin + (double)x_min * dx;
    params->xmax = old_xmin + (double)x_max * dx;
    params->ymax = old_ymax - (double)y_min * dy;
    params->ymin = old_ymax - (double)y_max * dy;
    return 1;
}

fractus_status fractus_app_draw_mandelbrot_selection_overlay(
    const fractus_platform_context *platform,
    fractus_framebuffer *framebuffer,
    const fractus_ui_context *ui,
    const fractus_app_mandelbrot_selection *selection)
{
    fractus_point_i32 cursor;

    if (platform == NULL || framebuffer == NULL || ui == NULL || selection == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (!selection->active ||
        !fractus_app_map_drawing_window_point(platform, framebuffer, ui->pointer_position, &cursor)) {
        return FRACTUS_STATUS_OK;
    }

    if (fractus_graphics_line(framebuffer, 0, cursor.y, (int32_t)framebuffer->size.width - 1, cursor.y, 15u) != FRACTUS_STATUS_OK ||
        fractus_graphics_line(framebuffer, cursor.x, 0, cursor.x, (int32_t)framebuffer->size.height - 1, 15u) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    if (selection->has_first_corner) {
        int32_t x = (selection->first_corner.x < cursor.x) ? selection->first_corner.x : cursor.x;
        int32_t y = (selection->first_corner.y < cursor.y) ? selection->first_corner.y : cursor.y;
        int32_t width = (selection->first_corner.x > cursor.x) ?
            selection->first_corner.x - cursor.x + 1 :
            cursor.x - selection->first_corner.x + 1;
        int32_t height = (selection->first_corner.y > cursor.y) ?
            selection->first_corner.y - cursor.y + 1 :
            cursor.y - selection->first_corner.y + 1;

        if (fractus_graphics_rect(framebuffer, (fractus_rect_i32){x, y, width, height}, 15u) != FRACTUS_STATUS_OK) {
            return FRACTUS_STATUS_ERROR;
        }
    }

    return FRACTUS_STATUS_OK;
}

int fractus_app_handle_mandelbrot_selection_input(
    const fractus_platform_context *platform,
    const fractus_framebuffer *framebuffer,
    const fractus_ui_context *ui,
    fractus_mandelbrot_params *params,
    fractus_app_mandelbrot_selection *selection)
{
    fractus_point_i32 selected_point;

    if (platform == NULL || framebuffer == NULL || ui == NULL || params == NULL || selection == NULL) {
        return 0;
    }

    if (ui->key_press_pending && (ui->key_pressed == 's' || ui->key_pressed == 'S')) {
        selection->active = 1;
        selection->has_first_corner = 0;
    }

    if (!selection->active) {
        return 0;
    }

    if ((ui->key_press_pending && ui->key_pressed == 27u) ||
        (ui->press_pending && ui->press_event.buttons.right)) {
        selection->active = 0;
        selection->has_first_corner = 0;
        return 0;
    }

    if (ui->press_pending && ui->press_event.buttons.left &&
        fractus_app_map_drawing_window_point(platform, framebuffer, ui->press_event.position, &selected_point)) {
        if (!selection->has_first_corner) {
            selection->first_corner = selected_point;
            selection->has_first_corner = 1;
        } else if (fractus_app_apply_mandelbrot_selection(framebuffer, params, selection->first_corner, selected_point)) {
            selection->active = 0;
            selection->has_first_corner = 0;
            return 1;
        }
    }

    return 0;
}

fractus_status fractus_app_copy_framebuffer_for_overlay(
    fractus_framebuffer *target,
    const fractus_framebuffer *source)
{
    size_t pixel_count;

    if (target == NULL || source == NULL || !source->initialized) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (!target->initialized ||
        target->size.width != source->size.width ||
        target->size.height != source->size.height) {
        if (target->initialized) {
            fractus_framebuffer_shutdown(target);
        }
        if (fractus_framebuffer_init(target, source->size) != FRACTUS_STATUS_OK) {
            return FRACTUS_STATUS_ERROR;
        }
    }

    pixel_count = (size_t)source->size.width * (size_t)source->size.height;
    memcpy(target->index_pixels, source->index_pixels, pixel_count);
    target->palette = source->palette;
    target->pixels_dirty = 1;
    target->palette_dirty = 1;
    return FRACTUS_STATUS_OK;
}

fractus_status fractus_app_render_mandelbrot(
    const fractus_platform_context *platform,
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const fractus_mandelbrot_params *params,
    int *save_next_graphic,
    char *error_message,
    size_t error_message_size)
{
    if (platform == NULL || framebuffer == NULL || fonts == NULL || params == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_fractal_render_mandelbrot(framebuffer, params) != FRACTUS_STATUS_OK ||
        fractus_app_save_next_graphic_if_requested(platform, framebuffer, save_next_graphic, error_message, error_message_size) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return fractus_app_draw_drawing_footer(framebuffer, fonts, "ESC/boton derecho: menu - G: grabar - S: seleccionar zona");
}

static size_t fractus_app_build_julia_config_entries(
    fractus_app_menu_entry *entries,
    size_t capacity)
{
    static const fractus_app_menu_entry controls[] = {
        {FRACTUS_APP_RECT(379, 124, 419, 144), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 124, 464, 144), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 154, 419, 174), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 154, 464, 174), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 184, 419, 204), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 184, 464, 204), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 214, 419, 234), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 214, 464, 234), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 244, 419, 264), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 244, 464, 264), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 274, 419, 294), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 274, 464, 294), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 304, 419, 324), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 304, 464, 324), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 334, 419, 354), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 334, 464, 354), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(210, 389, 310, 409), 8u, 0u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 389, 430, 409), 0u, 15u, "Cancelar"}
    };

    return fractus_app_copy_control_entries(
        entries,
        capacity,
        controls,
        FRACTUS_APP_ARRAY_COUNT(controls));
}

fractus_status fractus_app_run_julia_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_julia_params *params,
    fractus_julia_params *pending,
    fractus_app_view *view)
{
    char buffer[32];
    fractus_app_menu_entry dialog_entries[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    fractus_ui_menu_option dialog_options[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    size_t dialog_entry_count;
    int selected_menu = -1;
    int cancelled = 0;

    if (framebuffer == NULL || fonts == NULL || ui == NULL ||
        params == NULL || pending == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    dialog_entry_count = fractus_app_build_julia_config_entries(dialog_entries, FRACTUS_APP_ARRAY_COUNT(dialog_entries));
    fractus_app_build_options_from_entries(dialog_options, dialog_entries, dialog_entry_count);

    /* 1. Contenedor exterior. */
    if (fractus_app_render_main_menu(framebuffer, fonts, -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_window(framebuffer, 160, 84, 479, 419) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, 320, 88, 15u, "Conjuntos de Gaston Julia") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 2. Textos y controles. */
    if (fractus_ui_draw_group_box(framebuffer, fonts, 165, 114, 474, 363, 8u, 0u, "Parametros del conjunto") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    snprintf(buffer, sizeof(buffer), "%.3f", pending->xmin);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 129, "Minimo valor real", 299, 124, 371, 144, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", pending->xmax);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 159, "Maximo valor real", 299, 154, 371, 174, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", pending->ymin);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 189, "Minimo valor imaginario", 299, 184, 371, 204, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", pending->ymax);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 219, "Maximo valor imaginario", 299, 214, 371, 234, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", pending->constant_real);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 249, "Constante real", 299, 244, 371, 264, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", pending->constant_imag);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 279, "Constante imaginaria", 299, 274, 371, 294, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%u", pending->max_iterations);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 309, "Iteraciones", 299, 304, 371, 324, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.0f", pending->escape_radius_squared);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 339, "Radio de escape", 299, 334, 371, 354, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    if (fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, 320, 369, 0u, "Estos valores solo afectan al dibujo actual y no cambian la cfg global.") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_button_list(
            framebuffer,
            fonts,
            dialog_entries,
            dialog_entry_count,
            fractus_ui_active_menu_index(ui, dialog_options, dialog_entry_count)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones de botones. */
    if (fractus_ui_menu(ui, dialog_options, dialog_entry_count, &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == FRACTUS_APP_JULIA_CANCEL) {
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (selected_menu == FRACTUS_APP_JULIA_DRAW) {
            *params = *pending;
            *view = FRACTUS_APP_VIEW_JULIA;
        } else if (selected_menu == FRACTUS_APP_JULIA_XMIN_DEC) {
            pending->xmin = fractus_app_clamp_f64(pending->xmin - 0.1, -5.0, pending->xmax - 0.1);
        } else if (selected_menu == FRACTUS_APP_JULIA_XMIN_INC) {
            pending->xmin = fractus_app_clamp_f64(pending->xmin + 0.1, -5.0, pending->xmax - 0.1);
        } else if (selected_menu == FRACTUS_APP_JULIA_XMAX_DEC) {
            pending->xmax = fractus_app_clamp_f64(pending->xmax - 0.1, pending->xmin + 0.1, 5.0);
        } else if (selected_menu == FRACTUS_APP_JULIA_XMAX_INC) {
            pending->xmax = fractus_app_clamp_f64(pending->xmax + 0.1, pending->xmin + 0.1, 5.0);
        } else if (selected_menu == FRACTUS_APP_JULIA_YMIN_DEC) {
            pending->ymin = fractus_app_clamp_f64(pending->ymin - 0.1, -5.0, pending->ymax - 0.1);
        } else if (selected_menu == FRACTUS_APP_JULIA_YMIN_INC) {
            pending->ymin = fractus_app_clamp_f64(pending->ymin + 0.1, -5.0, pending->ymax - 0.1);
        } else if (selected_menu == FRACTUS_APP_JULIA_YMAX_DEC) {
            pending->ymax = fractus_app_clamp_f64(pending->ymax - 0.1, pending->ymin + 0.1, 5.0);
        } else if (selected_menu == FRACTUS_APP_JULIA_YMAX_INC) {
            pending->ymax = fractus_app_clamp_f64(pending->ymax + 0.1, pending->ymin + 0.1, 5.0);
        } else if (selected_menu == FRACTUS_APP_JULIA_CREAL_DEC) {
            pending->constant_real = fractus_app_clamp_f64(pending->constant_real - 0.05, -2.0, 2.0);
        } else if (selected_menu == FRACTUS_APP_JULIA_CREAL_INC) {
            pending->constant_real = fractus_app_clamp_f64(pending->constant_real + 0.05, -2.0, 2.0);
        } else if (selected_menu == FRACTUS_APP_JULIA_CIMAG_DEC) {
            pending->constant_imag = fractus_app_clamp_f64(pending->constant_imag - 0.05, -2.0, 2.0);
        } else if (selected_menu == FRACTUS_APP_JULIA_CIMAG_INC) {
            pending->constant_imag = fractus_app_clamp_f64(pending->constant_imag + 0.05, -2.0, 2.0);
        } else if (selected_menu == FRACTUS_APP_JULIA_ITER_DEC) {
            pending->max_iterations = (uint32_t)fractus_app_clamp_i32((int32_t)pending->max_iterations - 16, 16, 1024);
        } else if (selected_menu == FRACTUS_APP_JULIA_ITER_INC) {
            pending->max_iterations = (uint32_t)fractus_app_clamp_i32((int32_t)pending->max_iterations + 16, 16, 1024);
        } else if (selected_menu == FRACTUS_APP_JULIA_RADIUS_DEC) {
            pending->escape_radius_squared = (double)fractus_app_clamp_i32((int32_t)pending->escape_radius_squared - 1, 1, 1000);
        } else if (selected_menu == FRACTUS_APP_JULIA_RADIUS_INC) {
            pending->escape_radius_squared = (double)fractus_app_clamp_i32((int32_t)pending->escape_radius_squared + 1, 1, 1000);
        }
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_app_render_julia(
    const fractus_platform_context *platform,
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const fractus_julia_params *params,
    int *save_next_graphic,
    char *error_message,
    size_t error_message_size)
{
    if (platform == NULL || framebuffer == NULL || fonts == NULL || params == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_fractal_render_julia(framebuffer, params) != FRACTUS_STATUS_OK ||
        fractus_app_save_next_graphic_if_requested(platform, framebuffer, save_next_graphic, error_message, error_message_size) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return fractus_app_draw_drawing_footer(framebuffer, fonts, "ESC o boton derecho: menu - G: grabar dibujo");
}

static size_t fractus_app_build_biomorph_config_entries(
    fractus_app_menu_entry *entries,
    size_t capacity)
{
    static const fractus_app_menu_entry controls[] = {
        {FRACTUS_APP_RECT(379, 124, 419, 144), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 124, 464, 144), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 154, 419, 174), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 154, 464, 174), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 184, 419, 204), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 184, 464, 204), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 214, 419, 234), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 214, 464, 234), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 244, 419, 264), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 244, 464, 264), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 274, 419, 294), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 274, 464, 294), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 304, 419, 324), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 304, 464, 324), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(210, 359, 310, 379), 8u, 0u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 359, 430, 379), 0u, 15u, "Cancelar"}
    };

    return fractus_app_copy_control_entries(
        entries,
        capacity,
        controls,
        FRACTUS_APP_ARRAY_COUNT(controls));
}

fractus_status fractus_app_run_biomorph_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_biomorph_params *params,
    fractus_biomorph_params *pending,
    fractus_app_view *view)
{
    char buffer[32];
    fractus_app_menu_entry dialog_entries[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    fractus_ui_menu_option dialog_options[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    size_t dialog_entry_count;
    int selected_menu = -1;
    int cancelled = 0;

    if (framebuffer == NULL || fonts == NULL || ui == NULL ||
        params == NULL || pending == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    dialog_entry_count = fractus_app_build_biomorph_config_entries(dialog_entries, FRACTUS_APP_ARRAY_COUNT(dialog_entries));
    fractus_app_build_options_from_entries(dialog_options, dialog_entries, dialog_entry_count);

    /* 1. Contenedor exterior. */
    if (fractus_app_render_main_menu(framebuffer, fonts, -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_window(framebuffer, 160, 84, 479, 389) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, 320, 88, 15u, "Biomorfos de Clifford Pickover") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 2. Textos y controles. */
    if (fractus_ui_draw_group_box(framebuffer, fonts, 165, 114, 474, 331, 8u, 0u, "Parametros del biomorfo") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    snprintf(buffer, sizeof(buffer), "%.3f", pending->xmin);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 129, "Minimo valor real", 299, 124, 371, 144, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", pending->xmax);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 159, "Maximo valor real", 299, 154, 371, 174, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", pending->ymin);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 189, "Minimo valor imaginario", 299, 184, 371, 204, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", pending->ymax);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 219, "Maximo valor imaginario", 299, 214, 371, 234, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", pending->constant_real);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 249, "Constante real", 299, 244, 371, 264, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", pending->constant_imag);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 279, "Constante imaginaria", 299, 274, 371, 294, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.0f", pending->escape_radius_squared);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 309, "Radio de escape", 299, 304, 371, 324, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    if (fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, 320, 339, 0u, "Estos valores solo afectan al dibujo actual y no cambian la cfg global.") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_button_list(
            framebuffer,
            fonts,
            dialog_entries,
            dialog_entry_count,
            fractus_ui_active_menu_index(ui, dialog_options, dialog_entry_count)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones de botones. */
    if (fractus_ui_menu(ui, dialog_options, dialog_entry_count, &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == FRACTUS_APP_BIOMORPH_CANCEL) {
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_DRAW) {
            *params = *pending;
            *view = FRACTUS_APP_VIEW_BIOMORPH;
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_XMIN_DEC) {
            pending->xmin = fractus_app_clamp_f64(pending->xmin - 0.1, -5.0, pending->xmax - 0.1);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_XMIN_INC) {
            pending->xmin = fractus_app_clamp_f64(pending->xmin + 0.1, -5.0, pending->xmax - 0.1);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_XMAX_DEC) {
            pending->xmax = fractus_app_clamp_f64(pending->xmax - 0.1, pending->xmin + 0.1, 5.0);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_XMAX_INC) {
            pending->xmax = fractus_app_clamp_f64(pending->xmax + 0.1, pending->xmin + 0.1, 5.0);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_YMIN_DEC) {
            pending->ymin = fractus_app_clamp_f64(pending->ymin - 0.1, -5.0, pending->ymax - 0.1);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_YMIN_INC) {
            pending->ymin = fractus_app_clamp_f64(pending->ymin + 0.1, -5.0, pending->ymax - 0.1);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_YMAX_DEC) {
            pending->ymax = fractus_app_clamp_f64(pending->ymax - 0.1, pending->ymin + 0.1, 5.0);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_YMAX_INC) {
            pending->ymax = fractus_app_clamp_f64(pending->ymax + 0.1, pending->ymin + 0.1, 5.0);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_CREAL_DEC) {
            pending->constant_real = fractus_app_clamp_f64(pending->constant_real - 0.05, -2.0, 2.0);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_CREAL_INC) {
            pending->constant_real = fractus_app_clamp_f64(pending->constant_real + 0.05, -2.0, 2.0);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_CIMAG_DEC) {
            pending->constant_imag = fractus_app_clamp_f64(pending->constant_imag - 0.05, -2.0, 2.0);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_CIMAG_INC) {
            pending->constant_imag = fractus_app_clamp_f64(pending->constant_imag + 0.05, -2.0, 2.0);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_RADIUS_DEC) {
            pending->escape_radius_squared = (double)fractus_app_clamp_i32((int32_t)pending->escape_radius_squared - 4, 4, 1000);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_RADIUS_INC) {
            pending->escape_radius_squared = (double)fractus_app_clamp_i32((int32_t)pending->escape_radius_squared + 4, 4, 1000);
        }
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_app_render_biomorph(
    const fractus_platform_context *platform,
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const fractus_biomorph_params *params,
    int *save_next_graphic,
    char *error_message,
    size_t error_message_size)
{
    if (platform == NULL || framebuffer == NULL || fonts == NULL || params == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_fractal_render_biomorph(framebuffer, params) != FRACTUS_STATUS_OK ||
        fractus_app_save_next_graphic_if_requested(platform, framebuffer, save_next_graphic, error_message, error_message_size) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return fractus_app_draw_drawing_footer(framebuffer, fonts, "Biomorfo inicial - ESC o boton derecho: menu - G: grabar dibujo");
}

static fractus_status fractus_app_render_plasma_menu(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    int active_index,
    const fractus_app_menu_entry *plasma_menu_controls,
    size_t plasma_menu_control_count,
    fractus_app_plasma_method selected_method,
    int method_enabled)
{
    uint32_t i;

    if (framebuffer == NULL || fonts == NULL || plasma_menu_controls == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_app_render_main_menu(framebuffer, fonts, -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_window(framebuffer, 160, 120, 479, 320) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, 320, 124, 15u, "Fractales por el metodo de plasma") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_group_box(framebuffer, fonts, 165, 150, 474, 190, 8u, 0u, "Metodo de dibujo") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    for (i = 0u; i < plasma_menu_control_count; ++i) {
        fractus_app_menu_entry entry = plasma_menu_controls[i];
        int draw_active = active_index == (int)i;

        if (!method_enabled && i >= 2u) {
            continue;
        }

        if (i < 2u && !method_enabled) {
            entry.fill_color = 8u;
            entry.text_color = 7u;
            draw_active = 0;
        } else if (i == 2u && selected_method == FRACTUS_APP_PLASMA_METHOD_NONE) {
            entry.fill_color = 8u;
            entry.text_color = 7u;
            draw_active = 0;
        }

        if (fractus_ui_draw_button(framebuffer, fonts, &entry, draw_active) != FRACTUS_STATUS_OK) {
            return FRACTUS_STATUS_ERROR;
        }
    }

    if (selected_method != FRACTUS_APP_PLASMA_METHOD_NONE && !method_enabled) {
        return FRACTUS_STATUS_OK;
    }

    return fractus_ui_draw_text_centered(
        framebuffer,
        fonts,
        FRACTUS_FONT_SMALL,
        320,
        270,
        0u,
        "Seleccione Rectangulos o Circulos");
}

fractus_status fractus_app_run_plasma_menu_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_plasma_params *plasma_rectangular_params,
    fractus_plasma_params *plasma_rectangular_pending,
    fractus_plasma_circular_params *plasma_circular_params,
    fractus_plasma_circular_params *plasma_circular_pending,
    fractus_app_view *view)
{
    static const fractus_app_menu_entry plasma_menu_controls[] = {
        {FRACTUS_APP_RECT(210, 160, 310, 180), 8u, 0u, "Rectangulos"},
        {FRACTUS_APP_RECT(330, 160, 430, 180), 8u, 0u, "Circulos"},
        {FRACTUS_APP_RECT(210, 290, 310, 310), 6u, 8u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 290, 430, 310), 0u, 15u, "Cancelar"}
    };
    fractus_ui_menu_option plasma_menu_options[FRACTUS_APP_ARRAY_COUNT(plasma_menu_controls)];
    int selected_menu = -1;
    int cancelled = 0;

    if (framebuffer == NULL || fonts == NULL || ui == NULL ||
        plasma_rectangular_params == NULL || plasma_rectangular_pending == NULL || plasma_circular_params == NULL ||
        plasma_circular_pending == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    fractus_app_build_options_from_entries(
        plasma_menu_options,
        plasma_menu_controls,
        FRACTUS_APP_ARRAY_COUNT(plasma_menu_controls));

    /* 1. Contenedor exterior. */
    /* 2. Textos y controles. */
    if (fractus_app_render_plasma_menu(
            framebuffer,
            fonts,
            fractus_ui_active_menu_index(ui, plasma_menu_options, FRACTUS_APP_ARRAY_COUNT(plasma_menu_options)),
            plasma_menu_controls,
            FRACTUS_APP_ARRAY_COUNT(plasma_menu_controls),
            FRACTUS_APP_PLASMA_METHOD_NONE,
            1) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones de botones. */
    if (fractus_ui_menu(ui, plasma_menu_options, FRACTUS_APP_ARRAY_COUNT(plasma_menu_options), &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == 3) {
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (selected_menu == (int)FRACTUS_APP_PLASMA_METHOD_RECTANGLES) {
            *plasma_rectangular_pending = *plasma_rectangular_params;
            *view = FRACTUS_APP_VIEW_PLASMA_RECTANGULAR_CONFIG;
        } else if (selected_menu == (int)FRACTUS_APP_PLASMA_METHOD_CIRCLES) {
            *plasma_circular_pending = *plasma_circular_params;
            *view = FRACTUS_APP_VIEW_PLASMA_CIRCULAR_CONFIG;
        }
    }

    return FRACTUS_STATUS_OK;
}

static size_t fractus_app_build_plasma_rectangular_config_entries(
    fractus_app_menu_entry *entries,
    size_t capacity)
{
    static const fractus_app_menu_entry controls[] = {
        {FRACTUS_APP_RECT(379, 205, 419, 225), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 205, 464, 225), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(210, 290, 310, 310), 8u, 0u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 290, 430, 310), 0u, 15u, "Cancelar"}
    };

    return fractus_app_copy_control_entries(
        entries,
        capacity,
        controls,
        FRACTUS_APP_ARRAY_COUNT(controls));
}

fractus_status fractus_app_run_plasma_rectangular_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_plasma_params *params,
    fractus_plasma_params *pending,
    fractus_app_view *view)
{
    static const fractus_app_menu_entry plasma_menu_controls[] = {
        {FRACTUS_APP_RECT(210, 160, 310, 180), 8u, 0u, "Rectangulos"},
        {FRACTUS_APP_RECT(330, 160, 430, 180), 8u, 0u, "Circulos"},
        {FRACTUS_APP_RECT(210, 290, 310, 310), 6u, 8u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 290, 430, 310), 0u, 15u, "Cancelar"}
    };
    char buffer[32];
    fractus_app_menu_entry dialog_entries[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    fractus_ui_menu_option dialog_options[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    size_t dialog_entry_count;
    int selected_menu = -1;
    int cancelled = 0;

    if (framebuffer == NULL || fonts == NULL || ui == NULL ||
        params == NULL || pending == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    dialog_entry_count = fractus_app_build_plasma_rectangular_config_entries(dialog_entries, FRACTUS_APP_ARRAY_COUNT(dialog_entries));
    fractus_app_build_options_from_entries(dialog_options, dialog_entries, dialog_entry_count);

    /* 1. Contenedor exterior. */
    if (fractus_app_render_plasma_menu(
            framebuffer,
            fonts,
            -1,
            plasma_menu_controls,
            FRACTUS_APP_ARRAY_COUNT(plasma_menu_controls),
            FRACTUS_APP_PLASMA_METHOD_RECTANGLES,
            0) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_group_box(framebuffer, fonts, 165, 195, 474, 264, 8u, 0u, "Rectangulos") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 2. Textos y controles. */
    snprintf(buffer, sizeof(buffer), "%d", pending->dispersion);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 210, "Indice de dispersion", 299, 205, 371, 225, buffer) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, 320, 270, 0u, "Valores bajos suavizan el gradiente; altos lo acentuan.") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_button_list(
            framebuffer,
            fonts,
            dialog_entries,
            dialog_entry_count,
            fractus_ui_active_menu_index(ui, dialog_options, dialog_entry_count)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones de botones. */
    if (fractus_ui_menu(ui, dialog_options, dialog_entry_count, &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == FRACTUS_APP_PLASMA_RECTANGULAR_CANCEL) {
            *view = FRACTUS_APP_VIEW_PLASMA_MENU;
        } else if (selected_menu == FRACTUS_APP_PLASMA_RECTANGULAR_DRAW) {
            *params = *pending;
            *view = FRACTUS_APP_VIEW_PLASMA_RECTANGULAR;
        } else if (selected_menu == FRACTUS_APP_PLASMA_RECTANGULAR_DISPERSION_DEC) {
            pending->dispersion = fractus_app_clamp_i32(pending->dispersion - 5, 1, 500);
        } else if (selected_menu == FRACTUS_APP_PLASMA_RECTANGULAR_DISPERSION_INC) {
            pending->dispersion = fractus_app_clamp_i32(pending->dispersion + 5, 1, 500);
        }
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_app_render_plasma_rectangular(
    const fractus_platform_context *platform,
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const fractus_plasma_params *params,
    int *save_next_graphic,
    char *error_message,
    size_t error_message_size)
{
    if (platform == NULL || framebuffer == NULL || fonts == NULL || params == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_fractal_render_plasma(framebuffer, params) != FRACTUS_STATUS_OK ||
        fractus_app_save_next_graphic_if_requested(platform, framebuffer, save_next_graphic, error_message, error_message_size) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return fractus_app_draw_drawing_footer(framebuffer, fonts, "ESC o boton derecho: menu - G: grabar dibujo");
}

static size_t fractus_app_build_plasma_circular_config_entries(
    fractus_app_menu_entry *entries,
    size_t capacity)
{
    static const fractus_app_menu_entry controls[] = {
        {FRACTUS_APP_RECT(379, 205, 419, 225), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 205, 464, 225), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 235, 419, 255), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 235, 464, 255), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(210, 290, 310, 310), 8u, 0u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 290, 430, 310), 0u, 15u, "Cancelar"}
    };

    return fractus_app_copy_control_entries(
        entries,
        capacity,
        controls,
        FRACTUS_APP_ARRAY_COUNT(controls));
}

fractus_status fractus_app_run_plasma_circular_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_plasma_circular_params *params,
    fractus_plasma_circular_params *pending,
    fractus_app_view *view)
{
    static const fractus_app_menu_entry plasma_menu_controls[] = {
        {FRACTUS_APP_RECT(210, 160, 310, 180), 8u, 0u, "Rectangulos"},
        {FRACTUS_APP_RECT(330, 160, 430, 180), 8u, 0u, "Circulos"},
        {FRACTUS_APP_RECT(210, 290, 310, 310), 6u, 8u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 290, 430, 310), 0u, 15u, "Cancelar"}
    };
    char buffer[32];
    fractus_app_menu_entry dialog_entries[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    fractus_ui_menu_option dialog_options[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    size_t dialog_entry_count;
    int selected_menu = -1;
    int cancelled = 0;

    if (framebuffer == NULL || fonts == NULL || ui == NULL ||
        params == NULL || pending == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    dialog_entry_count = fractus_app_build_plasma_circular_config_entries(dialog_entries, FRACTUS_APP_ARRAY_COUNT(dialog_entries));
    fractus_app_build_options_from_entries(dialog_options, dialog_entries, dialog_entry_count);

    /* 1. Contenedor exterior. */
    if (fractus_app_render_plasma_menu(
            framebuffer,
            fonts,
            -1,
            plasma_menu_controls,
            FRACTUS_APP_ARRAY_COUNT(plasma_menu_controls),
            FRACTUS_APP_PLASMA_METHOD_CIRCLES,
            0) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_group_box(framebuffer, fonts, 165, 195, 474, 264, 8u, 0u, "Circulos") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 2. Textos y controles. */
    snprintf(buffer, sizeof(buffer), "%d", pending->circle_count);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 210, "Numero de circulos", 299, 205, 371, 225, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%d", pending->max_radius);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 240, "Radio maximo", 299, 235, 371, 255, buffer) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, 320, 270, 0u, "Mas circulos y radios mayores producen dibujos mas densos.") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_button_list(
            framebuffer,
            fonts,
            dialog_entries,
            dialog_entry_count,
            fractus_ui_active_menu_index(ui, dialog_options, dialog_entry_count)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones de botones. */
    if (fractus_ui_menu(ui, dialog_options, dialog_entry_count, &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == FRACTUS_APP_PLASMA_CIRCULAR_CANCEL) {
            *view = FRACTUS_APP_VIEW_PLASMA_MENU;
        } else if (selected_menu == FRACTUS_APP_PLASMA_CIRCULAR_DRAW) {
            *params = *pending;
            *view = FRACTUS_APP_VIEW_PLASMA_CIRCULAR;
        } else if (selected_menu == FRACTUS_APP_PLASMA_CIRCLES_DEC) {
            pending->circle_count = fractus_app_clamp_i32(pending->circle_count - 25, 10, 5000);
        } else if (selected_menu == FRACTUS_APP_PLASMA_CIRCLES_INC) {
            pending->circle_count = fractus_app_clamp_i32(pending->circle_count + 25, 10, 5000);
        } else if (selected_menu == FRACTUS_APP_PLASMA_RADIUS_DEC) {
            pending->max_radius = fractus_app_clamp_i32(pending->max_radius - 5, 1, 100);
        } else if (selected_menu == FRACTUS_APP_PLASMA_RADIUS_INC) {
            pending->max_radius = fractus_app_clamp_i32(pending->max_radius + 5, 1, 100);
        }
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_app_render_plasma_circular(
    const fractus_platform_context *platform,
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const fractus_plasma_circular_params *params,
    int *save_next_graphic,
    char *error_message,
    size_t error_message_size)
{
    if (platform == NULL || framebuffer == NULL || fonts == NULL || params == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_fractal_render_plasma_circular(framebuffer, params) != FRACTUS_STATUS_OK ||
        fractus_app_save_next_graphic_if_requested(platform, framebuffer, save_next_graphic, error_message, error_message_size) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return fractus_app_draw_drawing_footer(framebuffer, fonts, "ESC o boton derecho: menu - G: grabar dibujo");
}
