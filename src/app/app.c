#include "app/app.h"
#include "core/core.h"
#include "core/fractal.h"
#include "platform/graphics.h"
#include "platform/platform.h"
#include "ui/font.h"
#include "ui/ui.h"

#if defined(_WIN32)
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

#include <stdio.h>
#include <string.h>

#define FRACTUS_APP_DIALOG_BUTTON_CAPACITY 20u
#define FRACTUS_APP_GRAPHIC_FILE_CAPACITY 256u
#define FRACTUS_APP_GRAPHIC_FILE_PAGE_SIZE 10u
#define FRACTUS_APP_PALETTE_FILE_CAPACITY 256u
#define FRACTUS_APP_PALETTE_FILE_PAGE_SIZE 10u
#define FRACTUS_APP_MENU_LOAD_GRAPHIC_INDEX 10
#define FRACTUS_APP_MENU_SAVE_NEXT_GRAPHIC_INDEX 11
#define FRACTUS_APP_MENU_LOAD_PALETTE_INDEX 12
#define FRACTUS_APP_MENU_SAVE_PALETTE_INDEX 13
#define FRACTUS_APP_MENU_LOAD_GRAPHIC_PALETTE_INDEX 14
#define FRACTUS_APP_MENU_CHANGE_GRAPHIC_PALETTE_INDEX 15
#define FRACTUS_APP_MENU_PALETTE_INDEX 16
#define FRACTUS_APP_MENU_EDIT_PALETTE_COLOR_INDEX 17
#define FRACTUS_APP_MENU_COPY_PALETTE_COLOR_INDEX 18
#define FRACTUS_APP_MENU_GRADIENT_PALETTE_INDEX 19
#define FRACTUS_APP_MENU_RESTORE_PALETTE_INDEX 20
#define FRACTUS_APP_MENU_EXIT_INDEX 29
#define FRACTUS_APP_MENU_VIDEO_INDEX 23
#define FRACTUS_APP_MENU_ITERATIONS_INDEX 24
#define FRACTUS_APP_MENU_ESCAPE_RADIUS_INDEX 25
#define FRACTUS_APP_MENU_BIOMORPH_RADIUS_INDEX 26
#define FRACTUS_APP_PALETTE_GRID_X 48
#define FRACTUS_APP_PALETTE_GRID_Y 68
#define FRACTUS_APP_PALETTE_CELL_WIDTH 28
#define FRACTUS_APP_PALETTE_CELL_HEIGHT 16
#define FRACTUS_APP_PALETTE_CELL_STEP_X 34
#define FRACTUS_APP_PALETTE_CELL_STEP_Y 22
#define FRACTUS_APP_PALETTE_GRID_COLUMNS 16
#define FRACTUS_APP_PALETTE_GRID_ROWS 15
#define FRACTUS_APP_NUMBERED_FILE_LIMIT 9999u
#define FRACTUS_APP_DRAWING_MIN_WIDTH 640u
#define FRACTUS_APP_DRAWING_MIN_HEIGHT 480u

#define FRACTUS_APP_RECT(x1, y1, x2, y2) \
    { (x1), (y1), (x2) - (x1) + 1, (y2) - (y1) + 1 }
#define FRACTUS_APP_ARRAY_COUNT(values) (sizeof(values) / sizeof((values)[0]))

typedef enum fractus_app_view {
    FRACTUS_APP_VIEW_MAIN_MENU = 0,
    FRACTUS_APP_VIEW_MANDELBROT_CONFIG,
    FRACTUS_APP_VIEW_MANDELBROT,
    FRACTUS_APP_VIEW_JULIA_CONFIG,
    FRACTUS_APP_VIEW_JULIA,
    FRACTUS_APP_VIEW_BIOMORPH_CONFIG,
    FRACTUS_APP_VIEW_BIOMORPH,
    FRACTUS_APP_VIEW_PLASMA_MENU,
    FRACTUS_APP_VIEW_PLASMA_RECTANGULAR_CONFIG,
    FRACTUS_APP_VIEW_PLASMA_RECTANGULAR,
    FRACTUS_APP_VIEW_PLASMA_CIRCULAR_CONFIG,
    FRACTUS_APP_VIEW_PLASMA_CIRCULAR,
    FRACTUS_APP_VIEW_VIDEO_CONFIG,
    FRACTUS_APP_VIEW_ITERATIONS_CONFIG,
    FRACTUS_APP_VIEW_ESCAPE_RADIUS_CONFIG,
    FRACTUS_APP_VIEW_BIOMORPH_RADIUS_CONFIG,
    FRACTUS_APP_VIEW_LOAD_GRAPHIC,
    FRACTUS_APP_VIEW_LOAD_PALETTE,
    FRACTUS_APP_VIEW_LOAD_GRAPHIC_PALETTE,
    FRACTUS_APP_VIEW_CHANGE_GRAPHIC_PALETTE_GRAPHIC,
    FRACTUS_APP_VIEW_CHANGE_GRAPHIC_PALETTE_PALETTE,
    FRACTUS_APP_VIEW_GRAPHIC,
    FRACTUS_APP_VIEW_PALETTE,
    FRACTUS_APP_VIEW_PALETTE_EDIT_SELECT,
    FRACTUS_APP_VIEW_PALETTE_EDIT_COLOR,
    FRACTUS_APP_VIEW_PALETTE_COPY_SOURCE,
    FRACTUS_APP_VIEW_PALETTE_COPY_TARGETS,
    FRACTUS_APP_VIEW_PALETTE_GRADIENT_FIRST,
    FRACTUS_APP_VIEW_PALETTE_GRADIENT_SECOND,
    FRACTUS_APP_VIEW_ERROR
} fractus_app_view;

typedef enum fractus_app_mandel_dialog_action {
    FRACTUS_APP_MANDEL_XMIN_DEC = 0,
    FRACTUS_APP_MANDEL_XMIN_INC,
    FRACTUS_APP_MANDEL_XMAX_DEC,
    FRACTUS_APP_MANDEL_XMAX_INC,
    FRACTUS_APP_MANDEL_YMIN_DEC,
    FRACTUS_APP_MANDEL_YMIN_INC,
    FRACTUS_APP_MANDEL_YMAX_DEC,
    FRACTUS_APP_MANDEL_YMAX_INC,
    FRACTUS_APP_MANDEL_ITER_DEC,
    FRACTUS_APP_MANDEL_ITER_INC,
    FRACTUS_APP_MANDEL_RADIUS_DEC,
    FRACTUS_APP_MANDEL_RADIUS_INC,
    FRACTUS_APP_MANDEL_COLOR_CLASSIC,
    FRACTUS_APP_MANDEL_COLOR_SMOOTH,
    FRACTUS_APP_MANDEL_DRAW,
    FRACTUS_APP_MANDEL_CANCEL
} fractus_app_mandel_dialog_action;

typedef enum fractus_app_julia_dialog_action {
    FRACTUS_APP_JULIA_XMIN_DEC = 0,
    FRACTUS_APP_JULIA_XMIN_INC,
    FRACTUS_APP_JULIA_XMAX_DEC,
    FRACTUS_APP_JULIA_XMAX_INC,
    FRACTUS_APP_JULIA_YMIN_DEC,
    FRACTUS_APP_JULIA_YMIN_INC,
    FRACTUS_APP_JULIA_YMAX_DEC,
    FRACTUS_APP_JULIA_YMAX_INC,
    FRACTUS_APP_JULIA_CREAL_DEC,
    FRACTUS_APP_JULIA_CREAL_INC,
    FRACTUS_APP_JULIA_CIMAG_DEC,
    FRACTUS_APP_JULIA_CIMAG_INC,
    FRACTUS_APP_JULIA_ITER_DEC,
    FRACTUS_APP_JULIA_ITER_INC,
    FRACTUS_APP_JULIA_RADIUS_DEC,
    FRACTUS_APP_JULIA_RADIUS_INC,
    FRACTUS_APP_JULIA_DRAW,
    FRACTUS_APP_JULIA_CANCEL
} fractus_app_julia_dialog_action;

typedef enum fractus_app_biomorph_dialog_action {
    FRACTUS_APP_BIOMORPH_XMIN_DEC = 0,
    FRACTUS_APP_BIOMORPH_XMIN_INC,
    FRACTUS_APP_BIOMORPH_XMAX_DEC,
    FRACTUS_APP_BIOMORPH_XMAX_INC,
    FRACTUS_APP_BIOMORPH_YMIN_DEC,
    FRACTUS_APP_BIOMORPH_YMIN_INC,
    FRACTUS_APP_BIOMORPH_YMAX_DEC,
    FRACTUS_APP_BIOMORPH_YMAX_INC,
    FRACTUS_APP_BIOMORPH_CREAL_DEC,
    FRACTUS_APP_BIOMORPH_CREAL_INC,
    FRACTUS_APP_BIOMORPH_CIMAG_DEC,
    FRACTUS_APP_BIOMORPH_CIMAG_INC,
    FRACTUS_APP_BIOMORPH_RADIUS_DEC,
    FRACTUS_APP_BIOMORPH_RADIUS_INC,
    FRACTUS_APP_BIOMORPH_DRAW,
    FRACTUS_APP_BIOMORPH_CANCEL
} fractus_app_biomorph_dialog_action;

typedef enum fractus_app_plasma_rectangular_dialog_action {
    FRACTUS_APP_PLASMA_RECTANGULAR_DISPERSION_DEC = 0,
    FRACTUS_APP_PLASMA_RECTANGULAR_DISPERSION_INC,
    FRACTUS_APP_PLASMA_RECTANGULAR_DRAW,
    FRACTUS_APP_PLASMA_RECTANGULAR_CANCEL
} fractus_app_plasma_rectangular_dialog_action;

typedef enum fractus_app_plasma_circular_dialog_action {
    FRACTUS_APP_PLASMA_CIRCLES_DEC = 0,
    FRACTUS_APP_PLASMA_CIRCLES_INC,
    FRACTUS_APP_PLASMA_RADIUS_DEC,
    FRACTUS_APP_PLASMA_RADIUS_INC,
    FRACTUS_APP_PLASMA_CIRCULAR_DRAW,
    FRACTUS_APP_PLASMA_CIRCULAR_CANCEL
} fractus_app_plasma_circular_dialog_action;

typedef enum fractus_app_simple_config_action {
    FRACTUS_APP_SIMPLE_CONFIG_DEC = 0,
    FRACTUS_APP_SIMPLE_CONFIG_INC,
    FRACTUS_APP_SIMPLE_CONFIG_ACCEPT,
    FRACTUS_APP_SIMPLE_CONFIG_CANCEL
} fractus_app_simple_config_action;

typedef enum fractus_app_palette_color_action {
    FRACTUS_APP_PALETTE_RED_DEC = 0,
    FRACTUS_APP_PALETTE_RED_INC,
    FRACTUS_APP_PALETTE_GREEN_DEC,
    FRACTUS_APP_PALETTE_GREEN_INC,
    FRACTUS_APP_PALETTE_BLUE_DEC,
    FRACTUS_APP_PALETTE_BLUE_INC,
    FRACTUS_APP_PALETTE_COLOR_ACCEPT,
    FRACTUS_APP_PALETTE_COLOR_CANCEL
} fractus_app_palette_color_action;

typedef enum fractus_app_plasma_method {
    FRACTUS_APP_PLASMA_METHOD_NONE = -1,
    FRACTUS_APP_PLASMA_METHOD_RECTANGLES = 0,
    FRACTUS_APP_PLASMA_METHOD_CIRCLES = 1
} fractus_app_plasma_method;

typedef fractus_ui_button fractus_app_menu_entry;

static const fractus_app_menu_entry fractus_app_main_menu_controls[] = {
    {FRACTUS_APP_RECT(10, 70, 312, 90), 11u, 0u, "Conjunto de Benoit B. Mandelbrot"},
    {FRACTUS_APP_RECT(10, 90, 312, 110), 11u, 0u, "Conjuntos de Gaston Julia"},
    {FRACTUS_APP_RECT(10, 110, 312, 130), 11u, 0u, "Biomorfos de Clifford Pickover"},
    {FRACTUS_APP_RECT(10, 130, 312, 150), 11u, 0u, "Atractores dinamicos"},
    {FRACTUS_APP_RECT(10, 150, 312, 170), 11u, 0u, "Curvas fractales sencillas"},
    {FRACTUS_APP_RECT(10, 170, 312, 190), 11u, 0u, "Fractales por el metodo de plasma"},
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
    {FRACTUS_APP_RECT(327, 330, 629, 350), 3u, 15u, "Borrar un fichero del disco"},
    {FRACTUS_APP_RECT(327, 350, 629, 370), 3u, 15u, "Renombrar un fichero del disco"},
    {FRACTUS_APP_RECT(10, 290, 312, 310), 5u, 15u, "Resolucion de los dibujos"},
    {FRACTUS_APP_RECT(10, 310, 312, 330), 5u, 15u, "Numero maximo de iteraciones"},
    {FRACTUS_APP_RECT(10, 330, 312, 350), 5u, 15u, "Radio de escape para Mandelbrot y Julia"},
    {FRACTUS_APP_RECT(10, 350, 312, 370), 5u, 15u, "Radio de escape para biomorfos"},
    {FRACTUS_APP_RECT(10, 430, 130, 450), 8u, 0u, "Ayuda"},
    {FRACTUS_APP_RECT(509, 430, 629, 450), 8u, 0u, "Acerca de..."},
    {FRACTUS_APP_RECT(260, 430, 380, 450), 0u, 15u, "Salir del programa"}
};

typedef struct fractus_app_palette_file {
    char label[96];
    char path[512];
} fractus_app_palette_file;

typedef struct fractus_app_graphic_file {
    char label[96];
    char path[512];
} fractus_app_graphic_file;

typedef struct fractus_app_mandelbrot_selection {
    int active;
    int has_first_corner;
    fractus_point_i32 first_corner;
} fractus_app_mandelbrot_selection;

static void fractus_app_set_button(
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

static int32_t fractus_app_clamp_i32(int32_t value, int32_t minimum, int32_t maximum)
{
    if (value < minimum) {
        return minimum;
    }

    if (value > maximum) {
        return maximum;
    }

    return value;
}

static uint8_t fractus_app_scale_vga_to_rgb8(int32_t value)
{
    value = fractus_app_clamp_i32(value, 0, 63);
    return (uint8_t)((value * 255 + 31) / 63);
}

static int32_t fractus_app_scale_rgb8_to_vga(uint8_t value)
{
    return (int32_t)((uint32_t)value * 63u + 127u) / 255;
}

static fractus_color_rgba8 fractus_app_rgb8(uint8_t r, uint8_t g, uint8_t b)
{
    fractus_color_rgba8 color;

    color.r = r;
    color.g = g;
    color.b = b;
    color.a = 255u;
    return color;
}

static fractus_color_rgba8 fractus_app_vga_color(int32_t r, int32_t g, int32_t b)
{
    return fractus_app_rgb8(
        fractus_app_scale_vga_to_rgb8(r),
        fractus_app_scale_vga_to_rgb8(g),
        fractus_app_scale_vga_to_rgb8(b));
}

static void fractus_app_vga_channels_from_color(
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

static void fractus_app_init_palette_color_fields(
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

static double fractus_app_clamp_f64(double value, double minimum, double maximum)
{
    if (value < minimum) {
        return minimum;
    }

    if (value > maximum) {
        return maximum;
    }

    return value;
}

static void fractus_app_log(const char *message)
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

static void fractus_app_sort_palette_files(
    fractus_app_palette_file *files,
    size_t count)
{
    size_t i;
    size_t j;

    if (files == NULL) {
        return;
    }

    for (i = 0u; i < count; ++i) {
        for (j = i + 1u; j < count; ++j) {
            size_t k;
            int swap = 0;

            for (k = 0u; files[i].label[k] != '\0' || files[j].label[k] != '\0'; ++k) {
                char left = fractus_formats_ascii_lower(files[i].label[k]);
                char right = fractus_formats_ascii_lower(files[j].label[k]);

                if (left < right) {
                    break;
                }

                if (left > right) {
                    swap = 1;
                    break;
                }
            }

            if (swap) {
                fractus_app_palette_file temporary = files[i];
                files[i] = files[j];
                files[j] = temporary;
            }
        }
    }
}

static void fractus_app_sort_graphic_files(
    fractus_app_graphic_file *files,
    size_t count)
{
    size_t i;
    size_t j;

    if (files == NULL) {
        return;
    }

    for (i = 0u; i < count; ++i) {
        for (j = i + 1u; j < count; ++j) {
            size_t k;
            int swap = 0;

            for (k = 0u; files[i].label[k] != '\0' || files[j].label[k] != '\0'; ++k) {
                char left = fractus_formats_ascii_lower(files[i].label[k]);
                char right = fractus_formats_ascii_lower(files[j].label[k]);

                if (left < right) {
                    break;
                }

                if (left > right) {
                    swap = 1;
                    break;
                }
            }

            if (swap) {
                fractus_app_graphic_file temporary = files[i];
                files[i] = files[j];
                files[j] = temporary;
            }
        }
    }
}

static fractus_status fractus_app_set_message(
    char *destination,
    size_t destination_size,
    const char *message)
{
    if (destination == NULL || destination_size == 0u || message == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    return fractus_formats_copy_path(message, destination, destination_size);
}

static int fractus_app_view_is_generated_drawing(fractus_app_view view)
{
    return view == FRACTUS_APP_VIEW_MANDELBROT ||
           view == FRACTUS_APP_VIEW_JULIA ||
           view == FRACTUS_APP_VIEW_BIOMORPH ||
           view == FRACTUS_APP_VIEW_PLASMA_RECTANGULAR ||
           view == FRACTUS_APP_VIEW_PLASMA_CIRCULAR;
}

static fractus_status fractus_app_resolve_numbered_write_path(
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

static fractus_status fractus_app_apply_palette(
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

static fractus_status fractus_app_sync_framebuffer_palette(
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

static fractus_status fractus_app_resolve_drawing_video_mode(
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

static fractus_status fractus_app_resize_drawing_framebuffer(
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

static fractus_status fractus_app_ensure_drawing_framebuffer_size(
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

static fractus_status fractus_app_configure_platform_for_present(
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

static fractus_status fractus_app_apply_legacy_config(
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

static void fractus_app_apply_legacy_numeric_config(
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
}

static void fractus_app_capture_palette_to_config(
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

static fractus_status fractus_app_save_legacy_config(
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

static fractus_status fractus_app_persist_current_palette(
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

static fractus_status fractus_app_set_palette_entry(
    fractus_framebuffer *framebuffer,
    uint32_t index,
    fractus_color_rgba8 color)
{
    if (framebuffer == NULL || index < 16u || index >= FRACTUS_PALETTE_SIZE) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_palette_set_entry(&framebuffer->palette, index, color) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    framebuffer->palette_dirty = 1;
    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_app_apply_palette_edit(
    const char *cfg_path,
    fractus_framebuffer *framebuffer,
    fractus_legacy_config *config,
    uint32_t index,
    fractus_color_rgba8 color)
{
    if (fractus_app_set_palette_entry(framebuffer, index, color) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return fractus_app_persist_current_palette(cfg_path, framebuffer, config);
}

static fractus_status fractus_app_apply_palette_gradient(
    const char *cfg_path,
    fractus_framebuffer *framebuffer,
    fractus_legacy_config *config,
    uint32_t first_index,
    uint32_t second_index)
{
    fractus_color_rgba8 first_color;
    fractus_color_rgba8 second_color;
    int32_t first_red;
    int32_t first_green;
    int32_t first_blue;
    int32_t second_red;
    int32_t second_green;
    int32_t second_blue;
    uint32_t start;
    uint32_t end;
    uint32_t span;
    uint32_t i;

    if (framebuffer == NULL || config == NULL ||
        first_index < 16u || first_index >= FRACTUS_PALETTE_SIZE ||
        second_index < 16u || second_index >= FRACTUS_PALETTE_SIZE) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (first_index == second_index) {
        return FRACTUS_STATUS_OK;
    }

    if (fractus_palette_get_entry(&framebuffer->palette, first_index, &first_color) != FRACTUS_STATUS_OK ||
        fractus_palette_get_entry(&framebuffer->palette, second_index, &second_color) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    fractus_app_vga_channels_from_color(first_color, &first_red, &first_green, &first_blue);
    fractus_app_vga_channels_from_color(second_color, &second_red, &second_green, &second_blue);
    start = first_index < second_index ? first_index : second_index;
    end = first_index < second_index ? second_index : first_index;
    span = end - start;

    for (i = start; i <= end; ++i) {
        uint32_t offset = i - start;
        int32_t red = (int32_t)((((uint32_t)first_red * (span - offset)) + ((uint32_t)second_red * offset)) / span);
        int32_t green = (int32_t)((((uint32_t)first_green * (span - offset)) + ((uint32_t)second_green * offset)) / span);
        int32_t blue = (int32_t)((((uint32_t)first_blue * (span - offset)) + ((uint32_t)second_blue * offset)) / span);

        if (fractus_app_set_palette_entry(framebuffer, i, fractus_app_vga_color(red, green, blue)) != FRACTUS_STATUS_OK) {
            return FRACTUS_STATUS_ERROR;
        }
    }

    return fractus_app_persist_current_palette(cfg_path, framebuffer, config);
}

static fractus_status fractus_app_load_palette_into_state(
    const char *palette_path,
    fractus_framebuffer *framebuffer,
    fractus_legacy_config *config,
    const char *cfg_path)
{
    fractus_palette palette;

    if (palette_path == NULL || framebuffer == NULL || config == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_legacy_palette_load(palette_path, &palette) != FRACTUS_STATUS_OK ||
        fractus_app_apply_palette(framebuffer, &palette) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return fractus_app_persist_current_palette(cfg_path, framebuffer, config);
}

static fractus_status fractus_app_load_graphic_palette_into_state(
    const char *graphic_path,
    fractus_framebuffer *framebuffer,
    fractus_legacy_config *config,
    const char *cfg_path)
{
    fractus_indexed_image image;
    fractus_palette palette;
    uint16_t video_mode;
    fractus_status status;

    if (graphic_path == NULL || framebuffer == NULL || config == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    memset(&image, 0, sizeof(image));
    status = fractus_legacy_graphic_load(graphic_path, &image, &palette, &video_mode);
    if (status != FRACTUS_STATUS_OK) {
        return status;
    }

    fractus_indexed_image_shutdown(&image);
    if (fractus_app_apply_palette(framebuffer, &palette) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return fractus_app_persist_current_palette(cfg_path, framebuffer, config);
}

static fractus_status fractus_app_restore_default_palette(
    const fractus_platform_context *platform,
    fractus_framebuffer *framebuffer,
    fractus_legacy_config *config,
    const char *cfg_path)
{
    char palette_path[512];

    if (platform == NULL || framebuffer == NULL || config == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_formats_resolve_legacy_path(platform, "paletas/defecto.drsp", palette_path, sizeof(palette_path)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return fractus_app_load_palette_into_state(palette_path, framebuffer, config, cfg_path);
}

static fractus_status fractus_app_save_current_palette_file(
    const fractus_platform_context *platform,
    const fractus_framebuffer *framebuffer,
    fractus_legacy_config *config,
    const char *cfg_path)
{
    char palette_path[512];

    if (platform == NULL || framebuffer == NULL || config == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_app_resolve_numbered_write_path(platform, "paletas", ".drsp", palette_path, sizeof(palette_path)) != FRACTUS_STATUS_OK) {
        fractus_app_log("palette: could not resolve a free numbered .drsp write path");
        return FRACTUS_STATUS_UNSUPPORTED;
    }

    fractus_app_log(palette_path);
    if (fractus_legacy_palette_save(palette_path, &framebuffer->palette) != FRACTUS_STATUS_OK) {
        fractus_app_log("palette: saving numbered .drsp failed");
        return FRACTUS_STATUS_ERROR;
    }

    return fractus_app_persist_current_palette(cfg_path, framebuffer, config);
}

static fractus_status fractus_app_load_graphic_into_state(
    const char *graphic_path,
    fractus_framebuffer *framebuffer,
    fractus_legacy_config *config,
    const char *cfg_path)
{
    fractus_indexed_image image;
    fractus_palette palette;
    uint16_t video_mode;
    fractus_status status;

    if (graphic_path == NULL || framebuffer == NULL || config == NULL || !framebuffer->initialized) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    memset(&image, 0, sizeof(image));
    status = fractus_legacy_graphic_load(graphic_path, &image, &palette, &video_mode);
    if (status != FRACTUS_STATUS_OK) {
        return status;
    }

    if (framebuffer->size.width != image.size.width || framebuffer->size.height != image.size.height) {
        fractus_framebuffer_shutdown(framebuffer);
        if (fractus_framebuffer_init(framebuffer, image.size) != FRACTUS_STATUS_OK) {
            fractus_indexed_image_shutdown(&image);
            return FRACTUS_STATUS_ERROR;
        }
    }

    if (fractus_app_apply_palette(framebuffer, &palette) != FRACTUS_STATUS_OK ||
        fractus_framebuffer_clear(framebuffer, 0u) != FRACTUS_STATUS_OK ||
        fractus_graphics_blit(framebuffer, (fractus_point_i32){0, 0}, &image) != FRACTUS_STATUS_OK) {
        fractus_indexed_image_shutdown(&image);
        return FRACTUS_STATUS_ERROR;
    }

    fractus_indexed_image_shutdown(&image);
    return fractus_app_persist_current_palette(cfg_path, framebuffer, config);
}

static fractus_status fractus_app_change_graphic_palette_into_state(
    const fractus_platform_context *platform,
    const char *graphic_path,
    const char *palette_path,
    fractus_framebuffer *framebuffer,
    fractus_legacy_config *config,
    const char *cfg_path)
{
    fractus_indexed_image image;
    fractus_palette loaded_graphic_palette;
    fractus_palette new_palette;
    uint16_t video_mode;
    char output_path[512];
    fractus_status status;

    if (platform == NULL || graphic_path == NULL || palette_path == NULL ||
        framebuffer == NULL || config == NULL || !framebuffer->initialized) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    memset(&image, 0, sizeof(image));
    status = fractus_legacy_graphic_load(graphic_path, &image, &loaded_graphic_palette, &video_mode);
    if (status != FRACTUS_STATUS_OK) {
        return status;
    }

    status = fractus_legacy_palette_load(palette_path, &new_palette);
    if (status != FRACTUS_STATUS_OK) {
        fractus_indexed_image_shutdown(&image);
        return status;
    }

    status = fractus_app_resolve_numbered_write_path(platform, "dibujos", ".drsg", output_path, sizeof(output_path));
    if (status != FRACTUS_STATUS_OK) {
        fractus_indexed_image_shutdown(&image);
        return status;
    }

    fractus_app_log(output_path);
    status = fractus_legacy_graphic_save(output_path, &image, &new_palette);
    if (status != FRACTUS_STATUS_OK) {
        fractus_indexed_image_shutdown(&image);
        return status;
    }

    if (framebuffer->size.width != image.size.width || framebuffer->size.height != image.size.height) {
        fractus_framebuffer_shutdown(framebuffer);
        if (fractus_framebuffer_init(framebuffer, image.size) != FRACTUS_STATUS_OK) {
            fractus_indexed_image_shutdown(&image);
            return FRACTUS_STATUS_ERROR;
        }
    }

    if (fractus_app_apply_palette(framebuffer, &new_palette) != FRACTUS_STATUS_OK ||
        fractus_framebuffer_clear(framebuffer, 0u) != FRACTUS_STATUS_OK ||
        fractus_graphics_blit(framebuffer, (fractus_point_i32){0, 0}, &image) != FRACTUS_STATUS_OK) {
        fractus_indexed_image_shutdown(&image);
        return FRACTUS_STATUS_ERROR;
    }

    fractus_indexed_image_shutdown(&image);
    return fractus_app_persist_current_palette(cfg_path, framebuffer, config);
}

static fractus_status fractus_app_save_current_graphic_file(
    const fractus_platform_context *platform,
    const fractus_framebuffer *framebuffer)
{
    fractus_indexed_image image;
    char graphic_path[512];
    fractus_status status;

    if (platform == NULL || framebuffer == NULL || !framebuffer->initialized) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    memset(&image, 0, sizeof(image));
    if (fractus_app_resolve_numbered_write_path(platform, "dibujos", ".drsg", graphic_path, sizeof(graphic_path)) != FRACTUS_STATUS_OK) {
        fractus_app_log("graphic: could not resolve a free numbered .drsg write path");
        return FRACTUS_STATUS_UNSUPPORTED;
    }

    if (fractus_graphics_capture_region(
            framebuffer,
            (fractus_rect_i32){0, 0, (int32_t)framebuffer->size.width, (int32_t)framebuffer->size.height},
            &image) != FRACTUS_STATUS_OK) {
        fractus_app_log("graphic: capture failed");
        return FRACTUS_STATUS_ERROR;
    }

    fractus_app_log(graphic_path);
    status = fractus_legacy_graphic_save(graphic_path, &image, &framebuffer->palette);
    fractus_indexed_image_shutdown(&image);
    if (status != FRACTUS_STATUS_OK) {
        fractus_app_log("graphic: saving numbered .drsg failed");
        return status;
    }

    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_app_save_next_graphic_if_requested(
    const fractus_platform_context *platform,
    const fractus_framebuffer *framebuffer,
    int *save_next_graphic,
    char *error_message,
    size_t error_message_size)
{
    fractus_status status;

    if (save_next_graphic == NULL || *save_next_graphic == 0) {
        return FRACTUS_STATUS_OK;
    }

    *save_next_graphic = 0;
    status = fractus_app_save_current_graphic_file(platform, framebuffer);
    if (status != FRACTUS_STATUS_OK) {
        fractus_app_log("runtime: saving next graphic failed");
        if (status == FRACTUS_STATUS_UNSUPPORTED) {
            (void)fractus_app_set_message(
                error_message,
                error_message_size,
                "No hay nombres libres de fractus0001.drsg a fractus9999.drsg.");
        }
    }

    return FRACTUS_STATUS_OK;
}

static size_t fractus_app_list_graphic_files(
    const fractus_platform_context *platform,
    fractus_app_graphic_file *files,
    size_t capacity)
{
    char graphic_directory[512];
    size_t count = 0u;

    if (platform == NULL || files == NULL || capacity == 0u) {
        return 0u;
    }

    if (fractus_formats_resolve_legacy_directory(platform, "dibujos", graphic_directory, sizeof(graphic_directory)) != FRACTUS_STATUS_OK) {
        fractus_app_log("graphic: dibujos directory not found");
        return 0u;
    }

#if defined(_WIN32)
    {
        WIN32_FIND_DATAA find_data;
        HANDLE find_handle;
        char search_path[512];

        if (fractus_formats_join_directory_path(
                graphic_directory,
                "*.*",
                '\\',
                search_path,
                sizeof(search_path)) != FRACTUS_STATUS_OK) {
            return 0u;
        }

        find_handle = FindFirstFileA(search_path, &find_data);
        if (find_handle == INVALID_HANDLE_VALUE) {
            return 0u;
        }

        do {
            if ((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
                continue;
            }

            if (!fractus_formats_has_extension(find_data.cFileName, ".drsg")) {
                continue;
            }

            if (count >= capacity) {
                break;
            }

            if (fractus_formats_copy_path(find_data.cFileName, files[count].label, sizeof(files[count].label)) != FRACTUS_STATUS_OK ||
                fractus_formats_join_directory_path(
                    graphic_directory,
                    find_data.cFileName,
                    '/',
                    files[count].path,
                    sizeof(files[count].path)) != FRACTUS_STATUS_OK) {
                continue;
            }
            ++count;
        } while (FindNextFileA(find_handle, &find_data) != 0);

        FindClose(find_handle);
    }
#else
    {
        DIR *directory = opendir(graphic_directory);
        struct dirent *entry;

        if (directory == NULL) {
            return 0u;
        }

        while ((entry = readdir(directory)) != NULL) {
            if (count >= capacity) {
                break;
            }

            if (!fractus_formats_has_extension(entry->d_name, ".drsg")) {
                continue;
            }

            if (fractus_formats_copy_path(entry->d_name, files[count].label, sizeof(files[count].label)) != FRACTUS_STATUS_OK ||
                fractus_formats_join_directory_path(
                    graphic_directory,
                    entry->d_name,
                    '/',
                    files[count].path,
                    sizeof(files[count].path)) != FRACTUS_STATUS_OK) {
                continue;
            }
            ++count;
        }

        closedir(directory);
    }
#endif

    fractus_app_sort_graphic_files(files, count);
    return count;
}

static size_t fractus_app_list_palette_files(
    const fractus_platform_context *platform,
    fractus_app_palette_file *files,
    size_t capacity)
{
    char palette_directory[512];
    size_t count = 0u;

    if (platform == NULL || files == NULL || capacity == 0u) {
        return 0u;
    }

    if (fractus_formats_resolve_legacy_directory(platform, "paletas", palette_directory, sizeof(palette_directory)) != FRACTUS_STATUS_OK) {
        fractus_app_log("palette: paletas directory not found");
        return 0u;
    }

#if defined(_WIN32)
    {
        WIN32_FIND_DATAA find_data;
        HANDLE find_handle;
        char search_path[512];

        if (fractus_formats_join_directory_path(
                palette_directory,
                "*.drsp",
                '\\',
                search_path,
                sizeof(search_path)) != FRACTUS_STATUS_OK) {
            return 0u;
        }

        find_handle = FindFirstFileA(search_path, &find_data);
        if (find_handle == INVALID_HANDLE_VALUE) {
            return 0u;
        }

        do {
            if ((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
                continue;
            }

            if (count >= capacity) {
                break;
            }

            if (fractus_formats_copy_path(find_data.cFileName, files[count].label, sizeof(files[count].label)) != FRACTUS_STATUS_OK ||
                fractus_formats_join_directory_path(
                    palette_directory,
                    find_data.cFileName,
                    '/',
                    files[count].path,
                    sizeof(files[count].path)) != FRACTUS_STATUS_OK) {
                continue;
            }
            ++count;
        } while (FindNextFileA(find_handle, &find_data) != 0);

        FindClose(find_handle);
    }
#else
    {
        DIR *directory = opendir(palette_directory);
        struct dirent *entry;

        if (directory == NULL) {
            return 0u;
        }

        while ((entry = readdir(directory)) != NULL) {
            if (count >= capacity) {
                break;
            }

            if (!fractus_formats_has_extension(entry->d_name, ".drsp")) {
                continue;
            }

            if (fractus_formats_copy_path(entry->d_name, files[count].label, sizeof(files[count].label)) != FRACTUS_STATUS_OK ||
                fractus_formats_join_directory_path(
                    palette_directory,
                    entry->d_name,
                    '/',
                    files[count].path,
                    sizeof(files[count].path)) != FRACTUS_STATUS_OK) {
                continue;
            }
            ++count;
        }

        closedir(directory);
    }
#endif

    fractus_app_sort_palette_files(files, count);
    return count;
}

static fractus_status fractus_app_load_legacy_assets(
    const fractus_platform_context *platform,
    fractus_framebuffer *framebuffer,
    fractus_mandelbrot_params *mandelbrot_params,
    fractus_julia_params *julia_params,
    fractus_biomorph_params *biomorph_params,
    fractus_legacy_config *legacy_config,
    char *cfg_path,
    size_t cfg_path_size)
{
    fractus_palette palette;
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
    if (fractus_app_apply_legacy_config(framebuffer, legacy_config) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    status = fractus_formats_resolve_legacy_path(platform, "paletas/defecto.drsp", path_buffer, sizeof(path_buffer));
    if (status == FRACTUS_STATUS_OK) {
        fractus_app_log(path_buffer);
        if (fractus_legacy_palette_load(path_buffer, &palette) != FRACTUS_STATUS_OK) {
            fractus_app_log("startup: default .drsp found but load failed");
            return FRACTUS_STATUS_ERROR;
        }

        fractus_app_log("startup: legacy palette loaded");
        return fractus_app_apply_palette(framebuffer, &palette);
    }

    fractus_app_log("startup: default .drsp not found");

    return FRACTUS_STATUS_OK;
}

static void fractus_app_build_options_from_entries(
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
        fractus_ui_draw_group_box(framebuffer, fonts, 322, 60, 635, 375, 8u, 0u, "Graficos y colores") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_group_box(framebuffer, fonts, 5, 280, 318, 375, 8u, 0u, "Configuracion por defecto") != FRACTUS_STATUS_OK) {
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

static fractus_status fractus_app_render_main_menu(
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
    fractus_julia_params *julia_params,
    fractus_julia_params *julia_pending,
    fractus_biomorph_params *biomorph_params,
    fractus_biomorph_params *biomorph_pending,
    fractus_ui_numeric_field *iterations_field,
    fractus_ui_numeric_field *escape_radius_field,
    fractus_ui_numeric_field *biomorph_radius_field,
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
        mandelbrot_params == NULL || mandelbrot_pending == NULL || julia_params == NULL ||
        julia_pending == NULL || biomorph_params == NULL || biomorph_pending == NULL ||
        iterations_field == NULL || escape_radius_field == NULL || biomorph_radius_field == NULL ||
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
            *mandelbrot_pending = *mandelbrot_params;
            *view = FRACTUS_APP_VIEW_MANDELBROT_CONFIG;
        } else if (selected_menu == 1) {
            *julia_pending = *julia_params;
            *view = FRACTUS_APP_VIEW_JULIA_CONFIG;
        } else if (selected_menu == 2) {
            *biomorph_pending = *biomorph_params;
            *view = FRACTUS_APP_VIEW_BIOMORPH_CONFIG;
        } else if (selected_menu == 5) {
            *view = FRACTUS_APP_VIEW_PLASMA_MENU;
        } else if (selected_menu == FRACTUS_APP_MENU_VIDEO_INDEX) {
            *config_draft = *legacy_config;
            *view = FRACTUS_APP_VIEW_VIDEO_CONFIG;
        } else if (selected_menu == FRACTUS_APP_MENU_ITERATIONS_INDEX) {
            *config_draft = *legacy_config;
            (void)fractus_ui_numeric_field_init_int(
                iterations_field,
                (fractus_rect_i32){305, 250, 73, 22},
                (int32_t)config_draft->iterations,
                16,
                1024);
            *view = FRACTUS_APP_VIEW_ITERATIONS_CONFIG;
        } else if (selected_menu == FRACTUS_APP_MENU_ESCAPE_RADIUS_INDEX) {
            *config_draft = *legacy_config;
            (void)fractus_ui_numeric_field_init_int(
                escape_radius_field,
                (fractus_rect_i32){305, 250, 73, 22},
                (int32_t)config_draft->escape_radius_squared,
                4,
                1000);
            *view = FRACTUS_APP_VIEW_ESCAPE_RADIUS_CONFIG;
        } else if (selected_menu == FRACTUS_APP_MENU_BIOMORPH_RADIUS_INDEX) {
            *config_draft = *legacy_config;
            (void)fractus_ui_numeric_field_init_int(
                biomorph_radius_field,
                (fractus_rect_i32){305, 250, 73, 22},
                (int32_t)config_draft->biomorph_escape_radius_squared,
                4,
                1000);
            *view = FRACTUS_APP_VIEW_BIOMORPH_RADIUS_CONFIG;
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
        }
    }

    return FRACTUS_STATUS_OK;
}

static size_t fractus_app_copy_control_entries(
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

static size_t fractus_app_build_mandel_config_entries(
    fractus_app_menu_entry *entries,
    size_t capacity)
{
    static const fractus_app_menu_entry controls[] = {
        {FRACTUS_APP_RECT(379, 152, 419, 172), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 152, 464, 172), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 182, 419, 202), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 182, 464, 202), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 212, 419, 232), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 212, 464, 232), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 242, 419, 262), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 242, 464, 262), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 272, 419, 292), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 272, 464, 292), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 302, 419, 322), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 302, 464, 322), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(299, 332, 371, 352), 8u, 0u, "Clasico"},
        {FRACTUS_APP_RECT(379, 332, 464, 352), 8u, 0u, "Suave"},
        {FRACTUS_APP_RECT(210, 387, 310, 407), 8u, 0u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 387, 430, 407), 0u, 15u, "Cancelar"}
    };

    return fractus_app_copy_control_entries(
        entries,
        capacity,
        controls,
        FRACTUS_APP_ARRAY_COUNT(controls));
}

static fractus_status fractus_app_run_mandelbrot_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_mandelbrot_params *params,
    fractus_mandelbrot_params *pending,
    fractus_app_view *view)
{
    char buffer[32];
    const fractus_ui_radio_option color_options[] = {
        {FRACTUS_APP_RECT(299, 332, 371, 352), "Clasico"},
        {FRACTUS_APP_RECT(379, 332, 464, 352), "Suave"}
    };
    fractus_app_menu_entry dialog_entries[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    fractus_ui_menu_option dialog_options[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    size_t dialog_entry_count;
    int active_index;
    int selected_menu = -1;
    int cancelled = 0;

    if (framebuffer == NULL || fonts == NULL || ui == NULL ||
        params == NULL || pending == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    dialog_entry_count = fractus_app_build_mandel_config_entries(dialog_entries, FRACTUS_APP_ARRAY_COUNT(dialog_entries));
    fractus_app_build_options_from_entries(dialog_options, dialog_entries, dialog_entry_count);
    active_index = fractus_ui_active_menu_index(ui, dialog_options, dialog_entry_count);

    /* 1. Contenedor exterior. */
    if (fractus_app_render_main_menu(framebuffer, fonts, -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_window(framebuffer, 160, 112, 479, 417) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, 320, 116, 15u, "Conjunto de Benoit B. Mandelbrot") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 2. Textos y controles. */
    if (fractus_ui_draw_group_box(framebuffer, fonts, 165, 142, 474, 361, 8u, 0u, "Parametros del conjunto") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    snprintf(buffer, sizeof(buffer), "%.3f", pending->xmin);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 157, "Minimo valor real", 299, 152, 371, 172, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", pending->xmax);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 187, "Maximo valor real", 299, 182, 371, 202, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", pending->ymin);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 217, "Minimo valor imaginario", 299, 212, 371, 232, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", pending->ymax);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 247, "Maximo valor imaginario", 299, 242, 371, 262, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%u", pending->max_iterations);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 277, "Iteraciones", 299, 272, 371, 292, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.0f", pending->escape_radius_squared);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 307, "Radio de escape", 299, 302, 371, 322, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    if (fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 175, 337, 0u, "Color") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_radio_list(
            framebuffer,
            fonts,
            color_options,
            FRACTUS_APP_ARRAY_COUNT(color_options),
            pending->color_mode == FRACTUS_MANDELBROT_COLOR_SMOOTH ? 1 : 0,
            (active_index == FRACTUS_APP_MANDEL_COLOR_CLASSIC || active_index == FRACTUS_APP_MANDEL_COLOR_SMOOTH) ?
                active_index - FRACTUS_APP_MANDEL_COLOR_CLASSIC :
                -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, 320, 367, 0u, "Estos valores solo afectan al dibujo actual y no cambian la cfg global.") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_button_list(framebuffer, fonts, dialog_entries, FRACTUS_APP_MANDEL_COLOR_CLASSIC, active_index) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_button_list(
            framebuffer,
            fonts,
            &dialog_entries[FRACTUS_APP_MANDEL_DRAW],
            2u,
            (active_index >= FRACTUS_APP_MANDEL_DRAW) ? active_index - FRACTUS_APP_MANDEL_DRAW : -1) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones de botones. */
    if (fractus_ui_menu(ui, dialog_options, dialog_entry_count, &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == FRACTUS_APP_MANDEL_CANCEL) {
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (selected_menu == FRACTUS_APP_MANDEL_DRAW) {
            *params = *pending;
            *view = FRACTUS_APP_VIEW_MANDELBROT;
        } else if (selected_menu == FRACTUS_APP_MANDEL_XMIN_DEC) {
            pending->xmin = fractus_app_clamp_f64(pending->xmin - 0.1, -5.0, pending->xmax - 0.1);
        } else if (selected_menu == FRACTUS_APP_MANDEL_XMIN_INC) {
            pending->xmin = fractus_app_clamp_f64(pending->xmin + 0.1, -5.0, pending->xmax - 0.1);
        } else if (selected_menu == FRACTUS_APP_MANDEL_XMAX_DEC) {
            pending->xmax = fractus_app_clamp_f64(pending->xmax - 0.1, pending->xmin + 0.1, 5.0);
        } else if (selected_menu == FRACTUS_APP_MANDEL_XMAX_INC) {
            pending->xmax = fractus_app_clamp_f64(pending->xmax + 0.1, pending->xmin + 0.1, 5.0);
        } else if (selected_menu == FRACTUS_APP_MANDEL_YMIN_DEC) {
            pending->ymin = fractus_app_clamp_f64(pending->ymin - 0.1, -5.0, pending->ymax - 0.1);
        } else if (selected_menu == FRACTUS_APP_MANDEL_YMIN_INC) {
            pending->ymin = fractus_app_clamp_f64(pending->ymin + 0.1, -5.0, pending->ymax - 0.1);
        } else if (selected_menu == FRACTUS_APP_MANDEL_YMAX_DEC) {
            pending->ymax = fractus_app_clamp_f64(pending->ymax - 0.1, pending->ymin + 0.1, 5.0);
        } else if (selected_menu == FRACTUS_APP_MANDEL_YMAX_INC) {
            pending->ymax = fractus_app_clamp_f64(pending->ymax + 0.1, pending->ymin + 0.1, 5.0);
        } else if (selected_menu == FRACTUS_APP_MANDEL_ITER_DEC) {
            pending->max_iterations = (uint32_t)fractus_app_clamp_i32((int32_t)pending->max_iterations - 16, 16, 1024);
        } else if (selected_menu == FRACTUS_APP_MANDEL_ITER_INC) {
            pending->max_iterations = (uint32_t)fractus_app_clamp_i32((int32_t)pending->max_iterations + 16, 16, 1024);
        } else if (selected_menu == FRACTUS_APP_MANDEL_RADIUS_DEC) {
            pending->escape_radius_squared = (double)fractus_app_clamp_i32((int32_t)pending->escape_radius_squared - 1, 1, 1000);
        } else if (selected_menu == FRACTUS_APP_MANDEL_RADIUS_INC) {
            pending->escape_radius_squared = (double)fractus_app_clamp_i32((int32_t)pending->escape_radius_squared + 1, 1, 1000);
        } else if (selected_menu == FRACTUS_APP_MANDEL_COLOR_CLASSIC) {
            pending->color_mode = FRACTUS_MANDELBROT_COLOR_ESCAPE;
        } else if (selected_menu == FRACTUS_APP_MANDEL_COLOR_SMOOTH) {
            pending->color_mode = FRACTUS_MANDELBROT_COLOR_SMOOTH;
        }
    }

    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_app_draw_drawing_footer(
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

static fractus_status fractus_app_draw_save_feedback(
    fractus_framebuffer *framebuffer,
    uint32_t frame)
{
    int32_t thickness;
    int32_t i;

    if (framebuffer == NULL || !framebuffer->initialized) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (frame >= 3u) {
        return fractus_graphics_fill_rect(
            framebuffer,
            (fractus_rect_i32){0, 0, (int32_t)framebuffer->size.width, (int32_t)framebuffer->size.height},
            15u);
    }

    thickness = (int32_t)(framebuffer->size.height / 120u);
    if (thickness < 4) {
        thickness = 4;
    }

    for (i = 0; i < thickness; ++i) {
        if (fractus_graphics_rect(
                framebuffer,
                (fractus_rect_i32){
                    i,
                    i,
                    (int32_t)framebuffer->size.width - i * 2,
                    (int32_t)framebuffer->size.height - i * 2},
                15u) != FRACTUS_STATUS_OK) {
            return FRACTUS_STATUS_ERROR;
        }
    }

    return FRACTUS_STATUS_OK;
}

static int fractus_app_map_drawing_window_point(
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

static int fractus_app_apply_mandelbrot_selection(
    const fractus_framebuffer *framebuffer,
    fractus_mandelbrot_params *params,
    fractus_point_i32 first,
    fractus_point_i32 second)
{
    int32_t x_min;
    int32_t x_max;
    int32_t y_min;
    int32_t y_max;
    double old_xmin;
    double old_xmax;
    double old_ymin;
    double old_ymax;
    double dx;
    double dy;

    if (framebuffer == NULL || params == NULL ||
        !framebuffer->initialized || framebuffer->size.width < 2u || framebuffer->size.height < 2u) {
        return 0;
    }

    x_min = (first.x < second.x) ? first.x : second.x;
    x_max = (first.x > second.x) ? first.x : second.x;
    y_min = (first.y < second.y) ? first.y : second.y;
    y_max = (first.y > second.y) ? first.y : second.y;
    if (x_min == x_max || y_min == y_max) {
        return 0;
    }

    old_xmin = params->xmin;
    old_xmax = params->xmax;
    old_ymin = params->ymin;
    old_ymax = params->ymax;
    dx = (old_xmax - old_xmin) / (double)(framebuffer->size.width - 1u);
    dy = (old_ymax - old_ymin) / (double)(framebuffer->size.height - 1u);

    params->xmin = old_xmin + (double)x_min * dx;
    params->xmax = old_xmin + (double)x_max * dx;
    params->ymax = old_ymax - (double)y_min * dy;
    params->ymin = old_ymax - (double)y_max * dy;
    return 1;
}

static fractus_status fractus_app_draw_mandelbrot_selection_overlay(
    const fractus_platform_context *platform,
    fractus_framebuffer *framebuffer,
    const fractus_ui_context *ui,
    const fractus_app_mandelbrot_selection *selection)
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
        int32_t x = (selection->first_corner.x < cursor.x) ? selection->first_corner.x : cursor.x;
        int32_t y = (selection->first_corner.y < cursor.y) ? selection->first_corner.y : cursor.y;
        int32_t width = (selection->first_corner.x > cursor.x) ?
            selection->first_corner.x - cursor.x + 1 :
            cursor.x - selection->first_corner.x + 1;
        int32_t height = (selection->first_corner.y > cursor.y) ?
            selection->first_corner.y - cursor.y + 1 :
            cursor.y - selection->first_corner.y + 1;

        if (fractus_graphics_rect(framebuffer, (fractus_rect_i32){x, y, width, height}, 15u) != FRACTUS_STATUS_OK) {
            return FRACTUS_STATUS_ERROR;
        }
    }

    return FRACTUS_STATUS_OK;
}

static int fractus_app_handle_mandelbrot_selection_input(
    const fractus_platform_context *platform,
    const fractus_framebuffer *framebuffer,
    const fractus_ui_context *ui,
    fractus_mandelbrot_params *params,
    fractus_app_mandelbrot_selection *selection)
{
    fractus_point_i32 selected_point;

    if (platform == NULL || framebuffer == NULL || ui == NULL || params == NULL || selection == NULL) {
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
        } else if (fractus_app_apply_mandelbrot_selection(framebuffer, params, selection->first_corner, selected_point)) {
            selection->active = 0;
            selection->has_first_corner = 0;
            return 1;
        }
    }

    return 0;
}

static fractus_status fractus_app_copy_framebuffer_for_overlay(
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

static fractus_status fractus_app_render_mandelbrot(
    const fractus_platform_context *platform,
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const fractus_mandelbrot_params *params,
    int *save_next_graphic,
    char *error_message,
    size_t error_message_size)
{
    if (platform == NULL || framebuffer == NULL || fonts == NULL || params == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_fractal_render_mandelbrot(framebuffer, params) != FRACTUS_STATUS_OK ||
        fractus_app_save_next_graphic_if_requested(platform, framebuffer, save_next_graphic, error_message, error_message_size) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return fractus_app_draw_drawing_footer(framebuffer, fonts, "ESC/boton derecho: menu - G: grabar - S: seleccionar zona");
}

static size_t fractus_app_build_julia_config_entries(
    fractus_app_menu_entry *entries,
    size_t capacity)
{
    static const fractus_app_menu_entry controls[] = {
        {FRACTUS_APP_RECT(379, 124, 419, 144), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 124, 464, 144), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 154, 419, 174), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 154, 464, 174), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 184, 419, 204), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 184, 464, 204), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 214, 419, 234), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 214, 464, 234), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 244, 419, 264), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 244, 464, 264), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 274, 419, 294), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 274, 464, 294), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 304, 419, 324), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 304, 464, 324), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 334, 419, 354), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 334, 464, 354), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(210, 389, 310, 409), 8u, 0u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 389, 430, 409), 0u, 15u, "Cancelar"}
    };

    return fractus_app_copy_control_entries(
        entries,
        capacity,
        controls,
        FRACTUS_APP_ARRAY_COUNT(controls));
}

static fractus_status fractus_app_run_julia_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_julia_params *params,
    fractus_julia_params *pending,
    fractus_app_view *view)
{
    char buffer[32];
    fractus_app_menu_entry dialog_entries[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    fractus_ui_menu_option dialog_options[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    size_t dialog_entry_count;
    int selected_menu = -1;
    int cancelled = 0;

    if (framebuffer == NULL || fonts == NULL || ui == NULL ||
        params == NULL || pending == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    dialog_entry_count = fractus_app_build_julia_config_entries(dialog_entries, FRACTUS_APP_ARRAY_COUNT(dialog_entries));
    fractus_app_build_options_from_entries(dialog_options, dialog_entries, dialog_entry_count);

    /* 1. Contenedor exterior. */
    if (fractus_app_render_main_menu(framebuffer, fonts, -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_window(framebuffer, 160, 84, 479, 419) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, 320, 88, 15u, "Conjuntos de Gaston Julia") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 2. Textos y controles. */
    if (fractus_ui_draw_group_box(framebuffer, fonts, 165, 114, 474, 363, 8u, 0u, "Parametros del conjunto") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    snprintf(buffer, sizeof(buffer), "%.3f", pending->xmin);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 129, "Minimo valor real", 299, 124, 371, 144, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", pending->xmax);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 159, "Maximo valor real", 299, 154, 371, 174, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", pending->ymin);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 189, "Minimo valor imaginario", 299, 184, 371, 204, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", pending->ymax);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 219, "Maximo valor imaginario", 299, 214, 371, 234, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", pending->constant_real);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 249, "Constante real", 299, 244, 371, 264, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", pending->constant_imag);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 279, "Constante imaginaria", 299, 274, 371, 294, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%u", pending->max_iterations);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 309, "Iteraciones", 299, 304, 371, 324, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.0f", pending->escape_radius_squared);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 339, "Radio de escape", 299, 334, 371, 354, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    if (fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, 320, 369, 0u, "Estos valores solo afectan al dibujo actual y no cambian la cfg global.") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_button_list(
            framebuffer,
            fonts,
            dialog_entries,
            dialog_entry_count,
            fractus_ui_active_menu_index(ui, dialog_options, dialog_entry_count)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones de botones. */
    if (fractus_ui_menu(ui, dialog_options, dialog_entry_count, &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == FRACTUS_APP_JULIA_CANCEL) {
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (selected_menu == FRACTUS_APP_JULIA_DRAW) {
            *params = *pending;
            *view = FRACTUS_APP_VIEW_JULIA;
        } else if (selected_menu == FRACTUS_APP_JULIA_XMIN_DEC) {
            pending->xmin = fractus_app_clamp_f64(pending->xmin - 0.1, -5.0, pending->xmax - 0.1);
        } else if (selected_menu == FRACTUS_APP_JULIA_XMIN_INC) {
            pending->xmin = fractus_app_clamp_f64(pending->xmin + 0.1, -5.0, pending->xmax - 0.1);
        } else if (selected_menu == FRACTUS_APP_JULIA_XMAX_DEC) {
            pending->xmax = fractus_app_clamp_f64(pending->xmax - 0.1, pending->xmin + 0.1, 5.0);
        } else if (selected_menu == FRACTUS_APP_JULIA_XMAX_INC) {
            pending->xmax = fractus_app_clamp_f64(pending->xmax + 0.1, pending->xmin + 0.1, 5.0);
        } else if (selected_menu == FRACTUS_APP_JULIA_YMIN_DEC) {
            pending->ymin = fractus_app_clamp_f64(pending->ymin - 0.1, -5.0, pending->ymax - 0.1);
        } else if (selected_menu == FRACTUS_APP_JULIA_YMIN_INC) {
            pending->ymin = fractus_app_clamp_f64(pending->ymin + 0.1, -5.0, pending->ymax - 0.1);
        } else if (selected_menu == FRACTUS_APP_JULIA_YMAX_DEC) {
            pending->ymax = fractus_app_clamp_f64(pending->ymax - 0.1, pending->ymin + 0.1, 5.0);
        } else if (selected_menu == FRACTUS_APP_JULIA_YMAX_INC) {
            pending->ymax = fractus_app_clamp_f64(pending->ymax + 0.1, pending->ymin + 0.1, 5.0);
        } else if (selected_menu == FRACTUS_APP_JULIA_CREAL_DEC) {
            pending->constant_real = fractus_app_clamp_f64(pending->constant_real - 0.05, -2.0, 2.0);
        } else if (selected_menu == FRACTUS_APP_JULIA_CREAL_INC) {
            pending->constant_real = fractus_app_clamp_f64(pending->constant_real + 0.05, -2.0, 2.0);
        } else if (selected_menu == FRACTUS_APP_JULIA_CIMAG_DEC) {
            pending->constant_imag = fractus_app_clamp_f64(pending->constant_imag - 0.05, -2.0, 2.0);
        } else if (selected_menu == FRACTUS_APP_JULIA_CIMAG_INC) {
            pending->constant_imag = fractus_app_clamp_f64(pending->constant_imag + 0.05, -2.0, 2.0);
        } else if (selected_menu == FRACTUS_APP_JULIA_ITER_DEC) {
            pending->max_iterations = (uint32_t)fractus_app_clamp_i32((int32_t)pending->max_iterations - 16, 16, 1024);
        } else if (selected_menu == FRACTUS_APP_JULIA_ITER_INC) {
            pending->max_iterations = (uint32_t)fractus_app_clamp_i32((int32_t)pending->max_iterations + 16, 16, 1024);
        } else if (selected_menu == FRACTUS_APP_JULIA_RADIUS_DEC) {
            pending->escape_radius_squared = (double)fractus_app_clamp_i32((int32_t)pending->escape_radius_squared - 1, 1, 1000);
        } else if (selected_menu == FRACTUS_APP_JULIA_RADIUS_INC) {
            pending->escape_radius_squared = (double)fractus_app_clamp_i32((int32_t)pending->escape_radius_squared + 1, 1, 1000);
        }
    }

    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_app_render_julia(
    const fractus_platform_context *platform,
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const fractus_julia_params *params,
    int *save_next_graphic,
    char *error_message,
    size_t error_message_size)
{
    if (platform == NULL || framebuffer == NULL || fonts == NULL || params == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_fractal_render_julia(framebuffer, params) != FRACTUS_STATUS_OK ||
        fractus_app_save_next_graphic_if_requested(platform, framebuffer, save_next_graphic, error_message, error_message_size) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return fractus_app_draw_drawing_footer(framebuffer, fonts, "ESC o boton derecho: menu - G: grabar dibujo");
}

static size_t fractus_app_build_biomorph_config_entries(
    fractus_app_menu_entry *entries,
    size_t capacity)
{
    static const fractus_app_menu_entry controls[] = {
        {FRACTUS_APP_RECT(379, 124, 419, 144), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 124, 464, 144), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 154, 419, 174), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 154, 464, 174), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 184, 419, 204), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 184, 464, 204), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 214, 419, 234), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 214, 464, 234), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 244, 419, 264), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 244, 464, 264), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 274, 419, 294), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 274, 464, 294), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 304, 419, 324), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 304, 464, 324), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(210, 359, 310, 379), 8u, 0u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 359, 430, 379), 0u, 15u, "Cancelar"}
    };

    return fractus_app_copy_control_entries(
        entries,
        capacity,
        controls,
        FRACTUS_APP_ARRAY_COUNT(controls));
}

static fractus_status fractus_app_run_biomorph_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_biomorph_params *params,
    fractus_biomorph_params *pending,
    fractus_app_view *view)
{
    char buffer[32];
    fractus_app_menu_entry dialog_entries[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    fractus_ui_menu_option dialog_options[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    size_t dialog_entry_count;
    int selected_menu = -1;
    int cancelled = 0;

    if (framebuffer == NULL || fonts == NULL || ui == NULL ||
        params == NULL || pending == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    dialog_entry_count = fractus_app_build_biomorph_config_entries(dialog_entries, FRACTUS_APP_ARRAY_COUNT(dialog_entries));
    fractus_app_build_options_from_entries(dialog_options, dialog_entries, dialog_entry_count);

    /* 1. Contenedor exterior. */
    if (fractus_app_render_main_menu(framebuffer, fonts, -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_window(framebuffer, 160, 84, 479, 389) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, 320, 88, 15u, "Biomorfos de Clifford Pickover") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 2. Textos y controles. */
    if (fractus_ui_draw_group_box(framebuffer, fonts, 165, 114, 474, 331, 8u, 0u, "Parametros del biomorfo") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    snprintf(buffer, sizeof(buffer), "%.3f", pending->xmin);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 129, "Minimo valor real", 299, 124, 371, 144, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", pending->xmax);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 159, "Maximo valor real", 299, 154, 371, 174, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", pending->ymin);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 189, "Minimo valor imaginario", 299, 184, 371, 204, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", pending->ymax);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 219, "Maximo valor imaginario", 299, 214, 371, 234, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", pending->constant_real);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 249, "Constante real", 299, 244, 371, 264, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.3f", pending->constant_imag);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 279, "Constante imaginaria", 299, 274, 371, 294, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%.0f", pending->escape_radius_squared);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 309, "Radio de escape", 299, 304, 371, 324, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    if (fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, 320, 339, 0u, "Estos valores solo afectan al dibujo actual y no cambian la cfg global.") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_button_list(
            framebuffer,
            fonts,
            dialog_entries,
            dialog_entry_count,
            fractus_ui_active_menu_index(ui, dialog_options, dialog_entry_count)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones de botones. */
    if (fractus_ui_menu(ui, dialog_options, dialog_entry_count, &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == FRACTUS_APP_BIOMORPH_CANCEL) {
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_DRAW) {
            *params = *pending;
            *view = FRACTUS_APP_VIEW_BIOMORPH;
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_XMIN_DEC) {
            pending->xmin = fractus_app_clamp_f64(pending->xmin - 0.1, -5.0, pending->xmax - 0.1);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_XMIN_INC) {
            pending->xmin = fractus_app_clamp_f64(pending->xmin + 0.1, -5.0, pending->xmax - 0.1);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_XMAX_DEC) {
            pending->xmax = fractus_app_clamp_f64(pending->xmax - 0.1, pending->xmin + 0.1, 5.0);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_XMAX_INC) {
            pending->xmax = fractus_app_clamp_f64(pending->xmax + 0.1, pending->xmin + 0.1, 5.0);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_YMIN_DEC) {
            pending->ymin = fractus_app_clamp_f64(pending->ymin - 0.1, -5.0, pending->ymax - 0.1);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_YMIN_INC) {
            pending->ymin = fractus_app_clamp_f64(pending->ymin + 0.1, -5.0, pending->ymax - 0.1);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_YMAX_DEC) {
            pending->ymax = fractus_app_clamp_f64(pending->ymax - 0.1, pending->ymin + 0.1, 5.0);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_YMAX_INC) {
            pending->ymax = fractus_app_clamp_f64(pending->ymax + 0.1, pending->ymin + 0.1, 5.0);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_CREAL_DEC) {
            pending->constant_real = fractus_app_clamp_f64(pending->constant_real - 0.05, -2.0, 2.0);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_CREAL_INC) {
            pending->constant_real = fractus_app_clamp_f64(pending->constant_real + 0.05, -2.0, 2.0);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_CIMAG_DEC) {
            pending->constant_imag = fractus_app_clamp_f64(pending->constant_imag - 0.05, -2.0, 2.0);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_CIMAG_INC) {
            pending->constant_imag = fractus_app_clamp_f64(pending->constant_imag + 0.05, -2.0, 2.0);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_RADIUS_DEC) {
            pending->escape_radius_squared = (double)fractus_app_clamp_i32((int32_t)pending->escape_radius_squared - 4, 4, 1000);
        } else if (selected_menu == FRACTUS_APP_BIOMORPH_RADIUS_INC) {
            pending->escape_radius_squared = (double)fractus_app_clamp_i32((int32_t)pending->escape_radius_squared + 4, 4, 1000);
        }
    }

    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_app_render_biomorph(
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

static fractus_status fractus_app_render_plasma_menu(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    int active_index,
    const fractus_app_menu_entry *plasma_menu_controls,
    size_t plasma_menu_control_count,
    fractus_app_plasma_method selected_method,
    int method_enabled)
{
    uint32_t i;

    if (framebuffer == NULL || fonts == NULL || plasma_menu_controls == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_app_render_main_menu(framebuffer, fonts, -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_window(framebuffer, 160, 120, 479, 320) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, 320, 124, 15u, "Fractales por el metodo de plasma") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_group_box(framebuffer, fonts, 165, 150, 474, 190, 8u, 0u, "Metodo de dibujo") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    for (i = 0u; i < plasma_menu_control_count; ++i) {
        fractus_app_menu_entry entry = plasma_menu_controls[i];
        int draw_active = active_index == (int)i;

        if (!method_enabled && i >= 2u) {
            continue;
        }

        if (i < 2u && !method_enabled) {
            entry.fill_color = 8u;
            entry.text_color = 7u;
            draw_active = 0;
        } else if (i == 2u && selected_method == FRACTUS_APP_PLASMA_METHOD_NONE) {
            entry.fill_color = 8u;
            entry.text_color = 7u;
            draw_active = 0;
        }

        if (fractus_ui_draw_button(framebuffer, fonts, &entry, draw_active) != FRACTUS_STATUS_OK) {
            return FRACTUS_STATUS_ERROR;
        }
    }

    if (selected_method != FRACTUS_APP_PLASMA_METHOD_NONE && !method_enabled) {
        return FRACTUS_STATUS_OK;
    }

    return fractus_ui_draw_text_centered(
        framebuffer,
        fonts,
        FRACTUS_FONT_SMALL,
        320,
        270,
        0u,
        "Seleccione Rectangulos o Circulos");
}

static fractus_status fractus_app_run_plasma_menu_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_plasma_params *plasma_rectangular_params,
    fractus_plasma_params *plasma_rectangular_pending,
    fractus_plasma_circular_params *plasma_circular_params,
    fractus_plasma_circular_params *plasma_circular_pending,
    fractus_app_view *view)
{
    static const fractus_app_menu_entry plasma_menu_controls[] = {
        {FRACTUS_APP_RECT(210, 160, 310, 180), 8u, 0u, "Rectangulos"},
        {FRACTUS_APP_RECT(330, 160, 430, 180), 8u, 0u, "Circulos"},
        {FRACTUS_APP_RECT(210, 290, 310, 310), 6u, 8u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 290, 430, 310), 0u, 15u, "Cancelar"}
    };
    fractus_ui_menu_option plasma_menu_options[FRACTUS_APP_ARRAY_COUNT(plasma_menu_controls)];
    int selected_menu = -1;
    int cancelled = 0;

    if (framebuffer == NULL || fonts == NULL || ui == NULL ||
        plasma_rectangular_params == NULL || plasma_rectangular_pending == NULL || plasma_circular_params == NULL ||
        plasma_circular_pending == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    fractus_app_build_options_from_entries(
        plasma_menu_options,
        plasma_menu_controls,
        FRACTUS_APP_ARRAY_COUNT(plasma_menu_controls));

    /* 1. Contenedor exterior. */
    /* 2. Textos y controles. */
    if (fractus_app_render_plasma_menu(
            framebuffer,
            fonts,
            fractus_ui_active_menu_index(ui, plasma_menu_options, FRACTUS_APP_ARRAY_COUNT(plasma_menu_options)),
            plasma_menu_controls,
            FRACTUS_APP_ARRAY_COUNT(plasma_menu_controls),
            FRACTUS_APP_PLASMA_METHOD_NONE,
            1) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones de botones. */
    if (fractus_ui_menu(ui, plasma_menu_options, FRACTUS_APP_ARRAY_COUNT(plasma_menu_options), &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == 3) {
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (selected_menu == (int)FRACTUS_APP_PLASMA_METHOD_RECTANGLES) {
            *plasma_rectangular_pending = *plasma_rectangular_params;
            *view = FRACTUS_APP_VIEW_PLASMA_RECTANGULAR_CONFIG;
        } else if (selected_menu == (int)FRACTUS_APP_PLASMA_METHOD_CIRCLES) {
            *plasma_circular_pending = *plasma_circular_params;
            *view = FRACTUS_APP_VIEW_PLASMA_CIRCULAR_CONFIG;
        }
    }

    return FRACTUS_STATUS_OK;
}

static size_t fractus_app_build_plasma_rectangular_config_entries(
    fractus_app_menu_entry *entries,
    size_t capacity)
{
    static const fractus_app_menu_entry controls[] = {
        {FRACTUS_APP_RECT(379, 205, 419, 225), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 205, 464, 225), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(210, 290, 310, 310), 8u, 0u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 290, 430, 310), 0u, 15u, "Cancelar"}
    };

    return fractus_app_copy_control_entries(
        entries,
        capacity,
        controls,
        FRACTUS_APP_ARRAY_COUNT(controls));
}

static fractus_status fractus_app_run_plasma_rectangular_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_plasma_params *params,
    fractus_plasma_params *pending,
    fractus_app_view *view)
{
    static const fractus_app_menu_entry plasma_menu_controls[] = {
        {FRACTUS_APP_RECT(210, 160, 310, 180), 8u, 0u, "Rectangulos"},
        {FRACTUS_APP_RECT(330, 160, 430, 180), 8u, 0u, "Circulos"},
        {FRACTUS_APP_RECT(210, 290, 310, 310), 6u, 8u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 290, 430, 310), 0u, 15u, "Cancelar"}
    };
    char buffer[32];
    fractus_app_menu_entry dialog_entries[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    fractus_ui_menu_option dialog_options[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    size_t dialog_entry_count;
    int selected_menu = -1;
    int cancelled = 0;

    if (framebuffer == NULL || fonts == NULL || ui == NULL ||
        params == NULL || pending == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    dialog_entry_count = fractus_app_build_plasma_rectangular_config_entries(dialog_entries, FRACTUS_APP_ARRAY_COUNT(dialog_entries));
    fractus_app_build_options_from_entries(dialog_options, dialog_entries, dialog_entry_count);

    /* 1. Contenedor exterior. */
    if (fractus_app_render_plasma_menu(
            framebuffer,
            fonts,
            -1,
            plasma_menu_controls,
            FRACTUS_APP_ARRAY_COUNT(plasma_menu_controls),
            FRACTUS_APP_PLASMA_METHOD_RECTANGLES,
            0) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_group_box(framebuffer, fonts, 165, 195, 474, 264, 8u, 0u, "Rectangulos") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 2. Textos y controles. */
    snprintf(buffer, sizeof(buffer), "%d", pending->dispersion);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 210, "Indice de dispersion", 299, 205, 371, 225, buffer) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, 320, 270, 0u, "Valores bajos suavizan el gradiente; altos lo acentuan.") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_button_list(
            framebuffer,
            fonts,
            dialog_entries,
            dialog_entry_count,
            fractus_ui_active_menu_index(ui, dialog_options, dialog_entry_count)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones de botones. */
    if (fractus_ui_menu(ui, dialog_options, dialog_entry_count, &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == FRACTUS_APP_PLASMA_RECTANGULAR_CANCEL) {
            *view = FRACTUS_APP_VIEW_PLASMA_MENU;
        } else if (selected_menu == FRACTUS_APP_PLASMA_RECTANGULAR_DRAW) {
            *params = *pending;
            *view = FRACTUS_APP_VIEW_PLASMA_RECTANGULAR;
        } else if (selected_menu == FRACTUS_APP_PLASMA_RECTANGULAR_DISPERSION_DEC) {
            pending->dispersion = fractus_app_clamp_i32(pending->dispersion - 5, 1, 500);
        } else if (selected_menu == FRACTUS_APP_PLASMA_RECTANGULAR_DISPERSION_INC) {
            pending->dispersion = fractus_app_clamp_i32(pending->dispersion + 5, 1, 500);
        }
    }

    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_app_render_plasma_rectangular(
    const fractus_platform_context *platform,
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const fractus_plasma_params *params,
    int *save_next_graphic,
    char *error_message,
    size_t error_message_size)
{
    if (platform == NULL || framebuffer == NULL || fonts == NULL || params == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_fractal_render_plasma(framebuffer, params) != FRACTUS_STATUS_OK ||
        fractus_app_save_next_graphic_if_requested(platform, framebuffer, save_next_graphic, error_message, error_message_size) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return fractus_app_draw_drawing_footer(framebuffer, fonts, "ESC o boton derecho: menu - G: grabar dibujo");
}

static size_t fractus_app_build_plasma_circular_config_entries(
    fractus_app_menu_entry *entries,
    size_t capacity)
{
    static const fractus_app_menu_entry controls[] = {
        {FRACTUS_APP_RECT(379, 205, 419, 225), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 205, 464, 225), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(379, 235, 419, 255), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(424, 235, 464, 255), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(210, 290, 310, 310), 8u, 0u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 290, 430, 310), 0u, 15u, "Cancelar"}
    };

    return fractus_app_copy_control_entries(
        entries,
        capacity,
        controls,
        FRACTUS_APP_ARRAY_COUNT(controls));
}

static fractus_status fractus_app_run_plasma_circular_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_plasma_circular_params *params,
    fractus_plasma_circular_params *pending,
    fractus_app_view *view)
{
    static const fractus_app_menu_entry plasma_menu_controls[] = {
        {FRACTUS_APP_RECT(210, 160, 310, 180), 8u, 0u, "Rectangulos"},
        {FRACTUS_APP_RECT(330, 160, 430, 180), 8u, 0u, "Circulos"},
        {FRACTUS_APP_RECT(210, 290, 310, 310), 6u, 8u, "Dibujar"},
        {FRACTUS_APP_RECT(330, 290, 430, 310), 0u, 15u, "Cancelar"}
    };
    char buffer[32];
    fractus_app_menu_entry dialog_entries[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    fractus_ui_menu_option dialog_options[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    size_t dialog_entry_count;
    int selected_menu = -1;
    int cancelled = 0;

    if (framebuffer == NULL || fonts == NULL || ui == NULL ||
        params == NULL || pending == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    dialog_entry_count = fractus_app_build_plasma_circular_config_entries(dialog_entries, FRACTUS_APP_ARRAY_COUNT(dialog_entries));
    fractus_app_build_options_from_entries(dialog_options, dialog_entries, dialog_entry_count);

    /* 1. Contenedor exterior. */
    if (fractus_app_render_plasma_menu(
            framebuffer,
            fonts,
            -1,
            plasma_menu_controls,
            FRACTUS_APP_ARRAY_COUNT(plasma_menu_controls),
            FRACTUS_APP_PLASMA_METHOD_CIRCLES,
            0) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_group_box(framebuffer, fonts, 165, 195, 474, 264, 8u, 0u, "Circulos") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 2. Textos y controles. */
    snprintf(buffer, sizeof(buffer), "%d", pending->circle_count);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 210, "Numero de circulos", 299, 205, 371, 225, buffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    snprintf(buffer, sizeof(buffer), "%d", pending->max_radius);
    if (fractus_ui_draw_numeric_row(framebuffer, fonts, 175, 240, "Radio maximo", 299, 235, 371, 255, buffer) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, 320, 270, 0u, "Mas circulos y radios mayores producen dibujos mas densos.") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_button_list(
            framebuffer,
            fonts,
            dialog_entries,
            dialog_entry_count,
            fractus_ui_active_menu_index(ui, dialog_options, dialog_entry_count)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones de botones. */
    if (fractus_ui_menu(ui, dialog_options, dialog_entry_count, &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == FRACTUS_APP_PLASMA_CIRCULAR_CANCEL) {
            *view = FRACTUS_APP_VIEW_PLASMA_MENU;
        } else if (selected_menu == FRACTUS_APP_PLASMA_CIRCULAR_DRAW) {
            *params = *pending;
            *view = FRACTUS_APP_VIEW_PLASMA_CIRCULAR;
        } else if (selected_menu == FRACTUS_APP_PLASMA_CIRCLES_DEC) {
            pending->circle_count = fractus_app_clamp_i32(pending->circle_count - 25, 10, 5000);
        } else if (selected_menu == FRACTUS_APP_PLASMA_CIRCLES_INC) {
            pending->circle_count = fractus_app_clamp_i32(pending->circle_count + 25, 10, 5000);
        } else if (selected_menu == FRACTUS_APP_PLASMA_RADIUS_DEC) {
            pending->max_radius = fractus_app_clamp_i32(pending->max_radius - 5, 1, 100);
        } else if (selected_menu == FRACTUS_APP_PLASMA_RADIUS_INC) {
            pending->max_radius = fractus_app_clamp_i32(pending->max_radius + 5, 1, 100);
        }
    }

    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_app_render_plasma_circular(
    const fractus_platform_context *platform,
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const fractus_plasma_circular_params *params,
    int *save_next_graphic,
    char *error_message,
    size_t error_message_size)
{
    if (platform == NULL || framebuffer == NULL || fonts == NULL || params == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_fractal_render_plasma_circular(framebuffer, params) != FRACTUS_STATUS_OK ||
        fractus_app_save_next_graphic_if_requested(platform, framebuffer, save_next_graphic, error_message, error_message_size) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return fractus_app_draw_drawing_footer(framebuffer, fonts, "ESC o boton derecho: menu - G: grabar dibujo");
}

static size_t fractus_app_build_simple_config_entries(
    fractus_app_menu_entry *entries,
    size_t capacity)
{
    static const fractus_app_menu_entry controls[] = {
        {FRACTUS_APP_RECT(385, 250, 425, 270), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(430, 250, 470, 270), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(190, 300, 310, 320), 8u, 0u, "Guardar"},
        {FRACTUS_APP_RECT(330, 300, 450, 320), 0u, 15u, "Cancelar"}
    };

    return fractus_app_copy_control_entries(
        entries,
        capacity,
        controls,
        FRACTUS_APP_ARRAY_COUNT(controls));
}

static fractus_status fractus_app_run_iterations_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_legacy_config *legacy_config,
    fractus_legacy_config *config_draft,
    fractus_mandelbrot_params *mandelbrot_params,
    fractus_julia_params *julia_params,
    fractus_biomorph_params *biomorph_params,
    const char *cfg_path,
    fractus_ui_numeric_field *iterations_field,
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
        mandelbrot_params == NULL || julia_params == NULL || biomorph_params == NULL ||
        iterations_field == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    dialog_entry_count = fractus_app_build_simple_config_entries(dialog_entries, FRACTUS_APP_ARRAY_COUNT(dialog_entries));
    fractus_app_build_options_from_entries(dialog_options, dialog_entries, dialog_entry_count);

    /* 1. Contenedor exterior. */
    if (fractus_app_render_main_menu(framebuffer, fonts, -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_window(framebuffer, 150, 150, 489, 329) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, 320, 154, 15u, "Numero maximo de iteraciones") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 2. Textos y controles. */
    if (fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, 320, 210, 0u, "Valor conservado en fractus.cfg. Rango: 16 a 1024.") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 175, 255, 0u, "Iteraciones maximas") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, iterations_field) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_button_list(
            framebuffer,
            fonts,
            dialog_entries,
            dialog_entry_count,
            fractus_ui_active_menu_index(ui, dialog_options, dialog_entry_count)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado en el campo editable. */
    {
        int edit_accepted = 0;
        int edit_cancelled = 0;
        int was_editing = iterations_field->editing;

        if (fractus_ui_numeric_field_handle_input(iterations_field, ui, fonts, &edit_accepted, &edit_cancelled) != FRACTUS_STATUS_OK) {
            fractus_app_log("runtime: iterations inline edit failed");
            return FRACTUS_STATUS_ERROR;
        }

        if (edit_accepted) {
            int32_t edited_value;

            if (fractus_ui_numeric_field_get_int(iterations_field, &edited_value) == FRACTUS_STATUS_OK) {
                config_draft->iterations = (int16_t)edited_value;
            }
            (void)fractus_ui_numeric_field_init_int(iterations_field, (fractus_rect_i32){305, 250, 73, 22}, (int32_t)config_draft->iterations, 16, 1024);
        } else if (edit_cancelled) {
            (void)fractus_ui_numeric_field_init_int(iterations_field, (fractus_rect_i32){305, 250, 73, 22}, (int32_t)config_draft->iterations, 16, 1024);
        }

        if (was_editing || iterations_field->editing) {
            skip_mouse_input = 1;
        }
    }

    /* 4. Raton y acciones de botones. */
    if (!skip_mouse_input && fractus_ui_menu(ui, dialog_options, dialog_entry_count, &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == FRACTUS_APP_SIMPLE_CONFIG_CANCEL) {
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (selected_menu == FRACTUS_APP_SIMPLE_CONFIG_ACCEPT) {
            *legacy_config = *config_draft;
            fractus_app_capture_palette_to_config(framebuffer, legacy_config);
            fractus_app_apply_legacy_numeric_config(legacy_config, mandelbrot_params, julia_params, biomorph_params);
            if (cfg_path != NULL && cfg_path[0] != '\0' &&
                fractus_app_save_legacy_config(cfg_path, framebuffer, legacy_config) != FRACTUS_STATUS_OK) {
                fractus_app_log("runtime: saving fractus.cfg failed");
            }
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (selected_menu == FRACTUS_APP_SIMPLE_CONFIG_DEC) {
            config_draft->iterations = (int16_t)fractus_app_clamp_i32((int32_t)config_draft->iterations - 16, 16, 1024);
            (void)fractus_ui_numeric_field_init_int(iterations_field, (fractus_rect_i32){305, 250, 73, 22}, (int32_t)config_draft->iterations, 16, 1024);
        } else if (selected_menu == FRACTUS_APP_SIMPLE_CONFIG_INC) {
            config_draft->iterations = (int16_t)fractus_app_clamp_i32((int32_t)config_draft->iterations + 16, 16, 1024);
            (void)fractus_ui_numeric_field_init_int(iterations_field, (fractus_rect_i32){305, 250, 73, 22}, (int32_t)config_draft->iterations, 16, 1024);
        }
    }

    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_app_run_escape_radius_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_legacy_config *legacy_config,
    fractus_legacy_config *config_draft,
    fractus_mandelbrot_params *mandelbrot_params,
    fractus_julia_params *julia_params,
    fractus_biomorph_params *biomorph_params,
    const char *cfg_path,
    fractus_ui_numeric_field *escape_radius_field,
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
        mandelbrot_params == NULL || julia_params == NULL || biomorph_params == NULL ||
        escape_radius_field == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    dialog_entry_count = fractus_app_build_simple_config_entries(dialog_entries, FRACTUS_APP_ARRAY_COUNT(dialog_entries));
    fractus_app_build_options_from_entries(dialog_options, dialog_entries, dialog_entry_count);

    /* 1. Contenedor exterior. */
    if (fractus_app_render_main_menu(framebuffer, fonts, -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_window(framebuffer, 150, 150, 489, 329) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, 320, 154, 15u, "Radio de escape para Mandelbrot y Julia") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 2. Textos y controles. */
    if (fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, 320, 210, 0u, "Valor conservado en fractus.cfg. Rango: 4 a 1000.") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 175, 255, 0u, "Radio al cuadrado") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, escape_radius_field) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_button_list(
            framebuffer,
            fonts,
            dialog_entries,
            dialog_entry_count,
            fractus_ui_active_menu_index(ui, dialog_options, dialog_entry_count)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado en el campo editable. */
    {
        int edit_accepted = 0;
        int edit_cancelled = 0;
        int was_editing = escape_radius_field->editing;

        if (fractus_ui_numeric_field_handle_input(escape_radius_field, ui, fonts, &edit_accepted, &edit_cancelled) != FRACTUS_STATUS_OK) {
            fractus_app_log("runtime: escape radius inline edit failed");
            return FRACTUS_STATUS_ERROR;
        }

        if (edit_accepted) {
            int32_t edited_value;

            if (fractus_ui_numeric_field_get_int(escape_radius_field, &edited_value) == FRACTUS_STATUS_OK) {
                config_draft->escape_radius_squared = (int16_t)edited_value;
            }
            (void)fractus_ui_numeric_field_init_int(escape_radius_field, (fractus_rect_i32){305, 250, 73, 22}, (int32_t)config_draft->escape_radius_squared, 4, 1000);
        } else if (edit_cancelled) {
            (void)fractus_ui_numeric_field_init_int(escape_radius_field, (fractus_rect_i32){305, 250, 73, 22}, (int32_t)config_draft->escape_radius_squared, 4, 1000);
        }

        if (was_editing || escape_radius_field->editing) {
            skip_mouse_input = 1;
        }
    }

    /* 4. Raton y acciones de botones. */
    if (!skip_mouse_input && fractus_ui_menu(ui, dialog_options, dialog_entry_count, &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == FRACTUS_APP_SIMPLE_CONFIG_CANCEL) {
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (selected_menu == FRACTUS_APP_SIMPLE_CONFIG_ACCEPT) {
            *legacy_config = *config_draft;
            fractus_app_capture_palette_to_config(framebuffer, legacy_config);
            fractus_app_apply_legacy_numeric_config(legacy_config, mandelbrot_params, julia_params, biomorph_params);
            if (cfg_path != NULL && cfg_path[0] != '\0' &&
                fractus_app_save_legacy_config(cfg_path, framebuffer, legacy_config) != FRACTUS_STATUS_OK) {
                fractus_app_log("runtime: saving fractus.cfg failed");
            }
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (selected_menu == FRACTUS_APP_SIMPLE_CONFIG_DEC) {
            config_draft->escape_radius_squared = (int16_t)fractus_app_clamp_i32((int32_t)config_draft->escape_radius_squared - 4, 4, 1000);
            (void)fractus_ui_numeric_field_init_int(escape_radius_field, (fractus_rect_i32){305, 250, 73, 22}, (int32_t)config_draft->escape_radius_squared, 4, 1000);
        } else if (selected_menu == FRACTUS_APP_SIMPLE_CONFIG_INC) {
            config_draft->escape_radius_squared = (int16_t)fractus_app_clamp_i32((int32_t)config_draft->escape_radius_squared + 4, 4, 1000);
            (void)fractus_ui_numeric_field_init_int(escape_radius_field, (fractus_rect_i32){305, 250, 73, 22}, (int32_t)config_draft->escape_radius_squared, 4, 1000);
        }
    }

    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_app_run_biomorph_radius_config_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_legacy_config *legacy_config,
    fractus_legacy_config *config_draft,
    fractus_mandelbrot_params *mandelbrot_params,
    fractus_julia_params *julia_params,
    fractus_biomorph_params *biomorph_params,
    const char *cfg_path,
    fractus_ui_numeric_field *biomorph_radius_field,
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
        mandelbrot_params == NULL || julia_params == NULL || biomorph_params == NULL ||
        biomorph_radius_field == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    dialog_entry_count = fractus_app_build_simple_config_entries(dialog_entries, FRACTUS_APP_ARRAY_COUNT(dialog_entries));
    fractus_app_build_options_from_entries(dialog_options, dialog_entries, dialog_entry_count);

    /* 1. Contenedor exterior. */
    if (fractus_app_render_main_menu(framebuffer, fonts, -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_window(framebuffer, 150, 150, 489, 329) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, 320, 154, 15u, "Radio de escape para biomorfos") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 2. Textos y controles. */
    if (fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, 320, 210, 0u, "Valor conservado en fractus.cfg. Rango: 4 a 1000.") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 175, 255, 0u, "Radio al cuadrado") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, biomorph_radius_field) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_button_list(
            framebuffer,
            fonts,
            dialog_entries,
            dialog_entry_count,
            fractus_ui_active_menu_index(ui, dialog_options, dialog_entry_count)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado en el campo editable. */
    {
        int edit_accepted = 0;
        int edit_cancelled = 0;
        int was_editing = biomorph_radius_field->editing;

        if (fractus_ui_numeric_field_handle_input(biomorph_radius_field, ui, fonts, &edit_accepted, &edit_cancelled) != FRACTUS_STATUS_OK) {
            fractus_app_log("runtime: biomorph radius inline edit failed");
            return FRACTUS_STATUS_ERROR;
        }

        if (edit_accepted) {
            int32_t edited_value;

            if (fractus_ui_numeric_field_get_int(biomorph_radius_field, &edited_value) == FRACTUS_STATUS_OK) {
                config_draft->biomorph_escape_radius_squared = (int16_t)edited_value;
            }
            (void)fractus_ui_numeric_field_init_int(biomorph_radius_field, (fractus_rect_i32){305, 250, 73, 22}, (int32_t)config_draft->biomorph_escape_radius_squared, 4, 1000);
        } else if (edit_cancelled) {
            (void)fractus_ui_numeric_field_init_int(biomorph_radius_field, (fractus_rect_i32){305, 250, 73, 22}, (int32_t)config_draft->biomorph_escape_radius_squared, 4, 1000);
        }

        if (was_editing || biomorph_radius_field->editing) {
            skip_mouse_input = 1;
        }
    }

    /* 4. Raton y acciones de botones. */
    if (!skip_mouse_input && fractus_ui_menu(ui, dialog_options, dialog_entry_count, &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == FRACTUS_APP_SIMPLE_CONFIG_CANCEL) {
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (selected_menu == FRACTUS_APP_SIMPLE_CONFIG_ACCEPT) {
            *legacy_config = *config_draft;
            fractus_app_capture_palette_to_config(framebuffer, legacy_config);
            fractus_app_apply_legacy_numeric_config(legacy_config, mandelbrot_params, julia_params, biomorph_params);
            if (cfg_path != NULL && cfg_path[0] != '\0' &&
                fractus_app_save_legacy_config(cfg_path, framebuffer, legacy_config) != FRACTUS_STATUS_OK) {
                fractus_app_log("runtime: saving fractus.cfg failed");
            }
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (selected_menu == FRACTUS_APP_SIMPLE_CONFIG_DEC) {
            config_draft->biomorph_escape_radius_squared = (int16_t)fractus_app_clamp_i32((int32_t)config_draft->biomorph_escape_radius_squared - 4, 4, 1000);
            (void)fractus_ui_numeric_field_init_int(biomorph_radius_field, (fractus_rect_i32){305, 250, 73, 22}, (int32_t)config_draft->biomorph_escape_radius_squared, 4, 1000);
        } else if (selected_menu == FRACTUS_APP_SIMPLE_CONFIG_INC) {
            config_draft->biomorph_escape_radius_squared = (int16_t)fractus_app_clamp_i32((int32_t)config_draft->biomorph_escape_radius_squared + 4, 4, 1000);
            (void)fractus_ui_numeric_field_init_int(biomorph_radius_field, (fractus_rect_i32){305, 250, 73, 22}, (int32_t)config_draft->biomorph_escape_radius_squared, 4, 1000);
        }
    }

    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_app_run_video_config_view(
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
        {FRACTUS_APP_RECT(175, 135, 460, 150), 8u, 0u, "Ventana 640x480"},
        {FRACTUS_APP_RECT(175, 155, 460, 170), 8u, 0u, "Ventana 800x600"},
        {FRACTUS_APP_RECT(175, 175, 460, 190), 8u, 0u, "Ventana 1024x768"},
        {FRACTUS_APP_RECT(175, 195, 460, 210), 8u, 0u, "Ventana 1280x960 (Quad VGA)"},
        {FRACTUS_APP_RECT(175, 215, 460, 230), 8u, 0u, "Pantalla completa 1280x720 (HD)"},
        {FRACTUS_APP_RECT(175, 235, 460, 250), 8u, 0u, "Pantalla completa 1920x1080 (Full HD)"},
        {FRACTUS_APP_RECT(175, 255, 460, 270), 8u, 0u, "Pantalla completa 2560x1440 (Quad HD)"},
        {FRACTUS_APP_RECT(175, 275, 460, 290), 8u, 0u, "Pantalla completa 3840x2160 (4K / Quad Full HD)"},
        {FRACTUS_APP_RECT(210, 325, 310, 345), 8u, 0u, "Guardar"},
        {FRACTUS_APP_RECT(330, 325, 430, 345), 0u, 15u, "Cancelar"}
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
        fractus_ui_draw_window(framebuffer, 165, 92, 474, 355) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, 320, 96, 15u, "Resolucion de los dibujos") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 2. Textos y controles. */
    if (fractus_ui_draw_group_box(framebuffer, fonts, 170, 122, 469, 300, 8u, 0u, "Seleccione la resolucion") != FRACTUS_STATUS_OK ||
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
            306,
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

static size_t fractus_app_build_palette_load_entries(
    fractus_app_menu_entry *entries,
    size_t capacity,
    const fractus_app_palette_file *files,
    size_t file_count,
    size_t page,
    size_t *visible_file_count)
{
    size_t i;
    size_t first_file;
    size_t visible_count;
    int has_previous;
    int has_next;
    int32_t y;

    if (entries == NULL || files == NULL || visible_file_count == NULL || capacity < FRACTUS_APP_PALETTE_FILE_PAGE_SIZE + 3u) {
        return 0u;
    }

    first_file = page * FRACTUS_APP_PALETTE_FILE_PAGE_SIZE;
    if (first_file > file_count) {
        first_file = file_count;
    }

    visible_count = file_count - first_file;
    if (visible_count > FRACTUS_APP_PALETTE_FILE_PAGE_SIZE) {
        visible_count = FRACTUS_APP_PALETTE_FILE_PAGE_SIZE;
    }

    y = 168;
    for (i = 0u; i < visible_count; ++i) {
        fractus_app_set_button(&entries[i], 185, y, 455, y + 20, 8u, 0u, files[first_file + i].label);
        y += 24;
    }

    if (visible_count == 0u) {
        y = 204;
    }

    has_previous = page > 0u;
    has_next = first_file + visible_count < file_count;
    fractus_app_set_button(&entries[visible_count], 185, y + 10, 225, y + 30, 8u, has_previous ? 0u : 7u, "<");
    fractus_app_set_button(&entries[visible_count + 1u], 270, y + 10, 370, y + 30, 0u, 15u, "Cancelar");
    fractus_app_set_button(&entries[visible_count + 2u], 415, y + 10, 455, y + 30, 8u, has_next ? 0u : 7u, ">");
    *visible_file_count = visible_count;
    return visible_count + 3u;
}

static size_t fractus_app_build_graphic_load_entries(
    fractus_app_menu_entry *entries,
    size_t capacity,
    const fractus_app_graphic_file *files,
    size_t file_count,
    size_t page,
    size_t *visible_file_count)
{
    size_t i;
    size_t first_file;
    size_t visible_count;
    int has_previous;
    int has_next;
    int32_t y;

    if (entries == NULL || files == NULL || visible_file_count == NULL || capacity < FRACTUS_APP_GRAPHIC_FILE_PAGE_SIZE + 3u) {
        return 0u;
    }

    first_file = page * FRACTUS_APP_GRAPHIC_FILE_PAGE_SIZE;
    if (first_file > file_count) {
        first_file = file_count;
    }

    visible_count = file_count - first_file;
    if (visible_count > FRACTUS_APP_GRAPHIC_FILE_PAGE_SIZE) {
        visible_count = FRACTUS_APP_GRAPHIC_FILE_PAGE_SIZE;
    }

    y = 168;
    for (i = 0u; i < visible_count; ++i) {
        fractus_app_set_button(&entries[i], 185, y, 455, y + 20, 8u, 0u, files[first_file + i].label);
        y += 24;
    }

    if (visible_count == 0u) {
        y = 204;
    }

    has_previous = page > 0u;
    has_next = first_file + visible_count < file_count;
    fractus_app_set_button(&entries[visible_count], 185, y + 10, 225, y + 30, 8u, has_previous ? 0u : 7u, "<");
    fractus_app_set_button(&entries[visible_count + 1u], 270, y + 10, 370, y + 30, 0u, 15u, "Cancelar");
    fractus_app_set_button(&entries[visible_count + 2u], 415, y + 10, 455, y + 30, 8u, has_next ? 0u : 7u, ">");
    *visible_file_count = visible_count;
    return visible_count + 3u;
}

static fractus_status fractus_app_render_file_load(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const char *title,
    const char *help,
    const char *empty_message,
    const fractus_app_menu_entry *entries,
    size_t entry_count,
    size_t file_count,
    int active_index)
{
    int32_t window_bottom;
    int32_t message_y;

    if (framebuffer == NULL || fonts == NULL || title == NULL || help == NULL || empty_message == NULL || entries == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    window_bottom = 212 + (int32_t)file_count * 24;
    if (window_bottom < 254) {
        window_bottom = 254;
    }

    message_y = 146;
    if (fractus_app_render_main_menu(framebuffer, fonts, -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_window(framebuffer, 170, 120, 469, window_bottom) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, 320, 124, 15u, title) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, 320, message_y, 0u, help) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_button_list(framebuffer, fonts, entries, entry_count, active_index) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    if (file_count == 0u) {
        return fractus_ui_draw_text_centered(
            framebuffer,
            fonts,
            FRACTUS_FONT_SMALL,
            320,
            178,
            0u,
            empty_message);
    }

    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_app_render_palette_load(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const char *title,
    const char *help,
    const fractus_app_menu_entry *entries,
    size_t entry_count,
    size_t file_count,
    int active_index)
{
    return fractus_app_render_file_load(
        framebuffer,
        fonts,
        title,
        help,
        "No hay ficheros .drsp disponibles.",
        entries,
        entry_count,
        file_count,
        active_index);
}

static fractus_status fractus_app_render_graphic_load(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const char *title,
    const char *help,
    const fractus_app_menu_entry *entries,
    size_t entry_count,
    size_t file_count,
    int active_index)
{
    return fractus_app_render_file_load(
        framebuffer,
        fonts,
        title,
        help,
        "No hay ficheros .drsg disponibles.",
        entries,
        entry_count,
        file_count,
        active_index);
}

static fractus_status fractus_app_run_load_graphic_view(
    fractus_framebuffer *ui_framebuffer,
    fractus_framebuffer *drawing_framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    const fractus_app_graphic_file *graphic_files,
    size_t graphic_file_count,
    size_t *graphic_file_page,
    fractus_legacy_config *legacy_config,
    const char *cfg_path,
    fractus_app_view *view)
{
    fractus_app_menu_entry dialog_entries[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    fractus_ui_menu_option dialog_options[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    size_t dialog_entry_count;
    size_t visible_file_count;
    size_t first_file;
    int selected_menu = -1;
    int cancelled = 0;

    if (ui_framebuffer == NULL || drawing_framebuffer == NULL || fonts == NULL || ui == NULL ||
        graphic_files == NULL || graphic_file_page == NULL || legacy_config == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (graphic_file_count == 0u) {
        *graphic_file_page = 0u;
    } else if (*graphic_file_page * FRACTUS_APP_GRAPHIC_FILE_PAGE_SIZE >= graphic_file_count) {
        *graphic_file_page = (graphic_file_count - 1u) / FRACTUS_APP_GRAPHIC_FILE_PAGE_SIZE;
    }

    dialog_entry_count = fractus_app_build_graphic_load_entries(
        dialog_entries,
        FRACTUS_APP_ARRAY_COUNT(dialog_entries),
        graphic_files,
        graphic_file_count,
        *graphic_file_page,
        &visible_file_count);
    fractus_app_build_options_from_entries(dialog_options, dialog_entries, dialog_entry_count);
    first_file = *graphic_file_page * FRACTUS_APP_GRAPHIC_FILE_PAGE_SIZE;

    /* 1. Contenedor exterior. */
    /* 2. Textos y controles. */
    if (fractus_app_render_graphic_load(
            ui_framebuffer,
            fonts,
            "Cargar un dibujo",
            "Seleccione un fichero .drsg de la carpeta dibujos.",
            dialog_entries,
            dialog_entry_count,
            visible_file_count,
            fractus_ui_active_menu_index(ui, dialog_options, dialog_entry_count)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones de botones. */
    if (fractus_ui_menu(ui, dialog_options, dialog_entry_count, &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == (int)visible_file_count + 1) {
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (selected_menu == (int)visible_file_count) {
            if (*graphic_file_page > 0u) {
                --(*graphic_file_page);
            }
        } else if (selected_menu == (int)visible_file_count + 2) {
            if (first_file + visible_file_count < graphic_file_count) {
                ++(*graphic_file_page);
            }
        } else if (selected_menu >= 0 && (size_t)selected_menu < visible_file_count) {
            if (fractus_app_load_graphic_into_state(
                    graphic_files[first_file + (size_t)selected_menu].path,
                    drawing_framebuffer,
                    legacy_config,
                    cfg_path) != FRACTUS_STATUS_OK ||
                fractus_app_sync_framebuffer_palette(ui_framebuffer, drawing_framebuffer) != FRACTUS_STATUS_OK) {
                fractus_app_log("runtime: loading selected graphic failed");
                *view = FRACTUS_APP_VIEW_MAIN_MENU;
            } else {
                *view = FRACTUS_APP_VIEW_GRAPHIC;
            }
        }
    }

    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_app_run_load_palette_view(
    fractus_framebuffer *ui_framebuffer,
    fractus_framebuffer *drawing_framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    const fractus_app_palette_file *palette_files,
    size_t palette_file_count,
    size_t *palette_file_page,
    fractus_legacy_config *legacy_config,
    const char *cfg_path,
    fractus_app_view *view)
{
    fractus_app_menu_entry dialog_entries[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    fractus_ui_menu_option dialog_options[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    size_t dialog_entry_count;
    size_t visible_file_count;
    size_t first_file;
    int selected_menu = -1;
    int cancelled = 0;

    if (ui_framebuffer == NULL || drawing_framebuffer == NULL || fonts == NULL || ui == NULL ||
        palette_files == NULL || palette_file_page == NULL || legacy_config == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (palette_file_count == 0u) {
        *palette_file_page = 0u;
    } else if (*palette_file_page * FRACTUS_APP_PALETTE_FILE_PAGE_SIZE >= palette_file_count) {
        *palette_file_page = (palette_file_count - 1u) / FRACTUS_APP_PALETTE_FILE_PAGE_SIZE;
    }

    dialog_entry_count = fractus_app_build_palette_load_entries(
        dialog_entries,
        FRACTUS_APP_ARRAY_COUNT(dialog_entries),
        palette_files,
        palette_file_count,
        *palette_file_page,
        &visible_file_count);
    fractus_app_build_options_from_entries(dialog_options, dialog_entries, dialog_entry_count);
    first_file = *palette_file_page * FRACTUS_APP_PALETTE_FILE_PAGE_SIZE;

    /* 1. Contenedor exterior. */
    /* 2. Textos y controles. */
    if (fractus_app_render_palette_load(
            ui_framebuffer,
            fonts,
            "Cargar una paleta",
            "Seleccione un fichero .drsp de la carpeta paletas.",
            dialog_entries,
            dialog_entry_count,
            visible_file_count,
            fractus_ui_active_menu_index(ui, dialog_options, dialog_entry_count)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones de botones. */
    if (fractus_ui_menu(ui, dialog_options, dialog_entry_count, &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == (int)visible_file_count + 1) {
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (selected_menu == (int)visible_file_count) {
            if (*palette_file_page > 0u) {
                --(*palette_file_page);
            }
        } else if (selected_menu == (int)visible_file_count + 2) {
            if (first_file + visible_file_count < palette_file_count) {
                ++(*palette_file_page);
            }
        } else if (selected_menu >= 0 && (size_t)selected_menu < visible_file_count) {
            if (fractus_app_load_palette_into_state(
                    palette_files[first_file + (size_t)selected_menu].path,
                    ui_framebuffer,
                    legacy_config,
                    cfg_path) != FRACTUS_STATUS_OK ||
                fractus_app_sync_framebuffer_palette(drawing_framebuffer, ui_framebuffer) != FRACTUS_STATUS_OK) {
                fractus_app_log("runtime: loading selected palette failed");
            }

            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        }
    }

    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_app_run_load_graphic_palette_view(
    fractus_framebuffer *ui_framebuffer,
    fractus_framebuffer *drawing_framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    const fractus_app_graphic_file *graphic_files,
    size_t graphic_file_count,
    size_t *graphic_file_page,
    fractus_legacy_config *legacy_config,
    const char *cfg_path,
    fractus_app_view *view)
{
    fractus_app_menu_entry dialog_entries[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    fractus_ui_menu_option dialog_options[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    size_t dialog_entry_count;
    size_t visible_file_count;
    size_t first_file;
    int selected_menu = -1;
    int cancelled = 0;

    if (ui_framebuffer == NULL || drawing_framebuffer == NULL || fonts == NULL || ui == NULL ||
        graphic_files == NULL || graphic_file_page == NULL || legacy_config == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (graphic_file_count == 0u) {
        *graphic_file_page = 0u;
    } else if (*graphic_file_page * FRACTUS_APP_GRAPHIC_FILE_PAGE_SIZE >= graphic_file_count) {
        *graphic_file_page = (graphic_file_count - 1u) / FRACTUS_APP_GRAPHIC_FILE_PAGE_SIZE;
    }

    dialog_entry_count = fractus_app_build_graphic_load_entries(
        dialog_entries,
        FRACTUS_APP_ARRAY_COUNT(dialog_entries),
        graphic_files,
        graphic_file_count,
        *graphic_file_page,
        &visible_file_count);
    fractus_app_build_options_from_entries(dialog_options, dialog_entries, dialog_entry_count);
    first_file = *graphic_file_page * FRACTUS_APP_GRAPHIC_FILE_PAGE_SIZE;

    /* 1. Contenedor exterior. */
    /* 2. Textos y controles. */
    if (fractus_app_render_graphic_load(
            ui_framebuffer,
            fonts,
            "Cargar la paleta de un dibujo",
            "Seleccione un dibujo .drsg para usar su paleta.",
            dialog_entries,
            dialog_entry_count,
            visible_file_count,
            fractus_ui_active_menu_index(ui, dialog_options, dialog_entry_count)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones de botones. */
    if (fractus_ui_menu(ui, dialog_options, dialog_entry_count, &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == (int)visible_file_count + 1) {
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (selected_menu == (int)visible_file_count) {
            if (*graphic_file_page > 0u) {
                --(*graphic_file_page);
            }
        } else if (selected_menu == (int)visible_file_count + 2) {
            if (first_file + visible_file_count < graphic_file_count) {
                ++(*graphic_file_page);
            }
        } else if (selected_menu >= 0 && (size_t)selected_menu < visible_file_count) {
            if (fractus_app_load_graphic_palette_into_state(
                    graphic_files[first_file + (size_t)selected_menu].path,
                    ui_framebuffer,
                    legacy_config,
                    cfg_path) != FRACTUS_STATUS_OK ||
                fractus_app_sync_framebuffer_palette(drawing_framebuffer, ui_framebuffer) != FRACTUS_STATUS_OK) {
                fractus_app_log("runtime: loading palette from selected graphic failed");
            }

            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        }
    }

    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_app_run_change_graphic_palette_graphic_view(
    fractus_platform_context *platform,
    fractus_framebuffer *ui_framebuffer,
    fractus_framebuffer *drawing_framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    const fractus_app_graphic_file *graphic_files,
    size_t graphic_file_count,
    size_t *graphic_file_page,
    fractus_app_palette_file *palette_files,
    size_t *palette_file_count,
    size_t *palette_file_page,
    char *selected_graphic_path,
    size_t selected_graphic_path_size,
    fractus_legacy_config *legacy_config,
    const char *cfg_path,
    fractus_app_view *view)
{
    fractus_app_menu_entry dialog_entries[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    fractus_ui_menu_option dialog_options[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    size_t dialog_entry_count;
    size_t visible_file_count;
    size_t first_file;
    int selected_menu = -1;
    int cancelled = 0;

    if (platform == NULL || ui_framebuffer == NULL || drawing_framebuffer == NULL ||
        fonts == NULL || ui == NULL || graphic_files == NULL || palette_files == NULL ||
        graphic_file_page == NULL || palette_file_count == NULL || palette_file_page == NULL || selected_graphic_path == NULL || legacy_config == NULL ||
        view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (graphic_file_count == 0u) {
        *graphic_file_page = 0u;
    } else if (*graphic_file_page * FRACTUS_APP_GRAPHIC_FILE_PAGE_SIZE >= graphic_file_count) {
        *graphic_file_page = (graphic_file_count - 1u) / FRACTUS_APP_GRAPHIC_FILE_PAGE_SIZE;
    }

    dialog_entry_count = fractus_app_build_graphic_load_entries(
        dialog_entries,
        FRACTUS_APP_ARRAY_COUNT(dialog_entries),
        graphic_files,
        graphic_file_count,
        *graphic_file_page,
        &visible_file_count);
    fractus_app_build_options_from_entries(dialog_options, dialog_entries, dialog_entry_count);
    first_file = *graphic_file_page * FRACTUS_APP_GRAPHIC_FILE_PAGE_SIZE;

    /* 1. Contenedor exterior. */
    /* 2. Textos y controles. */
    if (fractus_app_render_graphic_load(
            ui_framebuffer,
            fonts,
            "Cambiar la paleta de un dibujo",
            "Primero seleccione el dibujo .drsg.",
            dialog_entries,
            dialog_entry_count,
            visible_file_count,
            fractus_ui_active_menu_index(ui, dialog_options, dialog_entry_count)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones de botones. */
    if (fractus_ui_menu(ui, dialog_options, dialog_entry_count, &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == (int)visible_file_count + 1) {
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (selected_menu == (int)visible_file_count) {
            if (*graphic_file_page > 0u) {
                --(*graphic_file_page);
            }
        } else if (selected_menu == (int)visible_file_count + 2) {
            if (first_file + visible_file_count < graphic_file_count) {
                ++(*graphic_file_page);
            }
        } else if (selected_menu >= 0 && (size_t)selected_menu < visible_file_count) {
            if (fractus_formats_copy_path(
                    graphic_files[first_file + (size_t)selected_menu].path,
                    selected_graphic_path,
                    selected_graphic_path_size) != FRACTUS_STATUS_OK ||
                fractus_app_load_graphic_into_state(
                    selected_graphic_path,
                    drawing_framebuffer,
                    legacy_config,
                    cfg_path) != FRACTUS_STATUS_OK ||
                fractus_app_sync_framebuffer_palette(ui_framebuffer, drawing_framebuffer) != FRACTUS_STATUS_OK) {
                fractus_app_log("runtime: loading graphic before palette change failed");
                selected_graphic_path[0] = '\0';
                *view = FRACTUS_APP_VIEW_MAIN_MENU;
            } else {
                *palette_file_count = fractus_app_list_palette_files(platform, palette_files, FRACTUS_APP_PALETTE_FILE_CAPACITY);
                *palette_file_page = 0u;
                *view = FRACTUS_APP_VIEW_CHANGE_GRAPHIC_PALETTE_PALETTE;
            }
        }
    }

    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_app_run_change_graphic_palette_palette_view(
    fractus_platform_context *platform,
    fractus_framebuffer *ui_framebuffer,
    fractus_framebuffer *drawing_framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    const fractus_app_palette_file *palette_files,
    size_t palette_file_count,
    size_t *palette_file_page,
    char *selected_graphic_path,
    fractus_legacy_config *legacy_config,
    const char *cfg_path,
    char *error_message,
    size_t error_message_size,
    fractus_app_view *view)
{
    fractus_app_menu_entry dialog_entries[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    fractus_ui_menu_option dialog_options[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    size_t dialog_entry_count;
    size_t visible_file_count;
    size_t first_file;
    int selected_menu = -1;
    int cancelled = 0;

    if (platform == NULL || ui_framebuffer == NULL || drawing_framebuffer == NULL ||
        fonts == NULL || ui == NULL || palette_files == NULL || selected_graphic_path == NULL ||
        palette_file_page == NULL || legacy_config == NULL || error_message == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (palette_file_count == 0u) {
        *palette_file_page = 0u;
    } else if (*palette_file_page * FRACTUS_APP_PALETTE_FILE_PAGE_SIZE >= palette_file_count) {
        *palette_file_page = (palette_file_count - 1u) / FRACTUS_APP_PALETTE_FILE_PAGE_SIZE;
    }

    dialog_entry_count = fractus_app_build_palette_load_entries(
        dialog_entries,
        FRACTUS_APP_ARRAY_COUNT(dialog_entries),
        palette_files,
        palette_file_count,
        *palette_file_page,
        &visible_file_count);
    fractus_app_build_options_from_entries(dialog_options, dialog_entries, dialog_entry_count);
    first_file = *palette_file_page * FRACTUS_APP_PALETTE_FILE_PAGE_SIZE;

    /* 1. Contenedor exterior. */
    /* 2. Textos y controles. */
    if (fractus_app_render_palette_load(
            ui_framebuffer,
            fonts,
            "Cambiar la paleta de un dibujo",
            "Ahora seleccione la nueva paleta .drsp.",
            dialog_entries,
            dialog_entry_count,
            visible_file_count,
            fractus_ui_active_menu_index(ui, dialog_options, dialog_entry_count)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones de botones. */
    if (fractus_ui_menu(ui, dialog_options, dialog_entry_count, &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == (int)visible_file_count + 1) {
            selected_graphic_path[0] = '\0';
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (selected_menu == (int)visible_file_count) {
            if (*palette_file_page > 0u) {
                --(*palette_file_page);
            }
        } else if (selected_menu == (int)visible_file_count + 2) {
            if (first_file + visible_file_count < palette_file_count) {
                ++(*palette_file_page);
            }
        } else if (selected_menu >= 0 && (size_t)selected_menu < visible_file_count) {
            fractus_status change_status = fractus_app_change_graphic_palette_into_state(
                platform,
                selected_graphic_path,
                palette_files[first_file + (size_t)selected_menu].path,
                drawing_framebuffer,
                legacy_config,
                cfg_path);
            if (change_status != FRACTUS_STATUS_OK) {
                fractus_app_log("runtime: changing selected graphic palette failed");
                if (change_status == FRACTUS_STATUS_UNSUPPORTED) {
                    (void)fractus_app_set_message(
                        error_message,
                        error_message_size,
                        "No hay nombres libres de fractus0001.drsg a fractus9999.drsg.");
                    selected_graphic_path[0] = '\0';
                    *view = FRACTUS_APP_VIEW_ERROR;
                } else {
                    selected_graphic_path[0] = '\0';
                    *view = FRACTUS_APP_VIEW_MAIN_MENU;
                }
            } else {
                if (fractus_app_sync_framebuffer_palette(ui_framebuffer, drawing_framebuffer) != FRACTUS_STATUS_OK) {
                    return FRACTUS_STATUS_ERROR;
                }
                selected_graphic_path[0] = '\0';
                *view = FRACTUS_APP_VIEW_GRAPHIC;
            }
        }
    }

    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_app_draw_palette_grid(fractus_framebuffer *framebuffer)
{
    uint32_t color_index = 16u;
    int32_t row;
    int32_t column;

    if (framebuffer == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    for (row = 0; row < FRACTUS_APP_PALETTE_GRID_ROWS; ++row) {
        for (column = 0; column < FRACTUS_APP_PALETTE_GRID_COLUMNS; ++column) {
            fractus_rect_i32 cell = {
                FRACTUS_APP_PALETTE_GRID_X + column * FRACTUS_APP_PALETTE_CELL_STEP_X,
                FRACTUS_APP_PALETTE_GRID_Y + row * FRACTUS_APP_PALETTE_CELL_STEP_Y,
                FRACTUS_APP_PALETTE_CELL_WIDTH,
                FRACTUS_APP_PALETTE_CELL_HEIGHT};

            if (fractus_graphics_fill_rect(framebuffer, cell, (uint8_t)color_index) != FRACTUS_STATUS_OK ||
                fractus_graphics_rect(framebuffer, cell, 15u) != FRACTUS_STATUS_OK) {
                return FRACTUS_STATUS_ERROR;
            }

            ++color_index;
        }
    }

    return FRACTUS_STATUS_OK;
}

static int fractus_app_palette_index_at(fractus_point_i32 point, uint32_t *index)
{
    int32_t relative_x = point.x - FRACTUS_APP_PALETTE_GRID_X;
    int32_t relative_y = point.y - FRACTUS_APP_PALETTE_GRID_Y;
    int32_t column;
    int32_t row;
    int32_t cell_x;
    int32_t cell_y;

    if (index == NULL || relative_x < 0 || relative_y < 0) {
        return 0;
    }

    column = relative_x / FRACTUS_APP_PALETTE_CELL_STEP_X;
    row = relative_y / FRACTUS_APP_PALETTE_CELL_STEP_Y;
    cell_x = relative_x % FRACTUS_APP_PALETTE_CELL_STEP_X;
    cell_y = relative_y % FRACTUS_APP_PALETTE_CELL_STEP_Y;

    if (column < 0 ||
        column >= FRACTUS_APP_PALETTE_GRID_COLUMNS ||
        row < 0 ||
        row >= FRACTUS_APP_PALETTE_GRID_ROWS ||
        cell_x <= 0 ||
        cell_y <= 0 ||
        cell_x >= FRACTUS_APP_PALETTE_CELL_WIDTH ||
        cell_y >= FRACTUS_APP_PALETTE_CELL_HEIGHT ||
        cell_x == FRACTUS_APP_PALETTE_CELL_WIDTH - 1 ||
        cell_y == FRACTUS_APP_PALETTE_CELL_HEIGHT - 1) {
        return 0;
    }

    *index = 16u + (uint32_t)(row * FRACTUS_APP_PALETTE_GRID_COLUMNS + column);
    return 1;
}

static int fractus_app_palette_pick(
    const fractus_ui_context *ui,
    uint32_t *index,
    int *cancelled)
{
    if (cancelled != NULL) {
        *cancelled = 0;
    }

    if (ui == NULL || index == NULL) {
        return 0;
    }

    if ((ui->key_press_pending && ui->key_pressed == 27u) ||
        (ui->press_pending && ui->press_event.buttons.right)) {
        if (cancelled != NULL) {
            *cancelled = 1;
        }
        return 1;
    }

    if (ui->release_pending &&
        ui->release_event.buttons.left &&
        fractus_ui_point_in_rect(ui->release_event.position, (fractus_rect_i32)FRACTUS_APP_RECT(270, 422, 370, 442))) {
        if (cancelled != NULL) {
            *cancelled = 1;
        }
        return 1;
    }

    if (ui->release_pending &&
        ui->release_event.buttons.left &&
        fractus_app_palette_index_at(ui->release_event.position, index)) {
        return 1;
    }

    return 0;
}

static int fractus_app_palette_back_is_pressed(const fractus_ui_context *ui)
{
    return ui != NULL &&
           ui->buttons_down.left &&
           fractus_ui_point_in_rect(ui->pointer_position, (fractus_rect_i32)FRACTUS_APP_RECT(270, 422, 370, 442));
}

static int fractus_app_palette_back_was_clicked(const fractus_ui_context *ui)
{
    return ui != NULL &&
           ui->release_pending &&
           ui->release_event.buttons.left &&
           fractus_ui_point_in_rect(ui->release_event.position, (fractus_rect_i32)FRACTUS_APP_RECT(270, 422, 370, 442));
}

static fractus_status fractus_app_render_palette_screen(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const char *title,
    const char *line1,
    int back_pressed)
{
    const fractus_app_menu_entry back_button = {
        FRACTUS_APP_RECT(270, 422, 370, 442), 0u, 15u, "Volver"
    };

    if (framebuffer == NULL || fonts == NULL || title == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_ui_draw_window(framebuffer, 0, 0, 639, 454) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, 320, 6, 15u, title) != FRACTUS_STATUS_OK ||
        fractus_app_draw_palette_grid(framebuffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    if (line1 != NULL &&
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, 320, 405, 0u, line1) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return fractus_ui_draw_button(
        framebuffer,
        fonts,
        &back_button,
        back_pressed);
}

static fractus_status fractus_app_render_palette_view_current(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    int back_pressed)
{
    return fractus_app_render_palette_screen(
        framebuffer,
        fonts,
        "Paleta actual",
        "La paleta .drsp usa trios de bytes RGB con valores VGA 0-63 por canal, no 0-255.",
        back_pressed);
}

static fractus_status fractus_app_render_palette_color_edit(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    uint32_t palette_index,
    const fractus_ui_numeric_field *red_field,
    const fractus_ui_numeric_field *green_field,
    const fractus_ui_numeric_field *blue_field,
    const fractus_app_menu_entry *entries,
    size_t entry_count,
    int active_index)
{
    char buffer[48];
    fractus_rect_i32 preview = {364, 152, 72, 80};

    if (framebuffer == NULL || fonts == NULL || red_field == NULL || green_field == NULL ||
        blue_field == NULL || entries == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    snprintf(buffer, sizeof(buffer), "Indice de la paleta a modificar %u", (unsigned)palette_index);

    if (fractus_app_render_palette_screen(
            framebuffer,
            fonts,
            "Modificar un color de la paleta",
            "",
            0) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_window(framebuffer, 120, 112, 599, 335) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, 360, 116, 15u, "Modificar color") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_group_box(framebuffer, fonts, 125, 142, 594, 270, 8u, 0u, buffer) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, 320, 265, 0u, "Ajuste los canales RGB con valores VGA entre 0 y 63 y pulse Guardar.") != FRACTUS_STATUS_OK ||
        fractus_graphics_fill_rect(framebuffer, preview, (uint8_t)palette_index) != FRACTUS_STATUS_OK ||
        fractus_graphics_rect(framebuffer, preview, 15u) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 364, 245, 0u, "Previsualizacion") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    if (fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 135, 157, 0u, "Rojo") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, red_field) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    if (fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 135, 187, 0u, "Verde") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, green_field) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    if (fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 135, 217, 0u, "Azul") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, blue_field) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return fractus_ui_draw_button_list(framebuffer, fonts, entries, entry_count, active_index);
}

static fractus_status fractus_app_run_palette_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_app_view *view)
{
    if (framebuffer == NULL || fonts == NULL || ui == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    /* 1. Contenedor exterior. */
    /* 2. Textos y controles. */
    if (fractus_app_render_palette_view_current(
            framebuffer,
            fonts,
            fractus_app_palette_back_is_pressed(ui)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones de botones. */
    if (ui->release_pending &&
        ui->release_event.buttons.left &&
        fractus_ui_point_in_rect(ui->release_event.position, (fractus_rect_i32)FRACTUS_APP_RECT(270, 422, 370, 442))) {
        *view = FRACTUS_APP_VIEW_MAIN_MENU;
    }

    return FRACTUS_STATUS_OK;
}

static size_t fractus_app_build_palette_color_entries(
    fractus_app_menu_entry *entries,
    size_t capacity)
{
    static const fractus_app_menu_entry controls[] = {
        {FRACTUS_APP_RECT(249, 152, 289, 172), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(294, 152, 334, 172), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(249, 182, 289, 202), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(294, 182, 334, 202), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(249, 212, 289, 232), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(294, 212, 334, 232), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(210, 300, 310, 320), 8u, 0u, "Guardar"},
        {FRACTUS_APP_RECT(330, 300, 430, 320), 0u, 15u, "Cancelar"}
    };

    return fractus_app_copy_control_entries(
        entries,
        capacity,
        controls,
        FRACTUS_APP_ARRAY_COUNT(controls));
}

static fractus_status fractus_app_run_palette_edit_select_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    uint32_t *palette_selected_index,
    fractus_color_rgba8 *palette_original_color,
    fractus_color_rgba8 *palette_pending_color,
    fractus_ui_numeric_field *red_field,
    fractus_ui_numeric_field *green_field,
    fractus_ui_numeric_field *blue_field,
    fractus_app_view *view)
{
    uint32_t picked_index;
    int cancelled = 0;

    if (framebuffer == NULL || fonts == NULL || ui == NULL ||
        palette_selected_index == NULL || palette_original_color == NULL ||
        palette_pending_color == NULL || red_field == NULL || green_field == NULL ||
        blue_field == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    /* 1. Contenedor exterior. */
    /* 2. Textos y controles. */
    if (fractus_app_render_palette_screen(
            framebuffer,
            fonts,
            "Modificar un color de la paleta",
            "Seleccione el color que desea modificar.",
            fractus_app_palette_back_is_pressed(ui)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones. */
    if (fractus_app_palette_pick(ui, &picked_index, &cancelled)) {
        if (cancelled) {
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else {
            *palette_selected_index = picked_index;
            if (fractus_palette_get_entry(&framebuffer->palette, *palette_selected_index, palette_original_color) != FRACTUS_STATUS_OK) {
                fractus_app_log("runtime: reading selected palette color failed");
                *view = FRACTUS_APP_VIEW_MAIN_MENU;
            } else {
                *palette_pending_color = *palette_original_color;
                fractus_app_init_palette_color_fields(red_field, green_field, blue_field, *palette_pending_color);
                *view = FRACTUS_APP_VIEW_PALETTE_EDIT_COLOR;
            }
        }
    }

    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_app_run_palette_edit_color_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    const char *cfg_path,
    fractus_legacy_config *legacy_config,
    uint32_t palette_selected_index,
    fractus_color_rgba8 palette_original_color,
    fractus_color_rgba8 *palette_pending_color,
    fractus_ui_numeric_field *red_field,
    fractus_ui_numeric_field *green_field,
    fractus_ui_numeric_field *blue_field,
    fractus_app_view *view)
{
    fractus_app_menu_entry dialog_entries[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    fractus_ui_menu_option dialog_options[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    size_t dialog_entry_count;
    int32_t red;
    int32_t green;
    int32_t blue;
    int selected_menu = -1;
    int cancelled = 0;
    int skip_palette_color_menu = 0;

    if (framebuffer == NULL || fonts == NULL || ui == NULL || legacy_config == NULL ||
        palette_pending_color == NULL || red_field == NULL || green_field == NULL ||
        blue_field == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    dialog_entry_count = fractus_app_build_palette_color_entries(dialog_entries, FRACTUS_APP_ARRAY_COUNT(dialog_entries));
    fractus_app_build_options_from_entries(dialog_options, dialog_entries, dialog_entry_count);
    fractus_app_set_palette_entry(framebuffer, palette_selected_index, *palette_pending_color);

    /* 1. Contenedor exterior. */
    /* 2. Textos y controles. */
    if (fractus_app_render_palette_color_edit(
            framebuffer,
            fonts,
            palette_selected_index,
            red_field,
            green_field,
            blue_field,
            dialog_entries,
            dialog_entry_count,
            fractus_ui_active_menu_index(ui, dialog_options, dialog_entry_count)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado en los campos editables. */
    fractus_app_vga_channels_from_color(*palette_pending_color, &red, &green, &blue);
    {
        int edit_accepted = 0;
        int edit_cancelled = 0;
        int was_editing = red_field->editing;

        if (fractus_ui_numeric_field_handle_input(red_field, ui, fonts, &edit_accepted, &edit_cancelled) != FRACTUS_STATUS_OK) {
            fractus_app_log("runtime: palette red inline edit failed");
            return FRACTUS_STATUS_ERROR;
        }
        if (edit_accepted) {
            int32_t edited_value;
            if (fractus_ui_numeric_field_get_int(red_field, &edited_value) == FRACTUS_STATUS_OK) {
                red = edited_value;
                *palette_pending_color = fractus_app_vga_color(red, green, blue);
            }
            fractus_app_init_palette_color_fields(red_field, green_field, blue_field, *palette_pending_color);
        } else if (edit_cancelled) {
            fractus_app_init_palette_color_fields(red_field, green_field, blue_field, *palette_pending_color);
        }
        if (was_editing || red_field->editing) {
            skip_palette_color_menu = 1;
        }
    }

    fractus_app_vga_channels_from_color(*palette_pending_color, &red, &green, &blue);
    {
        int edit_accepted = 0;
        int edit_cancelled = 0;
        int was_editing = green_field->editing;

        if (fractus_ui_numeric_field_handle_input(green_field, ui, fonts, &edit_accepted, &edit_cancelled) != FRACTUS_STATUS_OK) {
            fractus_app_log("runtime: palette green inline edit failed");
            return FRACTUS_STATUS_ERROR;
        }
        if (edit_accepted) {
            int32_t edited_value;
            if (fractus_ui_numeric_field_get_int(green_field, &edited_value) == FRACTUS_STATUS_OK) {
                green = edited_value;
                *palette_pending_color = fractus_app_vga_color(red, green, blue);
            }
            fractus_app_init_palette_color_fields(red_field, green_field, blue_field, *palette_pending_color);
        } else if (edit_cancelled) {
            fractus_app_init_palette_color_fields(red_field, green_field, blue_field, *palette_pending_color);
        }
        if (was_editing || green_field->editing) {
            skip_palette_color_menu = 1;
        }
    }

    fractus_app_vga_channels_from_color(*palette_pending_color, &red, &green, &blue);
    {
        int edit_accepted = 0;
        int edit_cancelled = 0;
        int was_editing = blue_field->editing;

        if (fractus_ui_numeric_field_handle_input(blue_field, ui, fonts, &edit_accepted, &edit_cancelled) != FRACTUS_STATUS_OK) {
            fractus_app_log("runtime: palette blue inline edit failed");
            return FRACTUS_STATUS_ERROR;
        }
        if (edit_accepted) {
            int32_t edited_value;
            if (fractus_ui_numeric_field_get_int(blue_field, &edited_value) == FRACTUS_STATUS_OK) {
                blue = edited_value;
                *palette_pending_color = fractus_app_vga_color(red, green, blue);
            }
            fractus_app_init_palette_color_fields(red_field, green_field, blue_field, *palette_pending_color);
        } else if (edit_cancelled) {
            fractus_app_init_palette_color_fields(red_field, green_field, blue_field, *palette_pending_color);
        }
        if (was_editing || blue_field->editing) {
            skip_palette_color_menu = 1;
        }
    }

    /* 4. Raton y acciones de botones. */
    if (!skip_palette_color_menu &&
        fractus_ui_menu(ui, dialog_options, dialog_entry_count, &selected_menu, &cancelled)) {
        fractus_app_vga_channels_from_color(*palette_pending_color, &red, &green, &blue);
        if (cancelled || selected_menu == FRACTUS_APP_PALETTE_COLOR_CANCEL) {
            fractus_app_set_palette_entry(framebuffer, palette_selected_index, palette_original_color);
            *view = FRACTUS_APP_VIEW_PALETTE_EDIT_SELECT;
        } else if (selected_menu == FRACTUS_APP_PALETTE_COLOR_ACCEPT) {
            if (fractus_app_apply_palette_edit(
                    cfg_path,
                    framebuffer,
                    legacy_config,
                    palette_selected_index,
                    *palette_pending_color) != FRACTUS_STATUS_OK) {
                fractus_app_log("runtime: saving edited palette color failed");
            }
            *view = FRACTUS_APP_VIEW_PALETTE_EDIT_SELECT;
        } else if (selected_menu == FRACTUS_APP_PALETTE_RED_DEC) {
            *palette_pending_color = fractus_app_vga_color(red - 1, green, blue);
            fractus_app_init_palette_color_fields(red_field, green_field, blue_field, *palette_pending_color);
        } else if (selected_menu == FRACTUS_APP_PALETTE_RED_INC) {
            *palette_pending_color = fractus_app_vga_color(red + 1, green, blue);
            fractus_app_init_palette_color_fields(red_field, green_field, blue_field, *palette_pending_color);
        } else if (selected_menu == FRACTUS_APP_PALETTE_GREEN_DEC) {
            *palette_pending_color = fractus_app_vga_color(red, green - 1, blue);
            fractus_app_init_palette_color_fields(red_field, green_field, blue_field, *palette_pending_color);
        } else if (selected_menu == FRACTUS_APP_PALETTE_GREEN_INC) {
            *palette_pending_color = fractus_app_vga_color(red, green + 1, blue);
            fractus_app_init_palette_color_fields(red_field, green_field, blue_field, *palette_pending_color);
        } else if (selected_menu == FRACTUS_APP_PALETTE_BLUE_DEC) {
            *palette_pending_color = fractus_app_vga_color(red, green, blue - 1);
            fractus_app_init_palette_color_fields(red_field, green_field, blue_field, *palette_pending_color);
        } else if (selected_menu == FRACTUS_APP_PALETTE_BLUE_INC) {
            *palette_pending_color = fractus_app_vga_color(red, green, blue + 1);
            fractus_app_init_palette_color_fields(red_field, green_field, blue_field, *palette_pending_color);
        }
    }

    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_app_run_palette_copy_source_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_color_rgba8 *palette_copy_color,
    uint32_t *palette_copy_source_index,
    fractus_app_view *view)
{
    uint32_t picked_index;
    int cancelled = 0;

    if (framebuffer == NULL || fonts == NULL || ui == NULL ||
        palette_copy_color == NULL || palette_copy_source_index == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    /* 1. Contenedor exterior. */
    /* 2. Textos y controles. */
    if (fractus_app_render_palette_screen(
            framebuffer,
            fonts,
            "Copiar un color de la paleta",
            "Seleccione el color origen.",
            fractus_app_palette_back_is_pressed(ui)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones. */
    if (fractus_app_palette_pick(ui, &picked_index, &cancelled)) {
        if (cancelled) {
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (fractus_palette_get_entry(&framebuffer->palette, picked_index, palette_copy_color) == FRACTUS_STATUS_OK) {
            *palette_copy_source_index = picked_index;
            *view = FRACTUS_APP_VIEW_PALETTE_COPY_TARGETS;
        } else {
            fractus_app_log("runtime: reading source palette color failed");
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        }
    }

    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_app_run_palette_copy_targets_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    const char *cfg_path,
    fractus_legacy_config *legacy_config,
    uint32_t palette_copy_source_index,
    fractus_color_rgba8 palette_copy_color,
    fractus_app_view *view)
{
    uint32_t picked_index;
    int cancelled = 0;
    char source_text[128];

    if (framebuffer == NULL || fonts == NULL || ui == NULL || legacy_config == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    snprintf(source_text, sizeof(source_text), "Origen: indice %u. Seleccione destinos con el boton izquierdo. Pulse boton derecho para dejar de copiar.", (unsigned)palette_copy_source_index);

    /* 1. Contenedor exterior. */
    /* 2. Textos y controles. */
    if (fractus_app_render_palette_screen(
            framebuffer,
            fonts,
            "Copiar un color de la paleta",
            source_text,
            fractus_app_palette_back_is_pressed(ui)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones. */
    if (fractus_app_palette_pick(ui, &picked_index, &cancelled)) {
        if (cancelled) {
            if (fractus_app_persist_current_palette(cfg_path, framebuffer, legacy_config) != FRACTUS_STATUS_OK) {
                fractus_app_log("runtime: saving copied palette colors failed");
            }
            *view = fractus_app_palette_back_was_clicked(ui) ?
                FRACTUS_APP_VIEW_MAIN_MENU :
                FRACTUS_APP_VIEW_PALETTE_COPY_SOURCE;
        } else if (fractus_app_set_palette_entry(framebuffer, picked_index, palette_copy_color) != FRACTUS_STATUS_OK) {
            fractus_app_log("runtime: applying copied palette color failed");
        }
    }

    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_app_run_palette_gradient_first_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    uint32_t *palette_gradient_first_index,
    fractus_app_view *view)
{
    uint32_t picked_index;
    int cancelled = 0;

    if (framebuffer == NULL || fonts == NULL || ui == NULL ||
        palette_gradient_first_index == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    /* 1. Contenedor exterior. */
    /* 2. Textos y controles. */
    if (fractus_app_render_palette_screen(
            framebuffer,
            fonts,
            "Crear un gradiente",
            "Seleccione el primer color del gradiente.",
            fractus_app_palette_back_is_pressed(ui)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones. */
    if (fractus_app_palette_pick(ui, &picked_index, &cancelled)) {
        if (cancelled) {
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else {
            *palette_gradient_first_index = picked_index;
            *view = FRACTUS_APP_VIEW_PALETTE_GRADIENT_SECOND;
        }
    }

    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_app_run_palette_gradient_second_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    const char *cfg_path,
    fractus_legacy_config *legacy_config,
    uint32_t palette_gradient_first_index,
    fractus_app_view *view)
{
    uint32_t picked_index;
    int cancelled = 0;
    char first_text[96];

    if (framebuffer == NULL || fonts == NULL || ui == NULL || legacy_config == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    snprintf(first_text, sizeof(first_text), "Primer color: indice %u. Seleccione el segundo color del gradiente. Boton derecho para cancelar.", (unsigned)palette_gradient_first_index);

    /* 1. Contenedor exterior. */
    /* 2. Textos y controles. */
    if (fractus_app_render_palette_screen(
            framebuffer,
            fonts,
            "Crear un gradiente",
            first_text,
            fractus_app_palette_back_is_pressed(ui)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones. */
    if (fractus_app_palette_pick(ui, &picked_index, &cancelled)) {
        if (cancelled) {
            *view = fractus_app_palette_back_was_clicked(ui) ?
                FRACTUS_APP_VIEW_MAIN_MENU :
                FRACTUS_APP_VIEW_PALETTE_GRADIENT_FIRST;
        } else {
            if (fractus_app_apply_palette_gradient(
                    cfg_path,
                    framebuffer,
                    legacy_config,
                    palette_gradient_first_index,
                    picked_index) != FRACTUS_STATUS_OK) {
                fractus_app_log("runtime: saving palette gradient failed");
            }
            *view = FRACTUS_APP_VIEW_PALETTE_GRADIENT_FIRST;
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
        fractus_ui_draw_window(framebuffer, 150, 150, 489, 270) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, 320, 154, 15u, "Error") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, 320, 195, 0u, message) != FRACTUS_STATUS_OK ||
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
        {FRACTUS_APP_RECT(270, 235, 370, 255), 0u, 15u, "Aceptar"}
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
    fractus_julia_params julia_params;
    fractus_julia_params julia_pending;
    fractus_biomorph_params biomorph_params;
    fractus_biomorph_params biomorph_pending;
    fractus_plasma_params plasma_rectangular_params;
    fractus_plasma_params plasma_rectangular_pending;
    fractus_plasma_circular_params plasma_circular_params;
    fractus_plasma_circular_params plasma_circular_pending;
    fractus_app_mandelbrot_selection mandelbrot_selection;
    fractus_color_rgba8 palette_original_color;
    fractus_color_rgba8 palette_pending_color;
    fractus_color_rgba8 palette_copy_color;
    fractus_ui_numeric_field iterations_field;
    fractus_ui_numeric_field escape_radius_field;
    fractus_ui_numeric_field biomorph_radius_field;
    fractus_ui_numeric_field palette_red_field;
    fractus_ui_numeric_field palette_green_field;
    fractus_ui_numeric_field palette_blue_field;
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
    memset(&palette_red_field, 0, sizeof(palette_red_field));
    memset(&palette_green_field, 0, sizeof(palette_green_field));
    memset(&palette_blue_field, 0, sizeof(palette_blue_field));
    memset(&mandelbrot_selection, 0, sizeof(mandelbrot_selection));
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

    mandelbrot_params = (fractus_mandelbrot_params){
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
    julia_params = (fractus_julia_params){
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
        240u
    };
    biomorph_params = (fractus_biomorph_params){
        -2.0,
        2.0,
        -1.5,
        1.5,
        -0.6,
        0.55,
        240u,
        1000.0,
        0u,
        16u,
        240u
    };
    plasma_rectangular_params = (fractus_plasma_params){
        1337u,
        25,
        16u,
        240u
    };
    plasma_circular_params = (fractus_plasma_circular_params){
        7331u,
        320,
        90,
        16u,
        240u
    };

    if (fractus_app_load_legacy_assets(
            &platform,
            &core.ui_framebuffer,
            &mandelbrot_params,
            &julia_params,
            &biomorph_params,
            &legacy_config,
            cfg_path,
            sizeof(cfg_path)) != FRACTUS_STATUS_OK) {
        fractus_app_log("startup: failed loading legacy config/palette, using defaults");
        if (fractus_legacy_config_init_default(&legacy_config) == FRACTUS_STATUS_OK) {
            fractus_app_apply_legacy_numeric_config(&legacy_config, &mandelbrot_params, &julia_params, &biomorph_params);
            (void)fractus_app_apply_legacy_config(&core.ui_framebuffer, &legacy_config);
        }
        if (fractus_formats_resolve_legacy_write_path(&platform, "fractus.cfg", cfg_path, sizeof(cfg_path)) != FRACTUS_STATUS_OK) {
            cfg_path[0] = '\0';
        }
    }
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
                    &julia_params,
                    &julia_pending,
                    &biomorph_params,
                    &biomorph_pending,
                    &iterations_field,
                    &escape_radius_field,
                    &biomorph_radius_field,
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
        } else if (view == FRACTUS_APP_VIEW_MANDELBROT_CONFIG) {
            if (fractus_app_run_mandelbrot_config_view(
                    &core.ui_framebuffer,
                    &fonts,
                    &ui,
                    &mandelbrot_params,
                    &mandelbrot_pending,
                    &view) != FRACTUS_STATUS_OK) {
                fractus_app_log("runtime: mandelbrot config failed");
                running = 0;
            }
        } else if (view == FRACTUS_APP_VIEW_JULIA_CONFIG) {
            if (fractus_app_run_julia_config_view(
                    &core.ui_framebuffer,
                    &fonts,
                    &ui,
                    &julia_params,
                    &julia_pending,
                    &view) != FRACTUS_STATUS_OK) {
                fractus_app_log("runtime: julia config failed");
                running = 0;
            }
        } else if (view == FRACTUS_APP_VIEW_BIOMORPH_CONFIG) {
            if (fractus_app_run_biomorph_config_view(
                    &core.ui_framebuffer,
                    &fonts,
                    &ui,
                    &biomorph_params,
                    &biomorph_pending,
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
        } else if (view == FRACTUS_APP_VIEW_ITERATIONS_CONFIG) {
            if (fractus_app_run_iterations_config_view(
                    &core.ui_framebuffer,
                    &fonts,
                    &ui,
                    &legacy_config,
                    &config_draft,
                    &mandelbrot_params,
                    &julia_params,
                    &biomorph_params,
                    cfg_path,
                    &iterations_field,
                    &view) != FRACTUS_STATUS_OK) {
                fractus_app_log("runtime: iterations config failed");
                running = 0;
            }
        } else if (view == FRACTUS_APP_VIEW_ESCAPE_RADIUS_CONFIG) {
            if (fractus_app_run_escape_radius_config_view(
                    &core.ui_framebuffer,
                    &fonts,
                    &ui,
                    &legacy_config,
                    &config_draft,
                    &mandelbrot_params,
                    &julia_params,
                    &biomorph_params,
                    cfg_path,
                    &escape_radius_field,
                    &view) != FRACTUS_STATUS_OK) {
                fractus_app_log("runtime: escape radius config failed");
                running = 0;
            }
        } else if (view == FRACTUS_APP_VIEW_BIOMORPH_RADIUS_CONFIG) {
            if (fractus_app_run_biomorph_radius_config_view(
                    &core.ui_framebuffer,
                    &fonts,
                    &ui,
                    &legacy_config,
                    &config_draft,
                    &mandelbrot_params,
                    &julia_params,
                    &biomorph_params,
                    cfg_path,
                    &biomorph_radius_field,
                    &view) != FRACTUS_STATUS_OK) {
                fractus_app_log("runtime: biomorph radius config failed");
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
            } else if ((ui.key_press_pending && ui.key_pressed == 27u) ||
                (ui.press_pending && ui.press_event.buttons.right)) {
                mandelbrot_selection.active = 0;
                mandelbrot_selection.has_first_corner = 0;
                view = FRACTUS_APP_VIEW_MAIN_MENU;
            }

            if (ui.key_press_pending &&
                (ui.key_pressed == 'g' || ui.key_pressed == 'G') &&
                current_drawing_saved == 0 &&
                (view == FRACTUS_APP_VIEW_MANDELBROT ||
                 view == FRACTUS_APP_VIEW_JULIA ||
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
            } else if (view == FRACTUS_APP_VIEW_JULIA) {
                mandelbrot_selection.active = 0;
                mandelbrot_selection.has_first_corner = 0;
                present_framebuffer = &core.drawing_framebuffer;
                present_is_drawing = 1;
                if (fractus_app_ensure_drawing_framebuffer_size(&legacy_config, &core.drawing_framebuffer, &core.ui_framebuffer) != FRACTUS_STATUS_OK ||
                    fractus_app_render_julia(&platform, &core.drawing_framebuffer, &fonts, &julia_params, &render_save_next_graphic, runtime_error_message, sizeof(runtime_error_message)) != FRACTUS_STATUS_OK) {
                    fractus_app_log("runtime: julia render failed");
                    running = 0;
                }
            } else if (view == FRACTUS_APP_VIEW_BIOMORPH) {
                present_framebuffer = &core.drawing_framebuffer;
                present_is_drawing = 1;
                if (fractus_app_ensure_drawing_framebuffer_size(&legacy_config, &core.drawing_framebuffer, &core.ui_framebuffer) != FRACTUS_STATUS_OK ||
                    fractus_app_render_biomorph(&platform, &core.drawing_framebuffer, &fonts, &biomorph_params, &render_save_next_graphic, runtime_error_message, sizeof(runtime_error_message)) != FRACTUS_STATUS_OK) {
                    fractus_app_log("runtime: biomorph render failed");
                    running = 0;
                }
            } else if (view == FRACTUS_APP_VIEW_PLASMA_RECTANGULAR) {
                present_framebuffer = &core.drawing_framebuffer;
                present_is_drawing = 1;
                if (fractus_app_ensure_drawing_framebuffer_size(&legacy_config, &core.drawing_framebuffer, &core.ui_framebuffer) != FRACTUS_STATUS_OK ||
                    fractus_app_render_plasma_rectangular(&platform, &core.drawing_framebuffer, &fonts, &plasma_rectangular_params, &render_save_next_graphic, runtime_error_message, sizeof(runtime_error_message)) != FRACTUS_STATUS_OK) {
                    fractus_app_log("runtime: plasma rectangular render failed");
                    running = 0;
                }
            } else if (view == FRACTUS_APP_VIEW_PLASMA_CIRCULAR) {
                present_framebuffer = &core.drawing_framebuffer;
                present_is_drawing = 1;
                if (fractus_app_ensure_drawing_framebuffer_size(&legacy_config, &core.drawing_framebuffer, &core.ui_framebuffer) != FRACTUS_STATUS_OK ||
                    fractus_app_render_plasma_circular(&platform, &core.drawing_framebuffer, &fonts, &plasma_circular_params, &render_save_next_graphic, runtime_error_message, sizeof(runtime_error_message)) != FRACTUS_STATUS_OK) {
                    fractus_app_log("runtime: circular plasma render failed");
                    running = 0;
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

        if (!fractus_app_view_is_generated_drawing(previous_view) &&
            fractus_app_view_is_generated_drawing(view)) {
            current_drawing_saved = 0;
            drawing_presented_once = 0;
            if (view == FRACTUS_APP_VIEW_MANDELBROT) {
                mandelbrot_needs_render = 1;
            }
        } else if (view == FRACTUS_APP_VIEW_GRAPHIC && previous_view != FRACTUS_APP_VIEW_GRAPHIC) {
            current_drawing_saved = 1;
            drawing_presented_once = 1;
        }
        }

        if (!running) {
            break;
        }

        if (fractus_app_sync_framebuffer_palette(&core.drawing_framebuffer, &core.ui_framebuffer) != FRACTUS_STATUS_OK) {
            fractus_app_log("runtime: drawing palette sync failed");
            running = 0;
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
