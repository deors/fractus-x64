#ifndef FRACTUS_X64_FORMATS_H
#define FRACTUS_X64_FORMATS_H

#include "platform/framebuffer.h"
#include "platform/graphics.h"
#include "platform/platform.h"
#include "platform/types.h"

#define FRACTUS_LEGACY_DETVIDEO_COUNT 7u
#define FRACTUS_DRAWING_VIDEO_MODE_COUNT 8u
#define FRACTUS_LEGACY_PALETTE_DATA_COUNT FRACTUS_PALETTE_SPAN
#define FRACTUS_LEGACY_GRAPHIC_HEADER_WORD_COUNT 121u

typedef enum fractus_drawing_video_mode {
    FRACTUS_DRAWING_VIDEO_MODE_WINDOW_640X480 = 0,
    FRACTUS_DRAWING_VIDEO_MODE_WINDOW_800X600,
    FRACTUS_DRAWING_VIDEO_MODE_WINDOW_1024X768,
    FRACTUS_DRAWING_VIDEO_MODE_WINDOW_1280X960,
    FRACTUS_DRAWING_VIDEO_MODE_FULLSCREEN_1280X720,
    FRACTUS_DRAWING_VIDEO_MODE_FULLSCREEN_1920X1080,
    FRACTUS_DRAWING_VIDEO_MODE_FULLSCREEN_2560X1440,
    FRACTUS_DRAWING_VIDEO_MODE_FULLSCREEN_3840X2160
} fractus_drawing_video_mode;

typedef struct fractus_legacy_config {
    uint8_t detvideo[FRACTUS_LEGACY_DETVIDEO_COUNT];
    uint8_t drawing_video_mode;
    int16_t iterations;
    int16_t escape_radius_squared;
    int16_t biomorph_iterations;
    int16_t biomorph_escape_radius_squared;
    int16_t biomorph_cutoff;
    uint32_t plasma_rectangular_seed;
    uint32_t plasma_circular_seed;
    fractus_color_rgba8 palette[FRACTUS_LEGACY_PALETTE_DATA_COUNT];
    fractus_color_rgba8 default_palette[FRACTUS_LEGACY_PALETTE_DATA_COUNT];
} fractus_legacy_config;

fractus_status fractus_legacy_config_init_default(fractus_legacy_config *config);
fractus_status fractus_legacy_config_load(
    const char *path,
    fractus_legacy_config *config);
fractus_status fractus_legacy_config_save(
    const char *path,
    const fractus_legacy_config *config);

fractus_status fractus_legacy_palette_load(
    const char *path,
    fractus_palette *palette);
fractus_status fractus_legacy_palette_save(
    const char *path,
    const fractus_palette *palette);

fractus_status fractus_legacy_graphic_load(
    const char *path,
    fractus_indexed_image *image,
    fractus_palette *palette,
    uint16_t *video_mode);
fractus_status fractus_legacy_graphic_save(
    const char *path,
    const fractus_indexed_image *image,
    const fractus_palette *palette);
fractus_status fractus_legacy_bmp_save(
    const char *path,
    const fractus_indexed_image *image,
    const fractus_palette *palette);

int fractus_formats_file_exists(const char *path);
int fractus_formats_directory_exists(const char *path);
char fractus_formats_ascii_lower(char value);
int fractus_formats_has_extension(const char *name, const char *extension);
fractus_status fractus_formats_copy_path(
    const char *source,
    char *destination,
    size_t destination_size);
fractus_status fractus_formats_join_directory_path(
    const char *directory,
    const char *file_name,
    char separator,
    char *buffer,
    size_t buffer_size);
fractus_status fractus_formats_join_search_path(
    const char *prefix,
    const char *relative_path,
    char *buffer,
    size_t buffer_size);
fractus_status fractus_formats_get_parent_relative_path(
    const char *relative_path,
    char *buffer,
    size_t buffer_size);
fractus_status fractus_formats_resolve_legacy_path(
    const fractus_platform_context *platform,
    const char *relative_path,
    char *buffer,
    size_t buffer_size);
fractus_status fractus_formats_resolve_legacy_directory(
    const fractus_platform_context *platform,
    const char *relative_path,
    char *buffer,
    size_t buffer_size);
fractus_status fractus_formats_resolve_legacy_write_path(
    const fractus_platform_context *platform,
    const char *relative_path,
    char *buffer,
    size_t buffer_size);

#include "core/attractors.h"
#include "core/biomorphs.h"
#include "core/julia.h"
#include "core/mandelbrot.h"
#include "core/plasma.h"

typedef enum fractus_graphic_kind {
    FRACTUS_GRAPHIC_KIND_UNKNOWN = 0,
    FRACTUS_GRAPHIC_KIND_MANDELBROT,
    FRACTUS_GRAPHIC_KIND_MANDELBROT_DEM,
    FRACTUS_GRAPHIC_KIND_JULIA,
    FRACTUS_GRAPHIC_KIND_JULIA_DEM,
    FRACTUS_GRAPHIC_KIND_BIOMORPH,
    FRACTUS_GRAPHIC_KIND_PLASMA_RECTANGULAR,
    FRACTUS_GRAPHIC_KIND_PLASMA_CIRCULAR,
    FRACTUS_GRAPHIC_KIND_LORENZ
} fractus_graphic_kind;

typedef struct fractus_graphic_metadata {
    fractus_graphic_kind kind;
    uint32_t width;
    uint32_t height;
    union {
        fractus_mandelbrot_params mandelbrot;
        fractus_mandelbrot_dem_params mandelbrot_dem;
        fractus_julia_params julia;
        fractus_julia_dem_params julia_dem;
        fractus_biomorph_params biomorph;
        fractus_plasma_params plasma_rectangular;
        fractus_plasma_circular_params plasma_circular;
        fractus_lorenz_params lorenz;
    } params;
} fractus_graphic_metadata;

fractus_graphic_metadata fractus_graphic_metadata_from_mandelbrot(
    const fractus_mandelbrot_params *params,
    uint32_t width,
    uint32_t height);
fractus_graphic_metadata fractus_graphic_metadata_from_mandelbrot_dem(
    const fractus_mandelbrot_dem_params *params,
    uint32_t width,
    uint32_t height);
fractus_graphic_metadata fractus_graphic_metadata_from_julia(
    const fractus_julia_params *params,
    uint32_t width,
    uint32_t height);
fractus_graphic_metadata fractus_graphic_metadata_from_julia_dem(
    const fractus_julia_dem_params *params,
    uint32_t width,
    uint32_t height);
fractus_graphic_metadata fractus_graphic_metadata_from_biomorph(
    const fractus_biomorph_params *params,
    uint32_t width,
    uint32_t height);
fractus_graphic_metadata fractus_graphic_metadata_from_plasma_rectangular(
    const fractus_plasma_params *params,
    uint32_t width,
    uint32_t height);
fractus_graphic_metadata fractus_graphic_metadata_from_plasma_circular(
    const fractus_plasma_circular_params *params,
    uint32_t width,
    uint32_t height);
fractus_graphic_metadata fractus_graphic_metadata_from_lorenz(
    const fractus_lorenz_params *params,
    uint32_t width,
    uint32_t height);

fractus_status fractus_graphic_metadata_save_json(
    const char *path,
    const fractus_graphic_metadata *metadata);
fractus_status fractus_graphic_metadata_load_json(
    const char *path,
    fractus_graphic_metadata *metadata);

#endif
