#include "app/plasma.h"
#include "app/files.h"

#include <stdio.h>
#include <string.h>

void fractus_app_init_plasma_fields(
    fractus_app_plasma_fields *fields,
    uint32_t rectangular_seed,
    int32_t dispersion,
    uint32_t circular_seed,
    int32_t circle_count,
    int32_t max_radius)
{
    if (fields == NULL) {
        return;
    }
    (void)fractus_ui_numeric_field_init_int(&fields->rectangular_seed, (fractus_rect_i32){299, 213, 73, 20}, (int32_t)rectangular_seed, 1, 999999);
    (void)fractus_ui_numeric_field_init_int(&fields->rectangular_dispersion, (fractus_rect_i32){299, 239, 73, 20}, dispersion, 1, 500);
    (void)fractus_ui_numeric_field_init_int(&fields->circular_seed, (fractus_rect_i32){299, 213, 73, 20}, (int32_t)circular_seed, 1, 999999);
    (void)fractus_ui_numeric_field_init_int(&fields->circular_circles, (fractus_rect_i32){299, 239, 73, 20}, circle_count, 5, 5000);
    (void)fractus_ui_numeric_field_init_int(&fields->circular_radius, (fractus_rect_i32){299, 265, 73, 20}, max_radius, 1, 500);
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
        fractus_ui_draw_window(framebuffer, 160, 126, 479, 355) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, 320, 130, 15u, "Fractales por el metodo de plasma") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_group_box(framebuffer, fonts, 165, 156, 474, 195, 8u, 0u, "Metodo de dibujo") != FRACTUS_STATUS_OK) {
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
        307,
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
    fractus_app_plasma_fields *plasma_fields,
    fractus_app_view *view)
{
    static const fractus_app_menu_entry plasma_menu_controls[] = {
        {FRACTUS_APP_RECT(210, 167, 310, 187), 8u, 0u, "Rectangulos"},
        {FRACTUS_APP_RECT(330, 167, 430, 187), 8u, 0u, "Circulos"},
        {FRACTUS_APP_RECT(210, 325, 310, 345), 6u, 8u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 325, 430, 345), 0u, 15u, "Cancelar"}
    };
    fractus_ui_menu_option plasma_menu_options[FRACTUS_APP_ARRAY_COUNT(plasma_menu_controls)];
    int selected_menu = -1;
    int cancelled = 0;

    if (framebuffer == NULL || fonts == NULL || ui == NULL ||
        plasma_rectangular_params == NULL || plasma_rectangular_pending == NULL || plasma_circular_params == NULL ||
        plasma_circular_pending == NULL || plasma_fields == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    fractus_app_build_options_from_entries(
        plasma_menu_options,
        plasma_menu_controls,
        FRACTUS_APP_ARRAY_COUNT(plasma_menu_controls));

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

    if (fractus_ui_menu(ui, plasma_menu_options, FRACTUS_APP_ARRAY_COUNT(plasma_menu_controls), &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == 3) {
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (selected_menu == (int)FRACTUS_APP_PLASMA_METHOD_RECTANGLES) {
            *plasma_rectangular_pending = *plasma_rectangular_params;
            fractus_app_init_plasma_fields(plasma_fields, plasma_rectangular_pending->seed, plasma_rectangular_pending->dispersion, 0u, 0, 0);
            *view = FRACTUS_APP_VIEW_PLASMA_RECTANGULAR_CONFIG;
        } else if (selected_menu == (int)FRACTUS_APP_PLASMA_METHOD_CIRCLES) {
            *plasma_circular_pending = *plasma_circular_params;
            fractus_app_init_plasma_fields(plasma_fields, 0u, 0, plasma_circular_pending->seed, plasma_circular_pending->circle_count, plasma_circular_pending->max_radius);
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
        {FRACTUS_APP_RECT(379, 213, 419, 233), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 213, 464, 233), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 239, 419, 259), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 239, 464, 259), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(210, 325, 310, 345), 8u, 0u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 325, 430, 345), 0u, 15u, "Cancelar"}
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
    static const fractus_app_menu_entry plasma_menu_controls[] = {
        {FRACTUS_APP_RECT(210, 167, 310, 187), 8u, 0u, "Rectangulos"},
        {FRACTUS_APP_RECT(330, 167, 430, 187), 8u, 0u, "Circulos"},
        {FRACTUS_APP_RECT(210, 325, 310, 345), 6u, 8u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 325, 430, 345), 0u, 15u, "Cancelar"}
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
        fractus_ui_draw_group_box(framebuffer, fonts, 165, 203, 474, 299, 8u, 0u, "Rectangulos") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 2. Textos y controles. */
    if (fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 175, 218, 0u, "Semilla") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->rectangular_seed) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 175, 244, 0u, "Indice de dispersion") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->rectangular_dispersion) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, 320, 307, 0u, "Indices de dispersion bajos suavizan el gradiente; altos lo acentuan.") != FRACTUS_STATUS_OK ||
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
        {FRACTUS_APP_RECT(379, 213, 419, 233), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 213, 464, 233), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 239, 419, 259), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 239, 464, 259), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 265, 419, 285), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 265, 464, 285), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(210, 325, 310, 345), 8u, 0u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 325, 430, 345), 0u, 15u, "Cancelar"}
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
    static const fractus_app_menu_entry plasma_menu_controls[] = {
        {FRACTUS_APP_RECT(210, 167, 310, 187), 8u, 0u, "Rectangulos"},
        {FRACTUS_APP_RECT(330, 167, 430, 187), 8u, 0u, "Circulos"},
        {FRACTUS_APP_RECT(210, 325, 310, 345), 6u, 8u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 325, 430, 345), 0u, 15u, "Cancelar"}
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
        fractus_ui_draw_group_box(framebuffer, fonts, 165, 203, 474, 299, 8u, 0u, "Circulos") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 2. Textos y controles. */
    if (fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 175, 218, 0u, "Semilla") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->circular_seed) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 175, 244, 0u, "Numero de circulos") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->circular_circles) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 175, 270, 0u, "Radio maximo") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->circular_radius) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, 320, 307, 0u, "Mas circulos y radios menores producen dibujos mas densos.") != FRACTUS_STATUS_OK ||
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
