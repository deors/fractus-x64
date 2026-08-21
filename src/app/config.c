#include "app/config.h"

fractus_status fractus_app_apply_legacy_config(
    fractus_framebuffer *framebuffer,
    const fractus_legacy_config *config)
{
    uint32_t i;

    if (framebuffer == NULL || config == NULL || !framebuffer->initialized) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    for (i = 0u; i < FRACTUS_LEGACY_PALETTE_DATA_COUNT; ++i) {
        framebuffer->palette.entries[i + 16u] = config->palette[i];
    }

    framebuffer->palette_dirty = 1;
    return FRACTUS_STATUS_OK;
}

void fractus_app_apply_legacy_numeric_config(
    const fractus_legacy_config *config,
    fractus_mandelbrot_params *mandelbrot_params,
    fractus_julia_params *julia_params,
    fractus_biomorph_params *biomorph_params)
{
    if (config == NULL || mandelbrot_params == NULL || julia_params == NULL || biomorph_params == NULL) {
        return;
    }

    mandelbrot_params->max_iterations = (config->iterations > 0)
        ? (uint32_t)config->iterations
        : mandelbrot_params->max_iterations;
    mandelbrot_params->escape_radius_squared = (config->escape_radius_squared > 0)
        ? (double)config->escape_radius_squared
        : mandelbrot_params->escape_radius_squared;

    julia_params->max_iterations = mandelbrot_params->max_iterations;
    julia_params->escape_radius_squared = mandelbrot_params->escape_radius_squared;

    biomorph_params->max_iterations = mandelbrot_params->max_iterations;
    biomorph_params->escape_radius_squared = (config->biomorph_escape_radius_squared > 0)
        ? (double)config->biomorph_escape_radius_squared
        : biomorph_params->escape_radius_squared;
    biomorph_params->cutoff = (config->biomorph_cutoff > 0)
        ? (double)config->biomorph_cutoff
        : biomorph_params->cutoff;
}

void fractus_app_capture_palette_to_config(
    const fractus_framebuffer *framebuffer,
    fractus_legacy_config *config)
{
    uint32_t i;

    if (framebuffer == NULL || config == NULL || !framebuffer->initialized) {
        return;
    }

    for (i = 0u; i < FRACTUS_LEGACY_PALETTE_DATA_COUNT; ++i) {
        config->palette[i] = framebuffer->palette.entries[i + 16u];
    }
}

fractus_status fractus_app_save_legacy_config(
    const char *path,
    const fractus_framebuffer *framebuffer,
    fractus_legacy_config *config)
{
    if (path == NULL || framebuffer == NULL || config == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    fractus_app_capture_palette_to_config(framebuffer, config);
    return fractus_legacy_config_save(path, config);
}

fractus_status fractus_app_persist_current_palette(
    const char *cfg_path,
    const fractus_framebuffer *framebuffer,
    fractus_legacy_config *config)
{
    if (framebuffer == NULL || config == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    fractus_app_capture_palette_to_config(framebuffer, config);
    if (cfg_path != NULL && cfg_path[0] != '\0') {
        return fractus_legacy_config_save(cfg_path, config);
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_app_load_legacy_assets(
    const fractus_platform_context *platform,
    fractus_framebuffer *framebuffer,
    fractus_mandelbrot_params *mandelbrot_params,
    fractus_julia_params *julia_params,
    fractus_biomorph_params *biomorph_params,
    fractus_legacy_config *legacy_config,
    char *cfg_path,
    size_t cfg_path_size)
{
    char path_buffer[512];
    fractus_status status;

    if (platform == NULL || framebuffer == NULL || mandelbrot_params == NULL ||
        julia_params == NULL || biomorph_params == NULL || legacy_config == NULL ||
        cfg_path == NULL || cfg_path_size == 0u) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_legacy_config_init_default(legacy_config) != FRACTUS_STATUS_OK ||
        fractus_formats_resolve_legacy_write_path(platform, "fractus.cfg", cfg_path, cfg_path_size) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    status = fractus_formats_resolve_legacy_path(platform, "fractus.cfg", path_buffer, sizeof(path_buffer));
    if (status == FRACTUS_STATUS_OK) {
        fractus_app_log(path_buffer);
        if (fractus_legacy_config_load(path_buffer, legacy_config) != FRACTUS_STATUS_OK) {
            fractus_app_log("startup: fractus.cfg found but load failed");
            return FRACTUS_STATUS_ERROR;
        }

        if (fractus_formats_copy_path(path_buffer, cfg_path, cfg_path_size) != FRACTUS_STATUS_OK) {
            return FRACTUS_STATUS_ERROR;
        }

        fractus_app_log("startup: legacy config loaded");
        fractus_app_apply_legacy_numeric_config(legacy_config, mandelbrot_params, julia_params, biomorph_params);
        return fractus_app_apply_legacy_config(framebuffer, legacy_config);
    }

    fractus_app_log("startup: fractus.cfg not found");
    fractus_app_apply_legacy_numeric_config(legacy_config, mandelbrot_params, julia_params, biomorph_params);
    return fractus_app_apply_legacy_config(framebuffer, legacy_config);
}

static size_t fractus_app_build_fractals_config_entries(
    fractus_app_menu_entry *entries,
    size_t capacity)
{
    static const fractus_app_menu_entry controls[] = {
        {FRACTUS_APP_RECT(404, 169, 444, 189), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(449, 169, 489, 189), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(404, 195, 444, 215), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(449, 195, 489, 215), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(404, 245, 444, 265), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(449, 245, 489, 265), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(404, 271, 444, 291), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(449, 271, 489, 291), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(210, 325, 310, 345), 8u, 0u, "Guardar"},
        {FRACTUS_APP_RECT(330, 325, 430, 345), 0u, 15u, "Cancelar"}
    };

    return fractus_app_copy_control_entries(
        entries,
        capacity,
        controls,
        FRACTUS_APP_ARRAY_COUNT(controls));
}

fractus_status fractus_app_run_fractals_default_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_legacy_config *legacy_config,
    fractus_legacy_config *config_draft,
    fractus_mandelbrot_params *mandelbrot_params,
    fractus_mandelbrot_dem_params *mandelbrot_dem_params,
    fractus_julia_params *julia_params,
    fractus_julia_dem_params *julia_dem_params,
    fractus_biomorph_params *biomorph_params,
    const char *cfg_path,
    fractus_ui_numeric_field *iterations_field,
    fractus_ui_numeric_field *escape_radius_field,
    fractus_ui_numeric_field *biomorph_radius_field,
    fractus_ui_numeric_field *biomorph_cutoff_field,
    fractus_app_view *view)
{
    fractus_app_menu_entry dialog_entries[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    fractus_ui_menu_option dialog_options[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    size_t dialog_entry_count;
    int selected_menu = -1;
    int cancelled = 0;
    int skip_mouse_input = 0;

    if (framebuffer == NULL || fonts == NULL || ui == NULL ||
        legacy_config == NULL || config_draft == NULL ||
        mandelbrot_params == NULL || mandelbrot_dem_params == NULL ||
        julia_params == NULL || julia_dem_params == NULL ||
        biomorph_params == NULL || iterations_field == NULL ||
        escape_radius_field == NULL || biomorph_radius_field == NULL ||
        biomorph_cutoff_field == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    dialog_entry_count = fractus_app_build_fractals_config_entries(dialog_entries, FRACTUS_APP_ARRAY_COUNT(dialog_entries));
    fractus_app_build_options_from_entries(dialog_options, dialog_entries, dialog_entry_count);

    /* 1. Contenedor exterior y paneles por tipo de conjunto fractal */
    if (fractus_app_render_main_menu(framebuffer, fonts, -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_window(framebuffer, 135, 123, 504, 355) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, 320, 127, 15u, "Parametros por defecto de conjuntos fractales") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_group_box(framebuffer, fonts, 140, 155, 499, 223, 8u, 0u, "Mandelbrot y Julia") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_group_box(framebuffer, fonts, 140, 231, 499, 299, 8u, 0u, "Biomorfos de Pickover") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 2. Textos de etiquetas, campos numericos, tip y botones */
    if (fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 150, 174, 0u, "Iteraciones maximas (16-1024)") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, iterations_field) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 150, 200, 0u, "Radio de escape al cuadrado (4-1000)") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, escape_radius_field) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 150, 250, 0u, "Radio de escape al cuadrado (4-1000)") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, biomorph_radius_field) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 150, 276, 0u, "Umbral de escape (1-100)") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, biomorph_cutoff_field) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, 320, 307, 0u, "Los valores por defecto de los parametros clave se guardan en fractus.cfg.") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_button_list(
            framebuffer,
            fonts,
            dialog_entries,
            dialog_entry_count,
            fractus_ui_active_menu_index(ui, dialog_options, dialog_entry_count)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Gestion de foco y edicion de campos editables */
    {
        int edit_accepted = 0;
        int edit_cancelled = 0;

        if (ui->release_pending && ui->release_event.buttons.left) {
            fractus_point_i32 click_pos = ui->release_event.position;

            if (fractus_ui_point_in_rect(click_pos, iterations_field->bounds)) {
                int32_t val;
                if (escape_radius_field->editing && fractus_ui_numeric_field_get_int(escape_radius_field, &val) == FRACTUS_STATUS_OK) {
                    config_draft->escape_radius_squared = (int16_t)val;
                }
                if (biomorph_radius_field->editing && fractus_ui_numeric_field_get_int(biomorph_radius_field, &val) == FRACTUS_STATUS_OK) {
                    config_draft->biomorph_escape_radius_squared = (int16_t)val;
                }
                if (biomorph_cutoff_field->editing && fractus_ui_numeric_field_get_int(biomorph_cutoff_field, &val) == FRACTUS_STATUS_OK) {
                    config_draft->biomorph_cutoff = (int16_t)val;
                }
                (void)fractus_ui_numeric_field_init_int(escape_radius_field, (fractus_rect_i32){324, 195, 73, 20}, (int32_t)config_draft->escape_radius_squared, 4, 1000);
                (void)fractus_ui_numeric_field_init_int(biomorph_radius_field, (fractus_rect_i32){324, 245, 73, 20}, (int32_t)config_draft->biomorph_escape_radius_squared, 4, 1000);
                (void)fractus_ui_numeric_field_init_int(biomorph_cutoff_field, (fractus_rect_i32){324, 271, 73, 20}, (int32_t)config_draft->biomorph_cutoff, 1, 100);
                if (!iterations_field->editing) {
                    (void)fractus_ui_numeric_field_begin_edit(iterations_field);
                }
            } else if (fractus_ui_point_in_rect(click_pos, escape_radius_field->bounds)) {
                int32_t val;
                if (iterations_field->editing && fractus_ui_numeric_field_get_int(iterations_field, &val) == FRACTUS_STATUS_OK) {
                    config_draft->iterations = (int16_t)val;
                }
                if (biomorph_radius_field->editing && fractus_ui_numeric_field_get_int(biomorph_radius_field, &val) == FRACTUS_STATUS_OK) {
                    config_draft->biomorph_escape_radius_squared = (int16_t)val;
                }
                if (biomorph_cutoff_field->editing && fractus_ui_numeric_field_get_int(biomorph_cutoff_field, &val) == FRACTUS_STATUS_OK) {
                    config_draft->biomorph_cutoff = (int16_t)val;
                }
                (void)fractus_ui_numeric_field_init_int(iterations_field, (fractus_rect_i32){324, 169, 73, 20}, (int32_t)config_draft->iterations, 16, 1024);
                (void)fractus_ui_numeric_field_init_int(biomorph_radius_field, (fractus_rect_i32){324, 245, 73, 20}, (int32_t)config_draft->biomorph_escape_radius_squared, 4, 1000);
                (void)fractus_ui_numeric_field_init_int(biomorph_cutoff_field, (fractus_rect_i32){324, 271, 73, 20}, (int32_t)config_draft->biomorph_cutoff, 1, 100);
                if (!escape_radius_field->editing) {
                    (void)fractus_ui_numeric_field_begin_edit(escape_radius_field);
                }
            } else if (fractus_ui_point_in_rect(click_pos, biomorph_radius_field->bounds)) {
                int32_t val;
                if (iterations_field->editing && fractus_ui_numeric_field_get_int(iterations_field, &val) == FRACTUS_STATUS_OK) {
                    config_draft->iterations = (int16_t)val;
                }
                if (escape_radius_field->editing && fractus_ui_numeric_field_get_int(escape_radius_field, &val) == FRACTUS_STATUS_OK) {
                    config_draft->escape_radius_squared = (int16_t)val;
                }
                if (biomorph_cutoff_field->editing && fractus_ui_numeric_field_get_int(biomorph_cutoff_field, &val) == FRACTUS_STATUS_OK) {
                    config_draft->biomorph_cutoff = (int16_t)val;
                }
                (void)fractus_ui_numeric_field_init_int(iterations_field, (fractus_rect_i32){324, 169, 73, 20}, (int32_t)config_draft->iterations, 16, 1024);
                (void)fractus_ui_numeric_field_init_int(escape_radius_field, (fractus_rect_i32){324, 195, 73, 20}, (int32_t)config_draft->escape_radius_squared, 4, 1000);
                (void)fractus_ui_numeric_field_init_int(biomorph_cutoff_field, (fractus_rect_i32){324, 271, 73, 20}, (int32_t)config_draft->biomorph_cutoff, 1, 100);
                if (!biomorph_radius_field->editing) {
                    (void)fractus_ui_numeric_field_begin_edit(biomorph_radius_field);
                }
            } else if (fractus_ui_point_in_rect(click_pos, biomorph_cutoff_field->bounds)) {
                int32_t val;
                if (iterations_field->editing && fractus_ui_numeric_field_get_int(iterations_field, &val) == FRACTUS_STATUS_OK) {
                    config_draft->iterations = (int16_t)val;
                }
                if (escape_radius_field->editing && fractus_ui_numeric_field_get_int(escape_radius_field, &val) == FRACTUS_STATUS_OK) {
                    config_draft->escape_radius_squared = (int16_t)val;
                }
                if (biomorph_radius_field->editing && fractus_ui_numeric_field_get_int(biomorph_radius_field, &val) == FRACTUS_STATUS_OK) {
                    config_draft->biomorph_escape_radius_squared = (int16_t)val;
                }
                (void)fractus_ui_numeric_field_init_int(iterations_field, (fractus_rect_i32){324, 169, 73, 20}, (int32_t)config_draft->iterations, 16, 1024);
                (void)fractus_ui_numeric_field_init_int(escape_radius_field, (fractus_rect_i32){324, 195, 73, 20}, (int32_t)config_draft->escape_radius_squared, 4, 1000);
                (void)fractus_ui_numeric_field_init_int(biomorph_radius_field, (fractus_rect_i32){324, 245, 73, 20}, (int32_t)config_draft->biomorph_escape_radius_squared, 4, 1000);
                if (!biomorph_cutoff_field->editing) {
                    (void)fractus_ui_numeric_field_begin_edit(biomorph_cutoff_field);
                }
            } else {
                int32_t val;
                if (iterations_field->editing) {
                    if (fractus_ui_numeric_field_get_int(iterations_field, &val) == FRACTUS_STATUS_OK) {
                        config_draft->iterations = (int16_t)val;
                    }
                    (void)fractus_ui_numeric_field_init_int(iterations_field, (fractus_rect_i32){324, 169, 73, 20}, (int32_t)config_draft->iterations, 16, 1024);
                }
                if (escape_radius_field->editing) {
                    if (fractus_ui_numeric_field_get_int(escape_radius_field, &val) == FRACTUS_STATUS_OK) {
                        config_draft->escape_radius_squared = (int16_t)val;
                    }
                    (void)fractus_ui_numeric_field_init_int(escape_radius_field, (fractus_rect_i32){324, 195, 73, 20}, (int32_t)config_draft->escape_radius_squared, 4, 1000);
                }
                if (biomorph_radius_field->editing) {
                    if (fractus_ui_numeric_field_get_int(biomorph_radius_field, &val) == FRACTUS_STATUS_OK) {
                        config_draft->biomorph_escape_radius_squared = (int16_t)val;
                    }
                    (void)fractus_ui_numeric_field_init_int(biomorph_radius_field, (fractus_rect_i32){324, 245, 73, 20}, (int32_t)config_draft->biomorph_escape_radius_squared, 4, 1000);
                }
                if (biomorph_cutoff_field->editing) {
                    if (fractus_ui_numeric_field_get_int(biomorph_cutoff_field, &val) == FRACTUS_STATUS_OK) {
                        config_draft->biomorph_cutoff = (int16_t)val;
                    }
                    (void)fractus_ui_numeric_field_init_int(biomorph_cutoff_field, (fractus_rect_i32){324, 271, 73, 20}, (int32_t)config_draft->biomorph_cutoff, 1, 100);
                }
            }
        }

        if (iterations_field->editing) {
            if (fractus_ui_numeric_field_handle_input(iterations_field, ui, fonts, &edit_accepted, &edit_cancelled) == FRACTUS_STATUS_OK) {
                if (edit_accepted) {
                    int32_t val;
                    if (fractus_ui_numeric_field_get_int(iterations_field, &val) == FRACTUS_STATUS_OK) {
                        config_draft->iterations = (int16_t)val;
                    }
                    (void)fractus_ui_numeric_field_init_int(iterations_field, (fractus_rect_i32){324, 169, 73, 20}, (int32_t)config_draft->iterations, 16, 1024);
                } else if (edit_cancelled) {
                    (void)fractus_ui_numeric_field_init_int(iterations_field, (fractus_rect_i32){324, 169, 73, 20}, (int32_t)config_draft->iterations, 16, 1024);
                }
            }
            skip_mouse_input = 1;
        } else if (escape_radius_field->editing) {
            if (fractus_ui_numeric_field_handle_input(escape_radius_field, ui, fonts, &edit_accepted, &edit_cancelled) == FRACTUS_STATUS_OK) {
                if (edit_accepted) {
                    int32_t val;
                    if (fractus_ui_numeric_field_get_int(escape_radius_field, &val) == FRACTUS_STATUS_OK) {
                        config_draft->escape_radius_squared = (int16_t)val;
                    }
                    (void)fractus_ui_numeric_field_init_int(escape_radius_field, (fractus_rect_i32){324, 195, 73, 20}, (int32_t)config_draft->escape_radius_squared, 4, 1000);
                } else if (edit_cancelled) {
                    (void)fractus_ui_numeric_field_init_int(escape_radius_field, (fractus_rect_i32){324, 195, 73, 20}, (int32_t)config_draft->escape_radius_squared, 4, 1000);
                }
            }
            skip_mouse_input = 1;
        } else if (biomorph_radius_field->editing) {
            if (fractus_ui_numeric_field_handle_input(biomorph_radius_field, ui, fonts, &edit_accepted, &edit_cancelled) == FRACTUS_STATUS_OK) {
                if (edit_accepted) {
                    int32_t val;
                    if (fractus_ui_numeric_field_get_int(biomorph_radius_field, &val) == FRACTUS_STATUS_OK) {
                        config_draft->biomorph_escape_radius_squared = (int16_t)val;
                    }
                    (void)fractus_ui_numeric_field_init_int(biomorph_radius_field, (fractus_rect_i32){324, 245, 73, 20}, (int32_t)config_draft->biomorph_escape_radius_squared, 4, 1000);
                } else if (edit_cancelled) {
                    (void)fractus_ui_numeric_field_init_int(biomorph_radius_field, (fractus_rect_i32){324, 245, 73, 20}, (int32_t)config_draft->biomorph_escape_radius_squared, 4, 1000);
                }
            }
            skip_mouse_input = 1;
        } else if (biomorph_cutoff_field->editing) {
            if (fractus_ui_numeric_field_handle_input(biomorph_cutoff_field, ui, fonts, &edit_accepted, &edit_cancelled) == FRACTUS_STATUS_OK) {
                if (edit_accepted) {
                    int32_t val;
                    if (fractus_ui_numeric_field_get_int(biomorph_cutoff_field, &val) == FRACTUS_STATUS_OK) {
                        config_draft->biomorph_cutoff = (int16_t)val;
                    }
                    (void)fractus_ui_numeric_field_init_int(biomorph_cutoff_field, (fractus_rect_i32){324, 271, 73, 20}, (int32_t)config_draft->biomorph_cutoff, 1, 100);
                } else if (edit_cancelled) {
                    (void)fractus_ui_numeric_field_init_int(biomorph_cutoff_field, (fractus_rect_i32){324, 271, 73, 20}, (int32_t)config_draft->biomorph_cutoff, 1, 100);
                }
            }
            skip_mouse_input = 1;
        }
    }

    /* 4. Raton y acciones de botones (logica de guardado unificada) */
    if (!skip_mouse_input && fractus_ui_menu(ui, dialog_options, dialog_entry_count, &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == FRACTUS_APP_FRACTALS_CONFIG_CANCEL) {
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (selected_menu == FRACTUS_APP_FRACTALS_CONFIG_SAVE) {
            *legacy_config = *config_draft;
            fractus_app_capture_palette_to_config(framebuffer, legacy_config);
            fractus_app_apply_legacy_numeric_config(legacy_config, mandelbrot_params, julia_params, biomorph_params);
            mandelbrot_dem_params->max_iterations = mandelbrot_params->max_iterations;
            mandelbrot_dem_params->escape_radius_squared = mandelbrot_params->escape_radius_squared;
            julia_dem_params->max_iterations = julia_params->max_iterations;
            julia_dem_params->escape_radius_squared = julia_params->escape_radius_squared;
            if (cfg_path != NULL && cfg_path[0] != '\0' &&
                fractus_app_save_legacy_config(cfg_path, framebuffer, legacy_config) != FRACTUS_STATUS_OK) {
                fractus_app_log("runtime: saving fractus.cfg failed");
            }
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (selected_menu == FRACTUS_APP_FRACTALS_CONFIG_ITER_DEC) {
            config_draft->iterations = (int16_t)fractus_app_clamp_i32((int32_t)config_draft->iterations - 4, 16, 1024);
            (void)fractus_ui_numeric_field_init_int(iterations_field, (fractus_rect_i32){324, 169, 73, 20}, (int32_t)config_draft->iterations, 16, 1024);
        } else if (selected_menu == FRACTUS_APP_FRACTALS_CONFIG_ITER_INC) {
            config_draft->iterations = (int16_t)fractus_app_clamp_i32((int32_t)config_draft->iterations + 4, 16, 1024);
            (void)fractus_ui_numeric_field_init_int(iterations_field, (fractus_rect_i32){324, 169, 73, 20}, (int32_t)config_draft->iterations, 16, 1024);
        } else if (selected_menu == FRACTUS_APP_FRACTALS_CONFIG_ESCAPE_DEC) {
            config_draft->escape_radius_squared = (int16_t)fractus_app_clamp_i32((int32_t)config_draft->escape_radius_squared - 2, 4, 1000);
            (void)fractus_ui_numeric_field_init_int(escape_radius_field, (fractus_rect_i32){324, 195, 73, 20}, (int32_t)config_draft->escape_radius_squared, 4, 1000);
        } else if (selected_menu == FRACTUS_APP_FRACTALS_CONFIG_ESCAPE_INC) {
            config_draft->escape_radius_squared = (int16_t)fractus_app_clamp_i32((int32_t)config_draft->escape_radius_squared + 2, 4, 1000);
            (void)fractus_ui_numeric_field_init_int(escape_radius_field, (fractus_rect_i32){324, 195, 73, 20}, (int32_t)config_draft->escape_radius_squared, 4, 1000);
        } else if (selected_menu == FRACTUS_APP_FRACTALS_CONFIG_BIO_RADIUS_DEC) {
            config_draft->biomorph_escape_radius_squared = (int16_t)fractus_app_clamp_i32((int32_t)config_draft->biomorph_escape_radius_squared - 2, 4, 1000);
            (void)fractus_ui_numeric_field_init_int(biomorph_radius_field, (fractus_rect_i32){324, 245, 73, 20}, (int32_t)config_draft->biomorph_escape_radius_squared, 4, 1000);
        } else if (selected_menu == FRACTUS_APP_FRACTALS_CONFIG_BIO_RADIUS_INC) {
            config_draft->biomorph_escape_radius_squared = (int16_t)fractus_app_clamp_i32((int32_t)config_draft->biomorph_escape_radius_squared + 2, 4, 1000);
            (void)fractus_ui_numeric_field_init_int(biomorph_radius_field, (fractus_rect_i32){324, 245, 73, 20}, (int32_t)config_draft->biomorph_escape_radius_squared, 4, 1000);
        } else if (selected_menu == FRACTUS_APP_FRACTALS_CONFIG_BIO_CUTOFF_DEC) {
            config_draft->biomorph_cutoff = (int16_t)fractus_app_clamp_i32((int32_t)config_draft->biomorph_cutoff - 1, 1, 100);
            (void)fractus_ui_numeric_field_init_int(biomorph_cutoff_field, (fractus_rect_i32){324, 271, 73, 20}, (int32_t)config_draft->biomorph_cutoff, 1, 100);
        } else if (selected_menu == FRACTUS_APP_FRACTALS_CONFIG_BIO_CUTOFF_INC) {
            config_draft->biomorph_cutoff = (int16_t)fractus_app_clamp_i32((int32_t)config_draft->biomorph_cutoff + 1, 1, 100);
            (void)fractus_ui_numeric_field_init_int(biomorph_cutoff_field, (fractus_rect_i32){324, 271, 73, 20}, (int32_t)config_draft->biomorph_cutoff, 1, 100);
        }
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_app_run_video_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_legacy_config *legacy_config,
    fractus_legacy_config *config_draft,
    fractus_mandelbrot_params *mandelbrot_params,
    fractus_julia_params *julia_params,
    fractus_biomorph_params *biomorph_params,
    const char *cfg_path,
    fractus_app_view *view)
{
    const fractus_app_menu_entry dialog_entries[] = {
        {FRACTUS_APP_RECT(175, 152, 460, 167), 8u, 0u, "Ventana 640x480"},
        {FRACTUS_APP_RECT(175, 172, 460, 187), 8u, 0u, "Ventana 800x600"},
        {FRACTUS_APP_RECT(175, 192, 460, 207), 8u, 0u, "Ventana 1024x768"},
        {FRACTUS_APP_RECT(175, 212, 460, 227), 8u, 0u, "Ventana 1280x960 (Quad VGA)"},
        {FRACTUS_APP_RECT(175, 232, 460, 247), 8u, 0u, "Pantalla completa 1280x720 (HD)"},
        {FRACTUS_APP_RECT(175, 252, 460, 267), 8u, 0u, "Pantalla completa 1920x1080 (Full HD)"},
        {FRACTUS_APP_RECT(175, 272, 460, 287), 8u, 0u, "Pantalla completa 2560x1440 (Quad HD)"},
        {FRACTUS_APP_RECT(175, 292, 460, 307), 8u, 0u, "Pantalla completa 3840x2160 (4K / Quad Full HD)"},
        {FRACTUS_APP_RECT(210, 341, 310, 361), 8u, 0u, "Guardar"},
        {FRACTUS_APP_RECT(330, 341, 430, 361), 0u, 15u, "Cancelar"}
    };
    fractus_ui_menu_option dialog_options[FRACTUS_APP_ARRAY_COUNT(dialog_entries)];
    fractus_ui_radio_option video_options[FRACTUS_DRAWING_VIDEO_MODE_COUNT];
    size_t i;
    size_t dialog_entry_count = FRACTUS_APP_ARRAY_COUNT(dialog_entries);
    const int save_button = (int)FRACTUS_DRAWING_VIDEO_MODE_COUNT;
    const int cancel_button = (int)FRACTUS_DRAWING_VIDEO_MODE_COUNT + 1;
    int active_index;
    int selected_menu = -1;
    int cancelled = 0;

    if (framebuffer == NULL || fonts == NULL || ui == NULL ||
        legacy_config == NULL || config_draft == NULL ||
        mandelbrot_params == NULL || julia_params == NULL || biomorph_params == NULL ||
        view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    fractus_app_build_options_from_entries(dialog_options, dialog_entries, dialog_entry_count);
    for (i = 0u; i < FRACTUS_DRAWING_VIDEO_MODE_COUNT; ++i) {
        video_options[i].bounds = dialog_entries[i].bounds;
        video_options[i].label = dialog_entries[i].label;
    }
    active_index = fractus_ui_active_menu_index(ui, dialog_options, dialog_entry_count);

    /* 1. Contenedor exterior. */
    if (fractus_app_render_main_menu(framebuffer, fonts, -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_window(framebuffer, 165, 107, 474, 371) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, 320, 111, 15u, "Resolucion de los dibujos") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 2. Textos y controles. */
    if (fractus_ui_draw_group_box(framebuffer, fonts, 170, 139, 469, 315, 8u, 0u, "Seleccione la resolucion") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_radio_list(
            framebuffer,
            fonts,
            video_options,
            FRACTUS_DRAWING_VIDEO_MODE_COUNT,
            config_draft->drawing_video_mode,
            active_index) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_button(framebuffer, fonts, &dialog_entries[save_button], active_index == save_button) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_button(framebuffer, fonts, &dialog_entries[cancel_button], active_index == cancel_button) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    if (fractus_ui_draw_text_centered(
            framebuffer,
            fonts,
            FRACTUS_FONT_SMALL,
            320,
            323,
            0u,
            "La resolucion seleccionada para los dibujos se guarda en fractus.cfg.") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones de botones. */
    if (fractus_ui_menu(ui, dialog_options, dialog_entry_count, &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == cancel_button) {
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (selected_menu == save_button) {
            *legacy_config = *config_draft;
            fractus_app_capture_palette_to_config(framebuffer, legacy_config);
            fractus_app_apply_legacy_numeric_config(legacy_config, mandelbrot_params, julia_params, biomorph_params);
            if (cfg_path != NULL && cfg_path[0] != '\0' &&
                fractus_app_save_legacy_config(cfg_path, framebuffer, legacy_config) != FRACTUS_STATUS_OK) {
                fractus_app_log("runtime: saving fractus.cfg failed");
            }
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (selected_menu >= 0 && selected_menu < (int)FRACTUS_DRAWING_VIDEO_MODE_COUNT) {
            config_draft->drawing_video_mode = (uint8_t)selected_menu;
        }
    }

    return FRACTUS_STATUS_OK;
}
