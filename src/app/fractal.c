#include "app/fractal.h"
#include "app/files.h"
#include "core/fractal.h"

#include <stdio.h>
#include <string.h>

static fractus_status fractus_app_render_mandelbrot_menu(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    int active_index,
    const fractus_app_menu_entry *mandel_menu_controls,
    size_t mandel_menu_control_count,
    fractus_app_mandel_method selected_method,
    int method_enabled)
{
    uint32_t i;

    if (framebuffer == NULL || fonts == NULL || mandel_menu_controls == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_app_render_main_menu(framebuffer, fonts, -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_window(framebuffer, 135, 67, 504, 414) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, 320, 71, 15u, "Conjunto de Benoit B. Mandelbrot") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_group_box(framebuffer, fonts, 140, 99, 499, 138, 8u, 0u, "Metodo de dibujo") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    for (i = 0u; i < mandel_menu_control_count; ++i) {
        fractus_app_menu_entry entry = mandel_menu_controls[i];
        int draw_active = active_index == (int)i;

        if (!method_enabled && i >= 2u) {
            continue;
        }

        if (i < 2u && !method_enabled) {
            if (i == (uint32_t)selected_method) {
                entry.fill_color = 8u;
                entry.text_color = 0u;
                draw_active = 1;
            } else {
                entry.fill_color = 8u;
                entry.text_color = 7u;
                draw_active = 0;
            }
        } else if (i == 2u && selected_method == FRACTUS_APP_MANDEL_METHOD_NONE) {
            entry.fill_color = 8u;
            entry.text_color = 7u;
            draw_active = 0;
        }

        if (fractus_ui_draw_button(framebuffer, fonts, &entry, draw_active) != FRACTUS_STATUS_OK) {
            return FRACTUS_STATUS_ERROR;
        }
    }

    if (selected_method != FRACTUS_APP_MANDEL_METHOD_NONE && !method_enabled) {
        return FRACTUS_STATUS_OK;
    }

    return fractus_ui_draw_text_centered(
        framebuffer,
        fonts,
        FRACTUS_FONT_SMALL,
        320,
        366,
        0u,
        "Selecciona el algoritmo que se utilizara para generar el dibujo.");
}

fractus_status fractus_app_run_mandelbrot_menu_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_mandelbrot_params *mandelbrot_params,
    fractus_mandelbrot_params *mandelbrot_pending,
    fractus_mandelbrot_dem_params *mandelbrot_dem_params,
    fractus_mandelbrot_dem_params *mandelbrot_dem_pending,
    fractus_app_view *view)
{
    static const fractus_app_menu_entry mandel_menu_controls[] = {
        {FRACTUS_APP_RECT(148, 110, 314, 130), 8u, 0u, "Tiempo de escape"},
        {FRACTUS_APP_RECT(325, 110, 491, 130), 8u, 0u, "Estimacion de distancias"},
        {FRACTUS_APP_RECT(210, 384, 310, 404), 6u, 8u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 384, 430, 404), 0u, 15u, "Cancelar"}
    };
    fractus_ui_menu_option mandel_menu_options[FRACTUS_APP_ARRAY_COUNT(mandel_menu_controls)];
    int selected_menu = -1;
    int cancelled = 0;

    if (framebuffer == NULL || fonts == NULL || ui == NULL ||
        mandelbrot_params == NULL || mandelbrot_pending == NULL ||
        mandelbrot_dem_params == NULL || mandelbrot_dem_pending == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    fractus_app_build_options_from_entries(
        mandel_menu_options,
        mandel_menu_controls,
        FRACTUS_APP_ARRAY_COUNT(mandel_menu_controls));

    if (fractus_app_render_mandelbrot_menu(
            framebuffer,
            fonts,
            fractus_ui_active_menu_index(ui, mandel_menu_options, FRACTUS_APP_ARRAY_COUNT(mandel_menu_controls)),
            mandel_menu_controls,
            FRACTUS_APP_ARRAY_COUNT(mandel_menu_controls),
            FRACTUS_APP_MANDEL_METHOD_NONE,
            1) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    if (fractus_ui_menu(ui, mandel_menu_options, FRACTUS_APP_ARRAY_COUNT(mandel_menu_controls), &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == 3) {
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (selected_menu == (int)FRACTUS_APP_MANDEL_METHOD_ESCAPE) {
            *mandelbrot_pending = *mandelbrot_params;
            *view = FRACTUS_APP_VIEW_MANDELBROT_CONFIG;
        } else if (selected_menu == (int)FRACTUS_APP_MANDEL_METHOD_DEM) {
            *mandelbrot_dem_pending = *mandelbrot_dem_params;
            *view = FRACTUS_APP_VIEW_MANDELBROT_DEM_CONFIG;
        }
    }

    return FRACTUS_STATUS_OK;
}

static size_t fractus_app_build_mandel_config_entries_generic(
    fractus_app_menu_entry *entries,
    size_t capacity,
    const char *color_opt0_label,
    const char *color_opt1_label)
{
    const fractus_app_menu_entry controls[] = {
        {FRACTUS_APP_RECT(404, 151, 444, 171), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(449, 151, 489, 171), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(404, 181, 444, 201), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(449, 181, 489, 201), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(404, 211, 444, 231), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(449, 211, 489, 231), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(404, 241, 444, 261), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(449, 241, 489, 261), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(404, 271, 444, 291), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(449, 271, 489, 291), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(404, 301, 444, 321), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(449, 301, 489, 321), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(324, 331, 396, 351), 8u, 0u, color_opt0_label},
        {FRACTUS_APP_RECT(404, 331, 489, 351), 8u, 0u, color_opt1_label},
        {FRACTUS_APP_RECT(210, 384, 310, 404), 8u, 0u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 384, 430, 404), 0u, 15u, "Cancelar"}
    };

    return fractus_app_copy_control_entries(
        entries,
        capacity,
        controls,
        FRACTUS_APP_ARRAY_COUNT(controls));
}

static fractus_status fractus_app_run_mandelbrot_generic_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_app_mandel_method method,
    const char *group_box_title,
    const char *color_opt0_label,
    const char *color_opt1_label,
    double *xmin,
    double *xmax,
    double *ymin,
    double *ymax,
    uint32_t *max_iterations,
    double *escape_radius_squared,
    int *color_mode,
    fractus_app_view draw_view,
    int *draw_confirmed,
    fractus_app_view *view)
{
    static const fractus_app_menu_entry mandel_menu_controls[] = {
        {FRACTUS_APP_RECT(148, 110, 314, 130), 8u, 0u, "Tiempo de escape"},
        {FRACTUS_APP_RECT(325, 110, 491, 130), 8u, 0u, "Estimacion de distancias"},
        {FRACTUS_APP_RECT(210, 384, 310, 404), 6u, 8u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 384, 430, 404), 0u, 15u, "Cancelar"}
    };
    char buffer[32];
    const fractus_ui_radio_option color_options[] = {
        {FRACTUS_APP_RECT(324, 331, 396, 351), color_opt0_label},
        {FRACTUS_APP_RECT(404, 331, 489, 351), color_opt1_label}
    };
    fractus_app_menu_entry dialog_entries[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    fractus_ui_menu_option dialog_options[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    size_t dialog_entry_count;
    int active_index;
    int selected_menu = -1;
    int cancelled = 0;

    if (framebuffer == NULL || fonts == NULL || ui == NULL ||
        xmin == NULL || xmax == NULL || ymin == NULL || ymax == NULL ||
        max_iterations == NULL || escape_radius_squared == NULL ||
        color_mode == NULL || draw_confirmed == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    *draw_confirmed = 0;

    dialog_entry_count = fractus_app_build_mandel_config_entries_generic(
        dialog_entries,
        FRACTUS_APP_ARRAY_COUNT(dialog_entries),
        color_opt0_label,
        color_opt1_label);
    fractus_app_build_options_from_entries(dialog_options, dialog_entries, dialog_entry_count);
    active_index = fractus_ui_active_menu_index(ui, dialog_options, dialog_entry_count);

    /* 1. Contenedor exterior y menu de metodos. */
    if (fractus_app_render_mandelbrot_menu(
            framebuffer,
            fonts,
            -1,
            mandel_menu_controls,
            FRACTUS_APP_ARRAY_COUNT(mandel_menu_controls),
            method,
            0) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_group_box(framebuffer, fonts, 140, 143, 499, 358, 8u, 0u, group_box_title) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 2. Textos y controles. */
    snprintf(buffer, sizeof(buffer), "%.3f", *xmin);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 150, 156, "Minimo valor real", 324, 151, 396, 171, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", *xmax);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 150, 186, "Maximo valor real", 324, 181, 396, 201, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", *ymin);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 150, 216, "Minimo valor imaginario", 324, 211, 396, 231, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", *ymax);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 150, 246, "Maximo valor imaginario", 324, 241, 396, 261, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%u", *max_iterations);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 150, 276, "Iteraciones maximas", 324, 271, 396, 291, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.0f", *escape_radius_squared);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 150, 306, "Radio de escape al cuadrado", 324, 301, 396, 321, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    if (fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 150, 336, 0u, "Modo de coloreado") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_radio_list(
            framebuffer,
            fonts,
            color_options,
            FRACTUS_APP_ARRAY_COUNT(color_options),
            *color_mode == 1 ? 1 : 0,
            (active_index == FRACTUS_APP_MANDEL_COLOR_MODE_0 || active_index == FRACTUS_APP_MANDEL_COLOR_MODE_1) ?
                active_index - FRACTUS_APP_MANDEL_COLOR_MODE_0 : -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, 320, 366, 0u, "Estos valores solo afectan al dibujo actual y no cambian la cfg global.") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_button_list(framebuffer, fonts, dialog_entries, FRACTUS_APP_MANDEL_COLOR_MODE_0, active_index) != FRACTUS_STATUS_OK ||
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
            *view = FRACTUS_APP_VIEW_MANDELBROT_MENU;
        } else if (selected_menu == FRACTUS_APP_MANDEL_DRAW) {
            *draw_confirmed = 1;
            *view = draw_view;
        } else if (selected_menu == FRACTUS_APP_MANDEL_XMIN_DEC) {
            *xmin = fractus_app_clamp_f64(*xmin - 0.1, -5.0, *xmax - 0.1);
        } else if (selected_menu == FRACTUS_APP_MANDEL_XMIN_INC) {
            *xmin = fractus_app_clamp_f64(*xmin + 0.1, -5.0, *xmax - 0.1);
        } else if (selected_menu == FRACTUS_APP_MANDEL_XMAX_DEC) {
            *xmax = fractus_app_clamp_f64(*xmax - 0.1, *xmin + 0.1, 5.0);
        } else if (selected_menu == FRACTUS_APP_MANDEL_XMAX_INC) {
            *xmax = fractus_app_clamp_f64(*xmax + 0.1, *xmin + 0.1, 5.0);
        } else if (selected_menu == FRACTUS_APP_MANDEL_YMIN_DEC) {
            *ymin = fractus_app_clamp_f64(*ymin - 0.1, -5.0, *ymax - 0.1);
        } else if (selected_menu == FRACTUS_APP_MANDEL_YMIN_INC) {
            *ymin = fractus_app_clamp_f64(*ymin + 0.1, -5.0, *ymax - 0.1);
        } else if (selected_menu == FRACTUS_APP_MANDEL_YMAX_DEC) {
            *ymax = fractus_app_clamp_f64(*ymax - 0.1, *ymin + 0.1, 5.0);
        } else if (selected_menu == FRACTUS_APP_MANDEL_YMAX_INC) {
            *ymax = fractus_app_clamp_f64(*ymax + 0.1, *ymin + 0.1, 5.0);
        } else if (selected_menu == FRACTUS_APP_MANDEL_ITER_DEC) {
            *max_iterations = (uint32_t)fractus_app_clamp_i32((int32_t)*max_iterations - 4, 16, 1024);
        } else if (selected_menu == FRACTUS_APP_MANDEL_ITER_INC) {
            *max_iterations = (uint32_t)fractus_app_clamp_i32((int32_t)*max_iterations + 4, 16, 1024);
        } else if (selected_menu == FRACTUS_APP_MANDEL_RADIUS_DEC) {
            *escape_radius_squared = (double)fractus_app_clamp_i32((int32_t)*escape_radius_squared - 2, 4, 1000);
        } else if (selected_menu == FRACTUS_APP_MANDEL_RADIUS_INC) {
            *escape_radius_squared = (double)fractus_app_clamp_i32((int32_t)*escape_radius_squared + 2, 4, 1000);
        } else if (selected_menu == FRACTUS_APP_MANDEL_COLOR_MODE_0) {
            *color_mode = 0;
        } else if (selected_menu == FRACTUS_APP_MANDEL_COLOR_MODE_1) {
            *color_mode = 1;
        }
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_app_run_mandelbrot_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_mandelbrot_params *params,
    fractus_mandelbrot_params *pending,
    fractus_app_view *view)
{
    int color_mode;
    int draw_confirmed = 0;
    fractus_status status;

    if (params == NULL || pending == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    color_mode = (int)pending->color_mode;
    status = fractus_app_run_mandelbrot_generic_config_view(
        framebuffer,
        fonts,
        ui,
        FRACTUS_APP_MANDEL_METHOD_ESCAPE,
        "Parametros del conjunto",
        "Clasico",
        "Suave",
        &pending->xmin,
        &pending->xmax,
        &pending->ymin,
        &pending->ymax,
        &pending->max_iterations,
        &pending->escape_radius_squared,
        &color_mode,
        FRACTUS_APP_VIEW_MANDELBROT,
        &draw_confirmed,
        view);

    if (status == FRACTUS_STATUS_OK) {
        pending->color_mode = (fractus_mandelbrot_color_mode)color_mode;
        if (draw_confirmed) {
            *params = *pending;
        }
    }

    return status;
}

fractus_status fractus_app_run_mandelbrot_dem_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_mandelbrot_dem_params *params,
    fractus_mandelbrot_dem_params *pending,
    fractus_app_view *view)
{
    int color_mode;
    int draw_confirmed = 0;
    fractus_status status;

    if (params == NULL || pending == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    color_mode = (int)pending->color_mode;
    status = fractus_app_run_mandelbrot_generic_config_view(
        framebuffer,
        fonts,
        ui,
        FRACTUS_APP_MANDEL_METHOD_DEM,
        "Parametros de estimacion",
        "Contorno",
        "Gradiente",
        &pending->xmin,
        &pending->xmax,
        &pending->ymin,
        &pending->ymax,
        &pending->max_iterations,
        &pending->escape_radius_squared,
        &color_mode,
        FRACTUS_APP_VIEW_MANDELBROT_DEM,
        &draw_confirmed,
        view);

    if (status == FRACTUS_STATUS_OK) {
        pending->color_mode = (fractus_mandelbrot_dem_color_mode)color_mode;
        if (draw_confirmed) {
            *params = *pending;
        }
    }

    return status;
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

static int fractus_app_apply_mandelbrot_dem_selection(
    const fractus_framebuffer *framebuffer,
    fractus_mandelbrot_dem_params *params,
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

int fractus_app_handle_mandelbrot_dem_selection_input(
    const fractus_platform_context *platform,
    const fractus_framebuffer *framebuffer,
    const fractus_ui_context *ui,
    fractus_mandelbrot_dem_params *params,
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
        } else if (fractus_app_apply_mandelbrot_dem_selection(framebuffer, params, selection->first_corner, selected_point)) {
            selection->active = 0;
            selection->has_first_corner = 0;
            return 1;
        }
    }

    return 0;
}

fractus_status fractus_app_render_mandelbrot_dem(
    const fractus_platform_context *platform,
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const fractus_mandelbrot_dem_params *params,
    int *save_next_graphic,
    char *error_message,
    size_t error_message_size)
{
    if (platform == NULL || framebuffer == NULL || fonts == NULL || params == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_fractal_render_mandelbrot_dem(framebuffer, params) != FRACTUS_STATUS_OK ||
        fractus_app_save_next_graphic_if_requested(platform, framebuffer, save_next_graphic, error_message, error_message_size) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return fractus_app_draw_drawing_footer(framebuffer, fonts, "ESC/boton derecho: menu - G: grabar - S: seleccionar zona");
}

static fractus_status fractus_app_render_julia_menu(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    int active_index,
    const fractus_app_menu_entry *julia_menu_controls,
    size_t julia_menu_control_count,
    fractus_app_julia_method selected_method,
    int method_enabled)
{
    uint32_t i;

    if (framebuffer == NULL || fonts == NULL || julia_menu_controls == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_app_render_main_menu(framebuffer, fonts, -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_window(framebuffer, 135, 53, 504, 427) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, 320, 57, 15u, "Conjuntos de Gaston Julia") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_group_box(framebuffer, fonts, 140, 85, 499, 124, 8u, 0u, "Metodo de dibujo") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    for (i = 0u; i < julia_menu_control_count; ++i) {
        fractus_app_menu_entry entry = julia_menu_controls[i];
        int draw_active = active_index == (int)i;

        if (!method_enabled && i >= 2u) {
            continue;
        }

        if (i < 2u && !method_enabled) {
            if (i == (uint32_t)selected_method) {
                entry.fill_color = 8u;
                entry.text_color = 0u;
                draw_active = 1;
            } else {
                entry.fill_color = 8u;
                entry.text_color = 7u;
                draw_active = 0;
            }
        } else if (i == 2u && selected_method == FRACTUS_APP_JULIA_METHOD_NONE) {
            entry.fill_color = 8u;
            entry.text_color = 7u;
            draw_active = 0;
        }

        if (fractus_ui_draw_button(framebuffer, fonts, &entry, draw_active) != FRACTUS_STATUS_OK) {
            return FRACTUS_STATUS_ERROR;
        }
    }

    if (selected_method != FRACTUS_APP_JULIA_METHOD_NONE && !method_enabled) {
        return FRACTUS_STATUS_OK;
    }

    return fractus_ui_draw_text_centered(
        framebuffer,
        fonts,
        FRACTUS_FONT_SMALL,
        320,
        379,
        0u,
        "Selecciona el algoritmo que se utilizara para generar el dibujo.");
}

fractus_status fractus_app_run_julia_menu_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_julia_params *julia_params,
    fractus_julia_params *julia_pending,
    fractus_julia_dem_params *julia_dem_params,
    fractus_julia_dem_params *julia_dem_pending,
    fractus_app_view *view)
{
    static const fractus_app_menu_entry julia_menu_controls[] = {
        {FRACTUS_APP_RECT(148, 96, 314, 116), 8u, 0u, "Tiempo de escape"},
        {FRACTUS_APP_RECT(325, 96, 491, 116), 8u, 0u, "Estimacion de distancias"},
        {FRACTUS_APP_RECT(210, 397, 310, 417), 6u, 8u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 397, 430, 417), 0u, 15u, "Cancelar"}
    };
    fractus_ui_menu_option julia_menu_options[FRACTUS_APP_ARRAY_COUNT(julia_menu_controls)];
    int selected_menu = -1;
    int cancelled = 0;

    if (framebuffer == NULL || fonts == NULL || ui == NULL ||
        julia_params == NULL || julia_pending == NULL ||
        julia_dem_params == NULL || julia_dem_pending == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    fractus_app_build_options_from_entries(
        julia_menu_options,
        julia_menu_controls,
        FRACTUS_APP_ARRAY_COUNT(julia_menu_controls));

    if (fractus_app_render_julia_menu(
            framebuffer,
            fonts,
            fractus_ui_active_menu_index(ui, julia_menu_options, FRACTUS_APP_ARRAY_COUNT(julia_menu_controls)),
            julia_menu_controls,
            FRACTUS_APP_ARRAY_COUNT(julia_menu_controls),
            FRACTUS_APP_JULIA_METHOD_NONE,
            1) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    if (fractus_ui_menu(ui, julia_menu_options, FRACTUS_APP_ARRAY_COUNT(julia_menu_controls), &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == 3) {
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (selected_menu == (int)FRACTUS_APP_JULIA_METHOD_ESCAPE) {
            *julia_pending = *julia_params;
            *view = FRACTUS_APP_VIEW_JULIA_CONFIG;
        } else if (selected_menu == (int)FRACTUS_APP_JULIA_METHOD_DEM) {
            *julia_dem_pending = *julia_dem_params;
            *view = FRACTUS_APP_VIEW_JULIA_DEM_CONFIG;
        }
    }

    return FRACTUS_STATUS_OK;
}

static size_t fractus_app_build_julia_config_entries_generic(
    fractus_app_menu_entry *entries,
    size_t capacity,
    const char *color_opt0_label,
    const char *color_opt1_label)
{
    const fractus_app_menu_entry controls[] = {
        {FRACTUS_APP_RECT(404, 136, 444, 156), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(449, 136, 489, 156), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(404, 162, 444, 182), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(449, 162, 489, 182), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(404, 188, 444, 208), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(449, 188, 489, 208), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(404, 214, 444, 234), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(449, 214, 489, 234), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(404, 240, 444, 260), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(449, 240, 489, 260), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(404, 266, 444, 286), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(449, 266, 489, 286), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(404, 292, 444, 312), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(449, 292, 489, 312), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(404, 318, 444, 338), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(449, 318, 489, 338), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(324, 344, 396, 364), 8u, 0u, color_opt0_label},
        {FRACTUS_APP_RECT(404, 344, 489, 364), 8u, 0u, color_opt1_label},
        {FRACTUS_APP_RECT(210, 397, 310, 417), 8u, 0u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 397, 430, 417), 0u, 15u, "Cancelar"}
    };

    return fractus_app_copy_control_entries(
        entries,
        capacity,
        controls,
        FRACTUS_APP_ARRAY_COUNT(controls));
}

static fractus_status fractus_app_run_julia_generic_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_app_julia_method method,
    const char *group_box_title,
    const char *color_opt0_label,
    const char *color_opt1_label,
    double *xmin,
    double *xmax,
    double *ymin,
    double *ymax,
    double *constant_real,
    double *constant_imag,
    uint32_t *max_iterations,
    double *escape_radius_squared,
    int *color_mode,
    fractus_app_view draw_view,
    int *draw_confirmed,
    fractus_app_view *view)
{
    static const fractus_app_menu_entry julia_menu_controls[] = {
        {FRACTUS_APP_RECT(148, 96, 314, 116), 8u, 0u, "Tiempo de escape"},
        {FRACTUS_APP_RECT(325, 96, 491, 116), 8u, 0u, "Estimacion de distancias"},
        {FRACTUS_APP_RECT(210, 397, 310, 417), 6u, 8u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 397, 430, 417), 0u, 15u, "Cancelar"}
    };
    char buffer[32];
    const fractus_ui_radio_option color_options[] = {
        {FRACTUS_APP_RECT(324, 344, 396, 364), color_opt0_label},
        {FRACTUS_APP_RECT(404, 344, 489, 364), color_opt1_label}
    };
    fractus_app_menu_entry dialog_entries[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    fractus_ui_menu_option dialog_options[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    size_t dialog_entry_count;
    int active_index;
    int selected_menu = -1;
    int cancelled = 0;

    if (framebuffer == NULL || fonts == NULL || ui == NULL ||
        xmin == NULL || xmax == NULL || ymin == NULL || ymax == NULL ||
        constant_real == NULL || constant_imag == NULL ||
        max_iterations == NULL || escape_radius_squared == NULL ||
        color_mode == NULL || draw_confirmed == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    *draw_confirmed = 0;

    dialog_entry_count = fractus_app_build_julia_config_entries_generic(
        dialog_entries,
        FRACTUS_APP_ARRAY_COUNT(dialog_entries),
        color_opt0_label,
        color_opt1_label);
    fractus_app_build_options_from_entries(dialog_options, dialog_entries, dialog_entry_count);
    active_index = fractus_ui_active_menu_index(ui, dialog_options, dialog_entry_count);

    /* 1. Contenedor exterior. */
    if (fractus_app_render_julia_menu(
            framebuffer,
            fonts,
            -1,
            julia_menu_controls,
            FRACTUS_APP_ARRAY_COUNT(julia_menu_controls),
            method,
            0) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_group_box(framebuffer, fonts, 140, 129, 499, 371, 8u, 0u, group_box_title) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 2. Textos y controles. */
    snprintf(buffer, sizeof(buffer), "%.3f", *xmin);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 150, 141, "Minimo valor real", 324, 136, 396, 156, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", *xmax);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 150, 167, "Maximo valor real", 324, 162, 396, 182, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", *ymin);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 150, 193, "Minimo valor imaginario", 324, 188, 396, 208, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", *ymax);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 150, 219, "Maximo valor imaginario", 324, 214, 396, 234, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", *constant_real);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 150, 245, "Constante real", 324, 240, 396, 260, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", *constant_imag);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 150, 271, "Constante imaginaria", 324, 266, 396, 286, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%u", *max_iterations);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 150, 297, "Iteraciones maximas", 324, 292, 396, 312, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.0f", *escape_radius_squared);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 150, 323, "Radio de escape al cuadrado", 324, 318, 396, 338, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    if (fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 150, 349, 0u, "Modo de coloreado") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_radio_list(
            framebuffer,
            fonts,
            color_options,
            FRACTUS_APP_ARRAY_COUNT(color_options),
            *color_mode == 1 ? 1 : 0,
            (active_index == FRACTUS_APP_JULIA_COLOR_MODE_0 || active_index == FRACTUS_APP_JULIA_COLOR_MODE_1) ?
                active_index - FRACTUS_APP_JULIA_COLOR_MODE_0 : -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, 320, 379, 0u, "Estos valores solo afectan al dibujo actual y no cambian la cfg global.") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_button_list(framebuffer, fonts, dialog_entries, FRACTUS_APP_JULIA_COLOR_MODE_0, active_index) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_button_list(
            framebuffer,
            fonts,
            &dialog_entries[FRACTUS_APP_JULIA_DRAW],
            2u,
            (active_index >= FRACTUS_APP_JULIA_DRAW) ? active_index - FRACTUS_APP_JULIA_DRAW : -1) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones de botones. */
    if (fractus_ui_menu(ui, dialog_options, dialog_entry_count, &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == FRACTUS_APP_JULIA_CANCEL) {
            *view = FRACTUS_APP_VIEW_JULIA_MENU;
        } else if (selected_menu == FRACTUS_APP_JULIA_DRAW) {
            *draw_confirmed = 1;
            *view = draw_view;
        } else if (selected_menu == FRACTUS_APP_JULIA_XMIN_DEC) {
            *xmin = fractus_app_clamp_f64(*xmin - 0.1, -5.0, *xmax - 0.1);
        } else if (selected_menu == FRACTUS_APP_JULIA_XMIN_INC) {
            *xmin = fractus_app_clamp_f64(*xmin + 0.1, -5.0, *xmax - 0.1);
        } else if (selected_menu == FRACTUS_APP_JULIA_XMAX_DEC) {
            *xmax = fractus_app_clamp_f64(*xmax - 0.1, *xmin + 0.1, 5.0);
        } else if (selected_menu == FRACTUS_APP_JULIA_XMAX_INC) {
            *xmax = fractus_app_clamp_f64(*xmax + 0.1, *xmin + 0.1, 5.0);
        } else if (selected_menu == FRACTUS_APP_JULIA_YMIN_DEC) {
            *ymin = fractus_app_clamp_f64(*ymin - 0.1, -5.0, *ymax - 0.1);
        } else if (selected_menu == FRACTUS_APP_JULIA_YMIN_INC) {
            *ymin = fractus_app_clamp_f64(*ymin + 0.1, -5.0, *ymax - 0.1);
        } else if (selected_menu == FRACTUS_APP_JULIA_YMAX_DEC) {
            *ymax = fractus_app_clamp_f64(*ymax - 0.1, *ymin + 0.1, 5.0);
        } else if (selected_menu == FRACTUS_APP_JULIA_YMAX_INC) {
            *ymax = fractus_app_clamp_f64(*ymax + 0.1, *ymin + 0.1, 5.0);
        } else if (selected_menu == FRACTUS_APP_JULIA_CREAL_DEC) {
            *constant_real = fractus_app_clamp_f64(*constant_real - 0.05, -2.0, 2.0);
        } else if (selected_menu == FRACTUS_APP_JULIA_CREAL_INC) {
            *constant_real = fractus_app_clamp_f64(*constant_real + 0.05, -2.0, 2.0);
        } else if (selected_menu == FRACTUS_APP_JULIA_CIMAG_DEC) {
            *constant_imag = fractus_app_clamp_f64(*constant_imag - 0.05, -2.0, 2.0);
        } else if (selected_menu == FRACTUS_APP_JULIA_CIMAG_INC) {
            *constant_imag = fractus_app_clamp_f64(*constant_imag + 0.05, -2.0, 2.0);
        } else if (selected_menu == FRACTUS_APP_JULIA_ITER_DEC) {
            *max_iterations = (uint32_t)fractus_app_clamp_i32((int32_t)*max_iterations - 4, 16, 1024);
        } else if (selected_menu == FRACTUS_APP_JULIA_ITER_INC) {
            *max_iterations = (uint32_t)fractus_app_clamp_i32((int32_t)*max_iterations + 4, 16, 1024);
        } else if (selected_menu == FRACTUS_APP_JULIA_RADIUS_DEC) {
            *escape_radius_squared = (double)fractus_app_clamp_i32((int32_t)*escape_radius_squared - 2, 4, 1000);
        } else if (selected_menu == FRACTUS_APP_JULIA_RADIUS_INC) {
            *escape_radius_squared = (double)fractus_app_clamp_i32((int32_t)*escape_radius_squared + 2, 4, 1000);
        } else if (selected_menu == FRACTUS_APP_JULIA_COLOR_MODE_0) {
            *color_mode = 0;
        } else if (selected_menu == FRACTUS_APP_JULIA_COLOR_MODE_1) {
            *color_mode = 1;
        }
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_app_run_julia_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_julia_params *params,
    fractus_julia_params *pending,
    fractus_app_view *view)
{
    int color_mode;
    int draw_confirmed = 0;
    fractus_status status;

    if (params == NULL || pending == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    color_mode = (int)pending->color_mode;
    status = fractus_app_run_julia_generic_config_view(
        framebuffer,
        fonts,
        ui,
        FRACTUS_APP_JULIA_METHOD_ESCAPE,
        "Parametros del conjunto",
        "Clasico",
        "Suave",
        &pending->xmin,
        &pending->xmax,
        &pending->ymin,
        &pending->ymax,
        &pending->constant_real,
        &pending->constant_imag,
        &pending->max_iterations,
        &pending->escape_radius_squared,
        &color_mode,
        FRACTUS_APP_VIEW_JULIA,
        &draw_confirmed,
        view);

    if (status == FRACTUS_STATUS_OK) {
        pending->color_mode = (fractus_julia_color_mode)color_mode;
        if (draw_confirmed) {
            *params = *pending;
        }
    }

    return status;
}

fractus_status fractus_app_run_julia_dem_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_julia_dem_params *params,
    fractus_julia_dem_params *pending,
    fractus_app_view *view)
{
    int color_mode;
    int draw_confirmed = 0;
    fractus_status status;

    if (params == NULL || pending == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    color_mode = (int)pending->color_mode;
    status = fractus_app_run_julia_generic_config_view(
        framebuffer,
        fonts,
        ui,
        FRACTUS_APP_JULIA_METHOD_DEM,
        "Parametros de estimacion",
        "Contorno",
        "Gradiente",
        &pending->xmin,
        &pending->xmax,
        &pending->ymin,
        &pending->ymax,
        &pending->constant_real,
        &pending->constant_imag,
        &pending->max_iterations,
        &pending->escape_radius_squared,
        &color_mode,
        FRACTUS_APP_VIEW_JULIA_DEM,
        &draw_confirmed,
        view);

    if (status == FRACTUS_STATUS_OK) {
        pending->color_mode = (fractus_julia_dem_color_mode)color_mode;
        if (draw_confirmed) {
            *params = *pending;
        }
    }

    return status;
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

fractus_status fractus_app_render_julia_dem(
    const fractus_platform_context *platform,
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const fractus_julia_dem_params *params,
    int *save_next_graphic,
    char *error_message,
    size_t error_message_size)
{
    if (platform == NULL || framebuffer == NULL || fonts == NULL || params == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_fractal_render_julia_dem(framebuffer, params) != FRACTUS_STATUS_OK ||
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
        {FRACTUS_APP_RECT(404, 99, 444, 119), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(449, 99, 489, 119), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(404, 123, 444, 143), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(449, 123, 489, 143), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(404, 147, 444, 167), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(449, 147, 489, 167), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(404, 171, 444, 191), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(449, 171, 489, 191), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(404, 195, 444, 215), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(449, 195, 489, 215), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(404, 219, 444, 239), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(449, 219, 489, 239), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(404, 243, 444, 263), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(449, 243, 489, 263), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(404, 267, 444, 287), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(449, 267, 489, 287), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(240, 291, 312, 311), 8u, 0u, "z^2 + c"},
        {FRACTUS_APP_RECT(325, 291, 397, 311), 8u, 0u, "z^3 + c"},
        {FRACTUS_APP_RECT(410, 291, 482, 311), 8u, 0u, "z^4 + c"},
        {FRACTUS_APP_RECT(240, 315, 312, 335), 8u, 0u, "z^5 + c"},
        {FRACTUS_APP_RECT(325, 315, 397, 335), 8u, 0u, "sen(z)+c"},
        {FRACTUS_APP_RECT(410, 315, 482, 335), 8u, 0u, "e^z + c"},
        {FRACTUS_APP_RECT(225, 339, 287, 359), 8u, 0u, "Re OR Im"},
        {FRACTUS_APP_RECT(292, 339, 364, 359), 8u, 0u, "Re AND Im"},
        {FRACTUS_APP_RECT(369, 339, 426, 359), 8u, 0u, "Solo Re"},
        {FRACTUS_APP_RECT(431, 339, 489, 359), 8u, 0u, "Solo Im"},
        {FRACTUS_APP_RECT(210, 393, 310, 413), 8u, 0u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 393, 430, 413), 0u, 15u, "Cancelar"}
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
    const fractus_ui_radio_option eq_options[] = {
        {FRACTUS_APP_RECT(240, 291, 312, 311), "z^2 + c"},
        {FRACTUS_APP_RECT(325, 291, 397, 311), "z^3 + c"},
        {FRACTUS_APP_RECT(410, 291, 482, 311), "z^4 + c"},
        {FRACTUS_APP_RECT(240, 315, 312, 335), "z^5 + c"},
        {FRACTUS_APP_RECT(325, 315, 397, 335), "sen(z)+c"},
        {FRACTUS_APP_RECT(410, 315, 482, 335), "e^z + c"}
    };
    const fractus_ui_radio_option trap_options[] = {
        {FRACTUS_APP_RECT(225, 339, 287, 359), "Re OR Im"},
        {FRACTUS_APP_RECT(292, 339, 364, 359), "Re AND Im"},
        {FRACTUS_APP_RECT(369, 339, 426, 359), "Solo Re"},
        {FRACTUS_APP_RECT(431, 339, 489, 359), "Solo Im"}
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

    dialog_entry_count = fractus_app_build_biomorph_config_entries(dialog_entries, FRACTUS_APP_ARRAY_COUNT(dialog_entries));
    fractus_app_build_options_from_entries(dialog_options, dialog_entries, dialog_entry_count);
    active_index = fractus_ui_active_menu_index(ui, dialog_options, dialog_entry_count);

    /* 1. Contenedor exterior. */
    if (fractus_app_render_main_menu(framebuffer, fonts, -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_window(framebuffer, 135, 57, 504, 423) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, 320, 61, 15u, "Biomorfos de Clifford Pickover") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_group_box(framebuffer, fonts, 140, 89, 499, 367, 8u, 0u, "Parametros del biomorfo") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 2. Textos y controles. */
    snprintf(buffer, sizeof(buffer), "%.3f", pending->xmin);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 150, 104, "Minimo valor real", 324, 99, 396, 119, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", pending->xmax);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 150, 128, "Maximo valor real", 324, 123, 396, 143, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", pending->ymin);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 150, 152, "Minimo valor imaginario", 324, 147, 396, 167, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", pending->ymax);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 150, 176, "Maximo valor imaginario", 324, 171, 396, 191, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", pending->constant_real);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 150, 200, "Constante real", 324, 195, 396, 215, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", pending->constant_imag);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 150, 224, "Constante imaginaria", 324, 219, 396, 239, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.0f", pending->escape_radius_squared);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 150, 248, "Radio de escape al cuadrado", 324, 243, 396, 263, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.0f", pending->cutoff);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 150, 272, "Umbral de escape", 324, 267, 396, 287, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    if (fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 150, 296, 0u, "Ecuacion") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_radio_list(
            framebuffer,
            fonts,
            eq_options,
            FRACTUS_APP_ARRAY_COUNT(eq_options),
            (int)pending->equation,
            (active_index >= FRACTUS_APP_BIOMORPH_EQ_0 && active_index <= FRACTUS_APP_BIOMORPH_EQ_5) ?
                active_index - FRACTUS_APP_BIOMORPH_EQ_0 : -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 150, 344, 0u, "Condicion") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_radio_list(
            framebuffer,
            fonts,
            trap_options,
            FRACTUS_APP_ARRAY_COUNT(trap_options),
            (int)pending->trap_mode,
            (active_index >= FRACTUS_APP_BIOMORPH_TRAP_0 && active_index <= FRACTUS_APP_BIOMORPH_TRAP_3) ?
                active_index - FRACTUS_APP_BIOMORPH_TRAP_0 : -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, 320, 375, 0u, "Estos valores solo afectan al dibujo actual y no cambian la cfg global.") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_button_list(framebuffer, fonts, dialog_entries, FRACTUS_APP_BIOMORPH_EQ_0, active_index) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_button_list(
            framebuffer,
            fonts,
            &dialog_entries[FRACTUS_APP_BIOMORPH_DRAW],
            2u,
            (active_index >= FRACTUS_APP_BIOMORPH_DRAW) ? active_index - FRACTUS_APP_BIOMORPH_DRAW : -1) != FRACTUS_STATUS_OK) {
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
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_CUTOFF_DEC) {
            pending->cutoff = (double)fractus_app_clamp_i32((int32_t)pending->cutoff - 1, 1, 1000);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_CUTOFF_INC) {
            pending->cutoff = (double)fractus_app_clamp_i32((int32_t)pending->cutoff + 1, 1, 1000);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_EQ_0) {
            pending->equation = FRACTUS_BIOMORPH_EQ_Z2;
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_EQ_1) {
            pending->equation = FRACTUS_BIOMORPH_EQ_Z3;
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_EQ_2) {
            pending->equation = FRACTUS_BIOMORPH_EQ_Z4;
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_EQ_3) {
            pending->equation = FRACTUS_BIOMORPH_EQ_Z5;
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_EQ_4) {
            pending->equation = FRACTUS_BIOMORPH_EQ_SIN_Z;
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_EQ_5) {
            pending->equation = FRACTUS_BIOMORPH_EQ_EXP_Z;
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_TRAP_0) {
            pending->trap_mode = FRACTUS_BIOMORPH_TRAP_RE_OR_IM;
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_TRAP_1) {
            pending->trap_mode = FRACTUS_BIOMORPH_TRAP_RE_AND_IM;
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_TRAP_2) {
            pending->trap_mode = FRACTUS_BIOMORPH_TRAP_SOLO_RE;
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_TRAP_3) {
            pending->trap_mode = FRACTUS_BIOMORPH_TRAP_SOLO_IM;
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
        fractus_ui_draw_window(framebuffer, 160, 140, 479, 341) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, 320, 144, 15u, "Fractales por el metodo de plasma") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_group_box(framebuffer, fonts, 165, 172, 474, 211, 8u, 0u, "Metodo de dibujo") != FRACTUS_STATUS_OK) {
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
        293,
        0u,
        "Selecciona el algoritmo que se utilizara para generar el dibujo.");
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
        {FRACTUS_APP_RECT(210, 183, 310, 203), 8u, 0u, "Rectangulos"},
        {FRACTUS_APP_RECT(330, 183, 430, 203), 8u, 0u, "Circulos"},
        {FRACTUS_APP_RECT(210, 311, 310, 331), 6u, 8u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 311, 430, 331), 0u, 15u, "Cancelar"}
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
    if (fractus_ui_menu(ui, plasma_menu_options, FRACTUS_APP_ARRAY_COUNT(plasma_menu_controls), &selected_menu, &cancelled)) {
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
        {FRACTUS_APP_RECT(379, 226, 419, 246), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 226, 464, 246), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(210, 311, 310, 331), 8u, 0u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 311, 430, 331), 0u, 15u, "Cancelar"}
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
        {FRACTUS_APP_RECT(210, 183, 310, 203), 8u, 0u, "Rectangulos"},
        {FRACTUS_APP_RECT(330, 183, 430, 203), 8u, 0u, "Circulos"},
        {FRACTUS_APP_RECT(210, 311, 310, 331), 6u, 8u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 311, 430, 331), 0u, 15u, "Cancelar"}
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
        fractus_ui_draw_group_box(framebuffer, fonts, 165, 216, 474, 285, 8u, 0u, "Rectangulos") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 2. Textos y controles. */
    snprintf(buffer, sizeof(buffer), "%d", pending->dispersion);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 231, "Indice de dispersion", 299, 226, 371, 246, buffer) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, 320, 293, 0u, "Valores bajos suavizan el gradiente; altos lo acentuan.") != FRACTUS_STATUS_OK ||
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
        {FRACTUS_APP_RECT(379, 226, 419, 246), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 226, 464, 246), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 256, 419, 276), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 256, 464, 276), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(210, 311, 310, 331), 8u, 0u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 311, 430, 331), 0u, 15u, "Cancelar"}
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
        {FRACTUS_APP_RECT(210, 183, 310, 203), 8u, 0u, "Rectangulos"},
        {FRACTUS_APP_RECT(330, 183, 430, 203), 8u, 0u, "Circulos"},
        {FRACTUS_APP_RECT(210, 311, 310, 331), 6u, 8u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 311, 430, 331), 0u, 15u, "Cancelar"}
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
        fractus_ui_draw_group_box(framebuffer, fonts, 165, 216, 474, 285, 8u, 0u, "Circulos") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 2. Textos y controles. */
    snprintf(buffer, sizeof(buffer), "%d", pending->circle_count);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 231, "Numero de circulos", 299, 226, 371, 246, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%d", pending->max_radius);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 261, "Radio maximo", 299, 256, 371, 276, buffer) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, 320, 293, 0u, "Mas circulos y radios mayores producen dibujos mas densos.") != FRACTUS_STATUS_OK ||
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
            pending->circle_count = fractus_app_clamp_i32(pending->circle_count - 5, 5, 5000);
        } else if (selected_menu == FRACTUS_APP_PLASMA_CIRCLES_INC) {
            pending->circle_count = fractus_app_clamp_i32(pending->circle_count + 5, 5, 5000);
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
