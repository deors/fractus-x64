#include "app/biomorphs.h"
#include "app/files.h"

#include <stdio.h>
#include <string.h>

void fractus_app_init_biomorph_fields(
    fractus_app_biomorph_fields *fields,
    double xmin,
    double xmax,
    double ymin,
    double ymax,
    double constant_real,
    double constant_imag,
    double escape_radius_squared,
    double cutoff)
{
    if (fields == NULL) {
        return;
    }
    (void)fractus_ui_numeric_field_init_float(&fields->xmin, (fractus_rect_i32){324, 99, 73, 20}, xmin, -5.0, 5.0, 3);
    (void)fractus_ui_numeric_field_init_float(&fields->xmax, (fractus_rect_i32){324, 123, 73, 20}, xmax, -5.0, 5.0, 3);
    (void)fractus_ui_numeric_field_init_float(&fields->ymin, (fractus_rect_i32){324, 147, 73, 20}, ymin, -5.0, 5.0, 3);
    (void)fractus_ui_numeric_field_init_float(&fields->ymax, (fractus_rect_i32){324, 171, 73, 20}, ymax, -5.0, 5.0, 3);
    (void)fractus_ui_numeric_field_init_float(&fields->constant_real, (fractus_rect_i32){324, 195, 73, 20}, constant_real, -5.0, 5.0, 3);
    (void)fractus_ui_numeric_field_init_float(&fields->constant_imag, (fractus_rect_i32){324, 219, 73, 20}, constant_imag, -5.0, 5.0, 3);
    (void)fractus_ui_numeric_field_init_int(&fields->escape_radius_squared, (fractus_rect_i32){324, 243, 73, 20}, (int32_t)escape_radius_squared, 4, 1000);
    (void)fractus_ui_numeric_field_init_int(&fields->cutoff, (fractus_rect_i32){324, 267, 73, 20}, (int32_t)cutoff, 1, 1000);
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
    fractus_app_biomorph_fields *fields,
    fractus_app_view *view)
{
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
    int skip_mouse_input = 0;

    if (framebuffer == NULL || fonts == NULL || ui == NULL ||
        params == NULL || pending == NULL || fields == NULL || view == NULL) {
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
    if (fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 150, 104, 0u, "Minimo valor real") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->xmin) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 150, 128, 0u, "Maximo valor real") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->xmax) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 150, 152, 0u, "Minimo valor imaginario") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->ymin) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 150, 176, 0u, "Maximo valor imaginario") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->ymax) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 150, 200, 0u, "Constante real") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->constant_real) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 150, 224, 0u, "Constante imaginaria") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->constant_imag) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 150, 248, 0u, "Radio de escape al cuadrado (4-1000)") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->escape_radius_squared) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 150, 272, 0u, "Umbral de escape (1-100)") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->cutoff) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 150, 296, 0u, "Ecuacion") != FRACTUS_STATUS_OK ||
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

    /* 3. Gestion de foco y edicion de campos editables con foco exclusivo. */
    if (ui->release_pending && ui->release_event.buttons.left) {
        fractus_point_i32 click_pos = ui->release_event.position;
        fractus_ui_numeric_field *clicked_field = NULL;

        if (fractus_ui_point_in_rect(click_pos, fields->xmin.bounds)) {
            clicked_field = &fields->xmin;
        } else if (fractus_ui_point_in_rect(click_pos, fields->xmax.bounds)) {
            clicked_field = &fields->xmax;
        } else if (fractus_ui_point_in_rect(click_pos, fields->ymin.bounds)) {
            clicked_field = &fields->ymin;
        } else if (fractus_ui_point_in_rect(click_pos, fields->ymax.bounds)) {
            clicked_field = &fields->ymax;
        } else if (fractus_ui_point_in_rect(click_pos, fields->constant_real.bounds)) {
            clicked_field = &fields->constant_real;
        } else if (fractus_ui_point_in_rect(click_pos, fields->constant_imag.bounds)) {
            clicked_field = &fields->constant_imag;
        } else if (fractus_ui_point_in_rect(click_pos, fields->escape_radius_squared.bounds)) {
            clicked_field = &fields->escape_radius_squared;
        } else if (fractus_ui_point_in_rect(click_pos, fields->cutoff.bounds)) {
            clicked_field = &fields->cutoff;
        }

        if (clicked_field != NULL) {
            double fval;
            int32_t ival;
            if (fields->xmin.editing && fractus_ui_numeric_field_get_float(&fields->xmin, &fval) == FRACTUS_STATUS_OK) {
                pending->xmin = fval;
            }
            if (fields->xmax.editing && fractus_ui_numeric_field_get_float(&fields->xmax, &fval) == FRACTUS_STATUS_OK) {
                pending->xmax = fval;
            }
            if (fields->ymin.editing && fractus_ui_numeric_field_get_float(&fields->ymin, &fval) == FRACTUS_STATUS_OK) {
                pending->ymin = fval;
            }
            if (fields->ymax.editing && fractus_ui_numeric_field_get_float(&fields->ymax, &fval) == FRACTUS_STATUS_OK) {
                pending->ymax = fval;
            }
            if (fields->constant_real.editing && fractus_ui_numeric_field_get_float(&fields->constant_real, &fval) == FRACTUS_STATUS_OK) {
                pending->constant_real = fval;
            }
            if (fields->constant_imag.editing && fractus_ui_numeric_field_get_float(&fields->constant_imag, &fval) == FRACTUS_STATUS_OK) {
                pending->constant_imag = fval;
            }
            if (fields->escape_radius_squared.editing && fractus_ui_numeric_field_get_int(&fields->escape_radius_squared, &ival) == FRACTUS_STATUS_OK) {
                pending->escape_radius_squared = (double)ival;
            }
            if (fields->cutoff.editing && fractus_ui_numeric_field_get_int(&fields->cutoff, &ival) == FRACTUS_STATUS_OK) {
                pending->cutoff = (double)ival;
            }
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->escape_radius_squared, pending->cutoff);
            if (clicked_field == &fields->xmin) {
                (void)fractus_ui_numeric_field_begin_edit(&fields->xmin);
            } else if (clicked_field == &fields->xmax) {
                (void)fractus_ui_numeric_field_begin_edit(&fields->xmax);
            } else if (clicked_field == &fields->ymin) {
                (void)fractus_ui_numeric_field_begin_edit(&fields->ymin);
            } else if (clicked_field == &fields->ymax) {
                (void)fractus_ui_numeric_field_begin_edit(&fields->ymax);
            } else if (clicked_field == &fields->constant_real) {
                (void)fractus_ui_numeric_field_begin_edit(&fields->constant_real);
            } else if (clicked_field == &fields->constant_imag) {
                (void)fractus_ui_numeric_field_begin_edit(&fields->constant_imag);
            } else if (clicked_field == &fields->escape_radius_squared) {
                (void)fractus_ui_numeric_field_begin_edit(&fields->escape_radius_squared);
            } else if (clicked_field == &fields->cutoff) {
                (void)fractus_ui_numeric_field_begin_edit(&fields->cutoff);
            }
        } else {
            double fval;
            int32_t ival;
            if (fields->xmin.editing && fractus_ui_numeric_field_get_float(&fields->xmin, &fval) == FRACTUS_STATUS_OK) {
                pending->xmin = fval;
            }
            if (fields->xmax.editing && fractus_ui_numeric_field_get_float(&fields->xmax, &fval) == FRACTUS_STATUS_OK) {
                pending->xmax = fval;
            }
            if (fields->ymin.editing && fractus_ui_numeric_field_get_float(&fields->ymin, &fval) == FRACTUS_STATUS_OK) {
                pending->ymin = fval;
            }
            if (fields->ymax.editing && fractus_ui_numeric_field_get_float(&fields->ymax, &fval) == FRACTUS_STATUS_OK) {
                pending->ymax = fval;
            }
            if (fields->constant_real.editing && fractus_ui_numeric_field_get_float(&fields->constant_real, &fval) == FRACTUS_STATUS_OK) {
                pending->constant_real = fval;
            }
            if (fields->constant_imag.editing && fractus_ui_numeric_field_get_float(&fields->constant_imag, &fval) == FRACTUS_STATUS_OK) {
                pending->constant_imag = fval;
            }
            if (fields->escape_radius_squared.editing && fractus_ui_numeric_field_get_int(&fields->escape_radius_squared, &ival) == FRACTUS_STATUS_OK) {
                pending->escape_radius_squared = (double)ival;
            }
            if (fields->cutoff.editing && fractus_ui_numeric_field_get_int(&fields->cutoff, &ival) == FRACTUS_STATUS_OK) {
                pending->cutoff = (double)ival;
            }
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->escape_radius_squared, pending->cutoff);
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
        } else if (fields->constant_real.editing) {
            active_field = &fields->constant_real;
        } else if (fields->constant_imag.editing) {
            active_field = &fields->constant_imag;
        } else if (fields->escape_radius_squared.editing) {
            active_field = &fields->escape_radius_squared;
        } else if (fields->cutoff.editing) {
            active_field = &fields->cutoff;
        }

        if (active_field != NULL) {
            if (fractus_ui_numeric_field_handle_input(active_field, ui, fonts, &edit_accepted, &edit_cancelled) == FRACTUS_STATUS_OK) {
                if (edit_accepted) {
                    double fval;
                    int32_t ival;
                    if (active_field == &fields->xmin && fractus_ui_numeric_field_get_float(active_field, &fval) == FRACTUS_STATUS_OK) {
                        pending->xmin = fval;
                    } else if (active_field == &fields->xmax && fractus_ui_numeric_field_get_float(active_field, &fval) == FRACTUS_STATUS_OK) {
                        pending->xmax = fval;
                    } else if (active_field == &fields->ymin && fractus_ui_numeric_field_get_float(active_field, &fval) == FRACTUS_STATUS_OK) {
                        pending->ymin = fval;
                    } else if (active_field == &fields->ymax && fractus_ui_numeric_field_get_float(active_field, &fval) == FRACTUS_STATUS_OK) {
                        pending->ymax = fval;
                    } else if (active_field == &fields->constant_real && fractus_ui_numeric_field_get_float(active_field, &fval) == FRACTUS_STATUS_OK) {
                        pending->constant_real = fval;
                    } else if (active_field == &fields->constant_imag && fractus_ui_numeric_field_get_float(active_field, &fval) == FRACTUS_STATUS_OK) {
                        pending->constant_imag = fval;
                    } else if (active_field == &fields->escape_radius_squared && fractus_ui_numeric_field_get_int(active_field, &ival) == FRACTUS_STATUS_OK) {
                        pending->escape_radius_squared = (double)ival;
                    } else if (active_field == &fields->cutoff && fractus_ui_numeric_field_get_int(active_field, &ival) == FRACTUS_STATUS_OK) {
                        pending->cutoff = (double)ival;
                    }
                    fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->escape_radius_squared, pending->cutoff);
                } else if (edit_cancelled) {
                    fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->escape_radius_squared, pending->cutoff);
                }
            }
            skip_mouse_input = 1;
        }
    }

    /* 4. Raton y acciones de botones. */
    if (!skip_mouse_input && fractus_ui_menu(ui, dialog_options, dialog_entry_count, &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == FRACTUS_APP_BIOMORPH_CANCEL) {
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_DRAW) {
            *params = *pending;
            *view = FRACTUS_APP_VIEW_BIOMORPH;
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_XMIN_DEC) {
            pending->xmin = fractus_app_clamp_f64(pending->xmin - 0.1, -5.0, pending->xmax - 0.1);
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->escape_radius_squared, pending->cutoff);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_XMIN_INC) {
            pending->xmin = fractus_app_clamp_f64(pending->xmin + 0.1, -5.0, pending->xmax - 0.1);
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->escape_radius_squared, pending->cutoff);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_XMAX_DEC) {
            pending->xmax = fractus_app_clamp_f64(pending->xmax - 0.1, pending->xmin + 0.1, 5.0);
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->escape_radius_squared, pending->cutoff);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_XMAX_INC) {
            pending->xmax = fractus_app_clamp_f64(pending->xmax + 0.1, pending->xmin + 0.1, 5.0);
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->escape_radius_squared, pending->cutoff);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_YMIN_DEC) {
            pending->ymin = fractus_app_clamp_f64(pending->ymin - 0.1, -5.0, pending->ymax - 0.1);
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->escape_radius_squared, pending->cutoff);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_YMIN_INC) {
            pending->ymin = fractus_app_clamp_f64(pending->ymin + 0.1, -5.0, pending->ymax - 0.1);
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->escape_radius_squared, pending->cutoff);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_YMAX_DEC) {
            pending->ymax = fractus_app_clamp_f64(pending->ymax - 0.1, pending->ymin + 0.1, 5.0);
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->escape_radius_squared, pending->cutoff);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_YMAX_INC) {
            pending->ymax = fractus_app_clamp_f64(pending->ymax + 0.1, pending->ymin + 0.1, 5.0);
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->escape_radius_squared, pending->cutoff);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_CREAL_DEC) {
            pending->constant_real = fractus_app_clamp_f64(pending->constant_real - 0.05, -2.0, 2.0);
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->escape_radius_squared, pending->cutoff);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_CREAL_INC) {
            pending->constant_real = fractus_app_clamp_f64(pending->constant_real + 0.05, -2.0, 2.0);
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->escape_radius_squared, pending->cutoff);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_CIMAG_DEC) {
            pending->constant_imag = fractus_app_clamp_f64(pending->constant_imag - 0.05, -2.0, 2.0);
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->escape_radius_squared, pending->cutoff);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_CIMAG_INC) {
            pending->constant_imag = fractus_app_clamp_f64(pending->constant_imag + 0.05, -2.0, 2.0);
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->escape_radius_squared, pending->cutoff);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_RADIUS_DEC) {
            pending->escape_radius_squared = (double)fractus_app_clamp_i32((int32_t)pending->escape_radius_squared - 4, 4, 1000);
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->escape_radius_squared, pending->cutoff);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_RADIUS_INC) {
            pending->escape_radius_squared = (double)fractus_app_clamp_i32((int32_t)pending->escape_radius_squared + 4, 4, 1000);
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->escape_radius_squared, pending->cutoff);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_CUTOFF_DEC) {
            pending->cutoff = (double)fractus_app_clamp_i32((int32_t)pending->cutoff - 1, 1, 1000);
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->escape_radius_squared, pending->cutoff);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_CUTOFF_INC) {
            pending->cutoff = (double)fractus_app_clamp_i32((int32_t)pending->cutoff + 1, 1, 1000);
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->escape_radius_squared, pending->cutoff);
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
