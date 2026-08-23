#include "app/config.h"

#include <stdio.h>
#include <string.h>

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

    return FRACTUS_STATUS_OK;
}

void fractus_app_apply_legacy_numeric_config(
    const fractus_legacy_config *config,
    fractus_mandelbrot_params *mandelbrot_params,
    fractus_julia_params *julia_params,
    fractus_biomorph_params *biomorph_params,
    fractus_plasma_params *plasma_rectangular_params,
    fractus_plasma_circular_params *plasma_circular_params)
{
    if (config == NULL) {
        return;
    }

    if (mandelbrot_params != NULL) {
        mandelbrot_params->max_iterations = (config->iterations > 0)
            ? (uint32_t)config->iterations
            : mandelbrot_params->max_iterations;
        mandelbrot_params->escape_radius_squared = (config->escape_radius_squared > 0)
            ? (double)config->escape_radius_squared
            : mandelbrot_params->escape_radius_squared;
    }

    if (julia_params != NULL && mandelbrot_params != NULL) {
        julia_params->max_iterations = mandelbrot_params->max_iterations;
        julia_params->escape_radius_squared = mandelbrot_params->escape_radius_squared;
    }

    if (biomorph_params != NULL) {
        biomorph_params->max_iterations = (config->biomorph_iterations > 0)
            ? (uint32_t)config->biomorph_iterations
            : biomorph_params->max_iterations;
        biomorph_params->escape_radius_squared = (config->biomorph_escape_radius_squared > 0)
            ? (double)config->biomorph_escape_radius_squared
            : biomorph_params->escape_radius_squared;
        biomorph_params->cutoff = (config->biomorph_cutoff > 0)
            ? (double)config->biomorph_cutoff
            : biomorph_params->cutoff;
    }

    if (plasma_rectangular_params != NULL) {
        plasma_rectangular_params->seed = (config->plasma_rectangular_seed > 0u)
            ? config->plasma_rectangular_seed
            : plasma_rectangular_params->seed;
    }

    if (plasma_circular_params != NULL) {
        plasma_circular_params->seed = (config->plasma_circular_seed > 0u)
            ? config->plasma_circular_seed
            : plasma_circular_params->seed;
    }
}

void fractus_app_reset_fractal_parameters(
    fractus_mandelbrot_params *mandelbrot_params,
    fractus_mandelbrot_dem_params *mandelbrot_dem_params,
    fractus_julia_params *julia_params,
    fractus_julia_dem_params *julia_dem_params,
    fractus_biomorph_params *biomorph_params,
    fractus_plasma_params *plasma_rectangular_params,
    fractus_plasma_circular_params *plasma_circular_params,
    const fractus_legacy_config *legacy_config)
{
    if (mandelbrot_params == NULL || mandelbrot_dem_params == NULL ||
        julia_params == NULL || julia_dem_params == NULL ||
        biomorph_params == NULL || plasma_rectangular_params == NULL ||
        plasma_circular_params == NULL || legacy_config == NULL) {
        return;
    }

    *mandelbrot_params = (fractus_mandelbrot_params){
        -2.4,
        1.2,
        -1.2,
        1.2,
        240u,
        4.0,
        0u,
        16u,
        240u,
        FRACTUS_MANDELBROT_COLOR_ESCAPE
    };
    *mandelbrot_dem_params = (fractus_mandelbrot_dem_params){
        -2.4,
        1.2,
        -1.2,
        1.2,
        240u,
        1000.0,
        0u,
        16u,
        240u,
        FRACTUS_MANDELBROT_DEM_COLOR_BOUNDARY
    };
    *julia_params = (fractus_julia_params){
        -1.8,
        1.8,
        -1.2,
        1.2,
        -0.745,
        0.113,
        240u,
        4.0,
        0u,
        16u,
        240u,
        FRACTUS_JULIA_COLOR_ESCAPE
    };
    *julia_dem_params = (fractus_julia_dem_params){
        -1.8,
        1.8,
        -1.2,
        1.2,
        -0.745,
        0.113,
        240u,
        100.0,
        0u,
        16u,
        240u,
        FRACTUS_JULIA_DEM_COLOR_BOUNDARY
    };
    *biomorph_params = (fractus_biomorph_params){
        -2.0,
        2.0,
        -1.5,
        1.5,
        -0.6,
        0.55,
        (legacy_config->biomorph_iterations > 0) ? (uint32_t)legacy_config->biomorph_iterations : 15u,
        1000.0,
        1.0,
        0u,
        16u,
        240u,
        FRACTUS_BIOMORPH_EQ_Z2,
        FRACTUS_BIOMORPH_TRAP_RE_OR_IM
    };
    *plasma_rectangular_params = (fractus_plasma_params){
        (legacy_config->plasma_rectangular_seed > 0u) ? legacy_config->plasma_rectangular_seed : 1337u,
        5,
        16u,
        240u
    };
    *plasma_circular_params = (fractus_plasma_circular_params){
        (legacy_config->plasma_circular_seed > 0u) ? legacy_config->plasma_circular_seed : 7331u,
        320,
        90,
        16u,
        240u
    };

    fractus_app_apply_legacy_numeric_config(
        legacy_config,
        mandelbrot_params,
        julia_params,
        biomorph_params,
        plasma_rectangular_params,
        plasma_circular_params);

    mandelbrot_dem_params->max_iterations = mandelbrot_params->max_iterations;
    mandelbrot_dem_params->escape_radius_squared = (legacy_config->escape_radius_squared > 0)
        ? (double)legacy_config->escape_radius_squared
        : mandelbrot_dem_params->escape_radius_squared;
    julia_dem_params->max_iterations = julia_params->max_iterations;
    julia_dem_params->escape_radius_squared = (legacy_config->escape_radius_squared > 0)
        ? (double)legacy_config->escape_radius_squared
        : julia_dem_params->escape_radius_squared;
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
    fractus_plasma_params *plasma_rectangular_params,
    fractus_plasma_circular_params *plasma_circular_params,
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
        fractus_app_apply_legacy_numeric_config(
            legacy_config,
            mandelbrot_params,
            julia_params,
            biomorph_params,
            plasma_rectangular_params,
            plasma_circular_params);
        return fractus_app_apply_legacy_config(framebuffer, legacy_config);
    }

    fractus_app_log("startup: fractus.cfg not found");
    fractus_app_apply_legacy_numeric_config(
        legacy_config,
        mandelbrot_params,
        julia_params,
        biomorph_params,
        plasma_rectangular_params,
        plasma_circular_params);
    return fractus_app_apply_legacy_config(framebuffer, legacy_config);
}

static size_t fractus_app_build_fractals_config_entries(
    fractus_app_menu_entry *entries,
    size_t capacity)
{
    static const fractus_app_menu_entry controls[] = {
        {FRACTUS_APP_RECT(404, 115, 444, 135), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(449, 115, 489, 135), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(404, 141, 444, 161), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(449, 141, 489, 161), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(404, 189, 444, 209), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(449, 189, 489, 209), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(404, 215, 444, 235), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(449, 215, 489, 235), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(404, 241, 444, 261), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(449, 241, 489, 261), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(404, 289, 444, 309), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(449, 289, 489, 309), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(404, 315, 444, 335), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(449, 315, 489, 335), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(210, 373, 310, 393), 8u, 0u, "Guardar"},
        {FRACTUS_APP_RECT(330, 373, 430, 393), 0u, 15u, "Cancelar"}
    };

    return fractus_app_copy_control_entries(
        entries,
        capacity,
        controls,
        FRACTUS_APP_ARRAY_COUNT(controls));
}

static void fractus_app_init_fractals_default_config_fields(
    const fractus_legacy_config *config,
    fractus_ui_numeric_field *iterations_field,
    fractus_ui_numeric_field *escape_radius_field,
    fractus_ui_numeric_field *biomorph_iterations_field,
    fractus_ui_numeric_field *biomorph_radius_field,
    fractus_ui_numeric_field *biomorph_cutoff_field,
    fractus_ui_numeric_field *plasma_rect_seed_field,
    fractus_ui_numeric_field *plasma_circ_seed_field)
{
    if (iterations_field != NULL) {
        (void)fractus_ui_numeric_field_init_int(iterations_field, (fractus_rect_i32){324, 115, 73, 20}, (int32_t)config->iterations, 1, 1000);
    }
    if (escape_radius_field != NULL) {
        (void)fractus_ui_numeric_field_init_int(escape_radius_field, (fractus_rect_i32){324, 141, 73, 20}, (int32_t)config->escape_radius_squared, 4, 1000);
    }
    if (biomorph_iterations_field != NULL) {
        (void)fractus_ui_numeric_field_init_int(biomorph_iterations_field, (fractus_rect_i32){324, 189, 73, 20}, (int32_t)config->biomorph_iterations, 1, 1000);
    }
    if (biomorph_radius_field != NULL) {
        (void)fractus_ui_numeric_field_init_int(biomorph_radius_field, (fractus_rect_i32){324, 215, 73, 20}, (int32_t)config->biomorph_escape_radius_squared, 4, 1000);
    }
    if (biomorph_cutoff_field != NULL) {
        (void)fractus_ui_numeric_field_init_int(biomorph_cutoff_field, (fractus_rect_i32){324, 241, 73, 20}, (int32_t)config->biomorph_cutoff, 1, 100);
    }
    if (plasma_rect_seed_field != NULL) {
        (void)fractus_ui_numeric_field_init_int(plasma_rect_seed_field, (fractus_rect_i32){324, 289, 73, 20}, (int32_t)config->plasma_rectangular_seed, 1, 999999);
    }
    if (plasma_circ_seed_field != NULL) {
        (void)fractus_ui_numeric_field_init_int(plasma_circ_seed_field, (fractus_rect_i32){324, 315, 73, 20}, (int32_t)config->plasma_circular_seed, 1, 999999);
    }
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
    fractus_plasma_params *plasma_rectangular_params,
    fractus_plasma_circular_params *plasma_circular_params,
    const char *cfg_path,
    fractus_ui_numeric_field *iterations_field,
    fractus_ui_numeric_field *escape_radius_field,
    fractus_ui_numeric_field *biomorph_iterations_field,
    fractus_ui_numeric_field *biomorph_radius_field,
    fractus_ui_numeric_field *biomorph_cutoff_field,
    fractus_ui_numeric_field *plasma_rect_seed_field,
    fractus_ui_numeric_field *plasma_circ_seed_field,
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
        biomorph_params == NULL || plasma_rectangular_params == NULL ||
        plasma_circular_params == NULL || iterations_field == NULL ||
        escape_radius_field == NULL || biomorph_iterations_field == NULL ||
        biomorph_radius_field == NULL || biomorph_cutoff_field == NULL ||
        plasma_rect_seed_field == NULL || plasma_circ_seed_field == NULL ||
        view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    dialog_entry_count = fractus_app_build_fractals_config_entries(dialog_entries, FRACTUS_APP_ARRAY_COUNT(dialog_entries));
    fractus_app_build_options_from_entries(dialog_options, dialog_entries, dialog_entry_count);

    /* 1. Contenedor exterior y paneles por tipo de conjunto fractal */
    if (fractus_app_render_main_menu(framebuffer, fonts, -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_window(framebuffer, 135, 75, 504, 403) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, 320, 79, 15u, "Parametros por defecto de conjuntos fractales") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_group_box(framebuffer, fonts, 140, 107, 499, 173, 8u, 0u, "Mandelbrot y Julia") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_group_box(framebuffer, fonts, 140, 181, 499, 273, 8u, 0u, "Biomorfos de Pickover") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_group_box(framebuffer, fonts, 140, 281, 499, 347, 8u, 0u, "Plasmas") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 2. Textos de etiquetas, campos numericos, tip y botones */
    if (fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 150, 120, 0u, "Iteraciones maximas (1-1000)") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, iterations_field) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 150, 146, 0u, "Radio de escape al cuadrado (4-1000)") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, escape_radius_field) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 150, 194, 0u, "Iteraciones maximas (1-1000)") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, biomorph_iterations_field) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 150, 220, 0u, "Radio de escape al cuadrado (4-1000)") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, biomorph_radius_field) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 150, 246, 0u, "Umbral de escape (1-100)") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, biomorph_cutoff_field) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 150, 294, 0u, "Semilla plasma rectangular (1-999999)") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, plasma_rect_seed_field) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 150, 320, 0u, "Semilla plasma circular (1-999999)") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, plasma_circ_seed_field) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, 320, 355, 0u, "Los valores por defecto de los parametros clave se guardan en fractus.cfg.") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_button_list(
            framebuffer,
            fonts,
            dialog_entries,
            dialog_entry_count,
            fractus_ui_active_menu_index(ui, dialog_options, dialog_entry_count)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Gestion de foco y edicion de campos editables */
    if (ui->release_pending && ui->release_event.buttons.left) {
        fractus_point_i32 click_pos = ui->release_event.position;
        fractus_ui_numeric_field *clicked_field = NULL;

        if (fractus_ui_point_in_rect(click_pos, iterations_field->bounds)) {
            clicked_field = iterations_field;
        } else if (fractus_ui_point_in_rect(click_pos, escape_radius_field->bounds)) {
            clicked_field = escape_radius_field;
        } else if (fractus_ui_point_in_rect(click_pos, biomorph_iterations_field->bounds)) {
            clicked_field = biomorph_iterations_field;
        } else if (fractus_ui_point_in_rect(click_pos, biomorph_radius_field->bounds)) {
            clicked_field = biomorph_radius_field;
        } else if (fractus_ui_point_in_rect(click_pos, biomorph_cutoff_field->bounds)) {
            clicked_field = biomorph_cutoff_field;
        } else if (fractus_ui_point_in_rect(click_pos, plasma_rect_seed_field->bounds)) {
            clicked_field = plasma_rect_seed_field;
        } else if (fractus_ui_point_in_rect(click_pos, plasma_circ_seed_field->bounds)) {
            clicked_field = plasma_circ_seed_field;
        }

        {
            int32_t val;
            if (iterations_field->editing && fractus_ui_numeric_field_get_int(iterations_field, &val) == FRACTUS_STATUS_OK) {
                config_draft->iterations = (int16_t)val;
            }
            if (escape_radius_field->editing && fractus_ui_numeric_field_get_int(escape_radius_field, &val) == FRACTUS_STATUS_OK) {
                config_draft->escape_radius_squared = (int16_t)val;
            }
            if (biomorph_iterations_field->editing && fractus_ui_numeric_field_get_int(biomorph_iterations_field, &val) == FRACTUS_STATUS_OK) {
                config_draft->biomorph_iterations = (int16_t)val;
            }
            if (biomorph_radius_field->editing && fractus_ui_numeric_field_get_int(biomorph_radius_field, &val) == FRACTUS_STATUS_OK) {
                config_draft->biomorph_escape_radius_squared = (int16_t)val;
            }
            if (biomorph_cutoff_field->editing && fractus_ui_numeric_field_get_int(biomorph_cutoff_field, &val) == FRACTUS_STATUS_OK) {
                config_draft->biomorph_cutoff = (int16_t)val;
            }
            if (plasma_rect_seed_field->editing && fractus_ui_numeric_field_get_int(plasma_rect_seed_field, &val) == FRACTUS_STATUS_OK) {
                config_draft->plasma_rectangular_seed = (uint32_t)val;
            }
            if (plasma_circ_seed_field->editing && fractus_ui_numeric_field_get_int(plasma_circ_seed_field, &val) == FRACTUS_STATUS_OK) {
                config_draft->plasma_circular_seed = (uint32_t)val;
            }
            fractus_app_init_fractals_default_config_fields(
                config_draft,
                iterations_field,
                escape_radius_field,
                biomorph_iterations_field,
                biomorph_radius_field,
                biomorph_cutoff_field,
                plasma_rect_seed_field,
                plasma_circ_seed_field);
            if (clicked_field != NULL && !clicked_field->editing) {
                (void)fractus_ui_numeric_field_begin_edit(clicked_field);
            }
        }
    }

    {
        int edit_accepted = 0;
        int edit_cancelled = 0;
        fractus_ui_numeric_field *active_field = NULL;

        if (iterations_field->editing) {
            active_field = iterations_field;
        } else if (escape_radius_field->editing) {
            active_field = escape_radius_field;
        } else if (biomorph_iterations_field->editing) {
            active_field = biomorph_iterations_field;
        } else if (biomorph_radius_field->editing) {
            active_field = biomorph_radius_field;
        } else if (biomorph_cutoff_field->editing) {
            active_field = biomorph_cutoff_field;
        } else if (plasma_rect_seed_field->editing) {
            active_field = plasma_rect_seed_field;
        } else if (plasma_circ_seed_field->editing) {
            active_field = plasma_circ_seed_field;
        }

        if (active_field != NULL) {
            if (fractus_ui_numeric_field_handle_input(active_field, ui, fonts, &edit_accepted, &edit_cancelled) == FRACTUS_STATUS_OK) {
                if (edit_accepted) {
                    int32_t val;
                    if (active_field == iterations_field && fractus_ui_numeric_field_get_int(active_field, &val) == FRACTUS_STATUS_OK) {
                        config_draft->iterations = (int16_t)val;
                    } else if (active_field == escape_radius_field && fractus_ui_numeric_field_get_int(active_field, &val) == FRACTUS_STATUS_OK) {
                        config_draft->escape_radius_squared = (int16_t)val;
                    } else if (active_field == biomorph_iterations_field && fractus_ui_numeric_field_get_int(active_field, &val) == FRACTUS_STATUS_OK) {
                        config_draft->biomorph_iterations = (int16_t)val;
                    } else if (active_field == biomorph_radius_field && fractus_ui_numeric_field_get_int(active_field, &val) == FRACTUS_STATUS_OK) {
                        config_draft->biomorph_escape_radius_squared = (int16_t)val;
                    } else if (active_field == biomorph_cutoff_field && fractus_ui_numeric_field_get_int(active_field, &val) == FRACTUS_STATUS_OK) {
                        config_draft->biomorph_cutoff = (int16_t)val;
                    } else if (active_field == plasma_rect_seed_field && fractus_ui_numeric_field_get_int(active_field, &val) == FRACTUS_STATUS_OK) {
                        config_draft->plasma_rectangular_seed = (uint32_t)val;
                    } else if (active_field == plasma_circ_seed_field && fractus_ui_numeric_field_get_int(active_field, &val) == FRACTUS_STATUS_OK) {
                        config_draft->plasma_circular_seed = (uint32_t)val;
                    }
                    fractus_app_init_fractals_default_config_fields(
                        config_draft,
                        iterations_field,
                        escape_radius_field,
                        biomorph_iterations_field,
                        biomorph_radius_field,
                        biomorph_cutoff_field,
                        plasma_rect_seed_field,
                        plasma_circ_seed_field);
                } else if (edit_cancelled) {
                    fractus_app_init_fractals_default_config_fields(
                        config_draft,
                        iterations_field,
                        escape_radius_field,
                        biomorph_iterations_field,
                        biomorph_radius_field,
                        biomorph_cutoff_field,
                        plasma_rect_seed_field,
                        plasma_circ_seed_field);
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
            fractus_app_apply_legacy_numeric_config(
                legacy_config,
                mandelbrot_params,
                julia_params,
                biomorph_params,
                plasma_rectangular_params,
                plasma_circular_params);
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
            config_draft->iterations = (int16_t)fractus_app_clamp_i32((int32_t)config_draft->iterations - 4, 1, 1000);
            fractus_app_init_fractals_default_config_fields(config_draft, iterations_field, escape_radius_field, biomorph_iterations_field, biomorph_radius_field, biomorph_cutoff_field, plasma_rect_seed_field, plasma_circ_seed_field);
        } else if (selected_menu == FRACTUS_APP_FRACTALS_CONFIG_ITER_INC) {
            config_draft->iterations = (int16_t)fractus_app_clamp_i32((int32_t)config_draft->iterations + 4, 1, 1000);
            fractus_app_init_fractals_default_config_fields(config_draft, iterations_field, escape_radius_field, biomorph_iterations_field, biomorph_radius_field, biomorph_cutoff_field, plasma_rect_seed_field, plasma_circ_seed_field);
        } else if (selected_menu == FRACTUS_APP_FRACTALS_CONFIG_ESCAPE_DEC) {
            config_draft->escape_radius_squared = (int16_t)fractus_app_clamp_i32((int32_t)config_draft->escape_radius_squared - 2, 4, 1000);
            fractus_app_init_fractals_default_config_fields(config_draft, iterations_field, escape_radius_field, biomorph_iterations_field, biomorph_radius_field, biomorph_cutoff_field, plasma_rect_seed_field, plasma_circ_seed_field);
        } else if (selected_menu == FRACTUS_APP_FRACTALS_CONFIG_ESCAPE_INC) {
            config_draft->escape_radius_squared = (int16_t)fractus_app_clamp_i32((int32_t)config_draft->escape_radius_squared + 2, 4, 1000);
            fractus_app_init_fractals_default_config_fields(config_draft, iterations_field, escape_radius_field, biomorph_iterations_field, biomorph_radius_field, biomorph_cutoff_field, plasma_rect_seed_field, plasma_circ_seed_field);
        } else if (selected_menu == FRACTUS_APP_FRACTALS_CONFIG_BIO_ITER_DEC) {
            config_draft->biomorph_iterations = (int16_t)fractus_app_clamp_i32((int32_t)config_draft->biomorph_iterations - 1, 1, 1000);
            fractus_app_init_fractals_default_config_fields(config_draft, iterations_field, escape_radius_field, biomorph_iterations_field, biomorph_radius_field, biomorph_cutoff_field, plasma_rect_seed_field, plasma_circ_seed_field);
        } else if (selected_menu == FRACTUS_APP_FRACTALS_CONFIG_BIO_ITER_INC) {
            config_draft->biomorph_iterations = (int16_t)fractus_app_clamp_i32((int32_t)config_draft->biomorph_iterations + 1, 1, 1000);
            fractus_app_init_fractals_default_config_fields(config_draft, iterations_field, escape_radius_field, biomorph_iterations_field, biomorph_radius_field, biomorph_cutoff_field, plasma_rect_seed_field, plasma_circ_seed_field);
        } else if (selected_menu == FRACTUS_APP_FRACTALS_CONFIG_BIO_RADIUS_DEC) {
            config_draft->biomorph_escape_radius_squared = (int16_t)fractus_app_clamp_i32((int32_t)config_draft->biomorph_escape_radius_squared - 2, 4, 1000);
            fractus_app_init_fractals_default_config_fields(config_draft, iterations_field, escape_radius_field, biomorph_iterations_field, biomorph_radius_field, biomorph_cutoff_field, plasma_rect_seed_field, plasma_circ_seed_field);
        } else if (selected_menu == FRACTUS_APP_FRACTALS_CONFIG_BIO_RADIUS_INC) {
            config_draft->biomorph_escape_radius_squared = (int16_t)fractus_app_clamp_i32((int32_t)config_draft->biomorph_escape_radius_squared + 2, 4, 1000);
            fractus_app_init_fractals_default_config_fields(config_draft, iterations_field, escape_radius_field, biomorph_iterations_field, biomorph_radius_field, biomorph_cutoff_field, plasma_rect_seed_field, plasma_circ_seed_field);
        } else if (selected_menu == FRACTUS_APP_FRACTALS_CONFIG_BIO_CUTOFF_DEC) {
            config_draft->biomorph_cutoff = (int16_t)fractus_app_clamp_i32((int32_t)config_draft->biomorph_cutoff - 1, 1, 100);
            fractus_app_init_fractals_default_config_fields(config_draft, iterations_field, escape_radius_field, biomorph_iterations_field, biomorph_radius_field, biomorph_cutoff_field, plasma_rect_seed_field, plasma_circ_seed_field);
        } else if (selected_menu == FRACTUS_APP_FRACTALS_CONFIG_BIO_CUTOFF_INC) {
            config_draft->biomorph_cutoff = (int16_t)fractus_app_clamp_i32((int32_t)config_draft->biomorph_cutoff + 1, 1, 100);
            fractus_app_init_fractals_default_config_fields(config_draft, iterations_field, escape_radius_field, biomorph_iterations_field, biomorph_radius_field, biomorph_cutoff_field, plasma_rect_seed_field, plasma_circ_seed_field);
        } else if (selected_menu == FRACTUS_APP_FRACTALS_CONFIG_PLASMA_RECT_SEED_DEC) {
            config_draft->plasma_rectangular_seed = (config_draft->plasma_rectangular_seed > 1u) ? (config_draft->plasma_rectangular_seed - 1u) : 1u;
            fractus_app_init_fractals_default_config_fields(config_draft, iterations_field, escape_radius_field, biomorph_iterations_field, biomorph_radius_field, biomorph_cutoff_field, plasma_rect_seed_field, plasma_circ_seed_field);
        } else if (selected_menu == FRACTUS_APP_FRACTALS_CONFIG_PLASMA_RECT_SEED_INC) {
            config_draft->plasma_rectangular_seed = (uint32_t)fractus_app_clamp_i32((int32_t)config_draft->plasma_rectangular_seed + 1, 1, 999999);
            fractus_app_init_fractals_default_config_fields(config_draft, iterations_field, escape_radius_field, biomorph_iterations_field, biomorph_radius_field, biomorph_cutoff_field, plasma_rect_seed_field, plasma_circ_seed_field);
        } else if (selected_menu == FRACTUS_APP_FRACTALS_CONFIG_PLASMA_CIRC_SEED_DEC) {
            config_draft->plasma_circular_seed = (config_draft->plasma_circular_seed > 1u) ? (config_draft->plasma_circular_seed - 1u) : 1u;
            fractus_app_init_fractals_default_config_fields(config_draft, iterations_field, escape_radius_field, biomorph_iterations_field, biomorph_radius_field, biomorph_cutoff_field, plasma_rect_seed_field, plasma_circ_seed_field);
        } else if (selected_menu == FRACTUS_APP_FRACTALS_CONFIG_PLASMA_CIRC_SEED_INC) {
            config_draft->plasma_circular_seed = (uint32_t)fractus_app_clamp_i32((int32_t)config_draft->plasma_circular_seed + 1, 1, 999999);
            fractus_app_init_fractals_default_config_fields(config_draft, iterations_field, escape_radius_field, biomorph_iterations_field, biomorph_radius_field, biomorph_cutoff_field, plasma_rect_seed_field, plasma_circ_seed_field);
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
        mandelbrot_params == NULL || julia_params == NULL ||
        biomorph_params == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    fractus_app_build_options_from_entries(dialog_options, dialog_entries, dialog_entry_count);
    active_index = fractus_ui_active_menu_index(ui, dialog_options, dialog_entry_count);

    for (i = 0u; i < FRACTUS_DRAWING_VIDEO_MODE_COUNT; ++i) {
        video_options[i].bounds = dialog_entries[i].bounds;
        video_options[i].label = dialog_entries[i].label;
    }

    /* 1. Contenedor exterior */
    if (fractus_app_render_main_menu(framebuffer, fonts, -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_window(framebuffer, 160, 115, 479, 371) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, 320, 119, 15u, "Resolucion de dibujo") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_group_box(framebuffer, fonts, 165, 142, 474, 317, 8u, 0u, "Modo de video") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 2. Textos y controles */
    if (fractus_ui_draw_radio_list(
            framebuffer,
            fonts,
            video_options,
            FRACTUS_DRAWING_VIDEO_MODE_COUNT,
            (int)config_draft->drawing_video_mode,
            (active_index >= 0 && active_index < (int)FRACTUS_DRAWING_VIDEO_MODE_COUNT) ? active_index : -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, 320, 323, 0u, "La resolucion elegida se guardara en fractus.cfg.") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_button_list(
            framebuffer,
            fonts,
            &dialog_entries[save_button],
            2u,
            (active_index >= save_button) ? active_index - save_button : -1) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana */

    /* 4. Raton y acciones de botones (logica de guardado unificada) */
    if (fractus_ui_menu(ui, dialog_options, dialog_entry_count, &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == cancel_button) {
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (selected_menu == save_button) {
            *legacy_config = *config_draft;
            fractus_app_capture_palette_to_config(framebuffer, legacy_config);
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
