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
    {FRACTUS_APP_RECT(10, 110, 312, 130), 11u, 0u, "Biomorfos de Clifford Pickover"},
    {FRACTUS_APP_RECT(10, 130, 312, 150), 11u, 0u, "Fractales por el metodo de plasma"},
    {FRACTUS_APP_RECT(10, 150, 312, 170), 11u, 0u, "Atractores dinamicos"},
    {FRACTUS_APP_RECT(10, 170, 312, 190), 11u, 0u, "Curvas fractales sencillas"},
    {FRACTUS_APP_RECT(10, 190, 312, 210), 11u, 0u, "Otros conjuntos fractales"},
    {FRACTUS_APP_RECT(10, 210, 312, 230), 11u, 0u, "Modelos fractales naturales"},
    {FRACTUS_APP_RECT(10, 230, 312, 250), 11u, 0u, "Lenguajes 0-L"},
    {FRACTUS_APP_RECT(10, 250, 312, 270), 11u, 0u, "Sistemas de funcion iterada (IFS)"},
    {FRACTUS_APP_RECT(327, 70, 629, 90), 3u, 15u, "Cargar un dibujo"},
    {FRACTUS_APP_RECT(327, 90, 629, 110), 3u, 15u, "Grabar automaticamente el dibujo siguiente"},
    {FRACTUS_APP_RECT(327, 130, 629, 150), 3u, 15u, "Cargar una paleta"},
    {FRACTUS_APP_RECT(327, 150, 629, 170), 3u, 15u, "Grabar la paleta actual"},
    {FRACTUS_APP_RECT(327, 170, 629, 190), 3u, 15u, "Cargar la paleta de un dibujo"},
    {FRACTUS_APP_RECT(327, 190, 629, 210), 3u, 15u, "Cambiar la paleta de un dibujo"},
    {FRACTUS_APP_RECT(327, 210, 629, 230), 3u, 15u, "Mostrar la paleta actual"},
    {FRACTUS_APP_RECT(327, 230, 629, 250), 3u, 15u, "Modificar un color de la paleta"},
    {FRACTUS_APP_RECT(327, 250, 629, 270), 3u, 15u, "Copiar un color de la paleta"},
    {FRACTUS_APP_RECT(327, 270, 629, 290), 3u, 15u, "Crear un gradiente"},
    {FRACTUS_APP_RECT(327, 290, 629, 310), 3u, 15u, "Restaurar la paleta por defecto"},
    {FRACTUS_APP_RECT(327, 334, 629, 354), 5u, 15u, "Resolucion de los dibujos"},
    {FRACTUS_APP_RECT(327, 354, 629, 374), 5u, 15u, "Parametros por defecto de conjuntos fractales"},
    {FRACTUS_APP_RECT(327, 374, 629, 394), 5u, 15u, "Restaurar parametros de los conjuntos"},
    {FRACTUS_APP_RECT(10, 430, 130, 450), 8u, 0u, "Ayuda"},
    {FRACTUS_APP_RECT(509, 430, 629, 450), 8u, 0u, "Acerca de..."},
    {FRACTUS_APP_RECT(260, 430, 380, 450), 0u, 15u, "Salir del programa"}
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
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_FRANCE, 320, 5, 0u, "Fractus I") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, 320, 30, 0u, "Dibujo de conjuntos fractales") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_group_box(framebuffer, fonts, 5, 60, 318, 275, 8u, 0u, "Tipos de conjuntos") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_group_box(framebuffer, fonts, 322, 60, 635, 316, 8u, 0u, "Graficos y colores") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_group_box(framebuffer, fonts, 322, 324, 635, 400, 8u, 0u, "Configuracion por defecto") != FRACTUS_STATUS_OK) {
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
    fractus_mandelbrot_params *mandelbrot_pending,
    fractus_mandelbrot_dem_params *mandelbrot_dem_params,
    fractus_julia_params *julia_params,
    fractus_julia_params *julia_pending,
    fractus_julia_dem_params *julia_dem_params,
    fractus_biomorph_params *biomorph_params,
    fractus_biomorph_params *biomorph_pending,
    fractus_plasma_params *plasma_rectangular_params,
    fractus_plasma_circular_params *plasma_circular_params,
    fractus_app_biomorph_fields *biomorph_fields,
    fractus_ui_numeric_field *iterations_field,
    fractus_ui_numeric_field *escape_radius_field,
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

    if (platform == NULL || framebuffer == NULL || fonts == NULL || ui == NULL ||
        view == NULL || running == NULL || legacy_config == NULL || config_draft == NULL ||
        mandelbrot_params == NULL || mandelbrot_pending == NULL || mandelbrot_dem_params == NULL ||
        julia_params == NULL || julia_pending == NULL || julia_dem_params == NULL ||
        biomorph_params == NULL || biomorph_pending == NULL ||
        plasma_rectangular_params == NULL || plasma_circular_params == NULL ||
        biomorph_fields == NULL ||
        iterations_field == NULL || escape_radius_field == NULL || biomorph_radius_field == NULL ||
        biomorph_cutoff_field == NULL || plasma_rect_seed_field == NULL || plasma_circ_seed_field == NULL ||
        graphic_files == NULL || graphic_file_count == NULL || graphic_file_page == NULL || palette_files == NULL ||
        palette_file_count == NULL || palette_file_page == NULL || runtime_error_message == NULL || cfg_path == NULL ||
        save_next_graphic == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    for (i = 0u; i < FRACTUS_APP_ARRAY_COUNT(fractus_app_main_menu_controls); ++i) {
        menu_options[i].bounds = fractus_app_main_menu_controls[i].bounds;
    }

    /* 1. Contenedor exterior. */
    /* 2. Textos y controles. */
    if (fractus_app_draw_main_menu_frame(framebuffer, fonts) != FRACTUS_STATUS_OK ||
        fractus_app_draw_main_menu_controls(
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
                biomorph_pending->escape_radius_squared,
                biomorph_pending->cutoff);
            *view = FRACTUS_APP_VIEW_BIOMORPH_CONFIG;
        } else if (selected_menu == 3) {
            *view = FRACTUS_APP_VIEW_PLASMA_MENU;
        } else if (selected_menu == FRACTUS_APP_MENU_VIDEO_INDEX) {
            *config_draft = *legacy_config;
            *view = FRACTUS_APP_VIEW_VIDEO_CONFIG;
        } else if (selected_menu == FRACTUS_APP_MENU_FRACTALS_CONFIG_INDEX) {
            *config_draft = *legacy_config;
            (void)fractus_ui_numeric_field_init_int(
                iterations_field,
                (fractus_rect_i32){324, 131, 73, 20},
                (int32_t)config_draft->iterations,
                16,
                1024);
            (void)fractus_ui_numeric_field_init_int(
                escape_radius_field,
                (fractus_rect_i32){324, 157, 73, 20},
                (int32_t)config_draft->escape_radius_squared,
                4,
                1000);
            (void)fractus_ui_numeric_field_init_int(
                biomorph_radius_field,
                (fractus_rect_i32){324, 207, 73, 20},
                (int32_t)config_draft->biomorph_escape_radius_squared,
                4,
                1000);
            (void)fractus_ui_numeric_field_init_int(
                biomorph_cutoff_field,
                (fractus_rect_i32){324, 233, 73, 20},
                (int32_t)config_draft->biomorph_cutoff,
                1,
                100);
            (void)fractus_ui_numeric_field_init_int(
                plasma_rect_seed_field,
                (fractus_rect_i32){324, 283, 73, 20},
                (int32_t)config_draft->plasma_rectangular_seed,
                1,
                999999);
            (void)fractus_ui_numeric_field_init_int(
                plasma_circ_seed_field,
                (fractus_rect_i32){324, 309, 73, 20},
                (int32_t)config_draft->plasma_circular_seed,
                1,
                999999);
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
                legacy_config);
            *mandelbrot_pending = *mandelbrot_params;
            *julia_pending = *julia_params;
            *biomorph_pending = *biomorph_params;
            fractus_app_init_biomorph_fields(
                biomorph_fields,
                biomorph_pending->xmin,
                biomorph_pending->xmax,
                biomorph_pending->ymin,
                biomorph_pending->ymax,
                biomorph_pending->constant_real,
                biomorph_pending->constant_imag,
                biomorph_pending->escape_radius_squared,
                biomorph_pending->cutoff);
            fractus_app_log("runtime: default fractal parameters restored");
        }
    }

    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_app_render_error(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const char *message,
    const fractus_app_menu_entry *entries,
    size_t entry_count,
    int active_index)
{
    if (framebuffer == NULL || fonts == NULL || message == NULL || entries == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_app_render_main_menu(framebuffer, fonts, -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_window(framebuffer, 150, 184, 489, 296) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, 320, 188, 15u, "Error") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, 320, 230, 0u, message) != FRACTUS_STATUS_OK ||
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
    static const fractus_app_menu_entry dialog_entries[] = {
        {FRACTUS_APP_RECT(270, 258, 370, 278), 0u, 15u, "Aceptar"}
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

int fractus_app_run(void)
{
    fractus_core core;
    fractus_ui_context ui;
    fractus_platform_context platform;
    fractus_platform_event event;
    fractus_platform_surface surface;
    fractus_framebuffer *present_framebuffer;
    fractus_framebuffer mandelbrot_overlay_framebuffer;
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
    fractus_app_mandelbrot_selection mandelbrot_selection;
    fractus_app_mandelbrot_selection mandelbrot_dem_selection;
    fractus_color_rgba8 palette_original_color;
    fractus_color_rgba8 palette_pending_color;
    fractus_color_rgba8 palette_copy_color;
    fractus_ui_numeric_field iterations_field;
    fractus_ui_numeric_field escape_radius_field;
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
    int running;
    int has_event;
    int save_next_graphic;
    int current_drawing_saved;
    int drawing_presented_once;
    int mandelbrot_needs_render;
    int mandelbrot_dem_needs_render;
    int julia_needs_render;
    int julia_dem_needs_render;
    int biomorph_needs_render;
    int plasma_rectangular_needs_render;
    int plasma_circular_needs_render;
    int palette_flow_active;
    int present_is_drawing;
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
    memset(&mandelbrot_overlay_framebuffer, 0, sizeof(mandelbrot_overlay_framebuffer));
    memset(&legacy_config, 0, sizeof(legacy_config));
    memset(&config_draft, 0, sizeof(config_draft));
    memset(&iterations_field, 0, sizeof(iterations_field));
    memset(&escape_radius_field, 0, sizeof(escape_radius_field));
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
    memset(&mandelbrot_selection, 0, sizeof(mandelbrot_selection));
    memset(&mandelbrot_dem_selection, 0, sizeof(mandelbrot_dem_selection));
    memset(graphic_files, 0, sizeof(graphic_files));
    memset(palette_files, 0, sizeof(palette_files));
    memset(cfg_path, 0, sizeof(cfg_path));
    memset(runtime_error_message, 0, sizeof(runtime_error_message));
    memset(selected_graphic_path, 0, sizeof(selected_graphic_path));
    palette_original_color = fractus_app_rgb8(0u, 0u, 0u);
    palette_pending_color = fractus_app_rgb8(0u, 0u, 0u);
    palette_copy_color = fractus_app_rgb8(0u, 0u, 0u);
    palette_selected_index = 16u;
    palette_copy_source_index = 16u;
    palette_gradient_first_index = 16u;
    save_next_graphic = 0;
    current_drawing_saved = 1;
    drawing_presented_once = 0;
    mandelbrot_needs_render = 1;
    mandelbrot_dem_needs_render = 1;
    julia_needs_render = 1;
    julia_dem_needs_render = 1;
    biomorph_needs_render = 1;
    plasma_rectangular_needs_render = 1;
    plasma_circular_needs_render = 1;
    palette_flow_active = 0;
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
                        &mandelbrot_pending,
                        &mandelbrot_dem_params,
                        &julia_params,
                        &julia_pending,
                        &julia_dem_params,
                        &biomorph_params,
                        &biomorph_pending,
                        &plasma_rectangular_params,
                        &plasma_circular_params,
                        &biomorph_fields,
                        &iterations_field,
                        &escape_radius_field,
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
            } else {
                int save_requested_this_frame = save_next_graphic;
                int render_save_next_graphic = save_next_graphic;

                if (fractus_app_view_is_generated_drawing(view) &&
                    save_next_graphic &&
                    !drawing_presented_once) {
                    render_save_next_graphic = 0;
                    save_requested_this_frame = 0;
                }

                if (view == FRACTUS_APP_VIEW_MANDELBROT &&
                    mandelbrot_selection.active &&
                    ((ui.key_press_pending && ui.key_pressed == 27u) ||
                     (ui.press_pending && ui.press_event.buttons.right))) {
                    mandelbrot_selection.active = 0;
                    mandelbrot_selection.has_first_corner = 0;
                } else if (view == FRACTUS_APP_VIEW_MANDELBROT_DEM &&
                    mandelbrot_dem_selection.active &&
                    ((ui.key_press_pending && ui.key_pressed == 27u) ||
                     (ui.press_pending && ui.press_event.buttons.right))) {
                    mandelbrot_dem_selection.active = 0;
                    mandelbrot_dem_selection.has_first_corner = 0;
                } else if ((ui.key_press_pending && ui.key_pressed == 27u) ||
                    (ui.press_pending && ui.press_event.buttons.right)) {
                    mandelbrot_selection.active = 0;
                    mandelbrot_selection.has_first_corner = 0;
                    mandelbrot_dem_selection.active = 0;
                    mandelbrot_dem_selection.has_first_corner = 0;
                    palette_flow_active = 0;
                    view = FRACTUS_APP_VIEW_MAIN_MENU;
                }

                if (ui.key_press_pending &&
                    (ui.key_pressed == 'f' || ui.key_pressed == 'F') &&
                    (fractus_app_view_is_generated_drawing(view) || view == FRACTUS_APP_VIEW_GRAPHIC)) {
                    palette_flow_active = !palette_flow_active;
                }

                if (ui.key_press_pending &&
                    (ui.key_pressed == 'g' || ui.key_pressed == 'G') &&
                    current_drawing_saved == 0 &&
                    (view == FRACTUS_APP_VIEW_MANDELBROT ||
                     view == FRACTUS_APP_VIEW_MANDELBROT_DEM ||
                     view == FRACTUS_APP_VIEW_JULIA ||
                     view == FRACTUS_APP_VIEW_JULIA_DEM ||
                     view == FRACTUS_APP_VIEW_BIOMORPH ||
                     view == FRACTUS_APP_VIEW_PLASMA_RECTANGULAR ||
                     view == FRACTUS_APP_VIEW_PLASMA_CIRCULAR ||
                     view == FRACTUS_APP_VIEW_GRAPHIC)) {
                    save_next_graphic = 1;
                    render_save_next_graphic = drawing_presented_once ? 1 : 0;
                    save_requested_this_frame = drawing_presented_once ? 1 : 0;
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
                        if (fractus_app_handle_mandelbrot_selection_input(
                                &platform,
                                &core.drawing_framebuffer,
                                &ui,
                                &mandelbrot_params,
                                &mandelbrot_selection)) {
                            current_drawing_saved = 0;
                            drawing_presented_once = 0;
                            save_next_graphic = 0;
                            render_save_next_graphic = 0;
                            save_requested_this_frame = 0;
                            mandelbrot_needs_render = 1;
                        }
                    }
                    if (running) {
                        if (mandelbrot_needs_render) {
                            if (fractus_app_render_mandelbrot(
                                    &platform,
                                    &core.drawing_framebuffer,
                                    &fonts,
                                    &mandelbrot_params,
                                    &render_save_next_graphic,
                                    runtime_error_message,
                                    sizeof(runtime_error_message)) != FRACTUS_STATUS_OK) {
                                fractus_app_log("runtime: mandelbrot render failed");
                                running = 0;
                            } else {
                                mandelbrot_needs_render = 0;
                            }
                        } else if (render_save_next_graphic &&
                            fractus_app_save_next_graphic_if_requested(
                                &platform,
                                &core.drawing_framebuffer,
                                &render_save_next_graphic,
                                runtime_error_message,
                                sizeof(runtime_error_message)) != FRACTUS_STATUS_OK) {
                            fractus_app_log("runtime: mandelbrot save failed");
                            running = 0;
                        }
                    }
                    if (running && mandelbrot_selection.active) {
                        if (fractus_app_copy_framebuffer_for_overlay(&mandelbrot_overlay_framebuffer, &core.drawing_framebuffer) != FRACTUS_STATUS_OK ||
                            fractus_app_draw_mandelbrot_selection_overlay(&platform, &mandelbrot_overlay_framebuffer, &ui, &mandelbrot_selection) != FRACTUS_STATUS_OK) {
                            fractus_app_log("runtime: mandelbrot selection overlay failed");
                            running = 0;
                        } else {
                            present_framebuffer = &mandelbrot_overlay_framebuffer;
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
                        if (fractus_app_handle_mandelbrot_dem_selection_input(
                                &platform,
                                &core.drawing_framebuffer,
                                &ui,
                                &mandelbrot_dem_params,
                                &mandelbrot_dem_selection)) {
                            current_drawing_saved = 0;
                            drawing_presented_once = 0;
                            save_next_graphic = 0;
                            render_save_next_graphic = 0;
                            save_requested_this_frame = 0;
                            mandelbrot_dem_needs_render = 1;
                        }
                    }
                    if (running) {
                        if (mandelbrot_dem_needs_render) {
                            if (fractus_app_render_mandelbrot_dem(
                                    &platform,
                                    &core.drawing_framebuffer,
                                    &fonts,
                                    &mandelbrot_dem_params,
                                    &render_save_next_graphic,
                                    runtime_error_message,
                                    sizeof(runtime_error_message)) != FRACTUS_STATUS_OK) {
                                fractus_app_log("runtime: mandelbrot dem render failed");
                                running = 0;
                            } else {
                                mandelbrot_dem_needs_render = 0;
                            }
                        } else if (render_save_next_graphic &&
                            fractus_app_save_next_graphic_if_requested(
                                &platform,
                                &core.drawing_framebuffer,
                                &render_save_next_graphic,
                                runtime_error_message,
                                sizeof(runtime_error_message)) != FRACTUS_STATUS_OK) {
                            fractus_app_log("runtime: mandelbrot dem save failed");
                            running = 0;
                        }
                    }
                    if (running && mandelbrot_dem_selection.active) {
                        if (fractus_app_copy_framebuffer_for_overlay(&mandelbrot_overlay_framebuffer, &core.drawing_framebuffer) != FRACTUS_STATUS_OK ||
                            fractus_app_draw_mandelbrot_selection_overlay(&platform, &mandelbrot_overlay_framebuffer, &ui, &mandelbrot_dem_selection) != FRACTUS_STATUS_OK) {
                            fractus_app_log("runtime: mandelbrot dem selection overlay failed");
                            running = 0;
                        } else {
                            present_framebuffer = &mandelbrot_overlay_framebuffer;
                            present_is_drawing = 1;
                        }
                    }
                } else if (view == FRACTUS_APP_VIEW_JULIA) {
                    fractus_size_u32 previous_drawing_size = core.drawing_framebuffer.size;
                    mandelbrot_selection.active = 0;
                    mandelbrot_selection.has_first_corner = 0;
                    mandelbrot_dem_selection.active = 0;
                    mandelbrot_dem_selection.has_first_corner = 0;
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
                    }
                    if (running) {
                        if (julia_needs_render) {
                            if (fractus_app_render_julia(
                                    &platform,
                                    &core.drawing_framebuffer,
                                    &fonts,
                                    &julia_params,
                                    &render_save_next_graphic,
                                    runtime_error_message,
                                    sizeof(runtime_error_message)) != FRACTUS_STATUS_OK) {
                                fractus_app_log("runtime: julia render failed");
                                running = 0;
                            } else {
                                julia_needs_render = 0;
                            }
                        } else if (render_save_next_graphic &&
                            fractus_app_save_next_graphic_if_requested(
                                &platform,
                                &core.drawing_framebuffer,
                                &render_save_next_graphic,
                                runtime_error_message,
                                sizeof(runtime_error_message)) != FRACTUS_STATUS_OK) {
                            fractus_app_log("runtime: julia save failed");
                            running = 0;
                        }
                    }
                } else if (view == FRACTUS_APP_VIEW_JULIA_DEM) {
                    fractus_size_u32 previous_drawing_size = core.drawing_framebuffer.size;
                    mandelbrot_selection.active = 0;
                    mandelbrot_selection.has_first_corner = 0;
                    mandelbrot_dem_selection.active = 0;
                    mandelbrot_dem_selection.has_first_corner = 0;
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
                    }
                    if (running) {
                        if (julia_dem_needs_render) {
                            if (fractus_app_render_julia_dem(
                                    &platform,
                                    &core.drawing_framebuffer,
                                    &fonts,
                                    &julia_dem_params,
                                    &render_save_next_graphic,
                                    runtime_error_message,
                                    sizeof(runtime_error_message)) != FRACTUS_STATUS_OK) {
                                fractus_app_log("runtime: julia dem render failed");
                                running = 0;
                            } else {
                                julia_dem_needs_render = 0;
                            }
                        } else if (render_save_next_graphic &&
                            fractus_app_save_next_graphic_if_requested(
                                &platform,
                                &core.drawing_framebuffer,
                                &render_save_next_graphic,
                                runtime_error_message,
                                sizeof(runtime_error_message)) != FRACTUS_STATUS_OK) {
                            fractus_app_log("runtime: julia dem save failed");
                            running = 0;
                        }
                    }
                } else if (view == FRACTUS_APP_VIEW_BIOMORPH) {
                    fractus_size_u32 previous_drawing_size = core.drawing_framebuffer.size;
                    mandelbrot_dem_selection.active = 0;
                    mandelbrot_dem_selection.has_first_corner = 0;
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
                    }
                    if (running) {
                        if (biomorph_needs_render) {
                            if (fractus_app_render_biomorph(
                                    &platform,
                                    &core.drawing_framebuffer,
                                    &fonts,
                                    &biomorph_params,
                                    &render_save_next_graphic,
                                    runtime_error_message,
                                    sizeof(runtime_error_message)) != FRACTUS_STATUS_OK) {
                                fractus_app_log("runtime: biomorph render failed");
                                running = 0;
                            } else {
                                biomorph_needs_render = 0;
                            }
                        } else if (render_save_next_graphic &&
                            fractus_app_save_next_graphic_if_requested(
                                &platform,
                                &core.drawing_framebuffer,
                                &render_save_next_graphic,
                                runtime_error_message,
                                sizeof(runtime_error_message)) != FRACTUS_STATUS_OK) {
                            fractus_app_log("runtime: biomorph save failed");
                            running = 0;
                        }
                    }
                } else if (view == FRACTUS_APP_VIEW_PLASMA_RECTANGULAR) {
                    fractus_size_u32 previous_drawing_size = core.drawing_framebuffer.size;
                    mandelbrot_dem_selection.active = 0;
                    mandelbrot_dem_selection.has_first_corner = 0;
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
                            if (fractus_app_render_plasma_rectangular(
                                    &platform,
                                    &core.drawing_framebuffer,
                                    &fonts,
                                    &plasma_rectangular_params,
                                    &render_save_next_graphic,
                                    runtime_error_message,
                                    sizeof(runtime_error_message)) != FRACTUS_STATUS_OK) {
                                fractus_app_log("runtime: plasma rectangular render failed");
                                running = 0;
                            } else {
                                plasma_rectangular_needs_render = 0;
                            }
                        } else if (render_save_next_graphic &&
                            fractus_app_save_next_graphic_if_requested(
                                &platform,
                                &core.drawing_framebuffer,
                                &render_save_next_graphic,
                                runtime_error_message,
                                sizeof(runtime_error_message)) != FRACTUS_STATUS_OK) {
                            fractus_app_log("runtime: plasma rectangular save failed");
                            running = 0;
                        }
                    }
                } else if (view == FRACTUS_APP_VIEW_PLASMA_CIRCULAR) {
                    fractus_size_u32 previous_drawing_size = core.drawing_framebuffer.size;
                    mandelbrot_dem_selection.active = 0;
                    mandelbrot_dem_selection.has_first_corner = 0;
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
                            if (fractus_app_render_plasma_circular(
                                    &platform,
                                    &core.drawing_framebuffer,
                                    &fonts,
                                    &plasma_circular_params,
                                    &render_save_next_graphic,
                                    runtime_error_message,
                                    sizeof(runtime_error_message)) != FRACTUS_STATUS_OK) {
                                fractus_app_log("runtime: circular plasma render failed");
                                running = 0;
                            } else {
                                plasma_circular_needs_render = 0;
                            }
                        } else if (render_save_next_graphic &&
                            fractus_app_save_next_graphic_if_requested(
                                &platform,
                                &core.drawing_framebuffer,
                                &render_save_next_graphic,
                                runtime_error_message,
                                sizeof(runtime_error_message)) != FRACTUS_STATUS_OK) {
                            fractus_app_log("runtime: circular plasma save failed");
                            running = 0;
                        }
                    }
                } else if (view == FRACTUS_APP_VIEW_GRAPHIC) {
                    present_framebuffer = &core.drawing_framebuffer;
                    present_is_drawing = 1;
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
                    current_drawing_saved = 1;
                    save_feedback_frames = 5u;
                }
            }

            if (fractus_app_view_is_generated_drawing(previous_view) &&
                !fractus_app_view_is_generated_drawing(view)) {
                palette_flow_active = 0;
                (void)fractus_app_sync_framebuffer_palette(&core.drawing_framebuffer, &core.ui_framebuffer);
            }

            if (!fractus_app_view_is_generated_drawing(previous_view) &&
                fractus_app_view_is_generated_drawing(view)) {
                current_drawing_saved = 0;
                drawing_presented_once = 0;
                palette_flow_active = 0;
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
                }
            } else if (view == FRACTUS_APP_VIEW_GRAPHIC && previous_view != FRACTUS_APP_VIEW_GRAPHIC) {
                current_drawing_saved = 1;
                drawing_presented_once = 1;
            }
        }

        if (!running) {
            break;
        }

        if (present_is_drawing && save_feedback_frames > 0u) {
            if (fractus_app_draw_save_feedback(&core.drawing_framebuffer, save_feedback_frames) != FRACTUS_STATUS_OK) {
                fractus_app_log("runtime: save feedback failed");
                running = 0;
                break;
            }
            --save_feedback_frames;
        }

        if (present_is_drawing && palette_flow_active) {
            (void)fractus_framebuffer_cycle_palette(&core.drawing_framebuffer, 16u, 240u, 1);
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
                (fractus_size_u32){config.logical_size.width * 2u, config.logical_size.height * 2u},
                0);
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
    fractus_framebuffer_shutdown(&mandelbrot_overlay_framebuffer);
    fractus_ui_shutdown(&ui);
    fractus_core_shutdown(&core);
    fractus_platform_shutdown(&platform);
    fractus_app_log("shutdown: end");
    return 0;
}
