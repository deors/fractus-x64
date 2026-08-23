#include "app/attractors.h"
#include "app/files.h"

#include <stdio.h>
#include <stdlib.h>
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

static void fractus_app_draw_line(
    fractus_framebuffer *framebuffer,
    int32_t x0,
    int32_t y0,
    int32_t x1,
    int32_t y1,
    uint8_t color,
    fractus_rect_i32 clip)
{
    int32_t dx = abs(x1 - x0);
    int32_t dy = -abs(y1 - y0);
    int32_t sx = (x0 < x1) ? 1 : -1;
    int32_t sy = (y0 < y1) ? 1 : -1;
    int32_t err = dx + dy;
    int32_t clip_x2 = clip.x + clip.width;
    int32_t clip_y2 = clip.y + clip.height;
    int32_t pitch = (int32_t)framebuffer->pitch_pixels;

    while (1) {
        if (x0 >= clip.x && x0 < clip_x2 && y0 >= clip.y && y0 < clip_y2) {
            framebuffer->index_pixels[(size_t)y0 * (size_t)pitch + (size_t)x0] = color;
        }
        if (x0 == x1 && y0 == y1) {
            break;
        }
        {
            int32_t e2 = 2 * err;
            if (e2 >= dy) {
                err += dy;
                x0 += sx;
            }
            if (e2 <= dx) {
                err += dx;
                y0 += sy;
            }
        }
    }
}

static void fractus_app_draw_3d_axes(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_rect_i32 box,
    double rot_x,
    double rot_y,
    double rot_z,
    int is_active)
{
    int32_t cx = box.x + box.width / 2;
    int32_t cy = box.y + box.height / 2;
    const double L_axis = 22.0;
    const double L_label = 30.0;
    double ux, vx, uy, vy, uz, vz;
    double lx, ly;
    int32_t px_x, py_x, px_y, py_y, px_z, py_z;
    int32_t lbl_x, lbl_y;

    /* 1. Fondo y marco 3D */
    (void)fractus_ui_fill_edges(framebuffer, box.x, box.y, box.x + box.width, box.y + box.height, 0u, 0u);
    (void)fractus_ui_draw_frame(framebuffer, box.x, box.y, box.x + box.width, box.y + box.height);

    /* 2. Proyeccion 3D de las lineas de los ejes */
    fractus_lorenz_project_point_3d(L_axis, 0.0, 0.0, rot_x, rot_y, rot_z, &ux, &vx);
    fractus_lorenz_project_point_3d(0.0, L_axis, 0.0, rot_x, rot_y, rot_z, &uy, &vy);
    fractus_lorenz_project_point_3d(0.0, 0.0, L_axis, rot_x, rot_y, rot_z, &uz, &vz);

    px_x = cx + (int32_t)(ux + (ux >= 0.0 ? 0.5 : -0.5));
    py_x = cy - (int32_t)(vx + (vx >= 0.0 ? 0.5 : -0.5));
    px_y = cx + (int32_t)(uy + (uy >= 0.0 ? 0.5 : -0.5));
    py_y = cy - (int32_t)(vy + (vy >= 0.0 ? 0.5 : -0.5));
    px_z = cx + (int32_t)(uz + (uz >= 0.0 ? 0.5 : -0.5));
    py_z = cy - (int32_t)(vz + (vz >= 0.0 ? 0.5 : -0.5));

    /* 3. Semiejes negativos atenuados */
    fractus_app_draw_line(framebuffer, cx, cy, cx - (int32_t)(ux * 0.6), cy + (int32_t)(vx * 0.6), 8u, box);
    fractus_app_draw_line(framebuffer, cx, cy, cx - (int32_t)(uy * 0.6), cy + (int32_t)(vy * 0.6), 8u, box);
    fractus_app_draw_line(framebuffer, cx, cy, cx - (int32_t)(uz * 0.6), cy + (int32_t)(vz * 0.6), 8u, box);

    /* 4. Semiejes positivos con colores distintivos */
    fractus_app_draw_line(framebuffer, cx, cy, px_x, py_x, 12u, box); /* X: Rojo */
    fractus_app_draw_line(framebuffer, cx, cy, px_y, py_y, 10u, box); /* Y: Verde */
    fractus_app_draw_line(framebuffer, cx, cy, px_z, py_z, 11u, box); /* Z: Cyan */

    /* 5. Punto de origen */
    if (cx >= box.x && cx < box.x + box.width && cy >= box.y && cy < box.y + box.height) {
        framebuffer->index_pixels[(size_t)cy * framebuffer->pitch_pixels + (size_t)cx] = 15u;
    }

    /* 6. Etiquetas separadas de los extremos de los ejes */
    /* Eje X */
    fractus_lorenz_project_point_3d(L_label, 0.0, 0.0, rot_x, rot_y, rot_z, &lx, &ly);
    lbl_x = cx + (int32_t)(lx + (lx >= 0.0 ? 0.5 : -0.5)) - 2;
    lbl_y = cy - (int32_t)(ly + (ly >= 0.0 ? 0.5 : -0.5)) - 4;
    if (lbl_x < box.x + 3) lbl_x = box.x + 3;
    if (lbl_x > box.x + box.width - 9) lbl_x = box.x + box.width - 9;
    if (lbl_y < box.y + 3) lbl_y = box.y + 3;
    if (lbl_y > box.y + box.height - 18) lbl_y = box.y + box.height - 18;
    (void)fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, lbl_x, lbl_y, 12u, "X");

    /* Eje Y */
    fractus_lorenz_project_point_3d(0.0, L_label, 0.0, rot_x, rot_y, rot_z, &lx, &ly);
    lbl_x = cx + (int32_t)(lx + (lx >= 0.0 ? 0.5 : -0.5)) - 2;
    lbl_y = cy - (int32_t)(ly + (ly >= 0.0 ? 0.5 : -0.5)) - 4;
    if (lbl_x < box.x + 3) lbl_x = box.x + 3;
    if (lbl_x > box.x + box.width - 9) lbl_x = box.x + box.width - 9;
    if (lbl_y < box.y + 3) lbl_y = box.y + 3;
    if (lbl_y > box.y + box.height - 18) lbl_y = box.y + box.height - 18;
    (void)fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, lbl_x, lbl_y, 10u, "Y");

    /* Eje Z */
    fractus_lorenz_project_point_3d(0.0, 0.0, L_label, rot_x, rot_y, rot_z, &lx, &ly);
    lbl_x = cx + (int32_t)(lx + (lx >= 0.0 ? 0.5 : -0.5)) - 2;
    lbl_y = cy - (int32_t)(ly + (ly >= 0.0 ? 0.5 : -0.5)) - 4;
    if (lbl_x < box.x + 3) lbl_x = box.x + 3;
    if (lbl_x > box.x + box.width - 9) lbl_x = box.x + box.width - 9;
    if (lbl_y < box.y + 3) lbl_y = box.y + 3;
    if (lbl_y > box.y + box.height - 18) lbl_y = box.y + box.height - 18;
    (void)fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, lbl_x, lbl_y, 11u, "Z");

    /* 7. Texto en la esquina inferior */
    (void)fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, cx, box.y + box.height - 13, is_active ? 0u : 7u, "Proyeccion 3D");
}

void fractus_app_init_lorenz_fields(
    fractus_app_lorenz_fields *fields,
    double sigma,
    double rho,
    double beta,
    double dt,
    uint32_t iterations,
    double rot_x,
    double rot_y,
    double rot_z)
{
    if (fields == NULL) {
        return;
    }

    (void)fractus_ui_numeric_field_init_float(&fields->sigma, (fractus_rect_i32){385, 104, 60, 20}, sigma, 0.10, 50.00, 2);
    (void)fractus_ui_numeric_field_init_float(&fields->rho, (fractus_rect_i32){385, 130, 60, 20}, rho, 0.10, 100.00, 2);
    (void)fractus_ui_numeric_field_init_float(&fields->beta, (fractus_rect_i32){385, 156, 60, 20}, beta, 0.10, 20.00, 2);
    (void)fractus_ui_numeric_field_init_float(&fields->dt, (fractus_rect_i32){385, 182, 60, 20}, dt, 0.001, 0.100, 3);
    (void)fractus_ui_numeric_field_init_int(&fields->iterations, (fractus_rect_i32){385, 208, 60, 20}, (int32_t)iterations, 1000, 99999);
    (void)fractus_ui_numeric_field_init_float(&fields->rot_x, (fractus_rect_i32){440, 264, 65, 20}, rot_x, -180.0, 180.0, 1);
    (void)fractus_ui_numeric_field_init_float(&fields->rot_y, (fractus_rect_i32){440, 288, 65, 20}, rot_y, -180.0, 180.0, 1);
    (void)fractus_ui_numeric_field_init_float(&fields->rot_z, (fractus_rect_i32){440, 312, 65, 20}, rot_z, -180.0, 180.0, 1);
}

static size_t fractus_app_build_lorenz_config_entries(
    fractus_app_menu_entry *entries,
    size_t capacity)
{
    static const fractus_app_menu_entry controls[] = {
        {FRACTUS_APP_RECT(450, 104, 480, 124), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(485, 104, 515, 124), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(450, 130, 480, 150), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(485, 130, 515, 150), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(450, 156, 480, 176), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(485, 156, 515, 176), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(450, 182, 480, 202), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(485, 182, 515, 202), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(450, 208, 480, 228), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(485, 208, 515, 228), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(340, 236, 380, 256), 8u, 0u, "X-Z"},
        {FRACTUS_APP_RECT(385, 236, 425, 256), 8u, 0u, "X-Y"},
        {FRACTUS_APP_RECT(430, 236, 470, 256), 8u, 0u, "Y-Z"},
        {FRACTUS_APP_RECT(475, 236, 515, 256), 8u, 0u, "3D"},
        {FRACTUS_APP_RECT(210, 395, 310, 415), 8u, 0u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 395, 430, 415), 0u, 15u, "Cancelar"}
    };

    return fractus_app_copy_control_entries(
        entries,
        capacity,
        controls,
        FRACTUS_APP_ARRAY_COUNT(controls));
}

static fractus_status fractus_app_render_attractors_base(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_app_attractor_method selected_method,
    int is_config_view)
{
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
        int draw_active = 0;

        if (is_config_view) {
            if (i == (uint32_t)selected_method) {
                entry.fill_color = 8u;
                entry.text_color = 0u;
                draw_active = 1;
            } else {
                entry.fill_color = 8u;
                entry.text_color = 7u;
                draw_active = 0;
            }
        } else {
            entry.fill_color = 8u;
            entry.text_color = 0u;
            draw_active = 0;
        }

        if (fractus_ui_draw_button(framebuffer, fonts, &entry, draw_active) != FRACTUS_STATUS_OK) {
            return FRACTUS_STATUS_ERROR;
        }
    }

    return FRACTUS_STATUS_OK;
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

    if (fractus_app_render_attractors_base(
            framebuffer,
            fonts,
            FRACTUS_APP_ATTRACTOR_METHOD_NONE,
            0) != FRACTUS_STATUS_OK) {
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

fractus_status fractus_app_run_attractors_menu_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_lorenz_params *lorenz_params,
    fractus_lorenz_params *lorenz_pending,
    fractus_app_attractor_fields *attractor_fields,
    fractus_app_attractor_method *selected_method,
    fractus_app_view *view)
{
    fractus_ui_menu_option options[13];
    int selected_menu = -1;
    int cancelled = 0;
    size_t i;
    const size_t method_count = FRACTUS_APP_ARRAY_COUNT(fractus_app_attractor_method_buttons);

    (void)lorenz_params;
    if (framebuffer == NULL || fonts == NULL || ui == NULL ||
        lorenz_pending == NULL || attractor_fields == NULL || selected_method == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    for (i = 0u; i < method_count; ++i) {
        options[i].bounds = fractus_app_attractor_method_buttons[i].bounds;
    }
    options[method_count].bounds = (fractus_rect_i32)FRACTUS_APP_RECT(210, 395, 310, 415);     /* Dibujar (disabled) */
    options[method_count + 1u].bounds = (fractus_rect_i32)FRACTUS_APP_RECT(330, 395, 430, 415); /* Cancelar */

    if (fractus_app_render_attractors_menu(
            framebuffer,
            fonts,
            fractus_ui_active_menu_index(ui, options, method_count + 2u)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    if (fractus_ui_menu(ui, options, method_count + 2u, &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == (int)(method_count + 1u)) {
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (selected_menu == 0) {
            *selected_method = FRACTUS_APP_ATTRACTOR_METHOD_LORENZ;
            *lorenz_pending = *lorenz_params;
            fractus_app_init_lorenz_fields(
                &attractor_fields->lorenz,
                lorenz_pending->sigma,
                lorenz_pending->rho,
                lorenz_pending->beta,
                lorenz_pending->dt,
                lorenz_pending->iterations,
                lorenz_pending->rot_x,
                lorenz_pending->rot_y,
                lorenz_pending->rot_z);
            *view = FRACTUS_APP_VIEW_ATTRACTORS_CONFIG;
        }
    }

    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_app_run_lorenz_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_lorenz_params *params,
    fractus_lorenz_params *pending,
    fractus_app_lorenz_fields *fields,
    fractus_app_view *view)
{
    const fractus_ui_radio_option proj_options[] = {
        {FRACTUS_APP_RECT(340, 236, 380, 256), "X-Z"},
        {FRACTUS_APP_RECT(385, 236, 425, 256), "X-Y"},
        {FRACTUS_APP_RECT(430, 236, 470, 256), "Y-Z"},
        {FRACTUS_APP_RECT(475, 236, 515, 256), "3D"}
    };
    fractus_app_menu_entry dialog_entries[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    fractus_ui_menu_option dialog_options[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    fractus_rect_i32 box_3d = {248, 262, 90, 80};
    size_t dialog_entry_count;
    int selected_menu = -1;
    int cancelled = 0;
    int skip_mouse_input = 0;
    int active_index;
    int is_3d_active;

    if (framebuffer == NULL || fonts == NULL || ui == NULL ||
        params == NULL || pending == NULL || fields == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    dialog_entry_count = fractus_app_build_lorenz_config_entries(dialog_entries, FRACTUS_APP_ARRAY_COUNT(dialog_entries));
    fractus_app_build_options_from_entries(dialog_options, dialog_entries, dialog_entry_count);
    active_index = fractus_ui_active_menu_index(ui, dialog_options, dialog_entry_count);
    is_3d_active = (pending->projection == FRACTUS_LORENZ_PROJECTION_CUSTOM);

    /* 1. Base y panel derecho */
    if (fractus_app_render_attractors_base(
            framebuffer,
            fonts,
            FRACTUS_APP_ATTRACTOR_METHOD_LORENZ,
            1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_group_box(
            framebuffer,
            fonts,
            238, 85, 519, 365,
            8u, 0u,
            "Parametros del atractor de Lorenz") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 2. Textos, campos numericos y radio buttons */
    if (fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 248, 108, 0u, "Sigma (0.10-50.00)") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->sigma) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 248, 134, 0u, "Rho (0.10-100.00)") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->rho) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 248, 160, 0u, "Beta (0.10-20.00)") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->beta) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 248, 186, 0u, "Paso dt (0.001-0.100)") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->dt) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 248, 212, 0u, "Iteraciones (1000-99999)") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->iterations) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 248, 240, 0u, "Proyeccion") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_radio_list(
            framebuffer,
            fonts,
            proj_options,
            FRACTUS_APP_ARRAY_COUNT(proj_options),
            (int)pending->projection,
            (active_index >= FRACTUS_APP_LORENZ_PROJ_XZ && active_index <= FRACTUS_APP_LORENZ_PROJ_3D) ?
                active_index - FRACTUS_APP_LORENZ_PROJ_XZ : -1) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Selector de perspectiva 3D interactivo */
    fractus_app_draw_3d_axes(framebuffer, fonts, box_3d, pending->rot_x, pending->rot_y, pending->rot_z, is_3d_active);

    /* 4. Campos numericos de rotacion */
    if (fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 372, 268, 0u, "Angulo X") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->rot_x) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 372, 292, 0u, "Angulo Y") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->rot_y) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 372, 316, 0u, "Angulo Z") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, &fields->rot_z) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, 378, 348, 7u, "Punto inicial: (0.1, 0.0, 0.0)") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(
            framebuffer,
            fonts,
            FRACTUS_FONT_SMALL,
            320,
            373,
            0u,
            "Estos valores solo afectan al dibujo actual y no cambian la cfg global.") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_button_list(
            framebuffer,
            fonts,
            dialog_entries,
            10u,
            (active_index < 10) ? active_index : -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_button_list(
            framebuffer,
            fonts,
            &dialog_entries[FRACTUS_APP_LORENZ_DRAW],
            2u,
            (active_index >= FRACTUS_APP_LORENZ_DRAW) ? active_index - FRACTUS_APP_LORENZ_DRAW : -1) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 5. Interaccion con el selector 3D (arrastre con raton) */
    if (ui->press_pending && fractus_ui_point_in_rect(ui->press_event.position, box_3d)) {
        fields->is_dragging_3d = (ui->press_event.buttons.right) ? 2 : 1;
        fields->drag_start_pos = ui->press_event.position;
        fields->drag_start_rot_x = pending->rot_x;
        fields->drag_start_rot_y = pending->rot_y;
        fields->drag_start_rot_z = pending->rot_z;
        pending->projection = FRACTUS_LORENZ_PROJECTION_CUSTOM;
    }

    if (fields->is_dragging_3d != 0) {
        if (!ui->buttons_down.left && !ui->buttons_down.right) {
            fields->is_dragging_3d = 0;
        } else {
            int32_t dx = ui->pointer_position.x - fields->drag_start_pos.x;
            int32_t dy = ui->pointer_position.y - fields->drag_start_pos.y;

            if (fields->is_dragging_3d == 1) {
                double nx = fields->drag_start_rot_x - (double)dy * 1.5;
                double ny = fields->drag_start_rot_y + (double)dx * 1.5;
                while (nx > 180.0) nx -= 360.0;
                while (nx < -180.0) nx += 360.0;
                while (ny > 180.0) ny -= 360.0;
                while (ny < -180.0) ny += 360.0;
                pending->rot_x = nx;
                pending->rot_y = ny;
            } else if (fields->is_dragging_3d == 2) {
                double nz = fields->drag_start_rot_z + (double)dx * 1.5;
                while (nz > 180.0) nz -= 360.0;
                while (nz < -180.0) nz += 360.0;
                pending->rot_z = nz;
            }

            pending->projection = FRACTUS_LORENZ_PROJECTION_CUSTOM;
            fractus_app_init_lorenz_fields(
                fields,
                pending->sigma,
                pending->rho,
                pending->beta,
                pending->dt,
                pending->iterations,
                pending->rot_x,
                pending->rot_y,
                pending->rot_z);
            skip_mouse_input = 1;
        }
    }

    /* 6. Gestion de foco y edicion de campos */
    if (ui->release_pending && ui->release_event.buttons.left && !fields->is_dragging_3d) {
        fractus_point_i32 click_pos = ui->release_event.position;
        fractus_ui_numeric_field *clicked_field = NULL;

        /* Re-clic sobre el boton Lorenz para deseleccionar y volver al menu */
        if (fractus_ui_point_in_rect(click_pos, fractus_app_attractor_method_buttons[0].bounds)) {
            *view = FRACTUS_APP_VIEW_ATTRACTORS_MENU;
            return FRACTUS_STATUS_OK;
        }

        if (fractus_ui_point_in_rect(click_pos, fields->sigma.bounds)) {
            clicked_field = &fields->sigma;
        } else if (fractus_ui_point_in_rect(click_pos, fields->rho.bounds)) {
            clicked_field = &fields->rho;
        } else if (fractus_ui_point_in_rect(click_pos, fields->beta.bounds)) {
            clicked_field = &fields->beta;
        } else if (fractus_ui_point_in_rect(click_pos, fields->dt.bounds)) {
            clicked_field = &fields->dt;
        } else if (fractus_ui_point_in_rect(click_pos, fields->iterations.bounds)) {
            clicked_field = &fields->iterations;
        } else if (fractus_ui_point_in_rect(click_pos, fields->rot_x.bounds)) {
            clicked_field = &fields->rot_x;
        } else if (fractus_ui_point_in_rect(click_pos, fields->rot_y.bounds)) {
            clicked_field = &fields->rot_y;
        } else if (fractus_ui_point_in_rect(click_pos, fields->rot_z.bounds)) {
            clicked_field = &fields->rot_z;
        }

        if (clicked_field != NULL) {
            double fval;
            int32_t ival;
            if (fields->sigma.editing && fractus_ui_numeric_field_get_float(&fields->sigma, &fval) == FRACTUS_STATUS_OK) {
                pending->sigma = fval;
            }
            if (fields->rho.editing && fractus_ui_numeric_field_get_float(&fields->rho, &fval) == FRACTUS_STATUS_OK) {
                pending->rho = fval;
            }
            if (fields->beta.editing && fractus_ui_numeric_field_get_float(&fields->beta, &fval) == FRACTUS_STATUS_OK) {
                pending->beta = fval;
            }
            if (fields->dt.editing && fractus_ui_numeric_field_get_float(&fields->dt, &fval) == FRACTUS_STATUS_OK) {
                pending->dt = fval;
            }
            if (fields->iterations.editing && fractus_ui_numeric_field_get_int(&fields->iterations, &ival) == FRACTUS_STATUS_OK) {
                pending->iterations = (uint32_t)ival;
            }
            if (fields->rot_x.editing && fractus_ui_numeric_field_get_float(&fields->rot_x, &fval) == FRACTUS_STATUS_OK) {
                pending->rot_x = fval;
                pending->projection = FRACTUS_LORENZ_PROJECTION_CUSTOM;
            }
            if (fields->rot_y.editing && fractus_ui_numeric_field_get_float(&fields->rot_y, &fval) == FRACTUS_STATUS_OK) {
                pending->rot_y = fval;
                pending->projection = FRACTUS_LORENZ_PROJECTION_CUSTOM;
            }
            if (fields->rot_z.editing && fractus_ui_numeric_field_get_float(&fields->rot_z, &fval) == FRACTUS_STATUS_OK) {
                pending->rot_z = fval;
                pending->projection = FRACTUS_LORENZ_PROJECTION_CUSTOM;
            }

            fractus_app_init_lorenz_fields(
                fields,
                pending->sigma,
                pending->rho,
                pending->beta,
                pending->dt,
                pending->iterations,
                pending->rot_x,
                pending->rot_y,
                pending->rot_z);

            if (clicked_field == &fields->sigma) {
                (void)fractus_ui_numeric_field_begin_edit(&fields->sigma);
            } else if (clicked_field == &fields->rho) {
                (void)fractus_ui_numeric_field_begin_edit(&fields->rho);
            } else if (clicked_field == &fields->beta) {
                (void)fractus_ui_numeric_field_begin_edit(&fields->beta);
            } else if (clicked_field == &fields->dt) {
                (void)fractus_ui_numeric_field_begin_edit(&fields->dt);
            } else if (clicked_field == &fields->iterations) {
                (void)fractus_ui_numeric_field_begin_edit(&fields->iterations);
            } else if (clicked_field == &fields->rot_x) {
                (void)fractus_ui_numeric_field_begin_edit(&fields->rot_x);
            } else if (clicked_field == &fields->rot_y) {
                (void)fractus_ui_numeric_field_begin_edit(&fields->rot_y);
            } else if (clicked_field == &fields->rot_z) {
                (void)fractus_ui_numeric_field_begin_edit(&fields->rot_z);
            }
        } else {
            double fval;
            int32_t ival;
            if (fields->sigma.editing && fractus_ui_numeric_field_get_float(&fields->sigma, &fval) == FRACTUS_STATUS_OK) {
                pending->sigma = fval;
            }
            if (fields->rho.editing && fractus_ui_numeric_field_get_float(&fields->rho, &fval) == FRACTUS_STATUS_OK) {
                pending->rho = fval;
            }
            if (fields->beta.editing && fractus_ui_numeric_field_get_float(&fields->beta, &fval) == FRACTUS_STATUS_OK) {
                pending->beta = fval;
            }
            if (fields->dt.editing && fractus_ui_numeric_field_get_float(&fields->dt, &fval) == FRACTUS_STATUS_OK) {
                pending->dt = fval;
            }
            if (fields->iterations.editing && fractus_ui_numeric_field_get_int(&fields->iterations, &ival) == FRACTUS_STATUS_OK) {
                pending->iterations = (uint32_t)ival;
            }
            if (fields->rot_x.editing && fractus_ui_numeric_field_get_float(&fields->rot_x, &fval) == FRACTUS_STATUS_OK) {
                pending->rot_x = fval;
                pending->projection = FRACTUS_LORENZ_PROJECTION_CUSTOM;
            }
            if (fields->rot_y.editing && fractus_ui_numeric_field_get_float(&fields->rot_y, &fval) == FRACTUS_STATUS_OK) {
                pending->rot_y = fval;
                pending->projection = FRACTUS_LORENZ_PROJECTION_CUSTOM;
            }
            if (fields->rot_z.editing && fractus_ui_numeric_field_get_float(&fields->rot_z, &fval) == FRACTUS_STATUS_OK) {
                pending->rot_z = fval;
                pending->projection = FRACTUS_LORENZ_PROJECTION_CUSTOM;
            }
            fractus_app_init_lorenz_fields(
                fields,
                pending->sigma,
                pending->rho,
                pending->beta,
                pending->dt,
                pending->iterations,
                pending->rot_x,
                pending->rot_y,
                pending->rot_z);
        }
    }

    /* 7. Edicion activa por teclado */
    {
        int edit_accepted = 0;
        int edit_cancelled = 0;
        fractus_ui_numeric_field *active_field = NULL;

        if (fields->sigma.editing) active_field = &fields->sigma;
        else if (fields->rho.editing) active_field = &fields->rho;
        else if (fields->beta.editing) active_field = &fields->beta;
        else if (fields->dt.editing) active_field = &fields->dt;
        else if (fields->iterations.editing) active_field = &fields->iterations;
        else if (fields->rot_x.editing) active_field = &fields->rot_x;
        else if (fields->rot_y.editing) active_field = &fields->rot_y;
        else if (fields->rot_z.editing) active_field = &fields->rot_z;

        if (active_field != NULL) {
            if (fractus_ui_numeric_field_handle_input(active_field, ui, fonts, &edit_accepted, &edit_cancelled) == FRACTUS_STATUS_OK) {
                if (edit_accepted) {
                    double fval;
                    int32_t ival;
                    if (active_field == &fields->sigma && fractus_ui_numeric_field_get_float(active_field, &fval) == FRACTUS_STATUS_OK) {
                        pending->sigma = fval;
                    } else if (active_field == &fields->rho && fractus_ui_numeric_field_get_float(active_field, &fval) == FRACTUS_STATUS_OK) {
                        pending->rho = fval;
                    } else if (active_field == &fields->beta && fractus_ui_numeric_field_get_float(active_field, &fval) == FRACTUS_STATUS_OK) {
                        pending->beta = fval;
                    } else if (active_field == &fields->dt && fractus_ui_numeric_field_get_float(active_field, &fval) == FRACTUS_STATUS_OK) {
                        pending->dt = fval;
                    } else if (active_field == &fields->iterations && fractus_ui_numeric_field_get_int(active_field, &ival) == FRACTUS_STATUS_OK) {
                        pending->iterations = (uint32_t)ival;
                    } else if (active_field == &fields->rot_x && fractus_ui_numeric_field_get_float(active_field, &fval) == FRACTUS_STATUS_OK) {
                        pending->rot_x = fval;
                        pending->projection = FRACTUS_LORENZ_PROJECTION_CUSTOM;
                    } else if (active_field == &fields->rot_y && fractus_ui_numeric_field_get_float(active_field, &fval) == FRACTUS_STATUS_OK) {
                        pending->rot_y = fval;
                        pending->projection = FRACTUS_LORENZ_PROJECTION_CUSTOM;
                    } else if (active_field == &fields->rot_z && fractus_ui_numeric_field_get_float(active_field, &fval) == FRACTUS_STATUS_OK) {
                        pending->rot_z = fval;
                        pending->projection = FRACTUS_LORENZ_PROJECTION_CUSTOM;
                    }
                    fractus_app_init_lorenz_fields(
                        fields,
                        pending->sigma,
                        pending->rho,
                        pending->beta,
                        pending->dt,
                        pending->iterations,
                        pending->rot_x,
                        pending->rot_y,
                        pending->rot_z);
                } else if (edit_cancelled) {
                    fractus_app_init_lorenz_fields(
                        fields,
                        pending->sigma,
                        pending->rho,
                        pending->beta,
                        pending->dt,
                        pending->iterations,
                        pending->rot_x,
                        pending->rot_y,
                        pending->rot_z);
                }
            }
            skip_mouse_input = 1;
        }
    }

    /* 8. Raton y acciones de botones y radio list */
    if (!skip_mouse_input && fractus_ui_menu(ui, dialog_options, dialog_entry_count, &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == FRACTUS_APP_LORENZ_CANCEL) {
            *view = FRACTUS_APP_VIEW_ATTRACTORS_MENU;
        } else if (selected_menu == FRACTUS_APP_LORENZ_DRAW) {
            *params = *pending;
            *view = FRACTUS_APP_VIEW_LORENZ;
        } else if (selected_menu == FRACTUS_APP_LORENZ_SIGMA_DEC) {
            pending->sigma = fractus_app_clamp_f64(pending->sigma - 0.5, 0.10, 50.00);
            fractus_app_init_lorenz_fields(fields, pending->sigma, pending->rho, pending->beta, pending->dt, pending->iterations, pending->rot_x, pending->rot_y, pending->rot_z);
        } else if (selected_menu == FRACTUS_APP_LORENZ_SIGMA_INC) {
            pending->sigma = fractus_app_clamp_f64(pending->sigma + 0.5, 0.10, 50.00);
            fractus_app_init_lorenz_fields(fields, pending->sigma, pending->rho, pending->beta, pending->dt, pending->iterations, pending->rot_x, pending->rot_y, pending->rot_z);
        } else if (selected_menu == FRACTUS_APP_LORENZ_RHO_DEC) {
            pending->rho = fractus_app_clamp_f64(pending->rho - 1.0, 0.10, 100.00);
            fractus_app_init_lorenz_fields(fields, pending->sigma, pending->rho, pending->beta, pending->dt, pending->iterations, pending->rot_x, pending->rot_y, pending->rot_z);
        } else if (selected_menu == FRACTUS_APP_LORENZ_RHO_INC) {
            pending->rho = fractus_app_clamp_f64(pending->rho + 1.0, 0.10, 100.00);
            fractus_app_init_lorenz_fields(fields, pending->sigma, pending->rho, pending->beta, pending->dt, pending->iterations, pending->rot_x, pending->rot_y, pending->rot_z);
        } else if (selected_menu == FRACTUS_APP_LORENZ_BETA_DEC) {
            pending->beta = fractus_app_clamp_f64(pending->beta - 0.1, 0.10, 20.00);
            fractus_app_init_lorenz_fields(fields, pending->sigma, pending->rho, pending->beta, pending->dt, pending->iterations, pending->rot_x, pending->rot_y, pending->rot_z);
        } else if (selected_menu == FRACTUS_APP_LORENZ_BETA_INC) {
            pending->beta = fractus_app_clamp_f64(pending->beta + 0.1, 0.10, 20.00);
            fractus_app_init_lorenz_fields(fields, pending->sigma, pending->rho, pending->beta, pending->dt, pending->iterations, pending->rot_x, pending->rot_y, pending->rot_z);
        } else if (selected_menu == FRACTUS_APP_LORENZ_DT_DEC) {
            pending->dt = fractus_app_clamp_f64(pending->dt - 0.001, 0.001, 0.100);
            fractus_app_init_lorenz_fields(fields, pending->sigma, pending->rho, pending->beta, pending->dt, pending->iterations, pending->rot_x, pending->rot_y, pending->rot_z);
        } else if (selected_menu == FRACTUS_APP_LORENZ_DT_INC) {
            pending->dt = fractus_app_clamp_f64(pending->dt + 0.001, 0.001, 0.100);
            fractus_app_init_lorenz_fields(fields, pending->sigma, pending->rho, pending->beta, pending->dt, pending->iterations, pending->rot_x, pending->rot_y, pending->rot_z);
        } else if (selected_menu == FRACTUS_APP_LORENZ_ITERATIONS_DEC) {
            pending->iterations = (pending->iterations > 1000u) ? (pending->iterations - 1000u) : 1000u;
            fractus_app_init_lorenz_fields(fields, pending->sigma, pending->rho, pending->beta, pending->dt, pending->iterations, pending->rot_x, pending->rot_y, pending->rot_z);
        } else if (selected_menu == FRACTUS_APP_LORENZ_ITERATIONS_INC) {
            pending->iterations = (pending->iterations <= 98999u) ? (pending->iterations + 1000u) : 99999u;
            fractus_app_init_lorenz_fields(fields, pending->sigma, pending->rho, pending->beta, pending->dt, pending->iterations, pending->rot_x, pending->rot_y, pending->rot_z);
        } else if (selected_menu == FRACTUS_APP_LORENZ_PROJ_XZ) {
            pending->projection = FRACTUS_LORENZ_PROJECTION_XZ;
            pending->rot_x = 0.0;
            pending->rot_y = 0.0;
            pending->rot_z = 0.0;
            fractus_app_init_lorenz_fields(fields, pending->sigma, pending->rho, pending->beta, pending->dt, pending->iterations, pending->rot_x, pending->rot_y, pending->rot_z);
        } else if (selected_menu == FRACTUS_APP_LORENZ_PROJ_XY) {
            pending->projection = FRACTUS_LORENZ_PROJECTION_XY;
            pending->rot_x = 90.0;
            pending->rot_y = 0.0;
            pending->rot_z = 0.0;
            fractus_app_init_lorenz_fields(fields, pending->sigma, pending->rho, pending->beta, pending->dt, pending->iterations, pending->rot_x, pending->rot_y, pending->rot_z);
        } else if (selected_menu == FRACTUS_APP_LORENZ_PROJ_YZ) {
            pending->projection = FRACTUS_LORENZ_PROJECTION_YZ;
            pending->rot_x = 0.0;
            pending->rot_y = 90.0;
            pending->rot_z = 0.0;
            fractus_app_init_lorenz_fields(fields, pending->sigma, pending->rho, pending->beta, pending->dt, pending->iterations, pending->rot_x, pending->rot_y, pending->rot_z);
        } else if (selected_menu == FRACTUS_APP_LORENZ_PROJ_3D) {
            pending->projection = FRACTUS_LORENZ_PROJECTION_CUSTOM;
            if (pending->rot_x == 0.0 && pending->rot_y == 0.0 && pending->rot_z == 0.0) {
                pending->rot_x = 35.0;
                pending->rot_y = 45.0;
                pending->rot_z = 0.0;
            }
            fractus_app_init_lorenz_fields(fields, pending->sigma, pending->rho, pending->beta, pending->dt, pending->iterations, pending->rot_x, pending->rot_y, pending->rot_z);
        }
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_app_run_attractors_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_lorenz_params *lorenz_params,
    fractus_lorenz_params *lorenz_pending,
    fractus_app_attractor_fields *attractor_fields,
    fractus_app_attractor_method *selected_method,
    fractus_app_view *view)
{
    if (selected_method == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (*selected_method == FRACTUS_APP_ATTRACTOR_METHOD_LORENZ) {
        return fractus_app_run_lorenz_config_view(
            framebuffer,
            fonts,
            ui,
            lorenz_params,
            lorenz_pending,
            &attractor_fields->lorenz,
            view);
    }

    *view = FRACTUS_APP_VIEW_ATTRACTORS_MENU;
    return FRACTUS_STATUS_OK;
}

fractus_status fractus_app_render_lorenz(
    const fractus_platform_context *platform,
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const fractus_lorenz_params *params,
    int *save_next_graphic,
    char *error_message,
    size_t error_message_size,
    char *saved_filename,
    size_t saved_filename_size)
{
    if (platform == NULL || framebuffer == NULL || fonts == NULL || params == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_fractal_render_lorenz(framebuffer, params) != FRACTUS_STATUS_OK ||
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
        "ESC o boton derecho: menu - G: grabar dibujo - F: flujo",
        saved_filename,
        0);
}
