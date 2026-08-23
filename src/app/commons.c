#include "app/commons.h"

#include <math.h>
#include <stdlib.h>

void fractus_app_set_button(
    fractus_app_menu_entry *entry,
    int32_t x1,
    int32_t y1,
    int32_t x2,
    int32_t y2,
    uint8_t fill_color,
    uint8_t text_color,
    const char *label)
{
    if (entry == NULL) {
        return;
    }

    entry->bounds.x = x1;
    entry->bounds.y = y1;
    entry->bounds.width = x2 - x1 + 1;
    entry->bounds.height = y2 - y1 + 1;
    entry->fill_color = fill_color;
    entry->text_color = text_color;
    entry->label = label;
}

int32_t fractus_app_clamp_i32(int32_t value, int32_t minimum, int32_t maximum)
{
    if (value < minimum) {
        return minimum;
    }

    if (value > maximum) {
        return maximum;
    }

    return value;
}

uint8_t fractus_app_scale_vga_to_rgb8(int32_t value)
{
    value = fractus_app_clamp_i32(value, 0, 63);
    return (uint8_t)((value * 255 + 31) / 63);
}

int32_t fractus_app_scale_rgb8_to_vga(uint8_t value)
{
    return (int32_t)((uint32_t)value * 63u + 127u) / 255;
}

fractus_color_rgba8 fractus_app_rgb8(uint8_t r, uint8_t g, uint8_t b)
{
    fractus_color_rgba8 color;

    color.r = r;
    color.g = g;
    color.b = b;
    color.a = 255u;
    return color;
}

fractus_color_rgba8 fractus_app_vga_color(int32_t r, int32_t g, int32_t b)
{
    return fractus_app_rgb8(
        fractus_app_scale_vga_to_rgb8(r),
        fractus_app_scale_vga_to_rgb8(g),
        fractus_app_scale_vga_to_rgb8(b));
}

void fractus_app_vga_channels_from_color(
    fractus_color_rgba8 color,
    int32_t *r,
    int32_t *g,
    int32_t *b)
{
    if (r != NULL) {
        *r = fractus_app_scale_rgb8_to_vga(color.r);
    }

    if (g != NULL) {
        *g = fractus_app_scale_rgb8_to_vga(color.g);
    }

    if (b != NULL) {
        *b = fractus_app_scale_rgb8_to_vga(color.b);
    }
}

static const int32_t color_x0 = 135;
static const int32_t color_y0 = 146;

void fractus_app_init_palette_color_fields(
    fractus_ui_numeric_field *red_field,
    fractus_ui_numeric_field *green_field,
    fractus_ui_numeric_field *blue_field,
    fractus_color_rgba8 color)
{
    const int32_t x0 = color_x0;
    const int32_t y0 = color_y0;
    int32_t red;
    int32_t green;
    int32_t blue;

    fractus_app_vga_channels_from_color(color, &red, &green, &blue);
    if (red_field != NULL) {
        (void)fractus_ui_numeric_field_init_int(red_field, (fractus_rect_i32){x0 + 60, y0 + 48, 43, 20}, red, 0, 63);
    }
    if (green_field != NULL) {
        (void)fractus_ui_numeric_field_init_int(green_field, (fractus_rect_i32){x0 + 60, y0 + 74, 43, 20}, green, 0, 63);
    }
    if (blue_field != NULL) {
        (void)fractus_ui_numeric_field_init_int(blue_field, (fractus_rect_i32){x0 + 60, y0 + 100, 43, 20}, blue, 0, 63);
    }
}

double fractus_app_clamp_f64(double value, double minimum, double maximum)
{
    if (value < minimum) {
        return minimum;
    }

    if (value > maximum) {
        return maximum;
    }

    return value;
}

void fractus_app_log(const char *message)
{
    FILE *file;

    if (message == NULL) {
        return;
    }

    file = fopen("fractus-x64.log", "ab");
    if (file == NULL) {
        return;
    }

    fwrite(message, strlen(message), 1u, file);
    fwrite("\r\n", 2u, 1u, file);
    fclose(file);
}

fractus_status fractus_app_set_message(
    char *destination,
    size_t destination_size,
    const char *message)
{
    if (destination == NULL || destination_size == 0u || message == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    return fractus_formats_copy_path(message, destination, destination_size);
}

int fractus_app_view_is_generated_drawing(fractus_app_view view)
{
    return view == FRACTUS_APP_VIEW_MANDELBROT ||
           view == FRACTUS_APP_VIEW_MANDELBROT_DEM ||
           view == FRACTUS_APP_VIEW_JULIA ||
           view == FRACTUS_APP_VIEW_JULIA_DEM ||
           view == FRACTUS_APP_VIEW_BIOMORPH ||
           view == FRACTUS_APP_VIEW_PLASMA_RECTANGULAR ||
           view == FRACTUS_APP_VIEW_PLASMA_CIRCULAR ||
           view == FRACTUS_APP_VIEW_LORENZ;
}

int fractus_app_view_supports_zone_selection(fractus_app_view view)
{
    return view == FRACTUS_APP_VIEW_MANDELBROT ||
           view == FRACTUS_APP_VIEW_MANDELBROT_DEM ||
           view == FRACTUS_APP_VIEW_JULIA ||
           view == FRACTUS_APP_VIEW_JULIA_DEM;
}

fractus_status fractus_app_resolve_numbered_write_path(
    const fractus_platform_context *platform,
    const char *directory_name,
    const char *extension,
    char *buffer,
    size_t buffer_size)
{
    char directory_path[512];
    char file_name[32];
    char candidate_path[512];
    uint32_t index;

    if (platform == NULL || directory_name == NULL || extension == NULL || buffer == NULL || buffer_size == 0u) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_formats_resolve_legacy_directory(platform, directory_name, directory_path, sizeof(directory_path)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    for (index = 1u; index <= FRACTUS_APP_NUMBERED_FILE_LIMIT; ++index) {
        int written = snprintf(file_name, sizeof(file_name), "fractus%04u%s", (unsigned)index, extension);
        if (written < 0 || (size_t)written >= sizeof(file_name)) {
            return FRACTUS_STATUS_ERROR;
        }

        if (fractus_formats_join_directory_path(
                directory_path,
                file_name,
                '/',
                candidate_path,
                sizeof(candidate_path)) != FRACTUS_STATUS_OK) {
            return FRACTUS_STATUS_ERROR;
        }

        if (!fractus_formats_file_exists(candidate_path)) {
            return fractus_formats_copy_path(candidate_path, buffer, buffer_size);
        }
    }

    return FRACTUS_STATUS_UNSUPPORTED;
}

fractus_status fractus_app_apply_palette(
    fractus_framebuffer *framebuffer,
    const fractus_palette *palette)
{
    if (framebuffer == NULL || palette == NULL || !framebuffer->initialized) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    framebuffer->palette = *palette;
    framebuffer->palette_dirty = 1;
    return FRACTUS_STATUS_OK;
}

fractus_status fractus_app_sync_framebuffer_palette(
    fractus_framebuffer *target,
    const fractus_framebuffer *source)
{
    if (target == NULL || source == NULL || !target->initialized || !source->initialized) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (memcmp(&target->palette, &source->palette, sizeof(target->palette)) != 0) {
        target->palette = source->palette;
        target->palette_dirty = 1;
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_app_resolve_drawing_video_mode(
    const fractus_legacy_config *config,
    fractus_size_u32 *size,
    int *fullscreen)
{
    if (config == NULL || size == NULL || fullscreen == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    *fullscreen = 0;
    switch (config->drawing_video_mode) {
    case FRACTUS_DRAWING_VIDEO_MODE_WINDOW_800X600:
        *size = (fractus_size_u32){800u, 600u};
        break;
    case FRACTUS_DRAWING_VIDEO_MODE_WINDOW_1024X768:
        *size = (fractus_size_u32){1024u, 768u};
        break;
    case FRACTUS_DRAWING_VIDEO_MODE_WINDOW_1280X960:
        *size = (fractus_size_u32){1280u, 960u};
        break;
    case FRACTUS_DRAWING_VIDEO_MODE_FULLSCREEN_1280X720:
        *size = (fractus_size_u32){1280u, 720u};
        *fullscreen = 1;
        break;
    case FRACTUS_DRAWING_VIDEO_MODE_FULLSCREEN_1920X1080:
        *size = (fractus_size_u32){1920u, 1080u};
        *fullscreen = 1;
        break;
    case FRACTUS_DRAWING_VIDEO_MODE_FULLSCREEN_2560X1440:
        *size = (fractus_size_u32){2560u, 1440u};
        *fullscreen = 1;
        break;
    case FRACTUS_DRAWING_VIDEO_MODE_FULLSCREEN_3840X2160:
        *size = (fractus_size_u32){3840u, 2160u};
        *fullscreen = 1;
        break;
    case FRACTUS_DRAWING_VIDEO_MODE_WINDOW_640X480:
    default:
        *size = (fractus_size_u32){640u, 480u};
        break;
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_app_resize_drawing_framebuffer(
    fractus_framebuffer *framebuffer,
    const fractus_framebuffer *palette_source,
    fractus_size_u32 size)
{
    fractus_palette palette;

    if (framebuffer == NULL || palette_source == NULL ||
        !framebuffer->initialized || !palette_source->initialized ||
        size.width == 0u || size.height == 0u) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (framebuffer->size.width == size.width && framebuffer->size.height == size.height) {
        return FRACTUS_STATUS_OK;
    }

    palette = palette_source->palette;
    fractus_framebuffer_shutdown(framebuffer);
    if (fractus_framebuffer_init(framebuffer, size) != FRACTUS_STATUS_OK ||
        fractus_app_apply_palette(framebuffer, &palette) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_app_ensure_drawing_framebuffer_size(
    const fractus_legacy_config *config,
    fractus_framebuffer *framebuffer,
    const fractus_framebuffer *palette_source)
{
    fractus_size_u32 size;
    int fullscreen;

    if (config == NULL || framebuffer == NULL || palette_source == NULL ||
        !framebuffer->initialized || !palette_source->initialized) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_app_resolve_drawing_video_mode(config, &size, &fullscreen) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return fractus_app_resize_drawing_framebuffer(framebuffer, palette_source, size);
}

fractus_status fractus_app_configure_platform_for_present(
    fractus_platform_context *platform,
    const fractus_legacy_config *config,
    int drawing)
{
    fractus_size_u32 size;
    int fullscreen;

    if (platform == NULL || config == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (drawing) {
        if (fractus_app_resolve_drawing_video_mode(config, &size, &fullscreen) != FRACTUS_STATUS_OK) {
            return FRACTUS_STATUS_ERROR;
        }
    } else {
        size.width = platform->config.logical_size.width * 2u;
        size.height = platform->config.logical_size.height * 2u;
        fullscreen = 0;
    }

    return fractus_platform_set_video_mode(platform, size, fullscreen);
}

void fractus_app_build_options_from_entries(
    fractus_ui_menu_option *options,
    const fractus_app_menu_entry *entries,
    size_t entry_count)
{
    size_t i;

    if (options == NULL || entries == NULL) {
        return;
    }

    for (i = 0u; i < entry_count; ++i) {
        options[i].bounds = entries[i].bounds;
    }
}

size_t fractus_app_copy_control_entries(
    fractus_app_menu_entry *entries,
    size_t capacity,
    const fractus_app_menu_entry *controls,
    size_t control_count)
{
    if (entries == NULL || controls == NULL || capacity < control_count) {
        return 0u;
    }

    memcpy(entries, controls, control_count * sizeof(*entries));
    return control_count;
}

fractus_status fractus_app_copy_framebuffer_for_overlay(
    fractus_framebuffer *target,
    const fractus_framebuffer *source)
{
    size_t pixel_count;

    if (target == NULL || source == NULL || !source->initialized) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (!target->initialized ||
        target->size.width != source->size.width ||
        target->size.height != source->size.height) {
        if (target->initialized) {
            fractus_framebuffer_shutdown(target);
        }
        if (fractus_framebuffer_init(target, source->size) != FRACTUS_STATUS_OK) {
            return FRACTUS_STATUS_ERROR;
        }
    }

    pixel_count = (size_t)source->size.width * (size_t)source->size.height;
    memcpy(target->index_pixels, source->index_pixels, pixel_count);
    target->palette = source->palette;
    target->pixels_dirty = 1;
    target->palette_dirty = 1;
    return FRACTUS_STATUS_OK;
}

fractus_status fractus_app_draw_drawing_footer(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const char *text)
{
    uint32_t scale;
    int32_t text_width;
    int32_t text_height;
    int32_t footer_height;
    int32_t footer_y;
    int32_t text_y;
    int32_t text_x;

    if (framebuffer == NULL || fonts == NULL || text == NULL || framebuffer->size.height < 24u) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    scale = 1u;
    if (framebuffer->size.height >= 2000u) {
        scale = 4u;
    } else if (framebuffer->size.height >= 1200u) {
        scale = 3u;
    } else if (framebuffer->size.height >= 720u) {
        scale = 2u;
    }

    if (fractus_font_measure_text(fonts, FRACTUS_FONT_ARIAL, text, &text_width, &text_height) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    text_width *= (int32_t)scale;
    text_height *= (int32_t)scale;
    footer_height = text_height + (int32_t)(12u * scale);
    if (footer_height < 24) {
        footer_height = 24;
    }
    if ((uint32_t)footer_height > framebuffer->size.height) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    footer_y = (int32_t)framebuffer->size.height - footer_height;
    text_y = footer_y + (footer_height - text_height) / 2;
    text_x = ((int32_t)framebuffer->size.width - text_width) / 2;
    if (text_x < 0) {
        text_x = 0;
    }

    if (fractus_graphics_fill_rect(
            framebuffer,
            (fractus_rect_i32){0, footer_y, (int32_t)framebuffer->size.width, footer_height},
            7u) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return fractus_font_draw_text_scaled(
        framebuffer,
        fonts,
        FRACTUS_FONT_ARIAL,
        text_x,
        text_y,
        15u,
        text,
        scale);
}

fractus_status fractus_app_draw_drawing_footer_ex(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const char *default_text,
    const char *saved_filename,
    int allow_selection)
{
    char footer_buf[160];
    const char *text = default_text;

    if (saved_filename != NULL && saved_filename[0] != '\0') {
        if (allow_selection) {
            (void)snprintf(footer_buf, sizeof(footer_buf), "Grabado %s - ESC / botón derecho: menú - S: seleccionar zona - M: metadatos - F: flujo", saved_filename);
        } else {
            (void)snprintf(footer_buf, sizeof(footer_buf), "Grabado %s - ESC o botón derecho: menú - M: metadatos - F: flujo", saved_filename);
        }
        text = footer_buf;
    }

    return fractus_app_draw_drawing_footer(framebuffer, fonts, text);
}

fractus_status fractus_app_draw_save_feedback(
    fractus_framebuffer *framebuffer,
    uint32_t frame)
{
    int32_t h;
    int32_t w;
    const int32_t total_frames = 8;
    int32_t step;
    int32_t y0, y1;

    if (framebuffer == NULL || !framebuffer->initialized) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    h = (int32_t)framebuffer->size.height;
    w = (int32_t)framebuffer->size.width;

    if (frame > (uint32_t)total_frames) {
        frame = (uint32_t)total_frames;
    }

    step = total_frames - (int32_t)frame;
    y0 = (step * h) / total_frames;
    y1 = ((step + 1) * h) / total_frames;

    if (fractus_graphics_fill_rect(
            framebuffer,
            (fractus_rect_i32){0, y0, w, y1 - y0},
            15u) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return FRACTUS_STATUS_OK;
}

int fractus_app_map_drawing_window_point(
    const fractus_platform_context *platform,
    const fractus_framebuffer *framebuffer,
    fractus_point_i32 window_point,
    fractus_point_i32 *framebuffer_point)
{
    fractus_size_u32 output_size;
    double scale_x;
    double scale_y;
    double scale;
    int32_t destination_x;
    int32_t destination_y;
    int32_t destination_width;
    int32_t destination_height;

    if (platform == NULL || framebuffer == NULL || framebuffer_point == NULL ||
        !framebuffer->initialized || framebuffer->size.width == 0u || framebuffer->size.height == 0u) {
        return 0;
    }

    if (fractus_platform_get_output_size(platform, &output_size) != FRACTUS_STATUS_OK ||
        output_size.width == 0u || output_size.height == 0u) {
        return 0;
    }

    scale_x = (double)output_size.width / (double)framebuffer->size.width;
    scale_y = (double)output_size.height / (double)framebuffer->size.height;
    scale = (scale_x < scale_y) ? scale_x : scale_y;
    if (scale <= 0.0) {
        return 0;
    }

    destination_width = (int32_t)((double)framebuffer->size.width * scale);
    destination_height = (int32_t)((double)framebuffer->size.height * scale);
    destination_x = ((int32_t)output_size.width - destination_width) / 2;
    destination_y = ((int32_t)output_size.height - destination_height) / 2;

    if (window_point.x < destination_x ||
        window_point.y < destination_y ||
        window_point.x >= destination_x + destination_width ||
        window_point.y >= destination_y + destination_height) {
        return 0;
    }

    framebuffer_point->x = (int32_t)((double)(window_point.x - destination_x) / scale);
    framebuffer_point->y = (int32_t)((double)(window_point.y - destination_y) / scale);
    framebuffer_point->x = fractus_app_clamp_i32(framebuffer_point->x, 0, (int32_t)framebuffer->size.width - 1);
    framebuffer_point->y = fractus_app_clamp_i32(framebuffer_point->y, 0, (int32_t)framebuffer->size.height - 1);
    return 1;
}

fractus_point_i32 fractus_app_constrain_selection_aspect_ratio(
    const fractus_framebuffer *framebuffer,
    fractus_point_i32 first,
    fractus_point_i32 current)
{
    fractus_point_i32 result;
    int32_t width;
    int32_t height;
    double target_ratio;
    double dx, dy;
    double abs_dx, abs_dy;
    double sx, sy;

    if (framebuffer == NULL || !framebuffer->initialized ||
        framebuffer->size.width < 2u || framebuffer->size.height < 2u) {
        return current;
    }

    width = (int32_t)framebuffer->size.width;
    height = (int32_t)framebuffer->size.height;
    target_ratio = (double)width / (double)height;

    dx = (double)(current.x - first.x);
    dy = (double)(current.y - first.y);
    abs_dx = fabs(dx);
    abs_dy = fabs(dy);
    sx = (dx >= 0.0) ? 1.0 : -1.0;
    sy = (dy >= 0.0) ? 1.0 : -1.0;

    if (abs_dx < 1.0 && abs_dy < 1.0) {
        return first;
    }

    /* Ajustar dimensiones para mantener la relacion de aspecto fija del dibujo */
    if (abs_dx / target_ratio >= abs_dy) {
        abs_dy = floor((abs_dx / target_ratio) + 0.5);
    } else {
        abs_dx = floor((abs_dy * target_ratio) + 0.5);
    }

    result.x = first.x + (int32_t)(sx * abs_dx);
    result.y = first.y + (int32_t)(sy * abs_dy);

    /* Limitar a los margenes del framebuffer reajustando la proporcion si se hace recorte */
    if (result.x < 0) {
        result.x = 0;
        abs_dx = (double)abs(result.x - first.x);
        abs_dy = floor((abs_dx / target_ratio) + 0.5);
        result.y = first.y + (int32_t)(sy * abs_dy);
    } else if (result.x >= width) {
        result.x = width - 1;
        abs_dx = (double)abs(result.x - first.x);
        abs_dy = floor((abs_dx / target_ratio) + 0.5);
        result.y = first.y + (int32_t)(sy * abs_dy);
    }

    if (result.y < 0) {
        result.y = 0;
        abs_dy = (double)abs(result.y - first.y);
        abs_dx = floor((abs_dy * target_ratio) + 0.5);
        result.x = first.x + (int32_t)(sx * abs_dx);
    } else if (result.y >= height) {
        result.y = height - 1;
        abs_dy = (double)abs(result.y - first.y);
        abs_dx = floor((abs_dy * target_ratio) + 0.5);
        result.x = first.x + (int32_t)(sx * abs_dx);
    }

    return result;
}

fractus_status fractus_app_draw_zone_selection_overlay(
    const fractus_platform_context *platform,
    fractus_framebuffer *framebuffer,
    const fractus_ui_context *ui,
    const fractus_app_zone_selection *selection)
{
    fractus_point_i32 cursor;

    if (platform == NULL || framebuffer == NULL || ui == NULL || selection == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (!selection->active ||
        !fractus_app_map_drawing_window_point(platform, framebuffer, ui->pointer_position, &cursor)) {
        return FRACTUS_STATUS_OK;
    }

    if (fractus_graphics_line(framebuffer, 0, cursor.y, (int32_t)framebuffer->size.width - 1, cursor.y, 15u) != FRACTUS_STATUS_OK ||
        fractus_graphics_line(framebuffer, cursor.x, 0, cursor.x, (int32_t)framebuffer->size.height - 1, 15u) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    if (selection->has_first_corner) {
        fractus_point_i32 constrained = fractus_app_constrain_selection_aspect_ratio(framebuffer, selection->first_corner, cursor);
        int32_t x = (selection->first_corner.x < constrained.x) ? selection->first_corner.x : constrained.x;
        int32_t y = (selection->first_corner.y < constrained.y) ? selection->first_corner.y : constrained.y;
        int32_t width = (selection->first_corner.x > constrained.x) ?
            selection->first_corner.x - constrained.x + 1 :
            constrained.x - selection->first_corner.x + 1;
        int32_t height = (selection->first_corner.y > constrained.y) ?
            selection->first_corner.y - constrained.y + 1 :
            constrained.y - selection->first_corner.y + 1;

        if (fractus_graphics_rect(framebuffer, (fractus_rect_i32){x, y, width, height}, 15u) != FRACTUS_STATUS_OK) {
            return FRACTUS_STATUS_ERROR;
        }
    }

    return FRACTUS_STATUS_OK;
}

int fractus_app_apply_complex_selection(
    const fractus_framebuffer *framebuffer,
    double *xmin,
    double *xmax,
    double *ymin,
    double *ymax,
    fractus_point_i32 first,
    fractus_point_i32 second)
{
    fractus_point_i32 constrained_second;
    int32_t x_min, x_max, y_min, y_max;
    double old_xmin, old_xmax, old_ymin, old_ymax;
    double dx, dy;

    if (framebuffer == NULL || xmin == NULL || xmax == NULL || ymin == NULL || ymax == NULL ||
        !framebuffer->initialized || framebuffer->size.width < 2u || framebuffer->size.height < 2u) {
        return 0;
    }

    constrained_second = fractus_app_constrain_selection_aspect_ratio(framebuffer, first, second);

    x_min = (first.x < constrained_second.x) ? first.x : constrained_second.x;
    x_max = (first.x > constrained_second.x) ? first.x : constrained_second.x;
    y_min = (first.y < constrained_second.y) ? first.y : constrained_second.y;
    y_max = (first.y > constrained_second.y) ? first.y : constrained_second.y;

    if (x_min == x_max || y_min == y_max) {
        return 0;
    }

    old_xmin = *xmin;
    old_xmax = *xmax;
    old_ymin = *ymin;
    old_ymax = *ymax;
    dx = (old_xmax - old_xmin) / (double)(framebuffer->size.width - 1u);
    dy = (old_ymax - old_ymin) / (double)(framebuffer->size.height - 1u);

    *xmin = old_xmin + (double)x_min * dx;
    *xmax = old_xmin + (double)x_max * dx;
    *ymax = old_ymax - (double)y_min * dy;
    *ymin = old_ymax - (double)y_max * dy;

    return 1;
}

int fractus_app_handle_zone_selection_input(
    const fractus_platform_context *platform,
    const fractus_framebuffer *framebuffer,
    const fractus_ui_context *ui,
    double *xmin,
    double *xmax,
    double *ymin,
    double *ymax,
    fractus_app_zone_selection *selection)
{
    fractus_point_i32 selected_point;

    if (platform == NULL || framebuffer == NULL || ui == NULL ||
        xmin == NULL || xmax == NULL || ymin == NULL || ymax == NULL || selection == NULL) {
        return 0;
    }

    if (ui->key_press_pending && (ui->key_pressed == 's' || ui->key_pressed == 'S')) {
        selection->active = 1;
        selection->has_first_corner = 0;
    }

    if (!selection->active) {
        return 0;
    }

    if ((ui->key_press_pending && ui->key_pressed == 27u) ||
        (ui->press_pending && ui->press_event.buttons.right)) {
        selection->active = 0;
        selection->has_first_corner = 0;
        return 0;
    }

    if (ui->press_pending && ui->press_event.buttons.left &&
        fractus_app_map_drawing_window_point(platform, framebuffer, ui->press_event.position, &selected_point)) {
        if (!selection->has_first_corner) {
            selection->first_corner = selected_point;
            selection->has_first_corner = 1;
        } else if (fractus_app_apply_complex_selection(framebuffer, xmin, xmax, ymin, ymax, selection->first_corner, selected_point)) {
            selection->active = 0;
            selection->has_first_corner = 0;
            return 1;
        }
    }

    return 0;
}

fractus_status fractus_app_draw_graphic_metadata_panel(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const fractus_graphic_metadata *metadata)
{
    char title[128];
    char lines[10][128];
    size_t line_count = 0u;
    int32_t max_text_width = 0;
    int32_t title_width = 0;
    int32_t text_height = 0;
    int32_t panel_width;
    int32_t panel_height;
    int32_t panel_x;
    int32_t panel_y;
    int32_t line_y;
    size_t i;

    if (framebuffer == NULL || fonts == NULL || metadata == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    title[0] = '\0';
    switch (metadata->kind) {
    case FRACTUS_GRAPHIC_KIND_MANDELBROT:
        (void)snprintf(title, sizeof(title), "Conjunto de Mandelbrot");
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "Resolucion: %ux%u", (unsigned)metadata->width, (unsigned)metadata->height);
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "X: [%.7g, %.7g]", metadata->params.mandelbrot.xmin, metadata->params.mandelbrot.xmax);
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "Y: [%.7g, %.7g]", metadata->params.mandelbrot.ymin, metadata->params.mandelbrot.ymax);
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "Iteraciones max.: %u", (unsigned)metadata->params.mandelbrot.max_iterations);
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "Radio escape: %.4g", metadata->params.mandelbrot.escape_radius_squared);
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "Modo color: %s",
            (metadata->params.mandelbrot.color_mode == FRACTUS_MANDELBROT_COLOR_SMOOTH) ? "Suave" : "Escape");
        break;

    case FRACTUS_GRAPHIC_KIND_MANDELBROT_DEM:
        (void)snprintf(title, sizeof(title), "Mandelbrot (DEM)");
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "Resolucion: %ux%u", (unsigned)metadata->width, (unsigned)metadata->height);
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "X: [%.7g, %.7g]", metadata->params.mandelbrot_dem.xmin, metadata->params.mandelbrot_dem.xmax);
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "Y: [%.7g, %.7g]", metadata->params.mandelbrot_dem.ymin, metadata->params.mandelbrot_dem.ymax);
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "Iteraciones max.: %u", (unsigned)metadata->params.mandelbrot_dem.max_iterations);
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "Radio escape: %.4g", metadata->params.mandelbrot_dem.escape_radius_squared);
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "Modo color: %s",
            (metadata->params.mandelbrot_dem.color_mode == FRACTUS_MANDELBROT_DEM_COLOR_GRADIENT) ? "Gradiente" : "Frontera");
        break;

    case FRACTUS_GRAPHIC_KIND_JULIA:
        (void)snprintf(title, sizeof(title), "Conjunto de Gaston Julia");
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "Resolucion: %ux%u", (unsigned)metadata->width, (unsigned)metadata->height);
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "X: [%.7g, %.7g]", metadata->params.julia.xmin, metadata->params.julia.xmax);
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "Y: [%.7g, %.7g]", metadata->params.julia.ymin, metadata->params.julia.ymax);
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "C: %.6g %c %.6g i",
            metadata->params.julia.constant_real,
            (metadata->params.julia.constant_imag >= 0.0) ? '+' : '-',
            fabs(metadata->params.julia.constant_imag));
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "Iteraciones max.: %u", (unsigned)metadata->params.julia.max_iterations);
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "Radio escape: %.4g", metadata->params.julia.escape_radius_squared);
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "Modo color: %s",
            (metadata->params.julia.color_mode == FRACTUS_JULIA_COLOR_SMOOTH) ? "Suave" : "Escape");
        break;

    case FRACTUS_GRAPHIC_KIND_JULIA_DEM:
        (void)snprintf(title, sizeof(title), "Julia (DEM)");
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "Resolucion: %ux%u", (unsigned)metadata->width, (unsigned)metadata->height);
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "X: [%.7g, %.7g]", metadata->params.julia_dem.xmin, metadata->params.julia_dem.xmax);
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "Y: [%.7g, %.7g]", metadata->params.julia_dem.ymin, metadata->params.julia_dem.ymax);
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "C: %.6g %c %.6g i",
            metadata->params.julia_dem.constant_real,
            (metadata->params.julia_dem.constant_imag >= 0.0) ? '+' : '-',
            fabs(metadata->params.julia_dem.constant_imag));
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "Iteraciones max.: %u", (unsigned)metadata->params.julia_dem.max_iterations);
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "Radio escape: %.4g", metadata->params.julia_dem.escape_radius_squared);
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "Modo color: %s",
            (metadata->params.julia_dem.color_mode == FRACTUS_JULIA_DEM_COLOR_GRADIENT) ? "Gradiente" : "Frontera");
        break;

    case FRACTUS_GRAPHIC_KIND_BIOMORPH:
        (void)snprintf(title, sizeof(title), "Biomorfo de Pickover");
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "Resolucion: %ux%u", (unsigned)metadata->width, (unsigned)metadata->height);
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "X: [%.7g, %.7g]", metadata->params.biomorph.xmin, metadata->params.biomorph.xmax);
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "Y: [%.7g, %.7g]", metadata->params.biomorph.ymin, metadata->params.biomorph.ymax);
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "C: %.6g %c %.6g i",
            metadata->params.biomorph.constant_real,
            (metadata->params.biomorph.constant_imag >= 0.0) ? '+' : '-',
            fabs(metadata->params.biomorph.constant_imag));
        {
            const char *eq_name = "z^2+c";
            const char *trap_name = "Re OR Im";
            switch (metadata->params.biomorph.equation) {
            case FRACTUS_BIOMORPH_EQ_Z2: eq_name = "z^2+c"; break;
            case FRACTUS_BIOMORPH_EQ_Z3: eq_name = "z^3+c"; break;
            case FRACTUS_BIOMORPH_EQ_Z4: eq_name = "z^4+c"; break;
            case FRACTUS_BIOMORPH_EQ_Z5: eq_name = "z^5+c"; break;
            case FRACTUS_BIOMORPH_EQ_SIN_Z: eq_name = "sin(z)+c"; break;
            case FRACTUS_BIOMORPH_EQ_EXP_Z: eq_name = "exp(z)+c"; break;
            }
            switch (metadata->params.biomorph.trap_mode) {
            case FRACTUS_BIOMORPH_TRAP_RE_OR_IM: trap_name = "Re OR Im"; break;
            case FRACTUS_BIOMORPH_TRAP_RE_AND_IM: trap_name = "Re AND Im"; break;
            case FRACTUS_BIOMORPH_TRAP_SOLO_RE: trap_name = "Solo Re"; break;
            case FRACTUS_BIOMORPH_TRAP_SOLO_IM: trap_name = "Solo Im"; break;
            }
            (void)snprintf(lines[line_count++], sizeof(lines[0]), "Ecuacion: %s | Modo: %s", eq_name, trap_name);
        }
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "Iteraciones max.: %u | Cutoff: %.4g",
            (unsigned)metadata->params.biomorph.max_iterations, metadata->params.biomorph.cutoff);
        break;

    case FRACTUS_GRAPHIC_KIND_PLASMA_RECTANGULAR:
        (void)snprintf(title, sizeof(title), "Plasma Rectangular");
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "Resolucion: %ux%u", (unsigned)metadata->width, (unsigned)metadata->height);
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "Semilla: %u", (unsigned)metadata->params.plasma_rectangular.seed);
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "Dispersion: %d", (int)metadata->params.plasma_rectangular.dispersion);
        break;

    case FRACTUS_GRAPHIC_KIND_PLASMA_CIRCULAR:
        (void)snprintf(title, sizeof(title), "Plasma Circular");
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "Resolucion: %ux%u", (unsigned)metadata->width, (unsigned)metadata->height);
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "Semilla: %u", (unsigned)metadata->params.plasma_circular.seed);
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "Circulos: %d | Radio max: %d",
            (int)metadata->params.plasma_circular.circle_count, (int)metadata->params.plasma_circular.max_radius);
        break;

    case FRACTUS_GRAPHIC_KIND_LORENZ:
        (void)snprintf(title, sizeof(title), "Atractor de Lorenz (3D)");
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "Resolucion: %ux%u", (unsigned)metadata->width, (unsigned)metadata->height);
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "Sigma: %.4g | Rho: %.4g | Beta: %.4g",
            metadata->params.lorenz.sigma, metadata->params.lorenz.rho, metadata->params.lorenz.beta);
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "dt: %.4g | Iteraciones: %u",
            metadata->params.lorenz.dt, (unsigned)metadata->params.lorenz.iterations);
        {
            const char *proj_name = "Plano X-Z";
            switch (metadata->params.lorenz.projection) {
            case FRACTUS_LORENZ_PROJECTION_XZ: proj_name = "Plano X-Z"; break;
            case FRACTUS_LORENZ_PROJECTION_XY: proj_name = "Plano X-Y"; break;
            case FRACTUS_LORENZ_PROJECTION_YZ: proj_name = "Plano Y-Z"; break;
            case FRACTUS_LORENZ_PROJECTION_CUSTOM: proj_name = "Rotacion libre 3D"; break;
            }
            (void)snprintf(lines[line_count++], sizeof(lines[0]), "Proyeccion: %s", proj_name);
        }
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "Rotacion: X=%.1f\370 Y=%.1f\370 Z=%.1f\370",
            metadata->params.lorenz.rot_x, metadata->params.lorenz.rot_y, metadata->params.lorenz.rot_z);
        break;

    case FRACTUS_GRAPHIC_KIND_UNKNOWN:
    default:
        (void)snprintf(title, sizeof(title), "Dibujo Fractus-x64");
        (void)snprintf(lines[line_count++], sizeof(lines[0]), "Resolucion: %ux%u", (unsigned)metadata->width, (unsigned)metadata->height);
        break;
    }

    fractus_font_kind font = (framebuffer->size.width <= 1024u && framebuffer->size.height <= 768u)
        ? FRACTUS_FONT_SMALL
        : FRACTUS_FONT_ARIAL;
    int32_t pad_x = (font == FRACTUS_FONT_ARIAL) ? 12 : 8;
    int32_t pad_y = (font == FRACTUS_FONT_ARIAL) ? 8 : 6;
    int32_t title_margin = (font == FRACTUS_FONT_ARIAL) ? 10 : 7;
    int32_t line_gap = (font == FRACTUS_FONT_ARIAL) ? 4 : 3;
    int32_t line_step;

    (void)fractus_font_measure_text(fonts, font, title, &title_width, &text_height);
    if (text_height <= 0) {
        text_height = (font == FRACTUS_FONT_ARIAL) ? 14 : 9;
    }
    max_text_width = title_width;
    for (i = 0u; i < line_count; ++i) {
        int32_t w = 0;
        int32_t h = 0;
        (void)fractus_font_measure_text(fonts, font, lines[i], &w, &h);
        if (w > max_text_width) {
            max_text_width = w;
        }
    }

    line_step = text_height + line_gap;
    panel_width = max_text_width + pad_x * 2;
    if (panel_width < 220) {
        panel_width = 220;
    }
    panel_height = pad_y + text_height + title_margin + (int32_t)line_count * line_step + pad_y;

    panel_x = (int32_t)framebuffer->size.width - panel_width - 8;
    panel_y = (int32_t)framebuffer->size.height - panel_height - 28;
    if (panel_x < 0) {
        panel_x = 0;
    }
    if (panel_y < 0) {
        panel_y = 0;
    }

    /* Fondo gris oscuro (7u) con relieve biselado y texto blanco (15u) */
    (void)fractus_graphics_fill_rect(
        framebuffer,
        (fractus_rect_i32){panel_x, panel_y, panel_width, panel_height},
        7u);
    (void)fractus_graphics_rect(
        framebuffer,
        (fractus_rect_i32){panel_x, panel_y, panel_width, panel_height},
        15u);
    (void)fractus_graphics_line(framebuffer, panel_x + 1, panel_y + panel_height - 1, panel_x + panel_width - 1, panel_y + panel_height - 1, 0u);
    (void)fractus_graphics_line(framebuffer, panel_x + panel_width - 1, panel_y + 1, panel_x + panel_width - 1, panel_y + panel_height - 1, 0u);

    /* Titulo del panel */
    (void)fractus_ui_draw_text_left(framebuffer, fonts, font, panel_x + pad_x, panel_y + pad_y, 15u, title);
    {
        int32_t sep_y = panel_y + pad_y + text_height + title_margin / 2;
        (void)fractus_graphics_line(framebuffer, panel_x + 6, sep_y, panel_x + panel_width - 7, sep_y, 8u);
        (void)fractus_graphics_line(framebuffer, panel_x + 6, sep_y + 1, panel_x + panel_width - 7, sep_y + 1, 0u);
    }

    /* Lineas de parametros y coordenadas */
    line_y = panel_y + pad_y + text_height + title_margin;
    for (i = 0u; i < line_count; ++i) {
        (void)fractus_ui_draw_text_left(framebuffer, fonts, font, panel_x + pad_x, line_y, 15u, lines[i]);
        line_y += line_step;
    }

    return FRACTUS_STATUS_OK;
}
