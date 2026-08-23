#include "app/biomorphs.h"
#include "app/files.h"

#include <stdio.h>
#include <string.h>

static const int32_t biomorph_x0 = 135;
static const int32_t biomorph_y0 = 30;

void fractus_app_init_biomorph_fields(
    fractus_app_biomorph_fields *fields,
    double xmin,
    double xmax,
    double ymin,
    double ymax,
    double constant_real,
    double constant_imag,
    uint32_t max_iterations,
    double escape_radius_squared,
    double cutoff)
{
    const int32_t x0 = biomorph_x0;
    const int32_t y0 = biomorph_y0;

    if (fields == NULL) {
        return;
    }
    (void)fractus_ui_numeric_field_init_float(&fields->xmin, (fractus_rect_i32){x0 + 189, y0 + 48, 73, 20}, xmin, -5.0, 5.0, 3);
    (void)fractus_ui_numeric_field_init_float(&fields->xmax, (fractus_rect_i32){x0 + 189, y0 + 74, 73, 20}, xmax, -5.0, 5.0, 3);
    (void)fractus_ui_numeric_field_init_float(&fields->ymin, (fractus_rect_i32){x0 + 189, y0 + 100, 73, 20}, ymin, -5.0, 5.0, 3);
    (void)fractus_ui_numeric_field_init_float(&fields->ymax, (fractus_rect_i32){x0 + 189, y0 + 126, 73, 20}, ymax, -5.0, 5.0, 3);
    (void)fractus_ui_numeric_field_init_float(&fields->constant_real, (fractus_rect_i32){x0 + 189, y0 + 152, 73, 20}, constant_real, -5.0, 5.0, 3);
    (void)fractus_ui_numeric_field_init_float(&fields->constant_imag, (fractus_rect_i32){x0 + 189, y0 + 178, 73, 20}, constant_imag, -5.0, 5.0, 3);
    (void)fractus_ui_numeric_field_init_int(&fields->max_iterations, (fractus_rect_i32){x0 + 189, y0 + 204, 73, 20}, (int32_t)max_iterations, 1, 1000);
    (void)fractus_ui_numeric_field_init_int(&fields->escape_radius_squared, (fractus_rect_i32){x0 + 189, y0 + 230, 73, 20}, (int32_t)escape_radius_squared, 4, 1000);
    (void)fractus_ui_numeric_field_init_int(&fields->cutoff, (fractus_rect_i32){x0 + 189, y0 + 256, 73, 20}, (int32_t)cutoff, 1, 100);
}

static size_t fractus_app_build_biomorph_config_entries(
    fractus_app_menu_entry *entries,
    size_t capacity)
{
    const int32_t x0 = biomorph_x0;
    const int32_t y0 = biomorph_y0;
    const fractus_app_menu_entry controls[] = {
        {FRACTUS_APP_RECT(x0 + 269, y0 + 48, x0 + 309, y0 + 68), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(x0 + 314, y0 + 48, x0 + 354, y0 + 68), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(x0 + 269, y0 + 74, x0 + 309, y0 + 94), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(x0 + 314, y0 + 74, x0 + 354, y0 + 94), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(x0 + 269, y0 + 100, x0 + 309, y0 + 120), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(x0 + 314, y0 + 100, x0 + 354, y0 + 120), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(x0 + 269, y0 + 126, x0 + 309, y0 + 146), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(x0 + 314, y0 + 126, x0 + 354, y0 + 146), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(x0 + 269, y0 + 152, x0 + 309, y0 + 172), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(x0 + 314, y0 + 152, x0 + 354, y0 + 172), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(x0 + 269, y0 + 178, x0 + 309, y0 + 198), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(x0 + 314, y0 + 178, x0 + 354, y0 + 198), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(x0 + 269, y0 + 204, x0 + 309, y0 + 224), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(x0 + 314, y0 + 204, x0 + 354, y0 + 224), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(x0 + 269, y0 + 230, x0 + 309, y0 + 250), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(x0 + 314, y0 + 230, x0 + 354, y0 + 250), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(x0 + 269, y0 + 256, x0 + 309, y0 + 276), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(x0 + 314, y0 + 256, x0 + 354, y0 + 276), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(x0 + 105, y0 + 282, x0 + 177, y0 + 302), 8u, 0u, "z² + c"},
        {FRACTUS_APP_RECT(x0 + 190, y0 + 282, x0 + 262, y0 + 302), 8u, 0u, "z³ + c"},
        {FRACTUS_APP_RECT(x0 + 275, y0 + 282, x0 + 347, y0 + 302), 8u, 0u, "z⁴ + c"},
        {FRACTUS_APP_RECT(x0 + 105, y0 + 306, x0 + 177, y0 + 326), 8u, 0u, "z⁵ + c"},
        {FRACTUS_APP_RECT(x0 + 190, y0 + 306, x0 + 262, y0 + 326), 8u, 0u, "sen z + c"},
        {FRACTUS_APP_RECT(x0 + 275, y0 + 306, x0 + 347, y0 + 326), 8u, 0u, "eᶻ + c"},
        {FRACTUS_APP_RECT(x0 + 90, y0 + 332, x0 + 152, y0 + 352), 8u, 0u, "Re OR Im"},
        {FRACTUS_APP_RECT(x0 + 157, y0 + 332, x0 + 229, y0 + 352), 8u, 0u, "Re AND Im"},
        {FRACTUS_APP_RECT(x0 + 234, y0 + 332, x0 + 291, y0 + 352), 8u, 0u, "Solo Re"},
        {FRACTUS_APP_RECT(x0 + 296, y0 + 332, x0 + 354, y0 + 352), 8u, 0u, "Solo Im"},
        {FRACTUS_APP_RECT(x0 + 75, y0 + 388, x0 + 175, y0 + 408), 8u, 0u, "Dibujar"},
        {FRACTUS_APP_RECT(x0 + 195, y0 + 388, x0 + 295, y0 + 408), 0u, 15u, "Cancelar"}
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
    const int32_t x0 = biomorph_x0;
    const int32_t y0 = biomorph_y0;
    const fractus_ui_radio_option eq_options[] = {
        {FRACTUS_APP_RECT(x0 + 160, y0 + 282, x0 + 177, y0 + 302), "z² + c"},
        {FRACTUS_APP_RECT(x0 + 230, y0 + 282, x0 + 262, y0 + 302), "z³ + c"},
        {FRACTUS_APP_RECT(x0 + 295, y0 + 282, x0 + 347, y0 + 302), "z⁴ + c"},
        {FRACTUS_APP_RECT(x0 + 160, y0 + 306, x0 + 177, y0 + 326), "z⁵ + c"},
        {FRACTUS_APP_RECT(x0 + 230, y0 + 306, x0 + 262, y0 + 326), "sen z + c"},
        {FRACTUS_APP_RECT(x0 + 295, y0 + 306, x0 + 347, y0 + 326), "eᶻ + c"}
    };
    const fractus_ui_radio_option trap_options[] = {
        {FRACTUS_APP_RECT(x0 + 95, y0 + 332, x0 + 152, y0 + 352), "Re OR Im"},
        {FRACTUS_APP_RECT(x0 + 160, y0 + 332, x0 + 229, y0 + 352), "Re AND Im"},
        {FRACTUS_APP_RECT(x0 + 230, y0 + 332, x0 + 291, y0 + 352), "Solo Re"},
        {FRACTUS_APP_RECT(x0 + 295, y0 + 332, x0 + 354, y0 + 352), "Solo Im"}
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
        fractus_ui_draw_window(framebuffer, x0, y0, x0 + 369, y0 + 418) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, x0 + 185, y0 + 4, 15u, "Biomorfos de Clifford A. Pickover") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_group_box(framebuffer, fonts, x0 + 5, y0 + 32, x0 + 364, y0 + 362, 8u, 0u, "Parámetros del biomorfo") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 2. Textos y controles. */
    if (fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, x0 + 15, y0 + 53, 0u, "Mínimo valor real") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->xmin) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, x0 + 15, y0 + 79, 0u, "Máximo valor real") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->xmax) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, x0 + 15, y0 + 105, 0u, "Mínimo valor imaginario") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->ymin) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, x0 + 15, y0 + 131, 0u, "Máximo valor imaginario") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->ymax) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, x0 + 15, y0 + 157, 0u, "Constante real") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->constant_real) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, x0 + 15, y0 + 183, 0u, "Constante imaginaria") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->constant_imag) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, x0 + 15, y0 + 209, 0u, "Iteraciones máximas (1-1000)") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->max_iterations) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, x0 + 15, y0 + 235, 0u, "Radio de escape al cuadrado (4-1000)") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->escape_radius_squared) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, x0 + 15, y0 + 261, 0u, "Umbral de escape (1-100)") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->cutoff) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, x0 + 15, y0 + 287, 0u, "Ecuación") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_radio_list(
            framebuffer,
            fonts,
            eq_options,
            FRACTUS_APP_ARRAY_COUNT(eq_options),
            (int)pending->equation,
            (active_index >= FRACTUS_APP_BIOMORPH_EQ_0 && active_index <= FRACTUS_APP_BIOMORPH_EQ_5) ?
                active_index - FRACTUS_APP_BIOMORPH_EQ_0 : -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, x0 + 15, y0 + 337, 0u, "Condición") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_radio_list(
            framebuffer,
            fonts,
            trap_options,
            FRACTUS_APP_ARRAY_COUNT(trap_options),
            (int)pending->trap_mode,
            (active_index >= FRACTUS_APP_BIOMORPH_TRAP_0 && active_index <= FRACTUS_APP_BIOMORPH_TRAP_3) ?
                active_index - FRACTUS_APP_BIOMORPH_TRAP_0 : -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, x0 + 185, y0 + 370, 0u, "Estos valores solo afectan al dibujo actual y no cambian la cfg global.") != FRACTUS_STATUS_OK ||
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
        } else if (fractus_ui_point_in_rect(click_pos, fields->max_iterations.bounds)) {
            clicked_field = &fields->max_iterations;
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
            if (fields->max_iterations.editing && fractus_ui_numeric_field_get_int(&fields->max_iterations, &ival) == FRACTUS_STATUS_OK) {
                pending->max_iterations = (uint32_t)ival;
            }
            if (fields->escape_radius_squared.editing && fractus_ui_numeric_field_get_int(&fields->escape_radius_squared, &ival) == FRACTUS_STATUS_OK) {
                pending->escape_radius_squared = (double)ival;
            }
            if (fields->cutoff.editing && fractus_ui_numeric_field_get_int(&fields->cutoff, &ival) == FRACTUS_STATUS_OK) {
                pending->cutoff = (double)ival;
            }
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->max_iterations, pending->escape_radius_squared, pending->cutoff);
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
            } else if (clicked_field == &fields->max_iterations) {
                (void)fractus_ui_numeric_field_begin_edit(&fields->max_iterations);
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
            if (fields->max_iterations.editing && fractus_ui_numeric_field_get_int(&fields->max_iterations, &ival) == FRACTUS_STATUS_OK) {
                pending->max_iterations = (uint32_t)ival;
            }
            if (fields->escape_radius_squared.editing && fractus_ui_numeric_field_get_int(&fields->escape_radius_squared, &ival) == FRACTUS_STATUS_OK) {
                pending->escape_radius_squared = (double)ival;
            }
            if (fields->cutoff.editing && fractus_ui_numeric_field_get_int(&fields->cutoff, &ival) == FRACTUS_STATUS_OK) {
                pending->cutoff = (double)ival;
            }
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->max_iterations, pending->escape_radius_squared, pending->cutoff);
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
        } else if (fields->max_iterations.editing) {
            active_field = &fields->max_iterations;
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
                    } else if (active_field == &fields->max_iterations && fractus_ui_numeric_field_get_int(active_field, &ival) == FRACTUS_STATUS_OK) {
                        pending->max_iterations = (uint32_t)ival;
                    } else if (active_field == &fields->escape_radius_squared && fractus_ui_numeric_field_get_int(active_field, &ival) == FRACTUS_STATUS_OK) {
                        pending->escape_radius_squared = (double)ival;
                    } else if (active_field == &fields->cutoff && fractus_ui_numeric_field_get_int(active_field, &ival) == FRACTUS_STATUS_OK) {
                        pending->cutoff = (double)ival;
                    }
                    fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->max_iterations, pending->escape_radius_squared, pending->cutoff);
                } else if (edit_cancelled) {
                    fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->max_iterations, pending->escape_radius_squared, pending->cutoff);
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
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->max_iterations, pending->escape_radius_squared, pending->cutoff);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_XMIN_INC) {
            pending->xmin = fractus_app_clamp_f64(pending->xmin + 0.1, -5.0, pending->xmax - 0.1);
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->max_iterations, pending->escape_radius_squared, pending->cutoff);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_XMAX_DEC) {
            pending->xmax = fractus_app_clamp_f64(pending->xmax - 0.1, pending->xmin + 0.1, 5.0);
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->max_iterations, pending->escape_radius_squared, pending->cutoff);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_XMAX_INC) {
            pending->xmax = fractus_app_clamp_f64(pending->xmax + 0.1, pending->xmin + 0.1, 5.0);
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->max_iterations, pending->escape_radius_squared, pending->cutoff);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_YMIN_DEC) {
            pending->ymin = fractus_app_clamp_f64(pending->ymin - 0.1, -5.0, pending->ymax - 0.1);
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->max_iterations, pending->escape_radius_squared, pending->cutoff);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_YMIN_INC) {
            pending->ymin = fractus_app_clamp_f64(pending->ymin + 0.1, -5.0, pending->ymax - 0.1);
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->max_iterations, pending->escape_radius_squared, pending->cutoff);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_YMAX_DEC) {
            pending->ymax = fractus_app_clamp_f64(pending->ymax - 0.1, pending->ymin + 0.1, 5.0);
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->max_iterations, pending->escape_radius_squared, pending->cutoff);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_YMAX_INC) {
            pending->ymax = fractus_app_clamp_f64(pending->ymax + 0.1, pending->ymin + 0.1, 5.0);
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->max_iterations, pending->escape_radius_squared, pending->cutoff);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_CREAL_DEC) {
            pending->constant_real = fractus_app_clamp_f64(pending->constant_real - 0.05, -2.0, 2.0);
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->max_iterations, pending->escape_radius_squared, pending->cutoff);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_CREAL_INC) {
            pending->constant_real = fractus_app_clamp_f64(pending->constant_real + 0.05, -2.0, 2.0);
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->max_iterations, pending->escape_radius_squared, pending->cutoff);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_CIMAG_DEC) {
            pending->constant_imag = fractus_app_clamp_f64(pending->constant_imag - 0.05, -2.0, 2.0);
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->max_iterations, pending->escape_radius_squared, pending->cutoff);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_CIMAG_INC) {
            pending->constant_imag = fractus_app_clamp_f64(pending->constant_imag + 0.05, -2.0, 2.0);
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->max_iterations, pending->escape_radius_squared, pending->cutoff);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_ITERATIONS_DEC) {
            pending->max_iterations = (uint32_t)fractus_app_clamp_i32((int32_t)pending->max_iterations - 1, 1, 1000);
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->max_iterations, pending->escape_radius_squared, pending->cutoff);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_ITERATIONS_INC) {
            pending->max_iterations = (uint32_t)fractus_app_clamp_i32((int32_t)pending->max_iterations + 1, 1, 1000);
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->max_iterations, pending->escape_radius_squared, pending->cutoff);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_RADIUS_DEC) {
            pending->escape_radius_squared = (double)fractus_app_clamp_i32((int32_t)pending->escape_radius_squared - 4, 4, 1000);
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->max_iterations, pending->escape_radius_squared, pending->cutoff);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_RADIUS_INC) {
            pending->escape_radius_squared = (double)fractus_app_clamp_i32((int32_t)pending->escape_radius_squared + 4, 4, 1000);
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->max_iterations, pending->escape_radius_squared, pending->cutoff);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_CUTOFF_DEC) {
            pending->cutoff = (double)fractus_app_clamp_i32((int32_t)pending->cutoff - 1, 1, 100);
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->max_iterations, pending->escape_radius_squared, pending->cutoff);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_CUTOFF_INC) {
            pending->cutoff = (double)fractus_app_clamp_i32((int32_t)pending->cutoff + 1, 1, 100);
            fractus_app_init_biomorph_fields(fields, pending->xmin, pending->xmax, pending->ymin, pending->ymax, pending->constant_real, pending->constant_imag, pending->max_iterations, pending->escape_radius_squared, pending->cutoff);
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
    size_t error_message_size,
    char *saved_filename,
    size_t saved_filename_size)
{
    if (platform == NULL || framebuffer == NULL || fonts == NULL || params == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_fractal_render_biomorph(framebuffer, params) != FRACTUS_STATUS_OK ||
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
        "Biomorfo inicial - ESC o botón derecho: menú - G: grabar dibujo - F: flujo",
        saved_filename,
        0);
}
