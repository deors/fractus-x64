#include "app/app.h"
#include "app/commons.h"
#include "app/config.h"
#include "app/files.h"
#include "app/fractal.h"

#include <stdio.h>
#include <string.h>

static const fractus_app_menu_entry fractus_app_main_menu_controls[] = {
    {FRACTUS_APP_RECT(10, 70, 312, 90), 11u, 0u, "Conjunto de Benoit B. Mandelbrot"},
    {FRACTUS_APP_RECT(10, 90, 312, 110), 11u, 0u, "Conjuntos de Gaston Julia"},
    {FRACTUS_APP_RECT(10, 110, 312, 130), 11u, 0u, "Biomorfos de Clifford A. Pickover"},
    {FRACTUS_APP_RECT(10, 130, 312, 150), 11u, 0u, "Fractales por el método de plasma"},
    {FRACTUS_APP_RECT(10, 150, 312, 170), 11u, 0u, "Atractores dinámicos"},
    {FRACTUS_APP_RECT(10, 170, 312, 190), 11u, 0u, "Curvas fractales sencillas"},
    {FRACTUS_APP_RECT(10, 190, 312, 210), 11u, 0u, "Otros conjuntos fractales"},
    {FRACTUS_APP_RECT(10, 210, 312, 230), 11u, 0u, "Modelos fractales naturales"},
    {FRACTUS_APP_RECT(10, 230, 312, 250), 11u, 0u, "Lenguajes 0-L"},
    {FRACTUS_APP_RECT(10, 250, 312, 270), 11u, 0u, "Sistemas de función iterada (IFS)"},
    {FRACTUS_APP_RECT(327, 70, 629, 90), 3u, 15u, "Cargar un dibujo"},
    {FRACTUS_APP_RECT(327, 90, 629, 110), 3u, 15u, "Grabar automáticamente el dibujo siguiente"},
    {FRACTUS_APP_RECT(327, 130, 629, 150), 3u, 15u, "Cargar una paleta"},
    {FRACTUS_APP_RECT(327, 150, 629, 170), 3u, 15u, "Grabar la paleta actual"},
    {FRACTUS_APP_RECT(327, 170, 629, 190), 3u, 15u, "Cargar la paleta de un dibujo"},
    {FRACTUS_APP_RECT(327, 190, 629, 210), 3u, 15u, "Cambiar la paleta de un dibujo"},
    {FRACTUS_APP_RECT(327, 210, 629, 230), 3u, 15u, "Mostrar la paleta actual"},
    {FRACTUS_APP_RECT(327, 230, 629, 250), 3u, 15u, "Modificar un color de la paleta"},
    {FRACTUS_APP_RECT(327, 250, 629, 270), 3u, 15u, "Copiar un color de la paleta"},
    {FRACTUS_APP_RECT(327, 270, 629, 290), 3u, 15u, "Crear un gradiente"},
    {FRACTUS_APP_RECT(327, 290, 629, 310), 3u, 15u, "Restaurar la paleta por defecto"},
    {FRACTUS_APP_RECT(327, 334, 629, 354), 5u, 15u, "Resolución de los dibujos"},
    {FRACTUS_APP_RECT(327, 354, 629, 374), 5u, 15u, "Parámetros por defecto de conjuntos fractales"},
    {FRACTUS_APP_RECT(327, 374, 629, 394), 5u, 15u, "Restaurar parámetros de los conjuntos"},
    {FRACTUS_APP_RECT(10, 448, 130, 468), 8u, 0u, "Ayuda"},
    {FRACTUS_APP_RECT(509, 448, 629, 468), 8u, 0u, "Acerca de..."},
    {FRACTUS_APP_RECT(260, 448, 380, 468), 0u, 15u, "Salir del programa"}
};

static fractus_status fractus_app_draw_main_menu_frame(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts)
{
    if (framebuffer == NULL || fonts == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_framebuffer_clear(framebuffer, 8u) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_window(framebuffer, 0, 0, 639, 479) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_frame(framebuffer, 4, 4, 635, 52) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_FRANCE, 320, 5, 0u, "Fractus-x64") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, 320, 30, 0u, "Dibujo de conjuntos fractales") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_group_box(framebuffer, fonts, 5, 60, 318, 275, 8u, 0u, "Tipos de conjuntos") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_group_box(framebuffer, fonts, 322, 60, 635, 316, 8u, 0u, "Gráficos y colores") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_group_box(framebuffer, fonts, 322, 324, 635, 400, 8u, 0u, "Configuración por defecto") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_app_draw_main_menu_controls(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const fractus_app_menu_entry *controls,
    size_t control_count,
    int active_index)
{
    size_t i;

    if (framebuffer == NULL || fonts == NULL || controls == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    for (i = 0u; i < control_count; ++i) {
        if (fractus_ui_draw_button(framebuffer, fonts, &controls[i], active_index == (int)i) != FRACTUS_STATUS_OK) {
            return FRACTUS_STATUS_ERROR;
        }
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_app_render_main_menu(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    int active_index)
{
    if (fractus_app_draw_main_menu_frame(framebuffer, fonts) != FRACTUS_STATUS_OK ||
        fractus_app_draw_main_menu_controls(
            framebuffer,
            fonts,
            fractus_app_main_menu_controls,
            FRACTUS_APP_ARRAY_COUNT(fractus_app_main_menu_controls),
            active_index) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_app_run_main_menu_view(
    const fractus_platform_context *platform,
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_app_view *view,
    int *running,
    fractus_legacy_config *legacy_config,
    fractus_legacy_config *config_draft,
    fractus_mandelbrot_params *mandelbrot_params,
    fractus_mandelbrot_dem_params *mandelbrot_dem_params,
    fractus_julia_params *julia_params,
    fractus_julia_dem_params *julia_dem_params,
    fractus_biomorph_params *biomorph_params,
    fractus_biomorph_params *biomorph_pending,
    fractus_plasma_params *plasma_rectangular_params,
    fractus_plasma_circular_params *plasma_circular_params,
    fractus_lorenz_params *lorenz_params,
    fractus_app_attractor_method *attractor_selected_method,
    fractus_app_biomorph_fields *biomorph_fields,
    fractus_ui_numeric_field *iterations_field,
    fractus_ui_numeric_field *escape_radius_field,
    fractus_ui_numeric_field *biomorph_iterations_field,
    fractus_ui_numeric_field *biomorph_radius_field,
    fractus_ui_numeric_field *biomorph_cutoff_field,
    fractus_ui_numeric_field *plasma_rect_seed_field,
    fractus_ui_numeric_field *plasma_circ_seed_field,
    fractus_app_graphic_file *graphic_files,
    size_t *graphic_file_count,
    size_t *graphic_file_page,
    fractus_app_palette_file *palette_files,
    size_t *palette_file_count,
    size_t *palette_file_page,
    char *runtime_error_message,
    size_t runtime_error_message_size,
    char *cfg_path,
    int *save_next_graphic)
{
    fractus_ui_menu_option menu_options[FRACTUS_APP_ARRAY_COUNT(fractus_app_main_menu_controls)];
    int selected_menu = -1;
    int cancelled = 0;
    size_t i;

    if (platform == NULL
        || framebuffer == NULL
        || fonts == NULL
        || ui == NULL
        || view == NULL
        || running == NULL
        || legacy_config == NULL
        || config_draft == NULL
        || mandelbrot_params == NULL
        || mandelbrot_dem_params == NULL
        || julia_params == NULL
        || julia_dem_params == NULL
        || biomorph_params == NULL
        || biomorph_pending == NULL
        || plasma_rectangular_params == NULL
        || plasma_circular_params == NULL
        || lorenz_params == NULL
        || attractor_selected_method == NULL
        || biomorph_fields == NULL
        || iterations_field == NULL
        || escape_radius_field == NULL
        || biomorph_iterations_field == NULL
        || biomorph_radius_field == NULL
        || biomorph_cutoff_field == NULL
        || plasma_rect_seed_field == NULL
        || plasma_circ_seed_field == NULL
        || graphic_files == NULL
        || graphic_file_count == NULL
        || graphic_file_page == NULL
        || palette_files == NULL
        || palette_file_count == NULL
        || palette_file_page == NULL
        || runtime_error_message == NULL
        || cfg_path == NULL
        || save_next_graphic == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    for (i = 0u; i < FRACTUS_APP_ARRAY_COUNT(fractus_app_main_menu_controls); ++i) {
        menu_options[i].bounds = fractus_app_main_menu_controls[i].bounds;
    }

    /* 1. Contenedor exterior. */
    if (fractus_app_draw_main_menu_frame(framebuffer, fonts) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 2. Textos y controles. */
    if (fractus_app_draw_main_menu_controls(
            framebuffer,
            fonts,
            fractus_app_main_menu_controls,
            FRACTUS_APP_ARRAY_COUNT(fractus_app_main_menu_controls),
            fractus_ui_active_menu_index(ui, menu_options, FRACTUS_APP_ARRAY_COUNT(menu_options))) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones de botones. */
    if (fractus_ui_menu(ui, menu_options, FRACTUS_APP_ARRAY_COUNT(menu_options), &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == FRACTUS_APP_MENU_EXIT_INDEX) {
            *running = 0;
        } else if (selected_menu == 0) {
            *view = FRACTUS_APP_VIEW_MANDELBROT_MENU;
        } else if (selected_menu == 1) {
            *view = FRACTUS_APP_VIEW_JULIA_MENU;
        } else if (selected_menu == 2) {
            *biomorph_pending = *biomorph_params;
            fractus_app_init_biomorph_fields(
                biomorph_fields,
                biomorph_pending->xmin,
                biomorph_pending->xmax,
                biomorph_pending->ymin,
                biomorph_pending->ymax,
                biomorph_pending->constant_real,
                biomorph_pending->constant_imag,
                biomorph_pending->max_iterations,
                biomorph_pending->escape_radius_squared,
                biomorph_pending->cutoff);
            *view = FRACTUS_APP_VIEW_BIOMORPH_CONFIG;
        } else if (selected_menu == 3) {
            *view = FRACTUS_APP_VIEW_PLASMA_MENU;
        } else if (selected_menu == 4) {
            *attractor_selected_method = FRACTUS_APP_ATTRACTOR_METHOD_NONE;
            *view = FRACTUS_APP_VIEW_ATTRACTORS_MENU;
        } else if (selected_menu == FRACTUS_APP_MENU_VIDEO_INDEX) {
            *config_draft = *legacy_config;
            *view = FRACTUS_APP_VIEW_VIDEO_CONFIG;
        } else if (selected_menu == FRACTUS_APP_MENU_FRACTALS_CONFIG_INDEX) {
            *config_draft = *legacy_config;
            fractus_app_init_fractals_default_config_fields(
                config_draft,
                iterations_field,
                escape_radius_field,
                biomorph_iterations_field,
                biomorph_radius_field,
                biomorph_cutoff_field,
                plasma_rect_seed_field,
                plasma_circ_seed_field);
            *view = FRACTUS_APP_VIEW_FRACTALS_DEFAULT_CONFIG;
        } else if (selected_menu == FRACTUS_APP_MENU_LOAD_GRAPHIC_INDEX) {
            *graphic_file_count = fractus_app_list_graphic_files(platform, graphic_files, FRACTUS_APP_GRAPHIC_FILE_CAPACITY);
            *graphic_file_page = 0u;
            *view = FRACTUS_APP_VIEW_LOAD_GRAPHIC;
        } else if (selected_menu == FRACTUS_APP_MENU_SAVE_NEXT_GRAPHIC_INDEX) {
            *save_next_graphic = 1;
            fractus_app_log("runtime: next graphic save armed");
        } else if (selected_menu == FRACTUS_APP_MENU_LOAD_PALETTE_INDEX) {
            *palette_file_count = fractus_app_list_palette_files(platform, palette_files, FRACTUS_APP_PALETTE_FILE_CAPACITY);
            *palette_file_page = 0u;
            *view = FRACTUS_APP_VIEW_LOAD_PALETTE;
        } else if (selected_menu == FRACTUS_APP_MENU_SAVE_PALETTE_INDEX) {
            fractus_status save_status = fractus_app_save_current_palette_file(platform, framebuffer, legacy_config, cfg_path);
            if (save_status != FRACTUS_STATUS_OK) {
                fractus_app_log("runtime: saving current palette failed");
                if (save_status == FRACTUS_STATUS_UNSUPPORTED) {
                    (void)fractus_app_set_message(
                        runtime_error_message,
                        runtime_error_message_size,
                        "No hay nombres libres de fractus0001.drsp a fractus9999.drsp.");
                    *view = FRACTUS_APP_VIEW_ERROR;
                }
            }
        } else if (selected_menu == FRACTUS_APP_MENU_LOAD_GRAPHIC_PALETTE_INDEX) {
            *graphic_file_count = fractus_app_list_graphic_files(platform, graphic_files, FRACTUS_APP_GRAPHIC_FILE_CAPACITY);
            *graphic_file_page = 0u;
            *view = FRACTUS_APP_VIEW_LOAD_GRAPHIC_PALETTE;
        } else if (selected_menu == FRACTUS_APP_MENU_CHANGE_GRAPHIC_PALETTE_INDEX) {
            *graphic_file_count = fractus_app_list_graphic_files(platform, graphic_files, FRACTUS_APP_GRAPHIC_FILE_CAPACITY);
            *graphic_file_page = 0u;
            *view = FRACTUS_APP_VIEW_CHANGE_GRAPHIC_PALETTE_GRAPHIC;
        } else if (selected_menu == FRACTUS_APP_MENU_PALETTE_INDEX) {
            *view = FRACTUS_APP_VIEW_PALETTE;
        } else if (selected_menu == FRACTUS_APP_MENU_EDIT_PALETTE_COLOR_INDEX) {
            *view = FRACTUS_APP_VIEW_PALETTE_EDIT_SELECT;
        } else if (selected_menu == FRACTUS_APP_MENU_COPY_PALETTE_COLOR_INDEX) {
            *view = FRACTUS_APP_VIEW_PALETTE_COPY_SOURCE;
        } else if (selected_menu == FRACTUS_APP_MENU_GRADIENT_PALETTE_INDEX) {
            *view = FRACTUS_APP_VIEW_PALETTE_GRADIENT_FIRST;
        } else if (selected_menu == FRACTUS_APP_MENU_RESTORE_PALETTE_INDEX) {
            if (fractus_app_restore_default_palette(platform, framebuffer, legacy_config, cfg_path) != FRACTUS_STATUS_OK) {
                fractus_app_log("runtime: restoring default palette failed");
            }
        } else if (selected_menu == FRACTUS_APP_MENU_RESTORE_FRACTALS_CONFIG_INDEX) {
            fractus_app_reset_fractal_parameters(
                mandelbrot_params,
                mandelbrot_dem_params,
                julia_params,
                julia_dem_params,
                biomorph_params,
                plasma_rectangular_params,
                plasma_circular_params,
                lorenz_params,
                legacy_config);
            fractus_app_log("runtime: default fractal parameters restored");
        } else if (selected_menu == FRACTUS_APP_MENU_ABOUT_INDEX) {
            *view = FRACTUS_APP_VIEW_ABOUT;
        }
    }

    return FRACTUS_STATUS_OK;
}

static const int32_t error_x0 = 150;
static const int32_t error_y0 = 187;

static fractus_status fractus_app_render_error(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const char *message,
    const fractus_app_menu_entry *entries,
    size_t entry_count,
    int active_index)
{
    const int32_t x0 = error_x0;
    const int32_t y0 = error_y0;

    if (framebuffer == NULL || fonts == NULL || message == NULL || entries == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_app_render_main_menu(framebuffer, fonts, -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_window(framebuffer, x0, y0, x0 + 339, y0 + 104) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, x0 + 170, y0 + 4, 15u, "Error") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, x0 + 170, y0 + 44, 0u, message) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_button_list(framebuffer, fonts, entries, entry_count, active_index) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_app_run_error_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    char *error_message,
    fractus_app_view *view)
{
    const int32_t x0 = error_x0;
    const int32_t y0 = error_y0;
    const fractus_app_menu_entry dialog_entries[] = {
        {FRACTUS_APP_RECT(x0 + 120, y0 + 74, x0 + 220, y0 + 94), 0u, 15u, "Aceptar"}
    };
    fractus_ui_menu_option dialog_options[FRACTUS_APP_ARRAY_COUNT(dialog_entries)];
    int selected_menu = -1;
    int cancelled = 0;

    if (framebuffer == NULL || fonts == NULL || ui == NULL || error_message == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    fractus_app_build_options_from_entries(dialog_options, dialog_entries, FRACTUS_APP_ARRAY_COUNT(dialog_entries));

    /* 1. Contenedor exterior. */
    /* 2. Textos y controles. */
    if (fractus_app_render_error(
            framebuffer,
            fonts,
            error_message,
            dialog_entries,
            FRACTUS_APP_ARRAY_COUNT(dialog_entries),
            fractus_ui_active_menu_index(ui, dialog_options, FRACTUS_APP_ARRAY_COUNT(dialog_options))) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones de botones. */
    if (fractus_ui_menu(ui, dialog_options, FRACTUS_APP_ARRAY_COUNT(dialog_options), &selected_menu, &cancelled)) {
        (void)selected_menu;
        (void)cancelled;
        error_message[0] = '\0';
        *view = FRACTUS_APP_VIEW_MAIN_MENU;
    }

    return FRACTUS_STATUS_OK;
}

static const int32_t about_x0 = 85;
static const int32_t about_y0 = 110;

static fractus_status fractus_app_render_about(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const fractus_app_menu_entry *entries,
    size_t entry_count,
    int active_index)
{
    const int32_t x0 = about_x0;
    const int32_t y0 = about_y0;

    if (framebuffer == NULL || fonts == NULL || entries == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_app_render_main_menu(framebuffer, fonts, -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_window(framebuffer, x0, y0, x0 + 469, y0 + 259) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, x0 + 235, y0 + 4, 15u, "Acerca de Fractus-x64") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_group_box(framebuffer, fonts, x0 + 5, y0 + 32, x0 + 464, y0 + 220, 8u, 0u, "Información") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_FRANCE, x0 + 235, y0 + 46, 0u, "Fractus-x64") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_COURIER, x0 + 235, y0 + 76, 0u, "Suite de generación de conjuntos fractales") != FRACTUS_STATUS_OK ||
        fractus_graphics_line(framebuffer, x0 + 15, y0 + 96, x0 + 454, y0 + 96, 7u) != FRACTUS_STATUS_OK ||
        fractus_graphics_line(framebuffer, x0 + 15, y0 + 97, x0 + 454, y0 + 97, 15u) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, x0 + 235, y0 + 106, 0u, "Port a x64 de la suite original para MS-DOS, creada") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, x0 + 235, y0 + 120, 0u, "en 1997 sobre Turbo C para 16-bits (CPUs 386/486 y VGA/SVGA).") != FRACTUS_STATUS_OK ||
        fractus_graphics_line(framebuffer, x0 + 15, y0 + 140, x0 + 454, y0 + 140, 7u) != FRACTUS_STATUS_OK ||
        fractus_graphics_line(framebuffer, x0 + 15, y0 + 141, x0 + 454, y0 + 141, 15u) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, x0 + 235, y0 + 150, 0u, "(c) 1997 - 2026") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_COURIER, x0 + 235, y0 + 174, 0u, "Autor: @deors (Jorge Hidalgo)") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, x0 + 235, y0 + 198, 0u, "https://github.com/deors/fractus-x64") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_button_list(framebuffer, fonts, entries, entry_count, active_index) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_app_run_about_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_app_view *view)
{
    const int32_t x0 = about_x0;
    const int32_t y0 = about_y0;
    const fractus_app_menu_entry dialog_entries[] = {
        {FRACTUS_APP_RECT(x0 + 185, y0 + 229, x0 + 285, y0 + 249), 0u, 15u, "Volver"}
    };
    fractus_ui_menu_option dialog_options[FRACTUS_APP_ARRAY_COUNT(dialog_entries)];
    int selected_menu = -1;
    int cancelled = 0;

    if (framebuffer == NULL || fonts == NULL || ui == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    fractus_app_build_options_from_entries(dialog_options, dialog_entries, FRACTUS_APP_ARRAY_COUNT(dialog_entries));

    /* 1. Contenedor exterior. */
    /* 2. Textos y controles. */
    if (fractus_app_render_about(
            framebuffer,
            fonts,
            dialog_entries,
            FRACTUS_APP_ARRAY_COUNT(dialog_entries),
            fractus_ui_active_menu_index(ui, dialog_options, FRACTUS_APP_ARRAY_COUNT(dialog_options))) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones de botones. */
    if (fractus_ui_menu(ui, dialog_options, FRACTUS_APP_ARRAY_COUNT(dialog_options), &selected_menu, &cancelled)) {
        (void)selected_menu;
        (void)cancelled;
        *view = FRACTUS_APP_VIEW_MAIN_MENU;
    }

    return FRACTUS_STATUS_OK;
}

static void fractus_app_note_render_time(
    const fractus_platform_context *platform,
    uint64_t started_ms,
    uint32_t *last_render_ms,
    int *has_render_time)
{
    uint64_t now_ms;
    uint64_t elapsed;

    if (platform == NULL || last_render_ms == NULL || has_render_time == NULL) {
        return;
    }

    now_ms = fractus_platform_get_ticks_ms(platform);
    elapsed = (now_ms >= started_ms) ? (now_ms - started_ms) : 0u;
    if (elapsed > 4294967295u) {
        elapsed = 4294967295u;
    }
    *last_render_ms = (uint32_t)elapsed;
    *has_render_time = 1;
}

static int fractus_app_plane_coordinate_digits(
    double span_x,
    double span_y,
    double plane_x,
    double plane_y,
    uint32_t width,
    uint32_t height)
{
    double step_x;
    double step_y;
    double step;
    double scale;
    double decades;
    int digits;

    if (width < 2u || height < 2u) {
        return 12;
    }

    step_x = span_x / (double)(width - 1u);
    step_y = span_y / (double)(height - 1u);
    step = step_x;
    if (step_y > 0.0 && (!(step > 0.0) || step_y < step)) {
        step = step_y;
    }
    if (!(step > 0.0)) {
        return 22;
    }

    scale = fabs(plane_x);
    if (fabs(plane_y) > scale) {
        scale = fabs(plane_y);
    }
    if (scale < 1.0) {
        scale = 1.0;
    }

    decades = log10(scale / step);
    if (!(decades > 11.0)) {
        return 12;
    }

    digits = (int)ceil(decades) + 1;
    if (digits < 12) {
        digits = 12;
    }
    if (digits > 22) {
        digits = 22;
    }
    return digits;
}

static void fractus_app_set_hud_plane(
    fractus_app_drawing_hud *hud,
    double xmin,
    double xmax,
    double ymin,
    double ymax,
    double reference_span,
    int on_image,
    uint32_t pixel_x,
    uint32_t pixel_y,
    uint32_t width,
    uint32_t height,
    const fractus_hp_window *precise)
{
    fractus_hp_window window = {0};
    double span_x;
    double span_y;

    if (hud == NULL) {
        return;
    }

    if (precise != NULL) {
        window = *precise;
    }
    fractus_hp_window_adopt_doubles(&window, xmin, xmax, ymin, ymax);
    if (window.valid && fractus_hp_window_spans_positive(&window)) {
        span_x = fractus_hp_to_double(fractus_hp_sub(window.xmax, window.xmin));
        span_y = fractus_hp_to_double(fractus_hp_sub(window.ymax, window.ymin));
    } else {
        span_x = xmax - xmin;
        span_y = ymax - ymin;
    }
    if (!(span_x > 0.0) || !(span_y > 0.0) || !(reference_span > 0.0)) {
        return;
    }

    hud->has_zoom = 1;
    hud->zoom = fractus_plane_zoom_level(reference_span, span_x);
    if (on_image) {
        fractus_hp real_part;
        fractus_hp imag_part;
        if (window.valid &&
            fractus_hp_plane_pixel(&window, width, height, pixel_x, pixel_y, &real_part, &imag_part)) {
            int digits;

            hud->plane_x = fractus_hp_to_double(real_part);
            hud->plane_y = fractus_hp_to_double(imag_part);
            hud->plane_x_text[0] = '\0';
            hud->plane_y_text[0] = '\0';
            hud->has_plane_point = 1;
            digits = fractus_app_plane_coordinate_digits(
                span_x, span_y, hud->plane_x, hud->plane_y, width, height);
            if (digits > 12 &&
                (fractus_hp_format(real_part, digits, hud->plane_x_text, sizeof(hud->plane_x_text)) != 0 ||
                 fractus_hp_format(imag_part, digits, hud->plane_y_text, sizeof(hud->plane_y_text)) != 0)) {
                hud->plane_x_text[0] = '\0';
                hud->plane_y_text[0] = '\0';
            }
        } else if (fractus_plane_point_from_pixel(
                       xmin, xmax, ymin, ymax, width, height, pixel_x, pixel_y, &hud->plane_x, &hud->plane_y)) {
            hud->has_plane_point = 1;
        }
    }
}

static void fractus_app_fill_drawing_hud(
    fractus_app_view view,
    const fractus_platform_context *platform,
    const fractus_framebuffer *drawing,
    const fractus_ui_context *ui,
    const fractus_mandelbrot_params *mandelbrot,
    const fractus_mandelbrot_dem_params *mandelbrot_dem,
    const fractus_julia_params *julia,
    const fractus_julia_dem_params *julia_dem,
    const fractus_biomorph_params *biomorph,
    const fractus_plasma_params *plasma_rectangular,
    const fractus_plasma_circular_params *plasma_circular,
    const fractus_lorenz_params *lorenz,
    const fractus_graphic_metadata *loaded_metadata,
    int has_loaded_metadata,
    uint32_t render_ms,
    int has_render_time,
    double fps,
    fractus_app_drawing_hud *hud)
{
    fractus_point_i32 pixel = {0, 0};
    int on_image = 0;
    uint8_t color_index = 0u;

    if (hud == NULL) {
        return;
    }

    (void)plasma_rectangular;
    (void)plasma_circular;
    (void)lorenz;

    memset(hud, 0, sizeof(*hud));
    hud->fps = fps;
    hud->render_ms = render_ms;
    hud->has_render_time = has_render_time;

    if (platform != NULL && drawing != NULL && ui != NULL &&
        fractus_app_map_drawing_window_point(platform, drawing, ui->pointer_position, &pixel)) {
        on_image = 1;
        if (fractus_graphics_get_pixel(drawing, pixel.x, pixel.y, &color_index) == FRACTUS_STATUS_OK) {
            hud->has_color = 1;
            hud->color_index = color_index;
            hud->color_r = drawing->palette.entries[color_index].r;
            hud->color_g = drawing->palette.entries[color_index].g;
            hud->color_b = drawing->palette.entries[color_index].b;
        }
    }

    if (drawing == NULL) {
        return;
    }

    switch (view) {
    case FRACTUS_APP_VIEW_MANDELBROT:
        if (mandelbrot != NULL) {
            fractus_app_set_hud_plane(
                hud, mandelbrot->xmin, mandelbrot->xmax, mandelbrot->ymin, mandelbrot->ymax,
                FRACTUS_PLANE_MANDELBROT_REFERENCE_SPAN,
                on_image, (uint32_t)pixel.x, (uint32_t)pixel.y, drawing->size.width, drawing->size.height,
                &mandelbrot->precise);
        }
        break;
    case FRACTUS_APP_VIEW_MANDELBROT_DEM:
        if (mandelbrot_dem != NULL) {
            fractus_app_set_hud_plane(
                hud, mandelbrot_dem->xmin, mandelbrot_dem->xmax, mandelbrot_dem->ymin, mandelbrot_dem->ymax,
                FRACTUS_PLANE_MANDELBROT_REFERENCE_SPAN,
                on_image, (uint32_t)pixel.x, (uint32_t)pixel.y, drawing->size.width, drawing->size.height,
                &mandelbrot_dem->precise);
        }
        break;
    case FRACTUS_APP_VIEW_JULIA:
        if (julia != NULL) {
            fractus_app_set_hud_plane(
                hud, julia->xmin, julia->xmax, julia->ymin, julia->ymax,
                FRACTUS_PLANE_JULIA_REFERENCE_SPAN,
                on_image, (uint32_t)pixel.x, (uint32_t)pixel.y, drawing->size.width, drawing->size.height,
                &julia->precise);
        }
        break;
    case FRACTUS_APP_VIEW_JULIA_DEM:
        if (julia_dem != NULL) {
            fractus_app_set_hud_plane(
                hud, julia_dem->xmin, julia_dem->xmax, julia_dem->ymin, julia_dem->ymax,
                FRACTUS_PLANE_JULIA_REFERENCE_SPAN,
                on_image, (uint32_t)pixel.x, (uint32_t)pixel.y, drawing->size.width, drawing->size.height,
                &julia_dem->precise);
        }
        break;
    case FRACTUS_APP_VIEW_BIOMORPH:
        if (biomorph != NULL) {
            fractus_app_set_hud_plane(
                hud, biomorph->xmin, biomorph->xmax, biomorph->ymin, biomorph->ymax,
                FRACTUS_PLANE_BIOMORPH_REFERENCE_SPAN,
                on_image, (uint32_t)pixel.x, (uint32_t)pixel.y, drawing->size.width, drawing->size.height,
                &biomorph->precise);
        }
        break;
    case FRACTUS_APP_VIEW_PLASMA_RECTANGULAR:
    case FRACTUS_APP_VIEW_PLASMA_CIRCULAR:
    case FRACTUS_APP_VIEW_LORENZ:
        break;
    case FRACTUS_APP_VIEW_GRAPHIC:
        if (has_loaded_metadata && loaded_metadata != NULL) {
            switch (loaded_metadata->kind) {
            case FRACTUS_GRAPHIC_KIND_MANDELBROT:
                fractus_app_set_hud_plane(
                    hud,
                    loaded_metadata->params.mandelbrot.xmin,
                    loaded_metadata->params.mandelbrot.xmax,
                    loaded_metadata->params.mandelbrot.ymin,
                    loaded_metadata->params.mandelbrot.ymax,
                    FRACTUS_PLANE_MANDELBROT_REFERENCE_SPAN,
                    on_image, (uint32_t)pixel.x, (uint32_t)pixel.y, drawing->size.width, drawing->size.height,
                    &loaded_metadata->params.mandelbrot.precise);
                break;
            case FRACTUS_GRAPHIC_KIND_MANDELBROT_DEM:
                fractus_app_set_hud_plane(
                    hud,
                    loaded_metadata->params.mandelbrot_dem.xmin,
                    loaded_metadata->params.mandelbrot_dem.xmax,
                    loaded_metadata->params.mandelbrot_dem.ymin,
                    loaded_metadata->params.mandelbrot_dem.ymax,
                    FRACTUS_PLANE_MANDELBROT_REFERENCE_SPAN,
                    on_image, (uint32_t)pixel.x, (uint32_t)pixel.y, drawing->size.width, drawing->size.height,
                    &loaded_metadata->params.mandelbrot_dem.precise);
                break;
            case FRACTUS_GRAPHIC_KIND_JULIA:
                fractus_app_set_hud_plane(
                    hud,
                    loaded_metadata->params.julia.xmin,
                    loaded_metadata->params.julia.xmax,
                    loaded_metadata->params.julia.ymin,
                    loaded_metadata->params.julia.ymax,
                    FRACTUS_PLANE_JULIA_REFERENCE_SPAN,
                    on_image, (uint32_t)pixel.x, (uint32_t)pixel.y, drawing->size.width, drawing->size.height,
                    &loaded_metadata->params.julia.precise);
                break;
            case FRACTUS_GRAPHIC_KIND_JULIA_DEM:
                fractus_app_set_hud_plane(
                    hud,
                    loaded_metadata->params.julia_dem.xmin,
                    loaded_metadata->params.julia_dem.xmax,
                    loaded_metadata->params.julia_dem.ymin,
                    loaded_metadata->params.julia_dem.ymax,
                    FRACTUS_PLANE_JULIA_REFERENCE_SPAN,
                    on_image, (uint32_t)pixel.x, (uint32_t)pixel.y, drawing->size.width, drawing->size.height,
                    &loaded_metadata->params.julia_dem.precise);
                break;
            case FRACTUS_GRAPHIC_KIND_BIOMORPH:
                fractus_app_set_hud_plane(
                    hud,
                    loaded_metadata->params.biomorph.xmin,
                    loaded_metadata->params.biomorph.xmax,
                    loaded_metadata->params.biomorph.ymin,
                    loaded_metadata->params.biomorph.ymax,
                    FRACTUS_PLANE_BIOMORPH_REFERENCE_SPAN,
                    on_image, (uint32_t)pixel.x, (uint32_t)pixel.y, drawing->size.width, drawing->size.height,
                    &loaded_metadata->params.biomorph.precise);
                break;
            case FRACTUS_GRAPHIC_KIND_PLASMA_RECTANGULAR:
            case FRACTUS_GRAPHIC_KIND_PLASMA_CIRCULAR:
            case FRACTUS_GRAPHIC_KIND_LORENZ:
            case FRACTUS_GRAPHIC_KIND_UNKNOWN:
            default:
                break;
            }
        }
        break;
    default:
        break;
    }
}

static const fractus_hp_window *fractus_app_plane_window_const(
    fractus_app_view view,
    const fractus_mandelbrot_params *mandelbrot,
    const fractus_mandelbrot_dem_params *mandelbrot_dem,
    const fractus_julia_params *julia,
    const fractus_julia_dem_params *julia_dem,
    const fractus_biomorph_params *biomorph)
{
    if (view == FRACTUS_APP_VIEW_MANDELBROT && mandelbrot != NULL) {
        return &mandelbrot->precise;
    }
    if (view == FRACTUS_APP_VIEW_MANDELBROT_DEM && mandelbrot_dem != NULL) {
        return &mandelbrot_dem->precise;
    }
    if (view == FRACTUS_APP_VIEW_JULIA && julia != NULL) {
        return &julia->precise;
    }
    if (view == FRACTUS_APP_VIEW_JULIA_DEM && julia_dem != NULL) {
        return &julia_dem->precise;
    }
    if (view == FRACTUS_APP_VIEW_BIOMORPH && biomorph != NULL) {
        return &biomorph->precise;
    }
    return NULL;
}

static fractus_hp_window *fractus_app_plane_window(
    fractus_app_view view,
    fractus_mandelbrot_params *mandelbrot,
    fractus_mandelbrot_dem_params *mandelbrot_dem,
    fractus_julia_params *julia,
    fractus_julia_dem_params *julia_dem,
    fractus_biomorph_params *biomorph)
{
    if (view == FRACTUS_APP_VIEW_MANDELBROT && mandelbrot != NULL) {
        return &mandelbrot->precise;
    }
    if (view == FRACTUS_APP_VIEW_MANDELBROT_DEM && mandelbrot_dem != NULL) {
        return &mandelbrot_dem->precise;
    }
    if (view == FRACTUS_APP_VIEW_JULIA && julia != NULL) {
        return &julia->precise;
    }
    if (view == FRACTUS_APP_VIEW_JULIA_DEM && julia_dem != NULL) {
        return &julia_dem->precise;
    }
    if (view == FRACTUS_APP_VIEW_BIOMORPH && biomorph != NULL) {
        return &biomorph->precise;
    }
    return NULL;
}

static void fractus_app_seed_zoom_history(
    fractus_app_zoom_history *history,
    fractus_app_view view,
    const fractus_mandelbrot_params *mandelbrot,
    const fractus_mandelbrot_dem_params *mandelbrot_dem,
    const fractus_julia_params *julia,
    const fractus_julia_dem_params *julia_dem,
    const fractus_biomorph_params *biomorph)
{
    double xmin = 0.0;
    double xmax = 0.0;
    double ymin = 0.0;
    double ymax = 0.0;
    int have_bounds = 0;

    if (history == NULL) {
        return;
    }

    if (view == FRACTUS_APP_VIEW_MANDELBROT && mandelbrot != NULL) {
        xmin = mandelbrot->xmin;
        xmax = mandelbrot->xmax;
        ymin = mandelbrot->ymin;
        ymax = mandelbrot->ymax;
        have_bounds = 1;
    } else if (view == FRACTUS_APP_VIEW_MANDELBROT_DEM && mandelbrot_dem != NULL) {
        xmin = mandelbrot_dem->xmin;
        xmax = mandelbrot_dem->xmax;
        ymin = mandelbrot_dem->ymin;
        ymax = mandelbrot_dem->ymax;
        have_bounds = 1;
    } else if (view == FRACTUS_APP_VIEW_JULIA && julia != NULL) {
        xmin = julia->xmin;
        xmax = julia->xmax;
        ymin = julia->ymin;
        ymax = julia->ymax;
        have_bounds = 1;
    } else if (view == FRACTUS_APP_VIEW_JULIA_DEM && julia_dem != NULL) {
        xmin = julia_dem->xmin;
        xmax = julia_dem->xmax;
        ymin = julia_dem->ymin;
        ymax = julia_dem->ymax;
        have_bounds = 1;
    } else if (view == FRACTUS_APP_VIEW_BIOMORPH && biomorph != NULL) {
        xmin = biomorph->xmin;
        xmax = biomorph->xmax;
        ymin = biomorph->ymin;
        ymax = biomorph->ymax;
        have_bounds = 1;
    }

    if (!have_bounds) {
        return;
    }

    {
        const fractus_hp_window *stored = fractus_app_plane_window_const(
            view, mandelbrot, mandelbrot_dem, julia, julia_dem, biomorph);
        fractus_hp_window current = {0};
        if (stored != NULL) {
            current = *stored;
        }
        fractus_hp_window_adopt_doubles(&current, xmin, xmax, ymin, ymax);
        if (history->view == view &&
            history->count > 0 &&
            history->cursor >= 0 &&
            history->cursor < history->count &&
            fractus_hp_window_equal(&history->frames[history->cursor].precise, &current)) {
            return;
        }

        fractus_app_zoom_history_reset(history, xmin, xmax, ymin, ymax, &current);
        history->view = view;
    }
}

static int fractus_app_recall_zoom_history(
    fractus_app_zoom_history *history,
    int forward,
    fractus_app_view view,
    fractus_mandelbrot_params *mandelbrot,
    fractus_mandelbrot_dem_params *mandelbrot_dem,
    fractus_julia_params *julia,
    fractus_julia_dem_params *julia_dem,
    fractus_biomorph_params *biomorph,
    int *mandelbrot_needs_render,
    int *mandelbrot_dem_needs_render,
    int *julia_needs_render,
    int *julia_dem_needs_render,
    int *biomorph_needs_render)
{
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    fractus_hp_window recalled = {0};
    int moved;

    if (!fractus_app_view_supports_zone_selection(view)) {
        return 0;
    }

    moved = forward
        ? fractus_app_zoom_history_forward(history, &xmin, &xmax, &ymin, &ymax, &recalled)
        : fractus_app_zoom_history_back(history, &xmin, &xmax, &ymin, &ymax, &recalled);
    if (!moved) {
        return 0;
    }

    if (view == FRACTUS_APP_VIEW_MANDELBROT && mandelbrot != NULL) {
        mandelbrot->xmin = xmin;
        mandelbrot->xmax = xmax;
        mandelbrot->ymin = ymin;
        mandelbrot->ymax = ymax;
        mandelbrot->precise = recalled;
        if (recalled.valid) {
            fractus_hp_window_publish_doubles(
                &mandelbrot->precise, &mandelbrot->xmin, &mandelbrot->xmax, &mandelbrot->ymin, &mandelbrot->ymax);
        }
        if (mandelbrot_needs_render != NULL) {
            *mandelbrot_needs_render = 1;
        }
    } else if (view == FRACTUS_APP_VIEW_MANDELBROT_DEM && mandelbrot_dem != NULL) {
        mandelbrot_dem->xmin = xmin;
        mandelbrot_dem->xmax = xmax;
        mandelbrot_dem->ymin = ymin;
        mandelbrot_dem->ymax = ymax;
        mandelbrot_dem->precise = recalled;
        if (recalled.valid) {
            fractus_hp_window_publish_doubles(
                &mandelbrot_dem->precise,
                &mandelbrot_dem->xmin,
                &mandelbrot_dem->xmax,
                &mandelbrot_dem->ymin,
                &mandelbrot_dem->ymax);
        }
        if (mandelbrot_dem_needs_render != NULL) {
            *mandelbrot_dem_needs_render = 1;
        }
    } else if (view == FRACTUS_APP_VIEW_JULIA && julia != NULL) {
        julia->xmin = xmin;
        julia->xmax = xmax;
        julia->ymin = ymin;
        julia->ymax = ymax;
        julia->precise = recalled;
        if (recalled.valid) {
            fractus_hp_window_publish_doubles(
                &julia->precise, &julia->xmin, &julia->xmax, &julia->ymin, &julia->ymax);
        }
        if (julia_needs_render != NULL) {
            *julia_needs_render = 1;
        }
    } else if (view == FRACTUS_APP_VIEW_JULIA_DEM && julia_dem != NULL) {
        julia_dem->xmin = xmin;
        julia_dem->xmax = xmax;
        julia_dem->ymin = ymin;
        julia_dem->ymax = ymax;
        julia_dem->precise = recalled;
        if (recalled.valid) {
            fractus_hp_window_publish_doubles(
                &julia_dem->precise, &julia_dem->xmin, &julia_dem->xmax, &julia_dem->ymin, &julia_dem->ymax);
        }
        if (julia_dem_needs_render != NULL) {
            *julia_dem_needs_render = 1;
        }
    } else if (view == FRACTUS_APP_VIEW_BIOMORPH && biomorph != NULL) {
        biomorph->xmin = xmin;
        biomorph->xmax = xmax;
        biomorph->ymin = ymin;
        biomorph->ymax = ymax;
        biomorph->precise = recalled;
        if (recalled.valid) {
            fractus_hp_window_publish_doubles(
                &biomorph->precise, &biomorph->xmin, &biomorph->xmax, &biomorph->ymin, &biomorph->ymax);
        }
        if (biomorph_needs_render != NULL) {
            *biomorph_needs_render = 1;
        }
    } else {
        return 0;
    }

    return 1;
}

static int fractus_app_read_plane_bounds(
    fractus_app_view view,
    const fractus_mandelbrot_params *mandelbrot,
    const fractus_mandelbrot_dem_params *mandelbrot_dem,
    const fractus_julia_params *julia,
    const fractus_julia_dem_params *julia_dem,
    const fractus_biomorph_params *biomorph,
    double *xmin,
    double *xmax,
    double *ymin,
    double *ymax)
{
    if (xmin == NULL || xmax == NULL || ymin == NULL || ymax == NULL) {
        return 0;
    }

    if (view == FRACTUS_APP_VIEW_MANDELBROT && mandelbrot != NULL) {
        *xmin = mandelbrot->xmin;
        *xmax = mandelbrot->xmax;
        *ymin = mandelbrot->ymin;
        *ymax = mandelbrot->ymax;
        return 1;
    }
    if (view == FRACTUS_APP_VIEW_MANDELBROT_DEM && mandelbrot_dem != NULL) {
        *xmin = mandelbrot_dem->xmin;
        *xmax = mandelbrot_dem->xmax;
        *ymin = mandelbrot_dem->ymin;
        *ymax = mandelbrot_dem->ymax;
        return 1;
    }
    if (view == FRACTUS_APP_VIEW_JULIA && julia != NULL) {
        *xmin = julia->xmin;
        *xmax = julia->xmax;
        *ymin = julia->ymin;
        *ymax = julia->ymax;
        return 1;
    }
    if (view == FRACTUS_APP_VIEW_JULIA_DEM && julia_dem != NULL) {
        *xmin = julia_dem->xmin;
        *xmax = julia_dem->xmax;
        *ymin = julia_dem->ymin;
        *ymax = julia_dem->ymax;
        return 1;
    }
    if (view == FRACTUS_APP_VIEW_BIOMORPH && biomorph != NULL) {
        *xmin = biomorph->xmin;
        *xmax = biomorph->xmax;
        *ymin = biomorph->ymin;
        *ymax = biomorph->ymax;
        return 1;
    }
    return 0;
}

static int fractus_app_write_plane_bounds(
    fractus_app_view view,
    fractus_mandelbrot_params *mandelbrot,
    fractus_mandelbrot_dem_params *mandelbrot_dem,
    fractus_julia_params *julia,
    fractus_julia_dem_params *julia_dem,
    fractus_biomorph_params *biomorph,
    int *mandelbrot_needs_render,
    int *mandelbrot_dem_needs_render,
    int *julia_needs_render,
    int *julia_dem_needs_render,
    int *biomorph_needs_render,
    double xmin,
    double xmax,
    double ymin,
    double ymax)
{
    fractus_hp_window *window = fractus_app_plane_window(
        view, mandelbrot, mandelbrot_dem, julia, julia_dem, biomorph);
    int double_ok = (xmax > xmin && ymax > ymin);
    int precise_ok = (window != NULL && fractus_hp_window_spans_positive(window));

    if (!double_ok && !precise_ok) {
        return 0;
    }

    if (view == FRACTUS_APP_VIEW_MANDELBROT && mandelbrot != NULL) {
        mandelbrot->xmin = xmin;
        mandelbrot->xmax = xmax;
        mandelbrot->ymin = ymin;
        mandelbrot->ymax = ymax;
        if (mandelbrot_needs_render != NULL) {
            *mandelbrot_needs_render = 1;
        }
        return 1;
    }
    if (view == FRACTUS_APP_VIEW_MANDELBROT_DEM && mandelbrot_dem != NULL) {
        mandelbrot_dem->xmin = xmin;
        mandelbrot_dem->xmax = xmax;
        mandelbrot_dem->ymin = ymin;
        mandelbrot_dem->ymax = ymax;
        if (mandelbrot_dem_needs_render != NULL) {
            *mandelbrot_dem_needs_render = 1;
        }
        return 1;
    }
    if (view == FRACTUS_APP_VIEW_JULIA && julia != NULL) {
        julia->xmin = xmin;
        julia->xmax = xmax;
        julia->ymin = ymin;
        julia->ymax = ymax;
        if (julia_needs_render != NULL) {
            *julia_needs_render = 1;
        }
        return 1;
    }
    if (view == FRACTUS_APP_VIEW_JULIA_DEM && julia_dem != NULL) {
        julia_dem->xmin = xmin;
        julia_dem->xmax = xmax;
        julia_dem->ymin = ymin;
        julia_dem->ymax = ymax;
        if (julia_dem_needs_render != NULL) {
            *julia_dem_needs_render = 1;
        }
        return 1;
    }
    if (view == FRACTUS_APP_VIEW_BIOMORPH && biomorph != NULL) {
        biomorph->xmin = xmin;
        biomorph->xmax = xmax;
        biomorph->ymin = ymin;
        biomorph->ymax = ymax;
        if (biomorph_needs_render != NULL) {
            *biomorph_needs_render = 1;
        }
        return 1;
    }
    return 0;
}

static void fractus_app_commit_plane_drag(
    int *drag_active,
    int *drag_moved,
    fractus_app_view drag_view,
    fractus_app_zoom_history *history,
    const fractus_mandelbrot_params *mandelbrot,
    const fractus_mandelbrot_dem_params *mandelbrot_dem,
    const fractus_julia_params *julia,
    const fractus_julia_dem_params *julia_dem,
    const fractus_biomorph_params *biomorph)
{
    double xmin;
    double xmax;
    double ymin;
    double ymax;

    if (drag_active == NULL || drag_moved == NULL || !*drag_active) {
        return;
    }

    if (*drag_moved &&
        fractus_app_read_plane_bounds(
            drag_view, mandelbrot, mandelbrot_dem, julia, julia_dem, biomorph, &xmin, &xmax, &ymin, &ymax)) {
        (void)fractus_app_zoom_history_push(
            history,
            xmin,
            xmax,
            ymin,
            ymax,
            fractus_app_plane_window_const(drag_view, mandelbrot, mandelbrot_dem, julia, julia_dem, biomorph));
    }
    *drag_active = 0;
    *drag_moved = 0;
}

static int fractus_app_handle_plane_pointer(
    fractus_app_view view,
    const fractus_platform_context *platform,
    const fractus_framebuffer *framebuffer,
    const fractus_ui_context *ui,
    const fractus_app_zone_selection *selection,
    int selection_cancelled,
    fractus_app_zoom_history *history,
    int *drag_active,
    int *drag_moved,
    fractus_app_view *drag_view,
    fractus_point_i32 *drag_pixel,
    fractus_mandelbrot_params *mandelbrot,
    fractus_mandelbrot_dem_params *mandelbrot_dem,
    fractus_julia_params *julia,
    fractus_julia_dem_params *julia_dem,
    fractus_biomorph_params *biomorph,
    int *mandelbrot_needs_render,
    int *mandelbrot_dem_needs_render,
    int *julia_needs_render,
    int *julia_dem_needs_render,
    int *biomorph_needs_render)
{
    int changed = 0;
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    fractus_point_i32 pixel;

    if (drag_active == NULL || drag_moved == NULL || drag_view == NULL || drag_pixel == NULL || ui == NULL) {
        return 0;
    }

    if (*drag_active && view != *drag_view) {
        fractus_app_commit_plane_drag(
            drag_active, drag_moved, *drag_view, history,
            mandelbrot, mandelbrot_dem, julia, julia_dem, biomorph);
        return 0;
    }

    if (!fractus_app_view_supports_zone_selection(view) || framebuffer == NULL || !framebuffer->initialized) {
        *drag_active = 0;
        *drag_moved = 0;
        return 0;
    }

    if (*drag_active && ui->key_press_pending && (ui->key_pressed == 's' || ui->key_pressed == 'S')) {
        int moved = *drag_moved;
        fractus_app_commit_plane_drag(
            drag_active, drag_moved, *drag_view, history,
            mandelbrot, mandelbrot_dem, julia, julia_dem, biomorph);
        if (moved) {
            changed = 1;
        }
    }

    if (!*drag_active && !selection_cancelled && selection != NULL && !selection->active &&
        ui->press_pending && ui->press_event.buttons.right &&
        fractus_app_map_drawing_window_point(platform, framebuffer, ui->press_event.position, drag_pixel)) {
        *drag_active = 1;
        *drag_moved = 0;
        *drag_view = view;
    }

    if (*drag_active) {
        fractus_hp_window *window = fractus_app_plane_window(
            view, mandelbrot, mandelbrot_dem, julia, julia_dem, biomorph);
        if (window != NULL &&
            fractus_app_read_plane_bounds(view, mandelbrot, mandelbrot_dem, julia, julia_dem, biomorph, &xmin, &xmax, &ymin, &ymax) &&
            fractus_app_map_drawing_window_point_unclamped(platform, framebuffer, ui->pointer_position, &pixel)) {
            fractus_hp_window_adopt_doubles(window, xmin, xmax, ymin, ymax);
            if (fractus_hp_plane_pan(
                    window,
                    framebuffer->size.width,
                    framebuffer->size.height,
                    drag_pixel->x,
                    drag_pixel->y,
                    pixel.x,
                    pixel.y) &&
                fractus_app_write_plane_bounds(
                    view, mandelbrot, mandelbrot_dem, julia, julia_dem, biomorph,
                    mandelbrot_needs_render, mandelbrot_dem_needs_render,
                    julia_needs_render, julia_dem_needs_render, biomorph_needs_render,
                    window->xmin.hi, window->xmax.hi, window->ymin.hi, window->ymax.hi)) {
                *drag_pixel = pixel;
                *drag_moved = 1;
                changed = 1;
            }
        }
    }

    if (*drag_active &&
        ((ui->release_pending && ui->release_event.buttons.right) ||
         (!ui->buttons_down.right && !(ui->press_pending && ui->press_event.buttons.right)))) {
        fractus_app_commit_plane_drag(
            drag_active, drag_moved, *drag_view, history,
            mandelbrot, mandelbrot_dem, julia, julia_dem, biomorph);
    }

    if ((selection == NULL || !selection->active) && ui->wheel_delta != 0 &&
        fractus_app_map_drawing_window_point(platform, framebuffer, ui->pointer_position, &pixel)) {
        fractus_hp_window *window = fractus_app_plane_window(
            view, mandelbrot, mandelbrot_dem, julia, julia_dem, biomorph);
        if (window != NULL &&
            fractus_app_read_plane_bounds(view, mandelbrot, mandelbrot_dem, julia, julia_dem, biomorph, &xmin, &xmax, &ymin, &ymax)) {
            fractus_hp_window_adopt_doubles(window, xmin, xmax, ymin, ymax);
            if (fractus_hp_plane_zoom_centered(
                    window,
                    framebuffer->size.width,
                    framebuffer->size.height,
                    (double)ui->wheel_delta) &&
                fractus_app_write_plane_bounds(
                    view, mandelbrot, mandelbrot_dem, julia, julia_dem, biomorph,
                    mandelbrot_needs_render, mandelbrot_dem_needs_render,
                    julia_needs_render, julia_dem_needs_render, biomorph_needs_render,
                    window->xmin.hi, window->xmax.hi, window->ymin.hi, window->ymax.hi)) {
                (void)fractus_app_zoom_history_push(history, window->xmin.hi, window->xmax.hi, window->ymin.hi, window->ymax.hi, window);
                changed = 1;
            }
        }
    }

    return changed;
}

typedef struct fractus_app_render_feedback {
    fractus_platform_context *platform;
    const fractus_legacy_config *config;
    fractus_framebuffer *source;
    fractus_framebuffer *scratch;
    const fractus_font_library *fonts;
    uint64_t started_ms;
    uint64_t last_present_ms;
    int painted;
    int escape_held;
} fractus_app_render_feedback;

static void fractus_app_present_drawing_now(
    fractus_platform_context *platform,
    const fractus_legacy_config *config,
    fractus_framebuffer *framebuffer)
{
    fractus_platform_surface surface;

    if (platform == NULL || config == NULL || framebuffer == NULL || !framebuffer->initialized) {
        return;
    }
    if (fractus_framebuffer_sync_rgba(framebuffer) != FRACTUS_STATUS_OK) {
        return;
    }

    surface.width = framebuffer->size.width;
    surface.height = framebuffer->size.height;
    surface.pitch_bytes = framebuffer->rgba_pitch_bytes;
    surface.pixel_format = FRACTUS_PLATFORM_PIXEL_FORMAT_RGBA8888;
    surface.present_mode = FRACTUS_PLATFORM_PRESENT_MODE_DRAWING;
    surface.pixels = framebuffer->rgba_pixels;
    if (fractus_app_configure_platform_for_present(platform, config, 1) != FRACTUS_STATUS_OK) {
        return;
    }
    (void)fractus_platform_present(platform, &surface);
}

static int fractus_app_report_render_progress(void *user, uint32_t rows_done, uint32_t rows_total)
{
    fractus_app_render_feedback *feedback = (fractus_app_render_feedback *)user;
    uint64_t now_ms;
    uint32_t percent;
    int escape_held = 0;

    if (feedback == NULL || feedback->platform == NULL || rows_total == 0u) {
        return 0;
    }

    fractus_platform_pump_events(feedback->platform);
    if (rows_done >= rows_total) {
        return 0;
    }

    now_ms = fractus_platform_get_ticks_ms(feedback->platform);
    if (now_ms >= feedback->started_ms &&
        now_ms - feedback->started_ms >= 400u &&
        !(feedback->painted && now_ms - feedback->last_present_ms < 200u) &&
        feedback->source != NULL &&
        feedback->scratch != NULL &&
        feedback->fonts != NULL &&
        fractus_app_copy_framebuffer_for_overlay(feedback->scratch, feedback->source) == FRACTUS_STATUS_OK) {
        percent = (uint32_t)(((uint64_t)rows_done * 100ull) / (uint64_t)rows_total);
        (void)fractus_app_draw_render_progress(feedback->scratch, feedback->fonts, percent);
        fractus_app_present_drawing_now(feedback->platform, feedback->config, feedback->scratch);
        feedback->last_present_ms = now_ms;
        feedback->painted = 1;
    }

    if (feedback->painted &&
        fractus_platform_consume_escape(feedback->platform, &escape_held)) {
        feedback->escape_held = escape_held;
        return 1;
    }
    return 0;
}

static void fractus_app_begin_render_feedback(
    fractus_app_render_feedback *feedback,
    fractus_render_progress *progress,
    fractus_platform_context *platform,
    const fractus_legacy_config *config,
    fractus_framebuffer *source,
    fractus_framebuffer *scratch,
    const fractus_font_library *fonts,
    uint64_t started_ms)
{
    feedback->platform = platform;
    feedback->config = config;
    feedback->source = source;
    feedback->scratch = scratch;
    feedback->fonts = fonts;
    feedback->started_ms = started_ms;
    feedback->last_present_ms = started_ms;
    feedback->painted = 0;
    feedback->escape_held = 0;
    progress->report = fractus_app_report_render_progress;
    progress->user = feedback;
}

static int fractus_app_finish_render(
    fractus_status status,
    const fractus_app_render_feedback *feedback,
    fractus_app_view config_view,
    int *needs_render,
    fractus_app_view *view,
    int *save_next_graphic,
    int *render_save_next_graphic,
    int *save_requested_this_frame,
    int *present_is_drawing,
    fractus_framebuffer **present_framebuffer,
    fractus_framebuffer *ui_framebuffer,
    int *render_aborted,
    int *swallow_escape,
    fractus_platform_context *platform,
    uint64_t render_started_ms,
    uint32_t *last_render_ms,
    int *has_render_time,
    const char *failure_log)
{
    if (status == FRACTUS_STATUS_CANCELLED) {
        *view = config_view;
        *save_next_graphic = 0;
        *render_save_next_graphic = 0;
        *save_requested_this_frame = 0;
        *present_is_drawing = 0;
        *present_framebuffer = ui_framebuffer;
        *render_aborted = 1;
        if (feedback != NULL && feedback->escape_held) {
            *swallow_escape = 1;
        }
        return 1;
    }
    if (status != FRACTUS_STATUS_OK) {
        fractus_app_log(failure_log);
        return 0;
    }
    *needs_render = 0;
    fractus_app_note_render_time(platform, render_started_ms, last_render_ms, has_render_time);
    return 1;
}

int fractus_app_run(void)
{
    fractus_core core;
    fractus_ui_context ui;
    fractus_platform_context platform;
    fractus_platform_event event;
    fractus_platform_surface surface;
    fractus_framebuffer *present_framebuffer;
    fractus_framebuffer zone_overlay_framebuffer;
    fractus_app_graphic_file graphic_files[FRACTUS_APP_GRAPHIC_FILE_CAPACITY];
    fractus_app_palette_file palette_files[FRACTUS_APP_PALETTE_FILE_CAPACITY];
    fractus_font_library fonts;
    fractus_legacy_config legacy_config;
    fractus_legacy_config config_draft;
    fractus_mandelbrot_params mandelbrot_params;
    fractus_mandelbrot_params mandelbrot_pending;
    fractus_mandelbrot_dem_params mandelbrot_dem_params;
    fractus_mandelbrot_dem_params mandelbrot_dem_pending;
    fractus_julia_params julia_params;
    fractus_julia_params julia_pending;
    fractus_julia_dem_params julia_dem_params;
    fractus_julia_dem_params julia_dem_pending;
    fractus_biomorph_params biomorph_params;
    fractus_biomorph_params biomorph_pending;
    fractus_plasma_params plasma_rectangular_params;
    fractus_plasma_params plasma_rectangular_pending;
    fractus_plasma_circular_params plasma_circular_params;
    fractus_plasma_circular_params plasma_circular_pending;
    fractus_app_zone_selection zone_selection;
    fractus_app_zoom_history zoom_history;
    int plane_drag_active;
    int plane_drag_moved;
    fractus_app_view plane_drag_view;
    fractus_point_i32 plane_drag_pixel;
    fractus_color_rgba8 palette_original_color;
    fractus_color_rgba8 palette_pending_color;
    fractus_color_rgba8 palette_copy_color;
    fractus_ui_numeric_field iterations_field;
    fractus_ui_numeric_field escape_radius_field;
    fractus_ui_numeric_field biomorph_iterations_field;
    fractus_ui_numeric_field biomorph_radius_field;
    fractus_ui_numeric_field biomorph_cutoff_field;
    fractus_ui_numeric_field plasma_rect_seed_field;
    fractus_ui_numeric_field plasma_circ_seed_field;
    fractus_ui_numeric_field palette_red_field;
    fractus_ui_numeric_field palette_green_field;
    fractus_ui_numeric_field palette_blue_field;
    fractus_app_mandelbrot_fields mandelbrot_fields;
    fractus_app_mandelbrot_fields mandelbrot_dem_fields;
    fractus_app_julia_fields julia_fields;
    fractus_app_julia_fields julia_dem_fields;
    fractus_app_biomorph_fields biomorph_fields;
    fractus_app_plasma_fields plasma_fields;
    fractus_app_view view;
    char font_path[512];
    char cfg_path[512];
    char runtime_error_message[160];
    char selected_graphic_path[512];
    char last_saved_filename[64];
    int running;
    int has_event;
    int save_next_graphic;
    int swallow_escape;
    int drawing_presented_once;
    int mandelbrot_needs_render;
    int mandelbrot_dem_needs_render;
    int julia_needs_render;
    int julia_dem_needs_render;
    int biomorph_needs_render;
    int plasma_rectangular_needs_render;
    int plasma_circular_needs_render;
    int lorenz_needs_render;
    int palette_flow_active;
    int drawing_hud_visible;
    int has_render_time;
    uint32_t last_render_ms;
    double frame_fps;
    uint64_t previous_frame_ms;
    uint64_t render_started_ms;
    int present_is_drawing;
    fractus_lorenz_params lorenz_params;
    fractus_lorenz_params lorenz_pending;
    fractus_app_attractor_fields attractor_fields;
    fractus_app_attractor_method attractor_selected_method;
    fractus_graphic_metadata loaded_graphic_metadata;
    int has_loaded_graphic_metadata;
    int graphic_metadata_panel_visible;
    int drawing_footer_visible;
    uint32_t save_feedback_frames;
    uint32_t palette_selected_index;
    uint32_t palette_copy_source_index;
    uint32_t palette_gradient_first_index;
    size_t graphic_file_count;
    size_t graphic_file_page;
    size_t palette_file_count;
    size_t palette_file_page;
    const fractus_platform_config config = {
        "Fractus x64",
        "Fractus",
        {640u, 480u}
    };

    memset(&fonts, 0, sizeof(fonts));
    memset(&zone_overlay_framebuffer, 0, sizeof(zone_overlay_framebuffer));
    memset(&legacy_config, 0, sizeof(legacy_config));
    memset(&config_draft, 0, sizeof(config_draft));
    memset(&iterations_field, 0, sizeof(iterations_field));
    memset(&escape_radius_field, 0, sizeof(escape_radius_field));
    memset(&biomorph_iterations_field, 0, sizeof(biomorph_iterations_field));
    memset(&biomorph_radius_field, 0, sizeof(biomorph_radius_field));
    memset(&biomorph_cutoff_field, 0, sizeof(biomorph_cutoff_field));
    memset(&plasma_rect_seed_field, 0, sizeof(plasma_rect_seed_field));
    memset(&plasma_circ_seed_field, 0, sizeof(plasma_circ_seed_field));
    memset(&palette_red_field, 0, sizeof(palette_red_field));
    memset(&palette_green_field, 0, sizeof(palette_green_field));
    memset(&palette_blue_field, 0, sizeof(palette_blue_field));
    memset(&mandelbrot_fields, 0, sizeof(mandelbrot_fields));
    memset(&mandelbrot_dem_fields, 0, sizeof(mandelbrot_dem_fields));
    memset(&julia_fields, 0, sizeof(julia_fields));
    memset(&julia_dem_fields, 0, sizeof(julia_dem_fields));
    memset(&biomorph_fields, 0, sizeof(biomorph_fields));
    memset(&plasma_fields, 0, sizeof(plasma_fields));
    memset(&attractor_fields, 0, sizeof(attractor_fields));
    memset(&zone_selection, 0, sizeof(zone_selection));
    fractus_app_zoom_history_clear(&zoom_history);
    plane_drag_active = 0;
    plane_drag_moved = 0;
    plane_drag_view = FRACTUS_APP_VIEW_MAIN_MENU;
    plane_drag_pixel.x = 0;
    plane_drag_pixel.y = 0;
    memset(&loaded_graphic_metadata, 0, sizeof(loaded_graphic_metadata));
    has_loaded_graphic_metadata = 0;
    graphic_metadata_panel_visible = 0;
    drawing_footer_visible = 0;
    memset(graphic_files, 0, sizeof(graphic_files));
    memset(palette_files, 0, sizeof(palette_files));
    memset(cfg_path, 0, sizeof(cfg_path));
    memset(runtime_error_message, 0, sizeof(runtime_error_message));
    memset(selected_graphic_path, 0, sizeof(selected_graphic_path));
    attractor_selected_method = FRACTUS_APP_ATTRACTOR_METHOD_NONE;
    palette_original_color = fractus_app_rgb8(0u, 0u, 0u);
    palette_pending_color = fractus_app_rgb8(0u, 0u, 0u);
    palette_copy_color = fractus_app_rgb8(0u, 0u, 0u);
    palette_selected_index = FRACTUS_PALETTE_OFFSET;
    palette_copy_source_index = FRACTUS_PALETTE_OFFSET;
    palette_gradient_first_index = FRACTUS_PALETTE_OFFSET;
    save_next_graphic = 0;
    swallow_escape = 0;
    drawing_presented_once = 0;
    memset(last_saved_filename, 0, sizeof(last_saved_filename));
    mandelbrot_needs_render = 1;
    mandelbrot_dem_needs_render = 1;
    julia_needs_render = 1;
    julia_dem_needs_render = 1;
    biomorph_needs_render = 1;
    plasma_rectangular_needs_render = 1;
    plasma_circular_needs_render = 1;
    lorenz_needs_render = 1;
    palette_flow_active = 0;
    drawing_hud_visible = 0;
    has_render_time = 0;
    last_render_ms = 0u;
    frame_fps = 0.0;
    previous_frame_ms = 0u;
    render_started_ms = 0u;
    save_feedback_frames = 0u;
    fractus_app_log("startup: begin");

    if (fractus_platform_init(&platform, &config) != FRACTUS_STATUS_OK) {
        fractus_app_log("startup: platform init failed");
        return 1;
    }

    if (fractus_core_init(&core, config.logical_size) != FRACTUS_STATUS_OK) {
        fractus_app_log("startup: core init failed");
        fractus_platform_shutdown(&platform);
        return 1;
    }

    if (fractus_ui_init(&ui, config.logical_size) != FRACTUS_STATUS_OK) {
        fractus_app_log("startup: ui init failed");
        fractus_core_shutdown(&core);
        fractus_platform_shutdown(&platform);
        return 1;
    }

    if (fractus_formats_resolve_legacy_path(&platform, "fractus.fon", font_path, sizeof(font_path)) != FRACTUS_STATUS_OK) {
        fractus_app_log("startup: fractus.fon not found, continuing without legacy fonts");
    } else if (fractus_font_library_load_archive(&fonts, font_path) != FRACTUS_STATUS_OK) {
        fractus_app_log("startup: failed loading fractus.fon, continuing without legacy fonts");
    } else {
        fractus_app_log("startup: legacy fonts loaded");
    }

    if (fractus_app_load_legacy_assets(
            &platform,
            &core.ui_framebuffer,
            &mandelbrot_params,
            &julia_params,
            &biomorph_params,
            &plasma_rectangular_params,
            &plasma_circular_params,
            &legacy_config,
            cfg_path,
            sizeof(cfg_path)) != FRACTUS_STATUS_OK) {
        fractus_app_log("startup: failed loading legacy config/palette, using defaults");
        if (fractus_legacy_config_init_default(&legacy_config) == FRACTUS_STATUS_OK) {
            (void)fractus_app_apply_legacy_config(&core.ui_framebuffer, &legacy_config);
        }
        if (fractus_formats_resolve_legacy_write_path(&platform, "fractus.cfg", cfg_path, sizeof(cfg_path)) != FRACTUS_STATUS_OK) {
            cfg_path[0] = '\0';
        }
    }
    fractus_app_reset_fractal_parameters(
        &mandelbrot_params,
        &mandelbrot_dem_params,
        &julia_params,
        &julia_dem_params,
        &biomorph_params,
        &plasma_rectangular_params,
        &plasma_circular_params,
        &lorenz_params,
        &legacy_config);
    if (fractus_app_sync_framebuffer_palette(&core.drawing_framebuffer, &core.ui_framebuffer) != FRACTUS_STATUS_OK) {
        fractus_app_log("startup: drawing palette sync failed");
        fractus_ui_shutdown(&ui);
        fractus_core_shutdown(&core);
        fractus_platform_shutdown(&platform);
        return 1;
    }

    view = FRACTUS_APP_VIEW_MAIN_MENU;
    running = 1;
    graphic_file_count = 0u;
    graphic_file_page = 0u;
    palette_file_count = 0u;
    palette_file_page = 0u;

    while (running) {
        uint64_t now_ms = fractus_platform_get_ticks_ms(&platform);

        if (previous_frame_ms != 0u && now_ms > previous_frame_ms) {
            frame_fps = 1000.0 / (double)(now_ms - previous_frame_ms);
        }
        previous_frame_ms = now_ms;
        present_framebuffer = &core.ui_framebuffer;
        present_is_drawing = 0;
        fractus_ui_begin_frame(&ui);

        do {
            if (fractus_platform_poll_event(&platform, &event, &has_event) != FRACTUS_STATUS_OK) {
                running = 0;
                break;
            }

            if (!has_event) {
                break;
            }

            if (swallow_escape &&
                (event.type == FRACTUS_PLATFORM_EVENT_KEY_DOWN || event.type == FRACTUS_PLATFORM_EVENT_KEY_UP) &&
                event.data.key.keycode == (uint32_t)FRACTUS_PLATFORM_KEY_ESCAPE) {
                if (event.type == FRACTUS_PLATFORM_EVENT_KEY_UP) {
                    swallow_escape = 0;
                }
                continue;
            }

            if (fractus_ui_handle_platform_event(&ui, &event) != FRACTUS_STATUS_OK) {
                running = 0;
                break;
            }
        } while (has_event);

        if (ui.quit_requested) {
            running = 0;
        }

        {
            fractus_app_view previous_view = view;

            if (view == FRACTUS_APP_VIEW_MAIN_MENU) {
                if (fractus_app_run_main_menu_view(
                        &platform,
                        &core.ui_framebuffer,
                        &fonts,
                        &ui,
                        &view,
                        &running,
                        &legacy_config,
                        &config_draft,
                        &mandelbrot_params,
                        &mandelbrot_dem_params,
                        &julia_params,
                        &julia_dem_params,
                        &biomorph_params,
                        &biomorph_pending,
                        &plasma_rectangular_params,
                        &plasma_circular_params,
                        &lorenz_params,
                        &attractor_selected_method,
                        &biomorph_fields,
                        &iterations_field,
                        &escape_radius_field,
                        &biomorph_iterations_field,
                        &biomorph_radius_field,
                        &biomorph_cutoff_field,
                        &plasma_rect_seed_field,
                        &plasma_circ_seed_field,
                        graphic_files,
                        &graphic_file_count,
                        &graphic_file_page,
                        palette_files,
                        &palette_file_count,
                        &palette_file_page,
                        runtime_error_message,
                        sizeof(runtime_error_message),
                        cfg_path,
                        &save_next_graphic) != FRACTUS_STATUS_OK) {
                    fractus_app_log("runtime: main menu failed");
                    running = 0;
                }
            } else if (view == FRACTUS_APP_VIEW_MANDELBROT_MENU) {
                if (fractus_app_run_mandelbrot_menu_view(
                        &core.ui_framebuffer,
                        &fonts,
                        &ui,
                        &mandelbrot_params,
                        &mandelbrot_pending,
                        &mandelbrot_fields,
                        &mandelbrot_dem_params,
                        &mandelbrot_dem_pending,
                        &mandelbrot_dem_fields,
                        &view) != FRACTUS_STATUS_OK) {
                    fractus_app_log("runtime: mandelbrot menu failed");
                    running = 0;
                }
            } else if (view == FRACTUS_APP_VIEW_MANDELBROT_CONFIG) {
                if (fractus_app_run_mandelbrot_config_view(
                        &core.ui_framebuffer,
                        &fonts,
                        &ui,
                        &mandelbrot_params,
                        &mandelbrot_pending,
                        &mandelbrot_fields,
                        &view) != FRACTUS_STATUS_OK) {
                    fractus_app_log("runtime: mandelbrot config failed");
                    running = 0;
                }
            } else if (view == FRACTUS_APP_VIEW_MANDELBROT_DEM_CONFIG) {
                if (fractus_app_run_mandelbrot_dem_config_view(
                        &core.ui_framebuffer,
                        &fonts,
                        &ui,
                        &mandelbrot_dem_params,
                        &mandelbrot_dem_pending,
                        &mandelbrot_dem_fields,
                        &view) != FRACTUS_STATUS_OK) {
                    fractus_app_log("runtime: mandelbrot dem config failed");
                    running = 0;
                }
            } else if (view == FRACTUS_APP_VIEW_JULIA_MENU) {
                if (fractus_app_run_julia_menu_view(
                        &core.ui_framebuffer,
                        &fonts,
                        &ui,
                        &julia_params,
                        &julia_pending,
                        &julia_fields,
                        &julia_dem_params,
                        &julia_dem_pending,
                        &julia_dem_fields,
                        &view) != FRACTUS_STATUS_OK) {
                    fractus_app_log("runtime: julia menu failed");
                    running = 0;
                }
            } else if (view == FRACTUS_APP_VIEW_JULIA_CONFIG) {
                if (fractus_app_run_julia_config_view(
                        &core.ui_framebuffer,
                        &fonts,
                        &ui,
                        &julia_params,
                        &julia_pending,
                        &julia_fields,
                        &view) != FRACTUS_STATUS_OK) {
                    fractus_app_log("runtime: julia config failed");
                    running = 0;
                }
            } else if (view == FRACTUS_APP_VIEW_JULIA_DEM_CONFIG) {
                if (fractus_app_run_julia_dem_config_view(
                        &core.ui_framebuffer,
                        &fonts,
                        &ui,
                        &julia_dem_params,
                        &julia_dem_pending,
                        &julia_dem_fields,
                        &view) != FRACTUS_STATUS_OK) {
                    fractus_app_log("runtime: julia dem config failed");
                    running = 0;
                }
            } else if (view == FRACTUS_APP_VIEW_BIOMORPH_CONFIG) {
                if (fractus_app_run_biomorph_config_view(
                        &core.ui_framebuffer,
                        &fonts,
                        &ui,
                        &biomorph_params,
                        &biomorph_pending,
                        &biomorph_fields,
                        &view) != FRACTUS_STATUS_OK) {
                    fractus_app_log("runtime: biomorph config failed");
                    running = 0;
                }
            } else if (view == FRACTUS_APP_VIEW_PLASMA_MENU) {
                if (fractus_app_run_plasma_menu_view(
                        &core.ui_framebuffer,
                        &fonts,
                        &ui,
                        &plasma_rectangular_params,
                        &plasma_rectangular_pending,
                        &plasma_circular_params,
                        &plasma_circular_pending,
                        &plasma_fields,
                        &view) != FRACTUS_STATUS_OK) {
                    fractus_app_log("runtime: plasma menu failed");
                    running = 0;
                }
            } else if (view == FRACTUS_APP_VIEW_PLASMA_RECTANGULAR_CONFIG) {
                if (fractus_app_run_plasma_rectangular_config_view(
                        &core.ui_framebuffer,
                        &fonts,
                        &ui,
                        &plasma_rectangular_params,
                        &plasma_rectangular_pending,
                        &plasma_fields,
                        &view) != FRACTUS_STATUS_OK) {
                    fractus_app_log("runtime: plasma rectangular config failed");
                    running = 0;
                }
            } else if (view == FRACTUS_APP_VIEW_PLASMA_CIRCULAR_CONFIG) {
                if (fractus_app_run_plasma_circular_config_view(
                        &core.ui_framebuffer,
                        &fonts,
                        &ui,
                        &plasma_circular_params,
                        &plasma_circular_pending,
                        &plasma_fields,
                        &view) != FRACTUS_STATUS_OK) {
                    fractus_app_log("runtime: plasma circular config failed");
                    running = 0;
                }
            } else if (view == FRACTUS_APP_VIEW_ATTRACTORS_MENU) {
                if (fractus_app_run_attractors_menu_view(
                        &core.ui_framebuffer,
                        &fonts,
                        &ui,
                        &lorenz_params,
                        &lorenz_pending,
                        &attractor_fields,
                        &attractor_selected_method,
                        &view) != FRACTUS_STATUS_OK) {
                    fractus_app_log("runtime: attractors menu failed");
                    running = 0;
                }
            } else if (view == FRACTUS_APP_VIEW_ATTRACTORS_CONFIG) {
                if (fractus_app_run_attractors_config_view(
                        &core.ui_framebuffer,
                        &fonts,
                        &ui,
                        &lorenz_params,
                        &lorenz_pending,
                        &attractor_fields,
                        &attractor_selected_method,
                        &view) != FRACTUS_STATUS_OK) {
                    fractus_app_log("runtime: attractors config failed");
                    running = 0;
                }
            } else if (view == FRACTUS_APP_VIEW_VIDEO_CONFIG) {
                if (fractus_app_run_video_config_view(
                        &core.ui_framebuffer,
                        &fonts,
                        &ui,
                        &legacy_config,
                        &config_draft,
                        &mandelbrot_params,
                        &julia_params,
                        &biomorph_params,
                        cfg_path,
                        &view) != FRACTUS_STATUS_OK) {
                    fractus_app_log("runtime: video config failed");
                    running = 0;
                }
            } else if (view == FRACTUS_APP_VIEW_FRACTALS_DEFAULT_CONFIG) {
                if (fractus_app_run_fractals_default_config_view(
                        &core.ui_framebuffer,
                        &fonts,
                        &ui,
                        &legacy_config,
                        &config_draft,
                        &mandelbrot_params,
                        &mandelbrot_dem_params,
                        &julia_params,
                        &julia_dem_params,
                        &biomorph_params,
                        &plasma_rectangular_params,
                        &plasma_circular_params,
                        cfg_path,
                        &iterations_field,
                        &escape_radius_field,
                        &biomorph_iterations_field,
                        &biomorph_radius_field,
                        &biomorph_cutoff_field,
                        &plasma_rect_seed_field,
                        &plasma_circ_seed_field,
                        &view) != FRACTUS_STATUS_OK) {
                    fractus_app_log("runtime: fractals default config failed");
                    running = 0;
                }
            } else if (view == FRACTUS_APP_VIEW_LOAD_GRAPHIC) {
                if (fractus_app_ensure_drawing_framebuffer_size(&legacy_config, &core.drawing_framebuffer, &core.ui_framebuffer) != FRACTUS_STATUS_OK) {
                    fractus_app_log("runtime: drawing framebuffer resize failed");
                    running = 0;
                } else if (fractus_app_run_load_graphic_view(
                        &core.ui_framebuffer,
                        &core.drawing_framebuffer,
                        &fonts,
                        &ui,
                        graphic_files,
                        graphic_file_count,
                        &graphic_file_page,
                        &legacy_config,
                        cfg_path,
                        &loaded_graphic_metadata,
                        &has_loaded_graphic_metadata,
                        &view) != FRACTUS_STATUS_OK) {
                    fractus_app_log("runtime: graphic load failed");
                    running = 0;
                }
            } else if (view == FRACTUS_APP_VIEW_LOAD_PALETTE) {
                if (fractus_app_run_load_palette_view(
                        &core.ui_framebuffer,
                        &core.drawing_framebuffer,
                        &fonts,
                        &ui,
                        palette_files,
                        palette_file_count,
                        &palette_file_page,
                        &legacy_config,
                        cfg_path,
                        &view) != FRACTUS_STATUS_OK) {
                    fractus_app_log("runtime: palette load failed");
                    running = 0;
                }
            } else if (view == FRACTUS_APP_VIEW_LOAD_GRAPHIC_PALETTE) {
                if (fractus_app_run_load_graphic_palette_view(
                        &core.ui_framebuffer,
                        &core.drawing_framebuffer,
                        &fonts,
                        &ui,
                        graphic_files,
                        graphic_file_count,
                        &graphic_file_page,
                        &legacy_config,
                        cfg_path,
                        &view) != FRACTUS_STATUS_OK) {
                    fractus_app_log("runtime: graphic palette load failed");
                    running = 0;
                }
            } else if (view == FRACTUS_APP_VIEW_CHANGE_GRAPHIC_PALETTE_GRAPHIC) {
                if (fractus_app_ensure_drawing_framebuffer_size(&legacy_config, &core.drawing_framebuffer, &core.ui_framebuffer) != FRACTUS_STATUS_OK) {
                    fractus_app_log("runtime: drawing framebuffer resize failed");
                    running = 0;
                } else if (fractus_app_run_change_graphic_palette_graphic_view(
                        &platform,
                        &core.ui_framebuffer,
                        &core.drawing_framebuffer,
                        &fonts,
                        &ui,
                        graphic_files,
                        graphic_file_count,
                        &graphic_file_page,
                        palette_files,
                        &palette_file_count,
                        &palette_file_page,
                        selected_graphic_path,
                        sizeof(selected_graphic_path),
                        &legacy_config,
                        cfg_path,
                        &view) != FRACTUS_STATUS_OK) {
                    fractus_app_log("runtime: change graphic palette graphic failed");
                    running = 0;
                }
            } else if (view == FRACTUS_APP_VIEW_CHANGE_GRAPHIC_PALETTE_PALETTE) {
                if (fractus_app_run_change_graphic_palette_palette_view(
                        &platform,
                        &core.ui_framebuffer,
                        &core.drawing_framebuffer,
                        &fonts,
                        &ui,
                        palette_files,
                        palette_file_count,
                        &palette_file_page,
                        selected_graphic_path,
                        &legacy_config,
                        cfg_path,
                        runtime_error_message,
                        sizeof(runtime_error_message),
                        &loaded_graphic_metadata,
                        &has_loaded_graphic_metadata,
                        &view) != FRACTUS_STATUS_OK) {
                    fractus_app_log("runtime: change graphic palette palette failed");
                    running = 0;
                }
            } else if (view == FRACTUS_APP_VIEW_PALETTE_EDIT_SELECT) {
                if (fractus_app_run_palette_edit_select_view(
                        &core.ui_framebuffer,
                        &fonts,
                        &ui,
                        &palette_selected_index,
                        &palette_original_color,
                        &palette_pending_color,
                        &palette_red_field,
                        &palette_green_field,
                        &palette_blue_field,
                        &view) != FRACTUS_STATUS_OK) {
                    fractus_app_log("runtime: palette edit selection failed");
                    running = 0;
                }
            } else if (view == FRACTUS_APP_VIEW_PALETTE_EDIT_COLOR) {
                if (fractus_app_run_palette_edit_color_view(
                        &core.ui_framebuffer,
                        &fonts,
                        &ui,
                        cfg_path,
                        &legacy_config,
                        palette_selected_index,
                        palette_original_color,
                        &palette_pending_color,
                        &palette_red_field,
                        &palette_green_field,
                        &palette_blue_field,
                        &view) != FRACTUS_STATUS_OK) {
                    fractus_app_log("runtime: palette color editor failed");
                    running = 0;
                }
            } else if (view == FRACTUS_APP_VIEW_PALETTE_COPY_SOURCE) {
                if (fractus_app_run_palette_copy_source_view(
                        &core.ui_framebuffer,
                        &fonts,
                        &ui,
                        &palette_copy_color,
                        &palette_copy_source_index,
                        &view) != FRACTUS_STATUS_OK) {
                    fractus_app_log("runtime: palette copy source failed");
                    running = 0;
                }
            } else if (view == FRACTUS_APP_VIEW_PALETTE_COPY_TARGETS) {
                if (fractus_app_run_palette_copy_targets_view(
                        &core.ui_framebuffer,
                        &fonts,
                        &ui,
                        cfg_path,
                        &legacy_config,
                        palette_copy_source_index,
                        palette_copy_color,
                        &view) != FRACTUS_STATUS_OK) {
                    fractus_app_log("runtime: palette copy target failed");
                    running = 0;
                }
            } else if (view == FRACTUS_APP_VIEW_PALETTE_GRADIENT_FIRST) {
                if (fractus_app_run_palette_gradient_first_view(
                        &core.ui_framebuffer,
                        &fonts,
                        &ui,
                        &palette_gradient_first_index,
                        &view) != FRACTUS_STATUS_OK) {
                    fractus_app_log("runtime: palette gradient first failed");
                    running = 0;
                }
            } else if (view == FRACTUS_APP_VIEW_PALETTE_GRADIENT_SECOND) {
                if (fractus_app_run_palette_gradient_second_view(
                        &core.ui_framebuffer,
                        &fonts,
                        &ui,
                        cfg_path,
                        &legacy_config,
                        palette_gradient_first_index,
                        &view) != FRACTUS_STATUS_OK) {
                    fractus_app_log("runtime: palette gradient second failed");
                    running = 0;
                }
            } else if (view == FRACTUS_APP_VIEW_ERROR) {
                if (fractus_app_run_error_view(
                        &core.ui_framebuffer,
                        &fonts,
                        &ui,
                        runtime_error_message,
                        &view) != FRACTUS_STATUS_OK) {
                    fractus_app_log("runtime: error dialog failed");
                    running = 0;
                }
            } else if (view == FRACTUS_APP_VIEW_ABOUT) {
                if (fractus_app_run_about_view(
                        &core.ui_framebuffer,
                        &fonts,
                        &ui,
                        &view) != FRACTUS_STATUS_OK) {
                    fractus_app_log("runtime: about dialog failed");
                    running = 0;
                }
            } else {
                int save_requested_this_frame = save_next_graphic;
                int render_save_next_graphic = save_next_graphic;
                int render_aborted = 0;

                if (fractus_app_view_is_generated_drawing(view) &&
                    save_next_graphic &&
                    !drawing_presented_once) {
                    render_save_next_graphic = 0;
                    save_requested_this_frame = 0;
                }

                {
                    int right_click = ui.press_pending && ui.press_event.buttons.right;
                    int escape_pressed = ui.key_press_pending && ui.key_pressed == 27u;
                    int selection_cancelled = 0;

                    if (fractus_app_view_supports_zone_selection(view) &&
                        zone_selection.active &&
                        (escape_pressed || right_click)) {
                        zone_selection.active = 0;
                        zone_selection.has_first_corner = 0;
                        selection_cancelled = 1;
                    } else if (escape_pressed ||
                        (right_click && !fractus_app_view_supports_zone_selection(view))) {
                        zone_selection.active = 0;
                        zone_selection.has_first_corner = 0;
                        palette_flow_active = 0;
                        view = FRACTUS_APP_VIEW_MAIN_MENU;
                    }

                    if (fractus_app_view_supports_zone_selection(view)) {
                        if (fractus_app_ensure_drawing_framebuffer_size(
                                &legacy_config,
                                &core.drawing_framebuffer,
                                &core.ui_framebuffer) != FRACTUS_STATUS_OK) {
                            fractus_app_log("runtime: drawing framebuffer resize failed");
                            running = 0;
                        } else if (fractus_app_handle_plane_pointer(
                                view,
                                &platform,
                                &core.drawing_framebuffer,
                                &ui,
                                &zone_selection,
                                selection_cancelled,
                                &zoom_history,
                                &plane_drag_active,
                                &plane_drag_moved,
                                &plane_drag_view,
                                &plane_drag_pixel,
                                &mandelbrot_params,
                                &mandelbrot_dem_params,
                                &julia_params,
                                &julia_dem_params,
                                &biomorph_params,
                                &mandelbrot_needs_render,
                                &mandelbrot_dem_needs_render,
                                &julia_needs_render,
                                &julia_dem_needs_render,
                                &biomorph_needs_render)) {
                            drawing_presented_once = 0;
                            save_next_graphic = 0;
                            render_save_next_graphic = 0;
                            save_requested_this_frame = 0;
                        }
                    } else if (plane_drag_active) {
                        fractus_app_commit_plane_drag(
                            &plane_drag_active,
                            &plane_drag_moved,
                            plane_drag_view,
                            &zoom_history,
                            &mandelbrot_params,
                            &mandelbrot_dem_params,
                            &julia_params,
                            &julia_dem_params,
                            &biomorph_params);
                    }
                }

                if (ui.key_press_pending &&
                    (fractus_app_view_is_generated_drawing(view) || view == FRACTUS_APP_VIEW_GRAPHIC)) {
                    int has_assigned_action = 0;
                    uint32_t k = ui.key_pressed;

                    if (k == 27u) {
                        has_assigned_action = 1;
                    } else if (k == 'f' || k == 'F') {
                        has_assigned_action = 1;
                    } else if (k == 'g' || k == 'G') {
                        if (fractus_app_view_is_generated_drawing(view)) {
                            has_assigned_action = 1;
                        }
                    } else if (k == 'm' || k == 'M') {
                        if (fractus_app_view_is_generated_drawing(view) ||
                            (view == FRACTUS_APP_VIEW_GRAPHIC && has_loaded_graphic_metadata)) {
                            has_assigned_action = 1;
                        }
                    } else if (k == 's' || k == 'S') {
                        if (fractus_app_view_supports_zone_selection(view)) {
                            has_assigned_action = 1;
                        }
                    } else if (k == 'i' || k == 'I') {
                        has_assigned_action = 1;
                    } else if (k == 'h' || k == 'H') {
                        has_assigned_action = 1;
                    } else if ((k == 'x' || k == 'X' || k == 'z' || k == 'Z') &&
                               fractus_app_view_supports_zone_selection(view)) {
                        has_assigned_action = 1;
                    }

                    if (!has_assigned_action) {
                        drawing_footer_visible = 1;
                    }
                }

                if (ui.key_press_pending &&
                    (ui.key_pressed == 'f' || ui.key_pressed == 'F') &&
                    (fractus_app_view_is_generated_drawing(view) || view == FRACTUS_APP_VIEW_GRAPHIC)) {
                    palette_flow_active = !palette_flow_active;
                }

                if (ui.key_press_pending &&
                    (ui.key_pressed == 'm' || ui.key_pressed == 'M') &&
                    (fractus_app_view_is_generated_drawing(view) ||
                     (view == FRACTUS_APP_VIEW_GRAPHIC && has_loaded_graphic_metadata))) {
                    graphic_metadata_panel_visible = !graphic_metadata_panel_visible;
                }

                if (ui.key_press_pending &&
                    (ui.key_pressed == 'g' || ui.key_pressed == 'G') &&
                    fractus_app_view_is_generated_drawing(view)) {
                    save_next_graphic = 1;
                    render_save_next_graphic = 1;
                    save_requested_this_frame = 1;
                }

                if (ui.key_press_pending &&
                    (ui.key_pressed == 'i' || ui.key_pressed == 'I') &&
                    (fractus_app_view_is_generated_drawing(view) || view == FRACTUS_APP_VIEW_GRAPHIC)) {
                    drawing_hud_visible = !drawing_hud_visible;
                }

                if (ui.key_press_pending &&
                    (ui.key_pressed == 'h' || ui.key_pressed == 'H') &&
                    (fractus_app_view_is_generated_drawing(view) || view == FRACTUS_APP_VIEW_GRAPHIC)) {
                    drawing_footer_visible = !drawing_footer_visible;
                }

                if (ui.key_press_pending &&
                    (ui.key_pressed == 'x' || ui.key_pressed == 'X' ||
                     ui.key_pressed == 'z' || ui.key_pressed == 'Z') &&
                    fractus_app_view_supports_zone_selection(view)) {
                    if (plane_drag_active) {
                        fractus_app_commit_plane_drag(
                            &plane_drag_active,
                            &plane_drag_moved,
                            plane_drag_view,
                            &zoom_history,
                            &mandelbrot_params,
                            &mandelbrot_dem_params,
                            &julia_params,
                            &julia_dem_params,
                            &biomorph_params);
                    }
                }

                if (ui.key_press_pending &&
                    (ui.key_pressed == 'x' || ui.key_pressed == 'X' ||
                     ui.key_pressed == 'z' || ui.key_pressed == 'Z') &&
                    fractus_app_view_supports_zone_selection(view) &&
                    fractus_app_recall_zoom_history(
                        &zoom_history,
                        ui.key_pressed == 'z' || ui.key_pressed == 'Z',
                        view,
                        &mandelbrot_params,
                        &mandelbrot_dem_params,
                        &julia_params,
                        &julia_dem_params,
                        &biomorph_params,
                        &mandelbrot_needs_render,
                        &mandelbrot_dem_needs_render,
                        &julia_needs_render,
                        &julia_dem_needs_render,
                        &biomorph_needs_render)) {
                    zone_selection.active = 0;
                    zone_selection.has_first_corner = 0;
                    drawing_presented_once = 0;
                    save_next_graphic = 0;
                    render_save_next_graphic = 0;
                    save_requested_this_frame = 0;
                    drawing_footer_visible = 0;
                    graphic_metadata_panel_visible = 0;
                }

                if (view == FRACTUS_APP_VIEW_MANDELBROT) {
                    fractus_size_u32 previous_drawing_size = core.drawing_framebuffer.size;
                    present_framebuffer = &core.drawing_framebuffer;
                    present_is_drawing = 1;
                    if (fractus_app_ensure_drawing_framebuffer_size(&legacy_config, &core.drawing_framebuffer, &core.ui_framebuffer) != FRACTUS_STATUS_OK) {
                        fractus_app_log("runtime: drawing framebuffer resize failed");
                        running = 0;
                    } else {
                        if (previous_drawing_size.width != core.drawing_framebuffer.size.width ||
                            previous_drawing_size.height != core.drawing_framebuffer.size.height) {
                            mandelbrot_needs_render = 1;
                        }
                        if (fractus_app_handle_zone_selection_input(
                                &platform,
                                &core.drawing_framebuffer,
                                &ui,
                                &mandelbrot_params.xmin,
                                &mandelbrot_params.xmax,
                                &mandelbrot_params.ymin,
                                &mandelbrot_params.ymax,
                                &mandelbrot_params.precise,
                                &zone_selection,
                                &zoom_history)) {
                            drawing_presented_once = 0;
                            save_next_graphic = 0;
                            render_save_next_graphic = 0;
                            save_requested_this_frame = 0;
                            drawing_footer_visible = 0;
                            graphic_metadata_panel_visible = 0;
                            mandelbrot_needs_render = 1;
                        }
                    }
                    if (running) {
                        if (mandelbrot_needs_render) {
                            fractus_app_render_feedback render_feedback;
                            fractus_render_progress render_progress;

                            render_started_ms = fractus_platform_get_ticks_ms(&platform);
                            fractus_app_begin_render_feedback(
                                &render_feedback,
                                &render_progress,
                                &platform,
                                &legacy_config,
                                &core.drawing_framebuffer,
                                &zone_overlay_framebuffer,
                                &fonts,
                                render_started_ms);
                            if (!fractus_app_finish_render(
                                    fractus_app_render_mandelbrot(
                                        &platform,
                                        &core.drawing_framebuffer,
                                        &fonts,
                                        &mandelbrot_params,
                                        &render_save_next_graphic,
                                        runtime_error_message,
                                        sizeof(runtime_error_message),
                                        last_saved_filename,
                                        sizeof(last_saved_filename),
                                        &render_progress),
                                    &render_feedback,
                                    FRACTUS_APP_VIEW_MANDELBROT_CONFIG,
                                    &mandelbrot_needs_render,
                                    &view,
                                    &save_next_graphic,
                                    &render_save_next_graphic,
                                    &save_requested_this_frame,
                                    &present_is_drawing,
                                    &present_framebuffer,
                                    &core.ui_framebuffer,
                                    &render_aborted,
                                    &swallow_escape,
                                    &platform,
                                    render_started_ms,
                                    &last_render_ms,
                                    &has_render_time,
                                    "runtime: mandelbrot render failed")) {
                                running = 0;
                            }
                            if (render_aborted) {
                                mandelbrot_pending = mandelbrot_params;
                                fractus_app_init_mandelbrot_fields(
                                    &mandelbrot_fields,
                                    mandelbrot_pending.xmin,
                                    mandelbrot_pending.xmax,
                                    mandelbrot_pending.ymin,
                                    mandelbrot_pending.ymax,
                                    mandelbrot_pending.max_iterations,
                                    mandelbrot_pending.escape_radius_squared);
                            }
                        } else if (render_save_next_graphic) {
                            fractus_graphic_metadata metadata = fractus_graphic_metadata_from_mandelbrot(
                                &mandelbrot_params,
                                core.drawing_framebuffer.size.width,
                                core.drawing_framebuffer.size.height);
                            if (fractus_app_save_next_graphic_if_requested_ex(
                                    &platform,
                                    &core.drawing_framebuffer,
                                    &metadata,
                                    &render_save_next_graphic,
                                    runtime_error_message,
                                    sizeof(runtime_error_message),
                                    last_saved_filename,
                                    sizeof(last_saved_filename)) != FRACTUS_STATUS_OK) {
                                fractus_app_log("runtime: mandelbrot save failed");
                                running = 0;
                            }
                        }
                    }
                    if (running && !render_aborted && (drawing_footer_visible || zone_selection.active || graphic_metadata_panel_visible)) {
                        if (fractus_app_copy_framebuffer_for_overlay(&zone_overlay_framebuffer, &core.drawing_framebuffer) != FRACTUS_STATUS_OK) {
                            fractus_app_log("runtime: mandelbrot selection overlay failed");
                            running = 0;
                        } else {
                            if (drawing_footer_visible) {
                                (void)fractus_app_draw_drawing_footer_ex(
                                    &zone_overlay_framebuffer,
                                    &fonts,
                                    "ESC: menú - G: grabar - S: sel. zona - derecho: desplazar - rueda: zoom - X: retroceder - Z: avanzar - M: metadatos - F: flujo - I: información",
                                    last_saved_filename,
                                    1);
                            }
                            if (zone_selection.active) {
                                (void)fractus_app_draw_zone_selection_overlay(&platform, &zone_overlay_framebuffer, &ui, &zone_selection);
                            }
                            if (graphic_metadata_panel_visible) {
                                fractus_graphic_metadata metadata = fractus_graphic_metadata_from_mandelbrot(
                                    &mandelbrot_params,
                                    core.drawing_framebuffer.size.width,
                                    core.drawing_framebuffer.size.height);
                                (void)fractus_app_draw_graphic_metadata_panel(&zone_overlay_framebuffer, &fonts, &metadata, drawing_footer_visible);
                            }
                            present_framebuffer = &zone_overlay_framebuffer;
                            present_is_drawing = 1;
                        }
                    }
                } else if (view == FRACTUS_APP_VIEW_MANDELBROT_DEM) {
                    fractus_size_u32 previous_drawing_size = core.drawing_framebuffer.size;
                    present_framebuffer = &core.drawing_framebuffer;
                    present_is_drawing = 1;
                    if (fractus_app_ensure_drawing_framebuffer_size(&legacy_config, &core.drawing_framebuffer, &core.ui_framebuffer) != FRACTUS_STATUS_OK) {
                        fractus_app_log("runtime: drawing framebuffer resize failed");
                        running = 0;
                    } else {
                        if (previous_drawing_size.width != core.drawing_framebuffer.size.width ||
                            previous_drawing_size.height != core.drawing_framebuffer.size.height) {
                            mandelbrot_dem_needs_render = 1;
                        }
                        if (fractus_app_handle_zone_selection_input(
                                &platform,
                                &core.drawing_framebuffer,
                                &ui,
                                &mandelbrot_dem_params.xmin,
                                &mandelbrot_dem_params.xmax,
                                &mandelbrot_dem_params.ymin,
                                &mandelbrot_dem_params.ymax,
                                &mandelbrot_dem_params.precise,
                                &zone_selection,
                                &zoom_history)) {
                            drawing_presented_once = 0;
                            save_next_graphic = 0;
                            render_save_next_graphic = 0;
                            save_requested_this_frame = 0;
                            drawing_footer_visible = 0;
                            graphic_metadata_panel_visible = 0;
                            mandelbrot_dem_needs_render = 1;
                        }
                    }
                    if (running) {
                        if (mandelbrot_dem_needs_render) {
                            fractus_app_render_feedback render_feedback;
                            fractus_render_progress render_progress;

                            render_started_ms = fractus_platform_get_ticks_ms(&platform);
                            fractus_app_begin_render_feedback(
                                &render_feedback,
                                &render_progress,
                                &platform,
                                &legacy_config,
                                &core.drawing_framebuffer,
                                &zone_overlay_framebuffer,
                                &fonts,
                                render_started_ms);
                            if (!fractus_app_finish_render(
                                    fractus_app_render_mandelbrot_dem(
                                        &platform,
                                        &core.drawing_framebuffer,
                                        &fonts,
                                        &mandelbrot_dem_params,
                                        &render_save_next_graphic,
                                        runtime_error_message,
                                        sizeof(runtime_error_message),
                                        last_saved_filename,
                                        sizeof(last_saved_filename),
                                        &render_progress),
                                    &render_feedback,
                                    FRACTUS_APP_VIEW_MANDELBROT_DEM_CONFIG,
                                    &mandelbrot_dem_needs_render,
                                    &view,
                                    &save_next_graphic,
                                    &render_save_next_graphic,
                                    &save_requested_this_frame,
                                    &present_is_drawing,
                                    &present_framebuffer,
                                    &core.ui_framebuffer,
                                    &render_aborted,
                                    &swallow_escape,
                                    &platform,
                                    render_started_ms,
                                    &last_render_ms,
                                    &has_render_time,
                                    "runtime: mandelbrot dem render failed")) {
                                running = 0;
                            }
                            if (render_aborted) {
                                mandelbrot_dem_pending = mandelbrot_dem_params;
                                fractus_app_init_mandelbrot_fields(
                                    &mandelbrot_dem_fields,
                                    mandelbrot_dem_pending.xmin,
                                    mandelbrot_dem_pending.xmax,
                                    mandelbrot_dem_pending.ymin,
                                    mandelbrot_dem_pending.ymax,
                                    mandelbrot_dem_pending.max_iterations,
                                    mandelbrot_dem_pending.escape_radius_squared);
                            }
                        } else if (render_save_next_graphic) {
                            fractus_graphic_metadata metadata = fractus_graphic_metadata_from_mandelbrot_dem(
                                &mandelbrot_dem_params,
                                core.drawing_framebuffer.size.width,
                                core.drawing_framebuffer.size.height);
                            if (fractus_app_save_next_graphic_if_requested_ex(
                                    &platform,
                                    &core.drawing_framebuffer,
                                    &metadata,
                                    &render_save_next_graphic,
                                    runtime_error_message,
                                    sizeof(runtime_error_message),
                                    last_saved_filename,
                                    sizeof(last_saved_filename)) != FRACTUS_STATUS_OK) {
                                fractus_app_log("runtime: mandelbrot dem save failed");
                                running = 0;
                            }
                        }
                    }
                    if (running && !render_aborted && (drawing_footer_visible || zone_selection.active || graphic_metadata_panel_visible)) {
                        if (fractus_app_copy_framebuffer_for_overlay(&zone_overlay_framebuffer, &core.drawing_framebuffer) != FRACTUS_STATUS_OK) {
                            fractus_app_log("runtime: mandelbrot dem selection overlay failed");
                            running = 0;
                        } else {
                            if (drawing_footer_visible) {
                                (void)fractus_app_draw_drawing_footer_ex(
                                    &zone_overlay_framebuffer,
                                    &fonts,
                                    "ESC: menú - G: grabar - S: sel. zona - derecho: desplazar - rueda: zoom - X: retroceder - Z: avanzar - M: metadatos - F: flujo - I: información",
                                    last_saved_filename,
                                    1);
                            }
                            if (zone_selection.active) {
                                (void)fractus_app_draw_zone_selection_overlay(&platform, &zone_overlay_framebuffer, &ui, &zone_selection);
                            }
                            if (graphic_metadata_panel_visible) {
                                fractus_graphic_metadata metadata = fractus_graphic_metadata_from_mandelbrot_dem(
                                    &mandelbrot_dem_params,
                                    core.drawing_framebuffer.size.width,
                                    core.drawing_framebuffer.size.height);
                                (void)fractus_app_draw_graphic_metadata_panel(&zone_overlay_framebuffer, &fonts, &metadata, drawing_footer_visible);
                            }
                            present_framebuffer = &zone_overlay_framebuffer;
                            present_is_drawing = 1;
                        }
                    }
                } else if (view == FRACTUS_APP_VIEW_JULIA) {
                    fractus_size_u32 previous_drawing_size = core.drawing_framebuffer.size;
                    present_framebuffer = &core.drawing_framebuffer;
                    present_is_drawing = 1;
                    if (fractus_app_ensure_drawing_framebuffer_size(&legacy_config, &core.drawing_framebuffer, &core.ui_framebuffer) != FRACTUS_STATUS_OK) {
                        fractus_app_log("runtime: drawing framebuffer resize failed");
                        running = 0;
                    } else {
                        if (previous_drawing_size.width != core.drawing_framebuffer.size.width ||
                            previous_drawing_size.height != core.drawing_framebuffer.size.height) {
                            julia_needs_render = 1;
                        }
                        if (fractus_app_handle_zone_selection_input(
                                &platform,
                                &core.drawing_framebuffer,
                                &ui,
                                &julia_params.xmin,
                                &julia_params.xmax,
                                &julia_params.ymin,
                                &julia_params.ymax,
                                &julia_params.precise,
                                &zone_selection,
                                &zoom_history)) {
                            drawing_presented_once = 0;
                            save_next_graphic = 0;
                            render_save_next_graphic = 0;
                            save_requested_this_frame = 0;
                            drawing_footer_visible = 0;
                            graphic_metadata_panel_visible = 0;
                            julia_needs_render = 1;
                        }
                    }
                    if (running) {
                        if (julia_needs_render) {
                            fractus_app_render_feedback render_feedback;
                            fractus_render_progress render_progress;

                            render_started_ms = fractus_platform_get_ticks_ms(&platform);
                            fractus_app_begin_render_feedback(
                                &render_feedback,
                                &render_progress,
                                &platform,
                                &legacy_config,
                                &core.drawing_framebuffer,
                                &zone_overlay_framebuffer,
                                &fonts,
                                render_started_ms);
                            if (!fractus_app_finish_render(
                                    fractus_app_render_julia(
                                        &platform,
                                        &core.drawing_framebuffer,
                                        &fonts,
                                        &julia_params,
                                        &render_save_next_graphic,
                                        runtime_error_message,
                                        sizeof(runtime_error_message),
                                        last_saved_filename,
                                        sizeof(last_saved_filename),
                                        &render_progress),
                                    &render_feedback,
                                    FRACTUS_APP_VIEW_JULIA_CONFIG,
                                    &julia_needs_render,
                                    &view,
                                    &save_next_graphic,
                                    &render_save_next_graphic,
                                    &save_requested_this_frame,
                                    &present_is_drawing,
                                    &present_framebuffer,
                                    &core.ui_framebuffer,
                                    &render_aborted,
                                    &swallow_escape,
                                    &platform,
                                    render_started_ms,
                                    &last_render_ms,
                                    &has_render_time,
                                    "runtime: julia render failed")) {
                                running = 0;
                            }
                            if (render_aborted) {
                                julia_pending = julia_params;
                                fractus_app_init_julia_fields(
                                    &julia_fields,
                                    julia_pending.xmin,
                                    julia_pending.xmax,
                                    julia_pending.ymin,
                                    julia_pending.ymax,
                                    julia_pending.constant_real,
                                    julia_pending.constant_imag,
                                    julia_pending.max_iterations,
                                    julia_pending.escape_radius_squared);
                            }
                        } else if (render_save_next_graphic) {
                            fractus_graphic_metadata metadata = fractus_graphic_metadata_from_julia(
                                &julia_params,
                                core.drawing_framebuffer.size.width,
                                core.drawing_framebuffer.size.height);
                            if (fractus_app_save_next_graphic_if_requested_ex(
                                    &platform,
                                    &core.drawing_framebuffer,
                                    &metadata,
                                    &render_save_next_graphic,
                                    runtime_error_message,
                                    sizeof(runtime_error_message),
                                    last_saved_filename,
                                    sizeof(last_saved_filename)) != FRACTUS_STATUS_OK) {
                                fractus_app_log("runtime: julia save failed");
                                running = 0;
                            }
                        }
                    }
                    if (running && !render_aborted && (drawing_footer_visible || zone_selection.active || graphic_metadata_panel_visible)) {
                        if (fractus_app_copy_framebuffer_for_overlay(&zone_overlay_framebuffer, &core.drawing_framebuffer) != FRACTUS_STATUS_OK) {
                            fractus_app_log("runtime: julia selection overlay failed");
                            running = 0;
                        } else {
                            if (drawing_footer_visible) {
                                (void)fractus_app_draw_drawing_footer_ex(
                                    &zone_overlay_framebuffer,
                                    &fonts,
                                    "ESC: menú - G: grabar - S: sel. zona - derecho: desplazar - rueda: zoom - X: retroceder - Z: avanzar - M: metadatos - F: flujo - I: información",
                                    last_saved_filename,
                                    1);
                            }
                            if (zone_selection.active) {
                                (void)fractus_app_draw_zone_selection_overlay(&platform, &zone_overlay_framebuffer, &ui, &zone_selection);
                            }
                            if (graphic_metadata_panel_visible) {
                                fractus_graphic_metadata metadata = fractus_graphic_metadata_from_julia(
                                    &julia_params,
                                    core.drawing_framebuffer.size.width,
                                    core.drawing_framebuffer.size.height);
                                (void)fractus_app_draw_graphic_metadata_panel(&zone_overlay_framebuffer, &fonts, &metadata, drawing_footer_visible);
                            }
                            present_framebuffer = &zone_overlay_framebuffer;
                            present_is_drawing = 1;
                        }
                    }
                } else if (view == FRACTUS_APP_VIEW_JULIA_DEM) {
                    fractus_size_u32 previous_drawing_size = core.drawing_framebuffer.size;
                    present_framebuffer = &core.drawing_framebuffer;
                    present_is_drawing = 1;
                    if (fractus_app_ensure_drawing_framebuffer_size(&legacy_config, &core.drawing_framebuffer, &core.ui_framebuffer) != FRACTUS_STATUS_OK) {
                        fractus_app_log("runtime: drawing framebuffer resize failed");
                        running = 0;
                    } else {
                        if (previous_drawing_size.width != core.drawing_framebuffer.size.width ||
                            previous_drawing_size.height != core.drawing_framebuffer.size.height) {
                            julia_dem_needs_render = 1;
                        }
                        if (fractus_app_handle_zone_selection_input(
                                &platform,
                                &core.drawing_framebuffer,
                                &ui,
                                &julia_dem_params.xmin,
                                &julia_dem_params.xmax,
                                &julia_dem_params.ymin,
                                &julia_dem_params.ymax,
                                &julia_dem_params.precise,
                                &zone_selection,
                                &zoom_history)) {
                            drawing_presented_once = 0;
                            save_next_graphic = 0;
                            render_save_next_graphic = 0;
                            save_requested_this_frame = 0;
                            drawing_footer_visible = 0;
                            graphic_metadata_panel_visible = 0;
                            julia_dem_needs_render = 1;
                        }
                    }
                    if (running) {
                        if (julia_dem_needs_render) {
                            fractus_app_render_feedback render_feedback;
                            fractus_render_progress render_progress;

                            render_started_ms = fractus_platform_get_ticks_ms(&platform);
                            fractus_app_begin_render_feedback(
                                &render_feedback,
                                &render_progress,
                                &platform,
                                &legacy_config,
                                &core.drawing_framebuffer,
                                &zone_overlay_framebuffer,
                                &fonts,
                                render_started_ms);
                            if (!fractus_app_finish_render(
                                    fractus_app_render_julia_dem(
                                        &platform,
                                        &core.drawing_framebuffer,
                                        &fonts,
                                        &julia_dem_params,
                                        &render_save_next_graphic,
                                        runtime_error_message,
                                        sizeof(runtime_error_message),
                                        last_saved_filename,
                                        sizeof(last_saved_filename),
                                        &render_progress),
                                    &render_feedback,
                                    FRACTUS_APP_VIEW_JULIA_DEM_CONFIG,
                                    &julia_dem_needs_render,
                                    &view,
                                    &save_next_graphic,
                                    &render_save_next_graphic,
                                    &save_requested_this_frame,
                                    &present_is_drawing,
                                    &present_framebuffer,
                                    &core.ui_framebuffer,
                                    &render_aborted,
                                    &swallow_escape,
                                    &platform,
                                    render_started_ms,
                                    &last_render_ms,
                                    &has_render_time,
                                    "runtime: julia dem render failed")) {
                                running = 0;
                            }
                            if (render_aborted) {
                                julia_dem_pending = julia_dem_params;
                                fractus_app_init_julia_fields(
                                    &julia_dem_fields,
                                    julia_dem_pending.xmin,
                                    julia_dem_pending.xmax,
                                    julia_dem_pending.ymin,
                                    julia_dem_pending.ymax,
                                    julia_dem_pending.constant_real,
                                    julia_dem_pending.constant_imag,
                                    julia_dem_pending.max_iterations,
                                    julia_dem_pending.escape_radius_squared);
                            }
                        } else if (render_save_next_graphic) {
                            fractus_graphic_metadata metadata = fractus_graphic_metadata_from_julia_dem(
                                &julia_dem_params,
                                core.drawing_framebuffer.size.width,
                                core.drawing_framebuffer.size.height);
                            if (fractus_app_save_next_graphic_if_requested_ex(
                                    &platform,
                                    &core.drawing_framebuffer,
                                    &metadata,
                                    &render_save_next_graphic,
                                    runtime_error_message,
                                    sizeof(runtime_error_message),
                                    last_saved_filename,
                                    sizeof(last_saved_filename)) != FRACTUS_STATUS_OK) {
                                fractus_app_log("runtime: julia dem save failed");
                                running = 0;
                            }
                        }
                    }
                    if (running && !render_aborted && (drawing_footer_visible || zone_selection.active || graphic_metadata_panel_visible)) {
                        if (fractus_app_copy_framebuffer_for_overlay(&zone_overlay_framebuffer, &core.drawing_framebuffer) != FRACTUS_STATUS_OK) {
                            fractus_app_log("runtime: julia dem selection overlay failed");
                            running = 0;
                        } else {
                            if (drawing_footer_visible) {
                                (void)fractus_app_draw_drawing_footer_ex(
                                    &zone_overlay_framebuffer,
                                    &fonts,
                                    "ESC: menú - G: grabar - S: sel. zona - derecho: desplazar - rueda: zoom - X: retroceder - Z: avanzar - M: metadatos - F: flujo - I: información",
                                    last_saved_filename,
                                    1);
                            }
                            if (zone_selection.active) {
                                (void)fractus_app_draw_zone_selection_overlay(&platform, &zone_overlay_framebuffer, &ui, &zone_selection);
                            }
                            if (graphic_metadata_panel_visible) {
                                fractus_graphic_metadata metadata = fractus_graphic_metadata_from_julia_dem(
                                    &julia_dem_params,
                                    core.drawing_framebuffer.size.width,
                                    core.drawing_framebuffer.size.height);
                                (void)fractus_app_draw_graphic_metadata_panel(&zone_overlay_framebuffer, &fonts, &metadata, drawing_footer_visible);
                            }
                            present_framebuffer = &zone_overlay_framebuffer;
                            present_is_drawing = 1;
                        }
                    }
                } else if (view == FRACTUS_APP_VIEW_BIOMORPH) {
                    fractus_size_u32 previous_drawing_size = core.drawing_framebuffer.size;
                    present_framebuffer = &core.drawing_framebuffer;
                    present_is_drawing = 1;
                    if (fractus_app_ensure_drawing_framebuffer_size(&legacy_config, &core.drawing_framebuffer, &core.ui_framebuffer) != FRACTUS_STATUS_OK) {
                        fractus_app_log("runtime: drawing framebuffer resize failed");
                        running = 0;
                    } else {
                        if (previous_drawing_size.width != core.drawing_framebuffer.size.width ||
                            previous_drawing_size.height != core.drawing_framebuffer.size.height) {
                            biomorph_needs_render = 1;
                        }
                        if (fractus_app_handle_zone_selection_input(
                                &platform,
                                &core.drawing_framebuffer,
                                &ui,
                                &biomorph_params.xmin,
                                &biomorph_params.xmax,
                                &biomorph_params.ymin,
                                &biomorph_params.ymax,
                                &biomorph_params.precise,
                                &zone_selection,
                                &zoom_history)) {
                            drawing_presented_once = 0;
                            save_next_graphic = 0;
                            render_save_next_graphic = 0;
                            save_requested_this_frame = 0;
                            drawing_footer_visible = 0;
                            graphic_metadata_panel_visible = 0;
                            biomorph_needs_render = 1;
                        }
                    }
                    if (running) {
                        if (biomorph_needs_render) {
                            fractus_app_render_feedback render_feedback;
                            fractus_render_progress render_progress;

                            render_started_ms = fractus_platform_get_ticks_ms(&platform);
                            fractus_app_begin_render_feedback(
                                &render_feedback,
                                &render_progress,
                                &platform,
                                &legacy_config,
                                &core.drawing_framebuffer,
                                &zone_overlay_framebuffer,
                                &fonts,
                                render_started_ms);
                            if (!fractus_app_finish_render(
                                    fractus_app_render_biomorph(
                                        &platform,
                                        &core.drawing_framebuffer,
                                        &fonts,
                                        &biomorph_params,
                                        &render_save_next_graphic,
                                        runtime_error_message,
                                        sizeof(runtime_error_message),
                                        last_saved_filename,
                                        sizeof(last_saved_filename),
                                        &render_progress),
                                    &render_feedback,
                                    FRACTUS_APP_VIEW_BIOMORPH_CONFIG,
                                    &biomorph_needs_render,
                                    &view,
                                    &save_next_graphic,
                                    &render_save_next_graphic,
                                    &save_requested_this_frame,
                                    &present_is_drawing,
                                    &present_framebuffer,
                                    &core.ui_framebuffer,
                                    &render_aborted,
                                    &swallow_escape,
                                    &platform,
                                    render_started_ms,
                                    &last_render_ms,
                                    &has_render_time,
                                    "runtime: biomorph render failed")) {
                                running = 0;
                            }
                            if (render_aborted) {
                                biomorph_pending = biomorph_params;
                                fractus_app_init_biomorph_fields(
                                    &biomorph_fields,
                                    biomorph_pending.xmin,
                                    biomorph_pending.xmax,
                                    biomorph_pending.ymin,
                                    biomorph_pending.ymax,
                                    biomorph_pending.constant_real,
                                    biomorph_pending.constant_imag,
                                    biomorph_pending.max_iterations,
                                    biomorph_pending.escape_radius_squared,
                                    biomorph_pending.cutoff);
                            }
                        } else if (render_save_next_graphic) {
                            fractus_graphic_metadata metadata = fractus_graphic_metadata_from_biomorph(
                                &biomorph_params,
                                core.drawing_framebuffer.size.width,
                                core.drawing_framebuffer.size.height);
                            if (fractus_app_save_next_graphic_if_requested_ex(
                                    &platform,
                                    &core.drawing_framebuffer,
                                    &metadata,
                                    &render_save_next_graphic,
                                    runtime_error_message,
                                    sizeof(runtime_error_message),
                                    last_saved_filename,
                                    sizeof(last_saved_filename)) != FRACTUS_STATUS_OK) {
                                fractus_app_log("runtime: biomorph save failed");
                                running = 0;
                            }
                        }
                    }
                    if (running && !render_aborted && (drawing_footer_visible || zone_selection.active || graphic_metadata_panel_visible)) {
                        if (fractus_app_copy_framebuffer_for_overlay(&zone_overlay_framebuffer, &core.drawing_framebuffer) != FRACTUS_STATUS_OK) {
                            fractus_app_log("runtime: biomorph overlay copy failed");
                            running = 0;
                        } else {
                            if (drawing_footer_visible) {
                                (void)fractus_app_draw_drawing_footer_ex(
                                    &zone_overlay_framebuffer,
                                    &fonts,
                                    "ESC: menú - G: grabar - S: sel. zona - derecho: desplazar - rueda: zoom - X: retroceder - Z: avanzar - M: metadatos - F: flujo - I: información",
                                    last_saved_filename,
                                    1);
                            }
                            if (zone_selection.active) {
                                (void)fractus_app_draw_zone_selection_overlay(&platform, &zone_overlay_framebuffer, &ui, &zone_selection);
                            }
                            if (graphic_metadata_panel_visible) {
                                fractus_graphic_metadata metadata = fractus_graphic_metadata_from_biomorph(
                                    &biomorph_params,
                                    core.drawing_framebuffer.size.width,
                                    core.drawing_framebuffer.size.height);
                                (void)fractus_app_draw_graphic_metadata_panel(&zone_overlay_framebuffer, &fonts, &metadata, drawing_footer_visible);
                            }
                            present_framebuffer = &zone_overlay_framebuffer;
                            present_is_drawing = 1;
                        }
                    }
                } else if (view == FRACTUS_APP_VIEW_PLASMA_RECTANGULAR) {
                    fractus_size_u32 previous_drawing_size = core.drawing_framebuffer.size;
                    zone_selection.active = 0;
                    zone_selection.has_first_corner = 0;
                    present_framebuffer = &core.drawing_framebuffer;
                    present_is_drawing = 1;
                    if (fractus_app_ensure_drawing_framebuffer_size(&legacy_config, &core.drawing_framebuffer, &core.ui_framebuffer) != FRACTUS_STATUS_OK) {
                        fractus_app_log("runtime: drawing framebuffer resize failed");
                        running = 0;
                    } else {
                        if (previous_drawing_size.width != core.drawing_framebuffer.size.width ||
                            previous_drawing_size.height != core.drawing_framebuffer.size.height) {
                            plasma_rectangular_needs_render = 1;
                        }
                    }
                    if (running) {
                        if (plasma_rectangular_needs_render) {
                            render_started_ms = fractus_platform_get_ticks_ms(&platform);
                            if (fractus_app_render_plasma_rectangular(
                                    &platform,
                                    &core.drawing_framebuffer,
                                    &fonts,
                                    &plasma_rectangular_params,
                                    &render_save_next_graphic,
                                    runtime_error_message,
                                    sizeof(runtime_error_message),
                                    last_saved_filename,
                                    sizeof(last_saved_filename)) != FRACTUS_STATUS_OK) {
                                fractus_app_log("runtime: plasma rectangular render failed");
                                running = 0;
                            } else {
                                plasma_rectangular_needs_render = 0;
                                fractus_app_note_render_time(&platform, render_started_ms, &last_render_ms, &has_render_time);
                            }
                        } else if (render_save_next_graphic) {
                            fractus_graphic_metadata metadata = fractus_graphic_metadata_from_plasma_rectangular(
                                &plasma_rectangular_params,
                                core.drawing_framebuffer.size.width,
                                core.drawing_framebuffer.size.height);
                            if (fractus_app_save_next_graphic_if_requested_ex(
                                    &platform,
                                    &core.drawing_framebuffer,
                                    &metadata,
                                    &render_save_next_graphic,
                                    runtime_error_message,
                                    sizeof(runtime_error_message),
                                    last_saved_filename,
                                    sizeof(last_saved_filename)) != FRACTUS_STATUS_OK) {
                                fractus_app_log("runtime: plasma rectangular save failed");
                                running = 0;
                            }
                        }
                    }
                    if (running && (drawing_footer_visible || graphic_metadata_panel_visible)) {
                        if (fractus_app_copy_framebuffer_for_overlay(&zone_overlay_framebuffer, &core.drawing_framebuffer) != FRACTUS_STATUS_OK) {
                            fractus_app_log("runtime: plasma rectangular overlay copy failed");
                            running = 0;
                        } else {
                            if (drawing_footer_visible) {
                                (void)fractus_app_draw_drawing_footer_ex(
                                    &zone_overlay_framebuffer,
                                    &fonts,
                                    "ESC o botón derecho: menú - G: grabar - M: metadatos - F: flujo - I: información",
                                    last_saved_filename,
                                    0);
                            }
                            if (graphic_metadata_panel_visible) {
                                fractus_graphic_metadata metadata = fractus_graphic_metadata_from_plasma_rectangular(
                                    &plasma_rectangular_params,
                                    core.drawing_framebuffer.size.width,
                                    core.drawing_framebuffer.size.height);
                                (void)fractus_app_draw_graphic_metadata_panel(&zone_overlay_framebuffer, &fonts, &metadata, drawing_footer_visible);
                            }
                            present_framebuffer = &zone_overlay_framebuffer;
                            present_is_drawing = 1;
                        }
                    }
                } else if (view == FRACTUS_APP_VIEW_PLASMA_CIRCULAR) {
                    fractus_size_u32 previous_drawing_size = core.drawing_framebuffer.size;
                    zone_selection.active = 0;
                    zone_selection.has_first_corner = 0;
                    present_framebuffer = &core.drawing_framebuffer;
                    present_is_drawing = 1;
                    if (fractus_app_ensure_drawing_framebuffer_size(&legacy_config, &core.drawing_framebuffer, &core.ui_framebuffer) != FRACTUS_STATUS_OK) {
                        fractus_app_log("runtime: drawing framebuffer resize failed");
                        running = 0;
                    } else {
                        if (previous_drawing_size.width != core.drawing_framebuffer.size.width ||
                            previous_drawing_size.height != core.drawing_framebuffer.size.height) {
                            plasma_circular_needs_render = 1;
                        }
                    }
                    if (running) {
                        if (plasma_circular_needs_render) {
                            render_started_ms = fractus_platform_get_ticks_ms(&platform);
                            if (fractus_app_render_plasma_circular(
                                    &platform,
                                    &core.drawing_framebuffer,
                                    &fonts,
                                    &plasma_circular_params,
                                    &render_save_next_graphic,
                                    runtime_error_message,
                                    sizeof(runtime_error_message),
                                    last_saved_filename,
                                    sizeof(last_saved_filename)) != FRACTUS_STATUS_OK) {
                                fractus_app_log("runtime: circular plasma render failed");
                                running = 0;
                            } else {
                                plasma_circular_needs_render = 0;
                                fractus_app_note_render_time(&platform, render_started_ms, &last_render_ms, &has_render_time);
                            }
                        } else if (render_save_next_graphic) {
                            fractus_graphic_metadata metadata = fractus_graphic_metadata_from_plasma_circular(
                                &plasma_circular_params,
                                core.drawing_framebuffer.size.width,
                                core.drawing_framebuffer.size.height);
                            if (fractus_app_save_next_graphic_if_requested_ex(
                                    &platform,
                                    &core.drawing_framebuffer,
                                    &metadata,
                                    &render_save_next_graphic,
                                    runtime_error_message,
                                    sizeof(runtime_error_message),
                                    last_saved_filename,
                                    sizeof(last_saved_filename)) != FRACTUS_STATUS_OK) {
                                fractus_app_log("runtime: circular plasma save failed");
                                running = 0;
                            }
                        }
                    }
                    if (running && (drawing_footer_visible || graphic_metadata_panel_visible)) {
                        if (fractus_app_copy_framebuffer_for_overlay(&zone_overlay_framebuffer, &core.drawing_framebuffer) != FRACTUS_STATUS_OK) {
                            fractus_app_log("runtime: plasma circular overlay copy failed");
                            running = 0;
                        } else {
                            if (drawing_footer_visible) {
                                (void)fractus_app_draw_drawing_footer_ex(
                                    &zone_overlay_framebuffer,
                                    &fonts,
                                    "ESC o botón derecho: menú - G: grabar - M: metadatos - F: flujo - I: información",
                                    last_saved_filename,
                                    0);
                            }
                            if (graphic_metadata_panel_visible) {
                                fractus_graphic_metadata metadata = fractus_graphic_metadata_from_plasma_circular(
                                    &plasma_circular_params,
                                    core.drawing_framebuffer.size.width,
                                    core.drawing_framebuffer.size.height);
                                (void)fractus_app_draw_graphic_metadata_panel(&zone_overlay_framebuffer, &fonts, &metadata, drawing_footer_visible);
                            }
                            present_framebuffer = &zone_overlay_framebuffer;
                            present_is_drawing = 1;
                        }
                    }
                } else if (view == FRACTUS_APP_VIEW_LORENZ) {
                    fractus_size_u32 previous_drawing_size = core.drawing_framebuffer.size;
                    zone_selection.active = 0;
                    zone_selection.has_first_corner = 0;
                    present_framebuffer = &core.drawing_framebuffer;
                    present_is_drawing = 1;
                    if (fractus_app_ensure_drawing_framebuffer_size(&legacy_config, &core.drawing_framebuffer, &core.ui_framebuffer) != FRACTUS_STATUS_OK) {
                        fractus_app_log("runtime: drawing framebuffer resize failed");
                        running = 0;
                    } else {
                        if (previous_drawing_size.width != core.drawing_framebuffer.size.width ||
                            previous_drawing_size.height != core.drawing_framebuffer.size.height) {
                            lorenz_needs_render = 1;
                        }
                    }
                    if (running) {
                        if (lorenz_needs_render) {
                            render_started_ms = fractus_platform_get_ticks_ms(&platform);
                            if (fractus_app_render_lorenz(
                                    &platform,
                                    &core.drawing_framebuffer,
                                    &fonts,
                                    &lorenz_params,
                                    &render_save_next_graphic,
                                    runtime_error_message,
                                    sizeof(runtime_error_message),
                                    last_saved_filename,
                                    sizeof(last_saved_filename)) != FRACTUS_STATUS_OK) {
                                fractus_app_log("runtime: lorenz render failed");
                                running = 0;
                            } else {
                                lorenz_needs_render = 0;
                                fractus_app_note_render_time(&platform, render_started_ms, &last_render_ms, &has_render_time);
                            }
                        } else if (render_save_next_graphic) {
                            fractus_graphic_metadata metadata = fractus_graphic_metadata_from_lorenz(
                                &lorenz_params,
                                core.drawing_framebuffer.size.width,
                                core.drawing_framebuffer.size.height);
                            if (fractus_app_save_next_graphic_if_requested_ex(
                                    &platform,
                                    &core.drawing_framebuffer,
                                    &metadata,
                                    &render_save_next_graphic,
                                    runtime_error_message,
                                    sizeof(runtime_error_message),
                                    last_saved_filename,
                                    sizeof(last_saved_filename)) != FRACTUS_STATUS_OK) {
                                fractus_app_log("runtime: lorenz save failed");
                                running = 0;
                            }
                        }
                    }
                    if (running && (drawing_footer_visible || graphic_metadata_panel_visible)) {
                        if (fractus_app_copy_framebuffer_for_overlay(&zone_overlay_framebuffer, &core.drawing_framebuffer) != FRACTUS_STATUS_OK) {
                            fractus_app_log("runtime: lorenz overlay copy failed");
                            running = 0;
                        } else {
                            if (drawing_footer_visible) {
                                (void)fractus_app_draw_drawing_footer_ex(
                                    &zone_overlay_framebuffer,
                                    &fonts,
                                    "ESC o botón derecho: menú - G: grabar - M: metadatos - F: flujo - I: información",
                                    last_saved_filename,
                                    0);
                            }
                            if (graphic_metadata_panel_visible) {
                                fractus_graphic_metadata metadata = fractus_graphic_metadata_from_lorenz(
                                    &lorenz_params,
                                    core.drawing_framebuffer.size.width,
                                    core.drawing_framebuffer.size.height);
                                (void)fractus_app_draw_graphic_metadata_panel(&zone_overlay_framebuffer, &fonts, &metadata, drawing_footer_visible);
                            }
                            present_framebuffer = &zone_overlay_framebuffer;
                            present_is_drawing = 1;
                        }
                    }
                } else if (view == FRACTUS_APP_VIEW_GRAPHIC) {
                    present_framebuffer = &core.drawing_framebuffer;
                    present_is_drawing = 1;
                    if (drawing_footer_visible || (has_loaded_graphic_metadata && graphic_metadata_panel_visible)) {
                        if (fractus_app_copy_framebuffer_for_overlay(&zone_overlay_framebuffer, &core.drawing_framebuffer) != FRACTUS_STATUS_OK) {
                            fractus_app_log("runtime: graphic overlay copy failed");
                            running = 0;
                        } else {
                            if (drawing_footer_visible) {
                                const char *footer_text = (has_loaded_graphic_metadata)
                                    ? "ESC o botón derecho: menú - M: metadatos - F: flujo - I: información"
                                    : "ESC o botón derecho: menú - F: flujo - I: información";
                                (void)fractus_app_draw_drawing_footer(&zone_overlay_framebuffer, &fonts, footer_text);
                            }
                            if (has_loaded_graphic_metadata && graphic_metadata_panel_visible) {
                                if (fractus_app_draw_graphic_metadata_panel(&zone_overlay_framebuffer, &fonts, &loaded_graphic_metadata, drawing_footer_visible) != FRACTUS_STATUS_OK) {
                                    fractus_app_log("runtime: metadata panel render failed");
                                    running = 0;
                                }
                            }
                            present_framebuffer = &zone_overlay_framebuffer;
                        }
                    }
                } else if (view == FRACTUS_APP_VIEW_PALETTE) {
                    if (fractus_app_run_palette_view(
                            &core.ui_framebuffer,
                            &fonts,
                            &ui,
                            &view) != FRACTUS_STATUS_OK) {
                        fractus_app_log("runtime: palette view render failed");
                        running = 0;
                    }
                }

                if (runtime_error_message[0] != '\0') {
                    view = FRACTUS_APP_VIEW_ERROR;
                } else if (save_requested_this_frame && render_save_next_graphic == 0) {
                    save_next_graphic = 0;
                    save_feedback_frames = 8u;
                }
            }

            if (previous_view == FRACTUS_APP_VIEW_GRAPHIC && view != FRACTUS_APP_VIEW_GRAPHIC) {
                graphic_metadata_panel_visible = 0;
                drawing_footer_visible = 0;
                drawing_hud_visible = 0;
            }

            if (fractus_app_view_is_generated_drawing(previous_view) &&
                !fractus_app_view_is_generated_drawing(view)) {
                palette_flow_active = 0;
                last_saved_filename[0] = '\0';
                graphic_metadata_panel_visible = 0;
                drawing_footer_visible = 0;
                drawing_hud_visible = 0;
                (void)fractus_app_sync_framebuffer_palette(&core.drawing_framebuffer, &core.ui_framebuffer);
            }

            if (!fractus_app_view_is_generated_drawing(previous_view) &&
                fractus_app_view_is_generated_drawing(view)) {
                drawing_presented_once = 0;
                has_render_time = 0;
                palette_flow_active = 0;
                last_saved_filename[0] = '\0';
                graphic_metadata_panel_visible = 0;
                drawing_footer_visible = 0;
                drawing_hud_visible = 0;
                fractus_app_seed_zoom_history(
                    &zoom_history,
                    view,
                    &mandelbrot_params,
                    &mandelbrot_dem_params,
                    &julia_params,
                    &julia_dem_params,
                    &biomorph_params);
                (void)fractus_app_sync_framebuffer_palette(&core.drawing_framebuffer, &core.ui_framebuffer);
                if (view == FRACTUS_APP_VIEW_MANDELBROT) {
                    mandelbrot_needs_render = 1;
                } else if (view == FRACTUS_APP_VIEW_MANDELBROT_DEM) {
                    mandelbrot_dem_needs_render = 1;
                } else if (view == FRACTUS_APP_VIEW_JULIA) {
                    julia_needs_render = 1;
                } else if (view == FRACTUS_APP_VIEW_JULIA_DEM) {
                    julia_dem_needs_render = 1;
                } else if (view == FRACTUS_APP_VIEW_BIOMORPH) {
                    biomorph_needs_render = 1;
                } else if (view == FRACTUS_APP_VIEW_PLASMA_RECTANGULAR) {
                    plasma_rectangular_needs_render = 1;
                } else if (view == FRACTUS_APP_VIEW_PLASMA_CIRCULAR) {
                    plasma_circular_needs_render = 1;
                } else if (view == FRACTUS_APP_VIEW_LORENZ) {
                    lorenz_needs_render = 1;
                }
            } else if (view == FRACTUS_APP_VIEW_GRAPHIC && previous_view != FRACTUS_APP_VIEW_GRAPHIC) {
                drawing_presented_once = 1;
                has_render_time = 0;
                graphic_metadata_panel_visible = 0;
                drawing_footer_visible = 0;
                drawing_hud_visible = 0;
            }
        }

        if (!running) {
            break;
        }

        if (present_is_drawing && palette_flow_active) {
            (void)fractus_framebuffer_cycle_palette(&core.drawing_framebuffer, FRACTUS_PALETTE_OFFSET, FRACTUS_PALETTE_SPAN, 1);
        }

        if (present_is_drawing &&
            present_framebuffer != &core.drawing_framebuffer) {
            present_framebuffer->palette = core.drawing_framebuffer.palette;
            present_framebuffer->palette_dirty = 1;
        }

        if (present_is_drawing && (drawing_hud_visible || save_feedback_frames > 0u)) {
            if (present_framebuffer == &core.drawing_framebuffer) {
                if (fractus_app_copy_framebuffer_for_overlay(&zone_overlay_framebuffer, &core.drawing_framebuffer) != FRACTUS_STATUS_OK) {
                    fractus_app_log("runtime: drawing overlay copy failed");
                    running = 0;
                    break;
                }
                present_framebuffer = &zone_overlay_framebuffer;
            }
            if (drawing_hud_visible) {
                fractus_app_drawing_hud hud;

                fractus_app_fill_drawing_hud(
                    view,
                    &platform,
                    &core.drawing_framebuffer,
                    &ui,
                    &mandelbrot_params,
                    &mandelbrot_dem_params,
                    &julia_params,
                    &julia_dem_params,
                    &biomorph_params,
                    &plasma_rectangular_params,
                    &plasma_circular_params,
                    &lorenz_params,
                    &loaded_graphic_metadata,
                    has_loaded_graphic_metadata,
                    last_render_ms,
                    has_render_time,
                    frame_fps,
                    &hud);
                if (fractus_app_draw_drawing_hud(present_framebuffer, &fonts, &hud) != FRACTUS_STATUS_OK) {
                    fractus_app_log("runtime: drawing hud failed");
                    running = 0;
                    break;
                }
            }
            if (save_feedback_frames > 0u) {
                if (fractus_app_draw_save_feedback(present_framebuffer, save_feedback_frames) != FRACTUS_STATUS_OK) {
                    fractus_app_log("runtime: save feedback failed");
                    running = 0;
                    break;
                }
                --save_feedback_frames;
            }
        }

        if (fractus_framebuffer_sync_rgba(present_framebuffer) != FRACTUS_STATUS_OK) {
            fractus_app_log("runtime: framebuffer sync failed");
            running = 0;
            break;
        }

        surface.width = present_framebuffer->size.width;
        surface.height = present_framebuffer->size.height;
        surface.pitch_bytes = present_framebuffer->rgba_pitch_bytes;
        surface.pixel_format = FRACTUS_PLATFORM_PIXEL_FORMAT_RGBA8888;
        surface.present_mode = present_is_drawing
            ? FRACTUS_PLATFORM_PRESENT_MODE_DRAWING
            : FRACTUS_PLATFORM_PRESENT_MODE_UI;
        surface.pixels = present_framebuffer->rgba_pixels;

        if (fractus_app_configure_platform_for_present(
                &platform,
                &legacy_config,
                present_is_drawing) != FRACTUS_STATUS_OK) {
            fractus_app_log("runtime: video mode change failed");
            running = 0;
            break;
        }

        if (fractus_platform_present(&platform, &surface) != FRACTUS_STATUS_OK) {
            fractus_app_log("runtime: present failed");
            (void)fractus_platform_set_video_mode(
                &platform,
                config.logical_size,
                0,
                FRACTUS_PLATFORM_WINDOW_FIT_INTEGER);
            (void)fractus_app_set_message(
                runtime_error_message,
                sizeof(runtime_error_message),
                "No se puede presentar el dibujo con el modo de video seleccionado.");
            view = FRACTUS_APP_VIEW_ERROR;
            continue;
        }

        if (present_is_drawing) {
            drawing_presented_once = 1;
        }

        fractus_platform_sleep_ms(16u);
    }

    fractus_font_library_shutdown(&fonts);
    fractus_framebuffer_shutdown(&zone_overlay_framebuffer);
    fractus_ui_shutdown(&ui);
    fractus_core_shutdown(&core);
    fractus_platform_shutdown(&platform);
    fractus_app_log("shutdown: end");
    return 0;
}
