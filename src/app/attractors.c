#include "app/attractors.h"
#include "app/files.h"

#include <stdio.h>
#include <string.h>

static const fractus_app_menu_entry fractus_app_attractor_method_buttons[] = {
    {FRACTUS_APP_RECT(126, 97, 224, 117), 8u, 0u, "Lorenz"},
    {FRACTUS_APP_RECT(126, 121, 224, 141), 8u, 0u, "Henon"},
    {FRACTUS_APP_RECT(126, 145, 224, 165), 8u, 0u, "Rossler"},
    {FRACTUS_APP_RECT(126, 169, 224, 189), 8u, 0u, "Ikeda"},
    {FRACTUS_APP_RECT(126, 193, 224, 213), 8u, 0u, "Gumowski-Mira"},
    {FRACTUS_APP_RECT(126, 217, 224, 237), 8u, 0u, "Aizawa"},
    {FRACTUS_APP_RECT(126, 241, 224, 261), 8u, 0u, "Peter de Jong"},
    {FRACTUS_APP_RECT(126, 265, 224, 285), 8u, 0u, "Pickover"},
    {FRACTUS_APP_RECT(126, 289, 224, 309), 8u, 0u, "Tinkerbell"},
    {FRACTUS_APP_RECT(126, 313, 224, 333), 8u, 0u, "Thomas"},
    {FRACTUS_APP_RECT(126, 337, 224, 357), 8u, 0u, "Svensson"}
};

const char *fractus_app_attractor_method_name(fractus_app_attractor_method method)
{
    switch (method) {
    case FRACTUS_APP_ATTRACTOR_METHOD_LORENZ:
        return "Lorenz";
    case FRACTUS_APP_ATTRACTOR_METHOD_HENON:
        return "Henon";
    case FRACTUS_APP_ATTRACTOR_METHOD_ROSSLER:
        return "Rossler";
    case FRACTUS_APP_ATTRACTOR_METHOD_IKEDA:
        return "Ikeda";
    case FRACTUS_APP_ATTRACTOR_METHOD_GUMOWSKI_MIRA:
        return "Gumowski-Mira";
    case FRACTUS_APP_ATTRACTOR_METHOD_AIZAWA:
        return "Aizawa";
    case FRACTUS_APP_ATTRACTOR_METHOD_PETER_DE_JONG:
        return "Peter de Jong";
    case FRACTUS_APP_ATTRACTOR_METHOD_PICKOVER:
        return "Pickover";
    case FRACTUS_APP_ATTRACTOR_METHOD_TINKERBELL:
        return "Tinkerbell";
    case FRACTUS_APP_ATTRACTOR_METHOD_THOMAS:
        return "Thomas";
    case FRACTUS_APP_ATTRACTOR_METHOD_SVENSSON:
        return "Svensson";
    case FRACTUS_APP_ATTRACTOR_METHOD_NONE:
    case FRACTUS_APP_ATTRACTOR_METHOD_COUNT:
    default:
        return "";
    }
}

static fractus_status fractus_app_render_attractors_menu(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    int active_index)
{
    static const fractus_app_menu_entry action_buttons[] = {
        {FRACTUS_APP_RECT(210, 395, 310, 415), 8u, 7u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 395, 430, 415), 0u, 15u, "Cancelar"}
    };
    uint32_t i;
    const uint32_t method_count = FRACTUS_APP_ARRAY_COUNT(fractus_app_attractor_method_buttons);

    if (framebuffer == NULL || fonts == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_app_render_main_menu(framebuffer, fonts, -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_window(framebuffer, 115, 55, 524, 425) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, 320, 59, 15u, "Atractores dinamicos") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_group_box(framebuffer, fonts, 120, 85, 230, 365, 8u, 0u, "Tipo de atractor") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    for (i = 0u; i < method_count; ++i) {
        fractus_app_menu_entry entry = fractus_app_attractor_method_buttons[i];
        int draw_active = (active_index == (int)i);

        if (fractus_ui_draw_button(framebuffer, fonts, &entry, draw_active) != FRACTUS_STATUS_OK) {
            return FRACTUS_STATUS_ERROR;
        }
    }

    for (i = 0u; i < FRACTUS_APP_ARRAY_COUNT(action_buttons); ++i) {
        fractus_app_menu_entry entry = action_buttons[i];
        int draw_active = (i == 1u && active_index == (int)(method_count + 1u));

        if (fractus_ui_draw_button(framebuffer, fonts, &entry, draw_active) != FRACTUS_STATUS_OK) {
            return FRACTUS_STATUS_ERROR;
        }
    }

    if (fractus_ui_draw_text_centered(
            framebuffer,
            fonts,
            FRACTUS_FONT_SMALL,
            320,
            373,
            0u,
            "Selecciona el tipo de atractor del que se quiere generar el dibujo.") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_app_render_attractors_config(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    int active_index,
    fractus_app_attractor_method selected_method)
{
    static const fractus_app_menu_entry action_buttons[] = {
        {FRACTUS_APP_RECT(210, 395, 310, 415), 8u, 0u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 395, 430, 415), 0u, 15u, "Cancelar"}
    };
    char params_title[96];
    uint32_t i;
    const uint32_t method_count = FRACTUS_APP_ARRAY_COUNT(fractus_app_attractor_method_buttons);

    if (framebuffer == NULL || fonts == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_app_render_main_menu(framebuffer, fonts, -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_window(framebuffer, 115, 55, 524, 425) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, 320, 59, 15u, "Atractores dinamicos") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_group_box(framebuffer, fonts, 120, 85, 230, 365, 8u, 0u, "Tipo de atractor") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    (void)snprintf(
        params_title,
        sizeof(params_title),
        "Parametros del atractor de %s",
        fractus_app_attractor_method_name(selected_method));

    if (fractus_ui_draw_group_box(framebuffer, fonts, 238, 85, 519, 365, 8u, 0u, params_title) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    for (i = 0u; i < method_count; ++i) {
        fractus_app_menu_entry entry = fractus_app_attractor_method_buttons[i];
        int draw_active = 0;

        if (i == (uint32_t)selected_method) {
            entry.fill_color = 8u;
            entry.text_color = 0u;
            draw_active = 1;
        } else {
            entry.fill_color = 8u;
            entry.text_color = 7u;
            draw_active = 0;
        }

        if (fractus_ui_draw_button(framebuffer, fonts, &entry, draw_active) != FRACTUS_STATUS_OK) {
            return FRACTUS_STATUS_ERROR;
        }
    }

    for (i = 0u; i < FRACTUS_APP_ARRAY_COUNT(action_buttons); ++i) {
        fractus_app_menu_entry entry = action_buttons[i];
        int draw_active = (active_index == (int)i);

        if (fractus_ui_draw_button(framebuffer, fonts, &entry, draw_active) != FRACTUS_STATUS_OK) {
            return FRACTUS_STATUS_ERROR;
        }
    }

    return fractus_ui_draw_text_centered(
        framebuffer,
        fonts,
        FRACTUS_FONT_SMALL,
        320,
        373,
        0u,
        "Pulsa Dibujar para generar el atractor o Cancelar para volver.");
}

fractus_status fractus_app_run_attractors_menu_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_app_attractor_method *selected_method,
    fractus_app_view *view)
{
    static const fractus_app_menu_entry action_buttons[] = {
        {FRACTUS_APP_RECT(210, 395, 310, 415), 8u, 7u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 395, 430, 415), 0u, 15u, "Cancelar"}
    };
    fractus_ui_menu_option menu_options[13];
    int selected_menu = -1;
    int cancelled = 0;
    size_t i;
    const size_t method_count = FRACTUS_APP_ARRAY_COUNT(fractus_app_attractor_method_buttons);

    if (framebuffer == NULL || fonts == NULL || ui == NULL || selected_method == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    for (i = 0u; i < method_count; ++i) {
        menu_options[i].bounds = fractus_app_attractor_method_buttons[i].bounds;
    }
    menu_options[method_count].bounds = action_buttons[0].bounds;
    menu_options[method_count + 1u].bounds = action_buttons[1].bounds;

    /* 1. Contenedor exterior y botones */
    if (fractus_app_render_attractors_menu(
            framebuffer,
            fonts,
            fractus_ui_active_menu_index(ui, menu_options, FRACTUS_APP_ARRAY_COUNT(menu_options))) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 2. Raton y acciones de menu */
    if (fractus_ui_menu(ui, menu_options, FRACTUS_APP_ARRAY_COUNT(menu_options), &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == (int)(method_count + 1u)) {
            *selected_method = FRACTUS_APP_ATTRACTOR_METHOD_NONE;
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (selected_menu >= 0 && selected_menu < (int)method_count) {
            *selected_method = (fractus_app_attractor_method)selected_menu;
            *view = FRACTUS_APP_VIEW_ATTRACTORS_CONFIG;
        }
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_app_run_attractors_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_app_attractor_method *selected_method,
    fractus_app_view *view)
{
    static const fractus_app_menu_entry action_buttons[] = {
        {FRACTUS_APP_RECT(210, 395, 310, 415), 8u, 0u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 395, 430, 415), 0u, 15u, "Cancelar"}
    };
    fractus_ui_menu_option action_options[FRACTUS_APP_ARRAY_COUNT(action_buttons)];
    int selected_menu = -1;
    int cancelled = 0;

    if (framebuffer == NULL || fonts == NULL || ui == NULL || selected_method == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    fractus_app_build_options_from_entries(
        action_options,
        action_buttons,
        FRACTUS_APP_ARRAY_COUNT(action_buttons));

    /* 1. Contenedor exterior con toggle button presionado a la izquierda y panel de parametros a la derecha */
    if (fractus_app_render_attractors_config(
            framebuffer,
            fonts,
            fractus_ui_active_menu_index(ui, action_options, FRACTUS_APP_ARRAY_COUNT(action_options)),
            *selected_method) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 2. Comprobar si se hace clic sobre el toggle button ya presionado para soltarlo y volver a la seleccion */
    if (ui->release_pending && ui->release_event.buttons.left) {
        const fractus_point_i32 click_pos = ui->release_event.position;
        if (*selected_method >= 0 && *selected_method < FRACTUS_APP_ATTRACTOR_METHOD_COUNT) {
            if (fractus_ui_point_in_rect(click_pos, fractus_app_attractor_method_buttons[(int)*selected_method].bounds)) {
                *selected_method = FRACTUS_APP_ATTRACTOR_METHOD_NONE;
                *view = FRACTUS_APP_VIEW_ATTRACTORS_MENU;
                return FRACTUS_STATUS_OK;
            }
        }
    }

    /* 3. Raton y acciones de botones de accion (Dibujar / Cancelar) */
    if (fractus_ui_menu(ui, action_options, FRACTUS_APP_ARRAY_COUNT(action_options), &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == 1) {
            *selected_method = FRACTUS_APP_ATTRACTOR_METHOD_NONE;
            *view = FRACTUS_APP_VIEW_ATTRACTORS_MENU;
        } else if (selected_menu == 0) {
            fractus_app_log("runtime: attractor draw requested");
        }
    }

    return FRACTUS_STATUS_OK;
}
