#include "app/commons.h"

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

void fractus_app_init_palette_color_fields(
    fractus_ui_numeric_field *red_field,
    fractus_ui_numeric_field *green_field,
    fractus_ui_numeric_field *blue_field,
    fractus_color_rgba8 color)
{
    int32_t red;
    int32_t green;
    int32_t blue;

    fractus_app_vga_channels_from_color(color, &red, &green, &blue);
    if (red_field != NULL) {
        (void)fractus_ui_numeric_field_init_int(red_field, (fractus_rect_i32){200, 152, 43, 21}, red, 0, 63);
    }
    if (green_field != NULL) {
        (void)fractus_ui_numeric_field_init_int(green_field, (fractus_rect_i32){200, 182, 43, 21}, green, 0, 63);
    }
    if (blue_field != NULL) {
        (void)fractus_ui_numeric_field_init_int(blue_field, (fractus_rect_i32){200, 212, 43, 21}, blue, 0, 63);
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
           view == FRACTUS_APP_VIEW_PLASMA_CIRCULAR;
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
        return fractus_app_sync_framebuffer_palette(framebuffer, palette_source);
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
