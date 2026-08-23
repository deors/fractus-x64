#include "app/mandelbrot.h"
#include "app/files.h"

#include <stdio.h>
#include <string.h>

static const int32_t mandel_x0 = 135;
static const int32_t mandel_y0 = 70;

void fractus_app_init_mandelbrot_fields(
    fractus_app_mandelbrot_fields *fields,
    double xmin,
    double xmax,
    double ymin,
    double ymax,
    uint32_t max_iterations,
    double escape_radius_squared)
{
    const int32_t x0 = mandel_x0;
    const int32_t y0 = mandel_y0;

    if (fields == NULL) {
        return;
    }
    (void)fractus_ui_numeric_field_init_float(&fields->xmin, (fractus_rect_i32){x0 + 189, y0 + 96, 73, 20}, xmin, -5.0, 5.0, 3);
    (void)fractus_ui_numeric_field_init_float(&fields->xmax, (fractus_rect_i32){x0 + 189, y0 + 122, 73, 20}, xmax, -5.0, 5.0, 3);
    (void)fractus_ui_numeric_field_init_float(&fields->ymin, (fractus_rect_i32){x0 + 189, y0 + 148, 73, 20}, ymin, -5.0, 5.0, 3);
    (void)fractus_ui_numeric_field_init_float(&fields->ymax, (fractus_rect_i32){x0 + 189, y0 + 174, 73, 20}, ymax, -5.0, 5.0, 3);
    (void)fractus_ui_numeric_field_init_int(&fields->max_iterations, (fractus_rect_i32){x0 + 189, y0 + 200, 73, 20}, (int32_t)max_iterations, 1, 1000);
    (void)fractus_ui_numeric_field_init_int(&fields->escape_radius_squared, (fractus_rect_i32){x0 + 189, y0 + 226, 73, 20}, (int32_t)escape_radius_squared, 4, 1000);
}

static fractus_status fractus_app_render_mandelbrot_menu(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    int active_index,
    const fractus_app_menu_entry *mandel_menu_controls,
    size_t mandel_menu_control_count,
    fractus_app_mandel_method selected_method,
    int method_enabled)
{
    const int32_t x0 = mandel_x0;
    const int32_t y0 = mandel_y0;
    uint32_t i;

    if (framebuffer == NULL || fonts == NULL || mandel_menu_controls == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_app_render_main_menu(framebuffer, fonts, -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_window(framebuffer, x0, y0, x0 + 369, y0 + 338) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, x0 + 185, y0 + 4, 15u, "Conjunto de Benoit B. Mandelbrot") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_group_box(framebuffer, fonts, x0 + 5, y0 + 32, x0 + 364, y0 + 72, 8u, 0u, "Metodo de dibujo") != FRACTUS_STATUS_OK) {
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
        x0 + 185,
        y0 + 290,
        0u,
        "Selecciona el algoritmo que se utilizara para generar el dibujo.");
}

fractus_status fractus_app_run_mandelbrot_menu_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_mandelbrot_params *mandelbrot_params,
    fractus_mandelbrot_params *mandelbrot_pending,
    fractus_app_mandelbrot_fields *mandelbrot_fields,
    fractus_mandelbrot_dem_params *mandelbrot_dem_params,
    fractus_mandelbrot_dem_params *mandelbrot_dem_pending,
    fractus_app_mandelbrot_fields *mandelbrot_dem_fields,
    fractus_app_view *view)
{
    const int32_t x0 = mandel_x0;
    const int32_t y0 = mandel_y0;
    const fractus_app_menu_entry mandel_menu_controls[] = {
        {FRACTUS_APP_RECT(x0 + 13, y0 + 43, x0 + 179, y0 + 63), 8u, 0u, "Tiempo de escape"},
        {FRACTUS_APP_RECT(x0 + 190, y0 + 43, x0 + 356, y0 + 63), 8u, 0u, "Estimacion de distancias"},
        {FRACTUS_APP_RECT(x0 + 135, y0 + 308, x0 + 235, y0 + 328), 0u, 15u, "Volver"}
    };
    fractus_ui_menu_option dialog_options[FRACTUS_APP_ARRAY_COUNT(mandel_menu_controls)];
    int selected_menu = -1;
    int cancelled = 0;

    if (framebuffer == NULL || fonts == NULL || ui == NULL ||
        mandelbrot_params == NULL || mandelbrot_pending == NULL || mandelbrot_fields == NULL ||
        mandelbrot_dem_params == NULL || mandelbrot_dem_pending == NULL ||
        mandelbrot_dem_fields == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    fractus_app_build_options_from_entries(dialog_options, mandel_menu_controls, FRACTUS_APP_ARRAY_COUNT(mandel_menu_controls));

    /* 1. Contenedor exterior. */
    /* 2. Textos y controles. */
    if (fractus_app_render_mandelbrot_menu(
            framebuffer,
            fonts,
            fractus_ui_active_menu_index(ui, dialog_options, FRACTUS_APP_ARRAY_COUNT(dialog_options)),
            mandel_menu_controls,
            FRACTUS_APP_ARRAY_COUNT(mandel_menu_controls),
            FRACTUS_APP_MANDEL_METHOD_NONE,
            1) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en este selector de modo */

    /* 4. Raton y acciones de botones. */
    if (fractus_ui_menu(ui, dialog_options, FRACTUS_APP_ARRAY_COUNT(dialog_options), &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == 2) {
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (selected_menu == 0) {
            *mandelbrot_pending = *mandelbrot_params;
            fractus_app_init_mandelbrot_fields(
                mandelbrot_fields,
                mandelbrot_pending->xmin,
                mandelbrot_pending->xmax,
                mandelbrot_pending->ymin,
                mandelbrot_pending->ymax,
                mandelbrot_pending->max_iterations,
                mandelbrot_pending->escape_radius_squared);
            *view = FRACTUS_APP_VIEW_MANDELBROT_CONFIG;
        } else if (selected_menu == 1) {
            *mandelbrot_dem_pending = *mandelbrot_dem_params;
            fractus_app_init_mandelbrot_fields(
                mandelbrot_dem_fields,
                mandelbrot_dem_pending->xmin,
                mandelbrot_dem_pending->xmax,
                mandelbrot_dem_pending->ymin,
                mandelbrot_dem_pending->ymax,
                mandelbrot_dem_pending->max_iterations,
                mandelbrot_dem_pending->escape_radius_squared);
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
    const int32_t x0 = mandel_x0;
    const int32_t y0 = mandel_y0;
    const fractus_app_menu_entry controls[] = {
        {FRACTUS_APP_RECT(x0 + 269, y0 + 96, x0 + 309, y0 + 116), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(x0 + 314, y0 + 96, x0 + 354, y0 + 116), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(x0 + 269, y0 + 122, x0 + 309, y0 + 142), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(x0 + 314, y0 + 122, x0 + 354, y0 + 142), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(x0 + 269, y0 + 148, x0 + 309, y0 + 168), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(x0 + 314, y0 + 148, x0 + 354, y0 + 168), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(x0 + 269, y0 + 174, x0 + 309, y0 + 194), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(x0 + 314, y0 + 174, x0 + 354, y0 + 194), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(x0 + 269, y0 + 200, x0 + 309, y0 + 220), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(x0 + 314, y0 + 200, x0 + 354, y0 + 220), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(x0 + 269, y0 + 226, x0 + 309, y0 + 246), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(x0 + 314, y0 + 226, x0 + 354, y0 + 246), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(x0 + 189, y0 + 252, x0 + 261, y0 + 272), 8u, 0u, color_opt0_label},
        {FRACTUS_APP_RECT(x0 + 269, y0 + 252, x0 + 354, y0 + 272), 8u, 0u, color_opt1_label},
        {FRACTUS_APP_RECT(x0 + 75, y0 + 308, x0 + 175, y0 + 328), 8u, 0u, "Dibujar"},
        {FRACTUS_APP_RECT(x0 + 195, y0 + 308, x0 + 295, y0 + 328), 0u, 15u, "Cancelar"}
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
    fractus_app_mandelbrot_fields *fields,
    fractus_app_view draw_view,
    int *draw_confirmed,
    fractus_app_view *view)
{
    const int32_t x0 = mandel_x0;
    const int32_t y0 = mandel_y0;
    const fractus_app_menu_entry mandel_menu_controls[] = {
        {FRACTUS_APP_RECT(x0 + 13, y0 + 43, x0 + 179, y0 + 63), 8u, 0u, "Tiempo de escape"},
        {FRACTUS_APP_RECT(x0 + 190, y0 + 43, x0 + 356, y0 + 63), 8u, 0u, "Estimacion de distancias"},
        {FRACTUS_APP_RECT(x0 + 75, y0 + 308, x0 + 175, y0 + 328), 6u, 8u, "Dibujar"},
        {FRACTUS_APP_RECT(x0 + 195, y0 + 308, x0 + 295, y0 + 328), 0u, 15u, "Cancelar"}
    };
    const fractus_ui_radio_option color_options[] = {
        {FRACTUS_APP_RECT(x0 + 189, y0 + 252, x0 + 261, y0 + 272), color_opt0_label},
        {FRACTUS_APP_RECT(x0 + 269, y0 + 252, x0 + 354, y0 + 272), color_opt1_label}
    };
    fractus_app_menu_entry dialog_entries[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    fractus_ui_menu_option dialog_options[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    size_t dialog_entry_count;
    int active_index;
    int selected_menu = -1;
    int cancelled = 0;
    int skip_mouse_input = 0;

    if (framebuffer == NULL || fonts == NULL || ui == NULL ||
        xmin == NULL || xmax == NULL || ymin == NULL || ymax == NULL ||
        max_iterations == NULL || escape_radius_squared == NULL ||
        color_mode == NULL || fields == NULL || draw_confirmed == NULL || view == NULL) {
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
        fractus_ui_draw_group_box(framebuffer, fonts, x0 + 5, y0 + 80, x0 + 364, y0 + 282, 8u, 0u, group_box_title) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 2. Textos y controles. */
    if (fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, x0 + 15, y0 + 101, 0u, "Minimo valor real") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->xmin) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, x0 + 15, y0 + 127, 0u, "Maximo valor real") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->xmax) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, x0 + 15, y0 + 153, 0u, "Minimo valor imaginario") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->ymin) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, x0 + 15, y0 + 179, 0u, "Maximo valor imaginario") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->ymax) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, x0 + 15, y0 + 205, 0u, "Iteraciones maximas (1-1000)") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->max_iterations) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, x0 + 15, y0 + 231, 0u, "Radio de escape al cuadrado (4-1000)") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->escape_radius_squared) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, x0 + 15, y0 + 257, 0u, "Modo de coloreado") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_radio_list(
            framebuffer,
            fonts,
            color_options,
            FRACTUS_APP_ARRAY_COUNT(color_options),
            *color_mode == 1 ? 1 : 0,
            (active_index == FRACTUS_APP_MANDEL_COLOR_MODE_0 || active_index == FRACTUS_APP_MANDEL_COLOR_MODE_1) ?
                active_index - FRACTUS_APP_MANDEL_COLOR_MODE_0 : -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, x0 + 185, y0 + 290, 0u, "Estos valores solo afectan al dibujo actual y no cambian la cfg global.") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_button_list(framebuffer, fonts, dialog_entries, FRACTUS_APP_MANDEL_COLOR_MODE_0, active_index) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_button_list(
            framebuffer,
            fonts,
            &dialog_entries[FRACTUS_APP_MANDEL_DRAW],
            2u,
            (active_index >= FRACTUS_APP_MANDEL_DRAW) ? active_index - FRACTUS_APP_MANDEL_DRAW : -1) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Gestion de foco y edicion de campos editables con foco exclusivo. */
    if (ui->release_pending && ui->release_event.buttons.left) {
        fractus_point_i32 click_pos = ui->release_event.position;
        fractus_ui_numeric_field *clicked_field = NULL;

        if (fractus_ui_point_in_rect(click_pos, mandel_menu_controls[(size_t)method].bounds)) {
            *view = FRACTUS_APP_VIEW_MANDELBROT_MENU;
            return FRACTUS_STATUS_OK;
        }

        if (fractus_ui_point_in_rect(click_pos, fields->xmin.bounds)) {
            clicked_field = &fields->xmin;
        } else if (fractus_ui_point_in_rect(click_pos, fields->xmax.bounds)) {
            clicked_field = &fields->xmax;
        } else if (fractus_ui_point_in_rect(click_pos, fields->ymin.bounds)) {
            clicked_field = &fields->ymin;
        } else if (fractus_ui_point_in_rect(click_pos, fields->ymax.bounds)) {
            clicked_field = &fields->ymax;
        } else if (fractus_ui_point_in_rect(click_pos, fields->max_iterations.bounds)) {
            clicked_field = &fields->max_iterations;
        } else if (fractus_ui_point_in_rect(click_pos, fields->escape_radius_squared.bounds)) {
            clicked_field = &fields->escape_radius_squared;
        }

        if (clicked_field != NULL) {
            double fval;
            int32_t ival;
            if (fields->xmin.editing && fractus_ui_numeric_field_get_float(&fields->xmin, &fval) == FRACTUS_STATUS_OK) {
                *xmin = fval;
            }
            if (fields->xmax.editing && fractus_ui_numeric_field_get_float(&fields->xmax, &fval) == FRACTUS_STATUS_OK) {
                *xmax = fval;
            }
            if (fields->ymin.editing && fractus_ui_numeric_field_get_float(&fields->ymin, &fval) == FRACTUS_STATUS_OK) {
                *ymin = fval;
            }
            if (fields->ymax.editing && fractus_ui_numeric_field_get_float(&fields->ymax, &fval) == FRACTUS_STATUS_OK) {
                *ymax = fval;
            }
            if (fields->max_iterations.editing && fractus_ui_numeric_field_get_int(&fields->max_iterations, &ival) == FRACTUS_STATUS_OK) {
                *max_iterations = (uint32_t)ival;
            }
            if (fields->escape_radius_squared.editing && fractus_ui_numeric_field_get_int(&fields->escape_radius_squared, &ival) == FRACTUS_STATUS_OK) {
                *escape_radius_squared = (double)ival;
            }
            fractus_app_init_mandelbrot_fields(fields, *xmin, *xmax, *ymin, *ymax, *max_iterations, *escape_radius_squared);
            if (clicked_field == &fields->xmin) {
                (void)fractus_ui_numeric_field_begin_edit(&fields->xmin);
            } else if (clicked_field == &fields->xmax) {
                (void)fractus_ui_numeric_field_begin_edit(&fields->xmax);
            } else if (clicked_field == &fields->ymin) {
                (void)fractus_ui_numeric_field_begin_edit(&fields->ymin);
            } else if (clicked_field == &fields->ymax) {
                (void)fractus_ui_numeric_field_begin_edit(&fields->ymax);
            } else if (clicked_field == &fields->max_iterations) {
                (void)fractus_ui_numeric_field_begin_edit(&fields->max_iterations);
            } else if (clicked_field == &fields->escape_radius_squared) {
                (void)fractus_ui_numeric_field_begin_edit(&fields->escape_radius_squared);
            }
        } else {
            double fval;
            int32_t ival;
            if (fields->xmin.editing && fractus_ui_numeric_field_get_float(&fields->xmin, &fval) == FRACTUS_STATUS_OK) {
                *xmin = fval;
            }
            if (fields->xmax.editing && fractus_ui_numeric_field_get_float(&fields->xmax, &fval) == FRACTUS_STATUS_OK) {
                *xmax = fval;
            }
            if (fields->ymin.editing && fractus_ui_numeric_field_get_float(&fields->ymin, &fval) == FRACTUS_STATUS_OK) {
                *ymin = fval;
            }
            if (fields->ymax.editing && fractus_ui_numeric_field_get_float(&fields->ymax, &fval) == FRACTUS_STATUS_OK) {
                *ymax = fval;
            }
            if (fields->max_iterations.editing && fractus_ui_numeric_field_get_int(&fields->max_iterations, &ival) == FRACTUS_STATUS_OK) {
                *max_iterations = (uint32_t)ival;
            }
            if (fields->escape_radius_squared.editing && fractus_ui_numeric_field_get_int(&fields->escape_radius_squared, &ival) == FRACTUS_STATUS_OK) {
                *escape_radius_squared = (double)ival;
            }
            fractus_app_init_mandelbrot_fields(fields, *xmin, *xmax, *ymin, *ymax, *max_iterations, *escape_radius_squared);
        }
    }

    {
        int edit_accepted = 0;
        int edit_cancelled = 0;
        fractus_ui_numeric_field *active_field = NULL;

        if (fields->xmin.editing) {
            active_field = &fields->xmin;
        } else if (fields->xmax.editing) {
            active_field = &fields->xmax;
        } else if (fields->ymin.editing) {
            active_field = &fields->ymin;
        } else if (fields->ymax.editing) {
            active_field = &fields->ymax;
        } else if (fields->max_iterations.editing) {
            active_field = &fields->max_iterations;
        } else if (fields->escape_radius_squared.editing) {
            active_field = &fields->escape_radius_squared;
        }

        if (active_field != NULL) {
            if (fractus_ui_numeric_field_handle_input(active_field, ui, fonts, &edit_accepted, &edit_cancelled) == FRACTUS_STATUS_OK) {
                if (edit_accepted) {
                    double fval;
                    int32_t ival;
                    if (active_field == &fields->xmin && fractus_ui_numeric_field_get_float(active_field, &fval) == FRACTUS_STATUS_OK) {
                        *xmin = fval;
                    } else if (active_field == &fields->xmax && fractus_ui_numeric_field_get_float(active_field, &fval) == FRACTUS_STATUS_OK) {
                        *xmax = fval;
                    } else if (active_field == &fields->ymin && fractus_ui_numeric_field_get_float(active_field, &fval) == FRACTUS_STATUS_OK) {
                        *ymin = fval;
                    } else if (active_field == &fields->ymax && fractus_ui_numeric_field_get_float(active_field, &fval) == FRACTUS_STATUS_OK) {
                        *ymax = fval;
                    } else if (active_field == &fields->max_iterations && fractus_ui_numeric_field_get_int(active_field, &ival) == FRACTUS_STATUS_OK) {
                        *max_iterations = (uint32_t)ival;
                    } else if (active_field == &fields->escape_radius_squared && fractus_ui_numeric_field_get_int(active_field, &ival) == FRACTUS_STATUS_OK) {
                        *escape_radius_squared = (double)ival;
                    }
                    fractus_app_init_mandelbrot_fields(fields, *xmin, *xmax, *ymin, *ymax, *max_iterations, *escape_radius_squared);
                } else if (edit_cancelled) {
                    fractus_app_init_mandelbrot_fields(fields, *xmin, *xmax, *ymin, *ymax, *max_iterations, *escape_radius_squared);
                }
            }
            skip_mouse_input = 1;
        }
    }

    /* 4. Raton y acciones de botones. */
    if (!skip_mouse_input && fractus_ui_menu(ui, dialog_options, dialog_entry_count, &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == FRACTUS_APP_MANDEL_CANCEL) {
            *view = FRACTUS_APP_VIEW_MANDELBROT_MENU;
        } else if (selected_menu == FRACTUS_APP_MANDEL_DRAW) {
            *draw_confirmed = 1;
            *view = draw_view;
        } else if (selected_menu == FRACTUS_APP_MANDEL_XMIN_DEC) {
            *xmin = fractus_app_clamp_f64(*xmin - 0.1, -5.0, *xmax - 0.1);
            fractus_app_init_mandelbrot_fields(fields, *xmin, *xmax, *ymin, *ymax, *max_iterations, *escape_radius_squared);
        } else if (selected_menu == FRACTUS_APP_MANDEL_XMIN_INC) {
            *xmin = fractus_app_clamp_f64(*xmin + 0.1, -5.0, *xmax - 0.1);
            fractus_app_init_mandelbrot_fields(fields, *xmin, *xmax, *ymin, *ymax, *max_iterations, *escape_radius_squared);
        } else if (selected_menu == FRACTUS_APP_MANDEL_XMAX_DEC) {
            *xmax = fractus_app_clamp_f64(*xmax - 0.1, *xmin + 0.1, 5.0);
            fractus_app_init_mandelbrot_fields(fields, *xmin, *xmax, *ymin, *ymax, *max_iterations, *escape_radius_squared);
        } else if (selected_menu == FRACTUS_APP_MANDEL_XMAX_INC) {
            *xmax = fractus_app_clamp_f64(*xmax + 0.1, *xmin + 0.1, 5.0);
            fractus_app_init_mandelbrot_fields(fields, *xmin, *xmax, *ymin, *ymax, *max_iterations, *escape_radius_squared);
        } else if (selected_menu == FRACTUS_APP_MANDEL_YMIN_DEC) {
            *ymin = fractus_app_clamp_f64(*ymin - 0.1, -5.0, *ymax - 0.1);
            fractus_app_init_mandelbrot_fields(fields, *xmin, *xmax, *ymin, *ymax, *max_iterations, *escape_radius_squared);
        } else if (selected_menu == FRACTUS_APP_MANDEL_YMIN_INC) {
            *ymin = fractus_app_clamp_f64(*ymin + 0.1, -5.0, *ymax - 0.1);
            fractus_app_init_mandelbrot_fields(fields, *xmin, *xmax, *ymin, *ymax, *max_iterations, *escape_radius_squared);
        } else if (selected_menu == FRACTUS_APP_MANDEL_YMAX_DEC) {
            *ymax = fractus_app_clamp_f64(*ymax - 0.1, *ymin + 0.1, 5.0);
            fractus_app_init_mandelbrot_fields(fields, *xmin, *xmax, *ymin, *ymax, *max_iterations, *escape_radius_squared);
        } else if (selected_menu == FRACTUS_APP_MANDEL_YMAX_INC) {
            *ymax = fractus_app_clamp_f64(*ymax + 0.1, *ymin + 0.1, 5.0);
            fractus_app_init_mandelbrot_fields(fields, *xmin, *xmax, *ymin, *ymax, *max_iterations, *escape_radius_squared);
        } else if (selected_menu == FRACTUS_APP_MANDEL_ITER_DEC) {
            *max_iterations = (uint32_t)fractus_app_clamp_i32((int32_t)*max_iterations - 4, 1, 1000);
            fractus_app_init_mandelbrot_fields(fields, *xmin, *xmax, *ymin, *ymax, *max_iterations, *escape_radius_squared);
        } else if (selected_menu == FRACTUS_APP_MANDEL_ITER_INC) {
            *max_iterations = (uint32_t)fractus_app_clamp_i32((int32_t)*max_iterations + 4, 1, 1000);
            fractus_app_init_mandelbrot_fields(fields, *xmin, *xmax, *ymin, *ymax, *max_iterations, *escape_radius_squared);
        } else if (selected_menu == FRACTUS_APP_MANDEL_RADIUS_DEC) {
            *escape_radius_squared = (double)fractus_app_clamp_i32((int32_t)*escape_radius_squared - 2, 4, 1000);
            fractus_app_init_mandelbrot_fields(fields, *xmin, *xmax, *ymin, *ymax, *max_iterations, *escape_radius_squared);
        } else if (selected_menu == FRACTUS_APP_MANDEL_RADIUS_INC) {
            *escape_radius_squared = (double)fractus_app_clamp_i32((int32_t)*escape_radius_squared + 2, 4, 1000);
            fractus_app_init_mandelbrot_fields(fields, *xmin, *xmax, *ymin, *ymax, *max_iterations, *escape_radius_squared);
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
    fractus_app_mandelbrot_fields *fields,
    fractus_app_view *view)
{
    int color_mode;
    int draw_confirmed = 0;
    fractus_status status;

    if (params == NULL || pending == NULL || fields == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    color_mode = (int)pending->color_mode;
    status = fractus_app_run_mandelbrot_generic_config_view(
        framebuffer,
        fonts,
        ui,
        FRACTUS_APP_MANDEL_METHOD_ESCAPE,
        "Parametros del conjunto mediante tiempo de escape",
        "Clasico",
        "Suave",
        &pending->xmin,
        &pending->xmax,
        &pending->ymin,
        &pending->ymax,
        &pending->max_iterations,
        &pending->escape_radius_squared,
        &color_mode,
        fields,
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
    fractus_app_mandelbrot_fields *fields,
    fractus_app_view *view)
{
    int color_mode;
    int draw_confirmed = 0;
    fractus_status status;

    if (params == NULL || pending == NULL || fields == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    color_mode = (int)pending->color_mode;
    status = fractus_app_run_mandelbrot_generic_config_view(
        framebuffer,
        fonts,
        ui,
        FRACTUS_APP_MANDEL_METHOD_DEM,
        "Parametros del conjunto mediante estimacion de distancias",
        "Contorno",
        "Gradiente",
        &pending->xmin,
        &pending->xmax,
        &pending->ymin,
        &pending->ymax,
        &pending->max_iterations,
        &pending->escape_radius_squared,
        &color_mode,
        fields,
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

fractus_status fractus_app_render_mandelbrot(
    const fractus_platform_context *platform,
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const fractus_mandelbrot_params *params,
    int *save_next_graphic,
    char *error_message,
    size_t error_message_size,
    char *saved_filename,
    size_t saved_filename_size)
{
    if (platform == NULL || framebuffer == NULL || fonts == NULL || params == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_fractal_render_mandelbrot(framebuffer, params) != FRACTUS_STATUS_OK ||
        fractus_app_save_next_graphic_if_requested_ex(
            platform,
            framebuffer,
            save_next_graphic,
            error_message,
            error_message_size,
            saved_filename,
            saved_filename_size) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return fractus_app_draw_drawing_footer_ex(
        framebuffer,
        fonts,
        "ESC/boton derecho: menu - G: grabar - S: seleccionar zona - F: flujo",
        saved_filename,
        1);
}

fractus_status fractus_app_render_mandelbrot_dem(
    const fractus_platform_context *platform,
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const fractus_mandelbrot_dem_params *params,
    int *save_next_graphic,
    char *error_message,
    size_t error_message_size,
    char *saved_filename,
    size_t saved_filename_size)
{
    if (platform == NULL || framebuffer == NULL || fonts == NULL || params == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_fractal_render_mandelbrot_dem(framebuffer, params) != FRACTUS_STATUS_OK ||
        fractus_app_save_next_graphic_if_requested_ex(
            platform,
            framebuffer,
            save_next_graphic,
            error_message,
            error_message_size,
            saved_filename,
            saved_filename_size) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return fractus_app_draw_drawing_footer_ex(
        framebuffer,
        fonts,
        "ESC/boton derecho: menu - G: grabar - S: seleccionar zona - F: flujo",
        saved_filename,
        1);
}
