#include "app/plasma.h"
#include "app/files.h"

#include <stdio.h>
#include <string.h>

static const int32_t plasma_x0 = 135;
static const int32_t plasma_y0 = 122;

void fractus_app_init_plasma_fields(
    fractus_app_plasma_fields *fields,
    uint32_t rectangular_seed,
    int32_t dispersion,
    uint32_t circular_seed,
    int32_t circle_count,
    int32_t max_radius)
{
    const int32_t x0 = plasma_x0;
    const int32_t y0 = plasma_y0;

    if (fields == NULL) {
        return;
    }
    (void)fractus_ui_numeric_field_init_int(&fields->rectangular_seed, (fractus_rect_i32){x0 + 189, y0 + 96, 73, 20}, (int32_t)rectangular_seed, 1, 999999);
    (void)fractus_ui_numeric_field_init_int(&fields->rectangular_dispersion, (fractus_rect_i32){x0 + 189, y0 + 122, 73, 20}, dispersion, 1, 500);
    (void)fractus_ui_numeric_field_init_int(&fields->circular_seed, (fractus_rect_i32){x0 + 189, y0 + 96, 73, 20}, (int32_t)circular_seed, 1, 999999);
    (void)fractus_ui_numeric_field_init_int(&fields->circular_circles, (fractus_rect_i32){x0 + 189, y0 + 122, 73, 20}, circle_count, 5, 5000);
    (void)fractus_ui_numeric_field_init_int(&fields->circular_radius, (fractus_rect_i32){x0 + 189, y0 + 148, 73, 20}, max_radius, 1, 500);
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
    const int32_t x0 = plasma_x0;
    const int32_t y0 = plasma_y0;
    uint32_t i;

    if (framebuffer == NULL || fonts == NULL || plasma_menu_controls == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_app_render_main_menu(framebuffer, fonts, -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_window(framebuffer, x0, y0, x0 + 369, y0 + 234) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, x0 + 185, y0 + 4, 15u, "Fractales por el método de plasma") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_group_box(framebuffer, fonts, x0 + 5, y0 + 32, x0 + 364, y0 + 72, 8u, 0u, "Método de dibujo") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    for (i = 0u; i < plasma_menu_control_count; ++i) {
        fractus_app_menu_entry entry = plasma_menu_controls[i];
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
        x0 + 185,
        y0 + 186,
        0u,
        "Selecciona el algoritmo que se utilizará para generar el dibujo.");
}

fractus_status fractus_app_run_plasma_menu_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_plasma_params *plasma_rectangular_params,
    fractus_plasma_params *plasma_rectangular_pending,
    fractus_plasma_circular_params *plasma_circular_params,
    fractus_plasma_circular_params *plasma_circular_pending,
    fractus_app_plasma_fields *plasma_fields,
    fractus_app_view *view)
{
    const int32_t x0 = plasma_x0;
    const int32_t y0 = plasma_y0;
    const fractus_app_menu_entry plasma_menu_controls[] = {
        {FRACTUS_APP_RECT(x0 + 75, y0 + 43, x0 + 175, y0 + 63), 8u, 0u, "Rectángulos"},
        {FRACTUS_APP_RECT(x0 + 195, y0 + 43, x0 + 295, y0 + 63), 8u, 0u, "Círculos"},
        {FRACTUS_APP_RECT(x0 + 135, y0 + 204, x0 + 235, y0 + 224), 0u, 15u, "Volver"}
    };
    fractus_ui_menu_option dialog_options[FRACTUS_APP_ARRAY_COUNT(plasma_menu_controls)];
    int selected_menu = -1;
    int cancelled = 0;

    if (framebuffer == NULL || fonts == NULL || ui == NULL ||
        plasma_rectangular_params == NULL || plasma_rectangular_pending == NULL ||
        plasma_circular_params == NULL || plasma_circular_pending == NULL ||
        plasma_fields == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    fractus_app_build_options_from_entries(dialog_options, plasma_menu_controls, FRACTUS_APP_ARRAY_COUNT(plasma_menu_controls));

    /* 1. Contenedor exterior. */
    /* 2. Textos y controles. */
    if (fractus_app_render_plasma_menu(
            framebuffer,
            fonts,
            fractus_ui_active_menu_index(ui, dialog_options, FRACTUS_APP_ARRAY_COUNT(dialog_options)),
            plasma_menu_controls,
            FRACTUS_APP_ARRAY_COUNT(plasma_menu_controls),
            FRACTUS_APP_PLASMA_METHOD_NONE,
            1) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en este selector de modo */

    /* 4. Raton y acciones de botones. */
    if (fractus_ui_menu(ui, dialog_options, FRACTUS_APP_ARRAY_COUNT(dialog_options), &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == 2) {
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (selected_menu == 0) {
            *plasma_rectangular_pending = *plasma_rectangular_params;
            fractus_app_init_plasma_fields(
                plasma_fields,
                plasma_rectangular_pending->seed,
                plasma_rectangular_pending->dispersion,
                plasma_circular_params->seed,
                plasma_circular_params->circle_count,
                plasma_circular_params->max_radius);
            *view = FRACTUS_APP_VIEW_PLASMA_RECTANGULAR_CONFIG;
        } else if (selected_menu == 1) {
            *plasma_circular_pending = *plasma_circular_params;
            fractus_app_init_plasma_fields(
                plasma_fields,
                plasma_rectangular_params->seed,
                plasma_rectangular_params->dispersion,
                plasma_circular_pending->seed,
                plasma_circular_pending->circle_count,
                plasma_circular_pending->max_radius);
            *view = FRACTUS_APP_VIEW_PLASMA_CIRCULAR_CONFIG;
        }
    }

    return FRACTUS_STATUS_OK;
}

static size_t fractus_app_build_plasma_rectangular_config_entries(
    fractus_app_menu_entry *entries,
    size_t capacity)
{
    const int32_t x0 = plasma_x0;
    const int32_t y0 = plasma_y0;
    const fractus_app_menu_entry controls[] = {
        {FRACTUS_APP_RECT(x0 + 269, y0 + 96, x0 + 309, y0 + 116), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(x0 + 314, y0 + 96, x0 + 354, y0 + 116), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(x0 + 269, y0 + 122, x0 + 309, y0 + 142), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(x0 + 314, y0 + 122, x0 + 354, y0 + 142), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(x0 + 75, y0 + 204, x0 + 175, y0 + 224), 8u, 0u, "Dibujar"},
        {FRACTUS_APP_RECT(x0 + 195, y0 + 204, x0 + 295, y0 + 224), 0u, 15u, "Cancelar"}
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
    fractus_app_plasma_fields *fields,
    fractus_app_view *view)
{
    const int32_t x0 = plasma_x0;
    const int32_t y0 = plasma_y0;
    const fractus_app_menu_entry plasma_menu_controls[] = {
        {FRACTUS_APP_RECT(x0 + 75, y0 + 43, x0 + 175, y0 + 63), 8u, 0u, "Rectángulos"},
        {FRACTUS_APP_RECT(x0 + 195, y0 + 43, x0 + 295, y0 + 63), 8u, 0u, "Círculos"},
        {FRACTUS_APP_RECT(x0 + 75, y0 + 204, x0 + 175, y0 + 224), 6u, 8u, "Dibujar"},
        {FRACTUS_APP_RECT(x0 + 195, y0 + 204, x0 + 295, y0 + 224), 0u, 15u, "Cancelar"}
    };
    fractus_app_menu_entry dialog_entries[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    fractus_ui_menu_option dialog_options[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    size_t dialog_entry_count;
    int selected_menu = -1;
    int cancelled = 0;
    int skip_mouse_input = 0;

    if (framebuffer == NULL || fonts == NULL || ui == NULL ||
        params == NULL || pending == NULL || fields == NULL || view == NULL) {
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
        fractus_ui_draw_group_box(framebuffer, fonts, x0 + 5, y0 + 80, x0 + 364, y0 + 178, 8u, 0u, "Parámetros del conjunto mediante rectángulos") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 2. Textos y controles. */
    if (fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, x0 + 15, y0 + 101, 0u, "Semilla (1-999999)") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->rectangular_seed) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, x0 + 15, y0 + 127, 0u, "Índice de dispersión (1-500)") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->rectangular_dispersion) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, x0 + 185, y0 + 186, 0u, "Índices de dispersión bajos suavizan el gradiente; altos lo acentúan.") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_button_list(
            framebuffer,
            fonts,
            dialog_entries,
            dialog_entry_count,
            fractus_ui_active_menu_index(ui, dialog_options, dialog_entry_count)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Gestion de foco y edicion de campos editables con foco exclusivo. */
    if (ui->release_pending && ui->release_event.buttons.left) {
        fractus_point_i32 click_pos = ui->release_event.position;
        fractus_ui_numeric_field *clicked_field = NULL;

        if (fractus_ui_point_in_rect(click_pos, plasma_menu_controls[0].bounds)) {
            *view = FRACTUS_APP_VIEW_PLASMA_MENU;
            return FRACTUS_STATUS_OK;
        }

        if (fractus_ui_point_in_rect(click_pos, fields->rectangular_seed.bounds)) {
            clicked_field = &fields->rectangular_seed;
        } else if (fractus_ui_point_in_rect(click_pos, fields->rectangular_dispersion.bounds)) {
            clicked_field = &fields->rectangular_dispersion;
        }

        if (clicked_field != NULL) {
            int32_t val;
            if (fields->rectangular_seed.editing && fractus_ui_numeric_field_get_int(&fields->rectangular_seed, &val) == FRACTUS_STATUS_OK) {
                pending->seed = (uint32_t)val;
            }
            if (fields->rectangular_dispersion.editing && fractus_ui_numeric_field_get_int(&fields->rectangular_dispersion, &val) == FRACTUS_STATUS_OK) {
                pending->dispersion = val;
            }
            fractus_app_init_plasma_fields(fields, pending->seed, pending->dispersion, 0u, 0, 0);
            if (clicked_field == &fields->rectangular_seed) {
                (void)fractus_ui_numeric_field_begin_edit(&fields->rectangular_seed);
            } else if (clicked_field == &fields->rectangular_dispersion) {
                (void)fractus_ui_numeric_field_begin_edit(&fields->rectangular_dispersion);
            }
        } else {
            int32_t val;
            if (fields->rectangular_seed.editing && fractus_ui_numeric_field_get_int(&fields->rectangular_seed, &val) == FRACTUS_STATUS_OK) {
                pending->seed = (uint32_t)val;
            }
            if (fields->rectangular_dispersion.editing && fractus_ui_numeric_field_get_int(&fields->rectangular_dispersion, &val) == FRACTUS_STATUS_OK) {
                pending->dispersion = val;
            }
            fractus_app_init_plasma_fields(fields, pending->seed, pending->dispersion, 0u, 0, 0);
        }
    }

    {
        int edit_accepted = 0;
        int edit_cancelled = 0;
        fractus_ui_numeric_field *active_field = NULL;

        if (fields->rectangular_seed.editing) {
            active_field = &fields->rectangular_seed;
        } else if (fields->rectangular_dispersion.editing) {
            active_field = &fields->rectangular_dispersion;
        }

        if (active_field != NULL) {
            if (fractus_ui_numeric_field_handle_input(active_field, ui, fonts, &edit_accepted, &edit_cancelled) == FRACTUS_STATUS_OK) {
                if (edit_accepted) {
                    int32_t val;
                    if (active_field == &fields->rectangular_seed && fractus_ui_numeric_field_get_int(active_field, &val) == FRACTUS_STATUS_OK) {
                        pending->seed = (uint32_t)val;
                    } else if (active_field == &fields->rectangular_dispersion && fractus_ui_numeric_field_get_int(active_field, &val) == FRACTUS_STATUS_OK) {
                        pending->dispersion = val;
                    }
                    fractus_app_init_plasma_fields(fields, pending->seed, pending->dispersion, 0u, 0, 0);
                } else if (edit_cancelled) {
                    fractus_app_init_plasma_fields(fields, pending->seed, pending->dispersion, 0u, 0, 0);
                }
            }
            skip_mouse_input = 1;
        }
    }

    /* 4. Raton y acciones de botones. */
    if (!skip_mouse_input && fractus_ui_menu(ui, dialog_options, dialog_entry_count, &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == FRACTUS_APP_PLASMA_RECTANGULAR_CANCEL) {
            *view = FRACTUS_APP_VIEW_PLASMA_MENU;
        } else if (selected_menu == FRACTUS_APP_PLASMA_RECTANGULAR_DRAW) {
            *params = *pending;
            *view = FRACTUS_APP_VIEW_PLASMA_RECTANGULAR;
        } else if (selected_menu == FRACTUS_APP_PLASMA_RECTANGULAR_SEED_DEC) {
            pending->seed = (pending->seed > 1u) ? (pending->seed - 1u) : 1u;
            fractus_app_init_plasma_fields(fields, pending->seed, pending->dispersion, 0u, 0, 0);
        } else if (selected_menu == FRACTUS_APP_PLASMA_RECTANGULAR_SEED_INC) {
            pending->seed = fractus_app_clamp_i32((int32_t)pending->seed + 1, 1, 999999);
            fractus_app_init_plasma_fields(fields, pending->seed, pending->dispersion, 0u, 0, 0);
        } else if (selected_menu == FRACTUS_APP_PLASMA_RECTANGULAR_DISPERSION_DEC) {
            pending->dispersion = fractus_app_clamp_i32(pending->dispersion - 5, 1, 500);
            fractus_app_init_plasma_fields(fields, pending->seed, pending->dispersion, 0u, 0, 0);
        } else if (selected_menu == FRACTUS_APP_PLASMA_RECTANGULAR_DISPERSION_INC) {
            pending->dispersion = fractus_app_clamp_i32(pending->dispersion + 5, 1, 500);
            fractus_app_init_plasma_fields(fields, pending->seed, pending->dispersion, 0u, 0, 0);
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
    size_t error_message_size,
    char *saved_filename,
    size_t saved_filename_size)
{
    fractus_graphic_metadata metadata;

    if (platform == NULL || framebuffer == NULL || fonts == NULL || params == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    metadata = fractus_graphic_metadata_from_plasma_rectangular(params, framebuffer->size.width, framebuffer->size.height);

    if (fractus_fractal_render_plasma(framebuffer, params) != FRACTUS_STATUS_OK ||
        fractus_app_save_next_graphic_if_requested_ex(
            platform,
            framebuffer,
            &metadata,
            save_next_graphic,
            error_message,
            error_message_size,
            saved_filename,
            saved_filename_size) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    (void)fonts;
    return FRACTUS_STATUS_OK;
}

static size_t fractus_app_build_plasma_circular_config_entries(
    fractus_app_menu_entry *entries,
    size_t capacity)
{
    const int32_t x0 = plasma_x0;
    const int32_t y0 = plasma_y0;
    const fractus_app_menu_entry controls[] = {
        {FRACTUS_APP_RECT(x0 + 269, y0 + 96, x0 + 309, y0 + 116), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(x0 + 314, y0 + 96, x0 + 354, y0 + 116), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(x0 + 269, y0 + 122, x0 + 309, y0 + 142), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(x0 + 314, y0 + 122, x0 + 354, y0 + 142), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(x0 + 269, y0 + 148, x0 + 309, y0 + 168), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(x0 + 314, y0 + 148, x0 + 354, y0 + 168), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(x0 + 75, y0 + 204, x0 + 175, y0 + 224), 8u, 0u, "Dibujar"},
        {FRACTUS_APP_RECT(x0 + 195, y0 + 204, x0 + 295, y0 + 224), 0u, 15u, "Cancelar"}
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
    fractus_app_plasma_fields *fields,
    fractus_app_view *view)
{
    const int32_t x0 = plasma_x0;
    const int32_t y0 = plasma_y0;
    const fractus_app_menu_entry plasma_menu_controls[] = {
        {FRACTUS_APP_RECT(x0 + 75, y0 + 43, x0 + 175, y0 + 63), 8u, 0u, "Rectángulos"},
        {FRACTUS_APP_RECT(x0 + 195, y0 + 43, x0 + 295, y0 + 63), 8u, 0u, "Círculos"},
        {FRACTUS_APP_RECT(x0 + 75, y0 + 204, x0 + 175, y0 + 224), 6u, 8u, "Dibujar"},
        {FRACTUS_APP_RECT(x0 + 195, y0 + 204, x0 + 295, y0 + 224), 0u, 15u, "Cancelar"}
    };
    fractus_app_menu_entry dialog_entries[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    fractus_ui_menu_option dialog_options[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    size_t dialog_entry_count;
    int selected_menu = -1;
    int cancelled = 0;
    int skip_mouse_input = 0;

    if (framebuffer == NULL || fonts == NULL || ui == NULL ||
        params == NULL || pending == NULL || fields == NULL || view == NULL) {
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
        fractus_ui_draw_group_box(framebuffer, fonts, x0 + 5, y0 + 80, x0 + 364, y0 + 178, 8u, 0u, "Parámetros del conjunto mediante círculos") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 2. Textos y controles. */
    if (fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, x0 + 15, y0 + 101, 0u, "Semilla (1-999999)") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->circular_seed) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, x0 + 15, y0 + 127, 0u, "Número de círculos (5-5000)") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->circular_circles) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, x0 + 15, y0 + 153, 0u, "Radio máximo (1-500)") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->circular_radius) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, x0 + 185, y0 + 186, 0u, "Más círculos y radios menores producen dibujos más densos.") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_button_list(
            framebuffer,
            fonts,
            dialog_entries,
            dialog_entry_count,
            fractus_ui_active_menu_index(ui, dialog_options, dialog_entry_count)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Gestion de foco y edicion de campos editables con foco exclusivo. */
    if (ui->release_pending && ui->release_event.buttons.left) {
        fractus_point_i32 click_pos = ui->release_event.position;
        fractus_ui_numeric_field *clicked_field = NULL;

        if (fractus_ui_point_in_rect(click_pos, plasma_menu_controls[1].bounds)) {
            *view = FRACTUS_APP_VIEW_PLASMA_MENU;
            return FRACTUS_STATUS_OK;
        }

        if (fractus_ui_point_in_rect(click_pos, fields->circular_seed.bounds)) {
            clicked_field = &fields->circular_seed;
        } else if (fractus_ui_point_in_rect(click_pos, fields->circular_circles.bounds)) {
            clicked_field = &fields->circular_circles;
        } else if (fractus_ui_point_in_rect(click_pos, fields->circular_radius.bounds)) {
            clicked_field = &fields->circular_radius;
        }

        if (clicked_field != NULL) {
            int32_t val;
            if (fields->circular_seed.editing && fractus_ui_numeric_field_get_int(&fields->circular_seed, &val) == FRACTUS_STATUS_OK) {
                pending->seed = (uint32_t)val;
            }
            if (fields->circular_circles.editing && fractus_ui_numeric_field_get_int(&fields->circular_circles, &val) == FRACTUS_STATUS_OK) {
                pending->circle_count = val;
            }
            if (fields->circular_radius.editing && fractus_ui_numeric_field_get_int(&fields->circular_radius, &val) == FRACTUS_STATUS_OK) {
                pending->max_radius = val;
            }
            fractus_app_init_plasma_fields(fields, 0u, 0, pending->seed, pending->circle_count, pending->max_radius);
            if (clicked_field == &fields->circular_seed) {
                (void)fractus_ui_numeric_field_begin_edit(&fields->circular_seed);
            } else if (clicked_field == &fields->circular_circles) {
                (void)fractus_ui_numeric_field_begin_edit(&fields->circular_circles);
            } else if (clicked_field == &fields->circular_radius) {
                (void)fractus_ui_numeric_field_begin_edit(&fields->circular_radius);
            }
        } else {
            int32_t val;
            if (fields->circular_seed.editing && fractus_ui_numeric_field_get_int(&fields->circular_seed, &val) == FRACTUS_STATUS_OK) {
                pending->seed = (uint32_t)val;
            }
            if (fields->circular_circles.editing && fractus_ui_numeric_field_get_int(&fields->circular_circles, &val) == FRACTUS_STATUS_OK) {
                pending->circle_count = val;
            }
            if (fields->circular_radius.editing && fractus_ui_numeric_field_get_int(&fields->circular_radius, &val) == FRACTUS_STATUS_OK) {
                pending->max_radius = val;
            }
            fractus_app_init_plasma_fields(fields, 0u, 0, pending->seed, pending->circle_count, pending->max_radius);
        }
    }

    {
        int edit_accepted = 0;
        int edit_cancelled = 0;
        fractus_ui_numeric_field *active_field = NULL;

        if (fields->circular_seed.editing) {
            active_field = &fields->circular_seed;
        } else if (fields->circular_circles.editing) {
            active_field = &fields->circular_circles;
        } else if (fields->circular_radius.editing) {
            active_field = &fields->circular_radius;
        }

        if (active_field != NULL) {
            if (fractus_ui_numeric_field_handle_input(active_field, ui, fonts, &edit_accepted, &edit_cancelled) == FRACTUS_STATUS_OK) {
                if (edit_accepted) {
                    int32_t val;
                    if (active_field == &fields->circular_seed && fractus_ui_numeric_field_get_int(active_field, &val) == FRACTUS_STATUS_OK) {
                        pending->seed = (uint32_t)val;
                    } else if (active_field == &fields->circular_circles && fractus_ui_numeric_field_get_int(active_field, &val) == FRACTUS_STATUS_OK) {
                        pending->circle_count = val;
                    } else if (active_field == &fields->circular_radius && fractus_ui_numeric_field_get_int(active_field, &val) == FRACTUS_STATUS_OK) {
                        pending->max_radius = val;
                    }
                    fractus_app_init_plasma_fields(fields, 0u, 0, pending->seed, pending->circle_count, pending->max_radius);
                } else if (edit_cancelled) {
                    fractus_app_init_plasma_fields(fields, 0u, 0, pending->seed, pending->circle_count, pending->max_radius);
                }
            }
            skip_mouse_input = 1;
        }
    }

    /* 4. Raton y acciones de botones. */
    if (!skip_mouse_input && fractus_ui_menu(ui, dialog_options, dialog_entry_count, &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == FRACTUS_APP_PLASMA_CIRCULAR_CANCEL) {
            *view = FRACTUS_APP_VIEW_PLASMA_MENU;
        } else if (selected_menu == FRACTUS_APP_PLASMA_CIRCULAR_DRAW) {
            *params = *pending;
            *view = FRACTUS_APP_VIEW_PLASMA_CIRCULAR;
        } else if (selected_menu == FRACTUS_APP_PLASMA_CIRCULAR_SEED_DEC) {
            pending->seed = (pending->seed > 1u) ? (pending->seed - 1u) : 1u;
            fractus_app_init_plasma_fields(fields, 0u, 0, pending->seed, pending->circle_count, pending->max_radius);
        } else if (selected_menu == FRACTUS_APP_PLASMA_CIRCULAR_SEED_INC) {
            pending->seed = fractus_app_clamp_i32((int32_t)pending->seed + 1, 1, 999999);
            fractus_app_init_plasma_fields(fields, 0u, 0, pending->seed, pending->circle_count, pending->max_radius);
        } else if (selected_menu == FRACTUS_APP_PLASMA_CIRCLES_DEC) {
            pending->circle_count = fractus_app_clamp_i32(pending->circle_count - 5, 5, 5000);
            fractus_app_init_plasma_fields(fields, 0u, 0, pending->seed, pending->circle_count, pending->max_radius);
        } else if (selected_menu == FRACTUS_APP_PLASMA_CIRCLES_INC) {
            pending->circle_count = fractus_app_clamp_i32(pending->circle_count + 5, 5, 5000);
            fractus_app_init_plasma_fields(fields, 0u, 0, pending->seed, pending->circle_count, pending->max_radius);
        } else if (selected_menu == FRACTUS_APP_PLASMA_RADIUS_DEC) {
            pending->max_radius = fractus_app_clamp_i32(pending->max_radius - 5, 1, 500);
            fractus_app_init_plasma_fields(fields, 0u, 0, pending->seed, pending->circle_count, pending->max_radius);
        } else if (selected_menu == FRACTUS_APP_PLASMA_RADIUS_INC) {
            pending->max_radius = fractus_app_clamp_i32(pending->max_radius + 5, 1, 500);
            fractus_app_init_plasma_fields(fields, 0u, 0, pending->seed, pending->circle_count, pending->max_radius);
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
    size_t error_message_size,
    char *saved_filename,
    size_t saved_filename_size)
{
    fractus_graphic_metadata metadata;

    if (platform == NULL || framebuffer == NULL || fonts == NULL || params == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    metadata = fractus_graphic_metadata_from_plasma_circular(params, framebuffer->size.width, framebuffer->size.height);

    if (fractus_fractal_render_plasma_circular(framebuffer, params) != FRACTUS_STATUS_OK ||
        fractus_app_save_next_graphic_if_requested_ex(
            platform,
            framebuffer,
            &metadata,
            save_next_graphic,
            error_message,
            error_message_size,
            saved_filename,
            saved_filename_size) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    (void)fonts;
    return FRACTUS_STATUS_OK;
}
