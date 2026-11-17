#include "core/formats.h"

#if defined(_WIN32)
#include <windows.h>
#else
#include <sys/stat.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *fractus_formats_search_prefixes[] = {"", "../", "../../", "../../../"};

static const unsigned char fractus_palette_header_drsp[13] = {
    'd', 'e', 'o', 'r', 's', 0xb7, 'p', 'a', 'l', 'e', 't', 'a', 0x1a
};
static const unsigned char fractus_legacy_palette_header_mhp[13] = {
    'M', 'H', 'I', 'D', 'S', ' ', 'P', 'a', 'l', 'e', 't', 'a', 0x1a
};
static const unsigned char fractus_graphic_header_drsg[14] = {
    'd', 'e', 'o', 'r', 's', 0xb7, 'g', 'r', 'a', 'f', 'i', 'c', 'o', 0x1a
};
static const unsigned char fractus_legacy_graphic_header_dos[14] = {
    'M', 'H', 'I', 'D', 'S', ' ', 'G', 'r', 0xa0, 'f', 'i', 'c', 'o', 0x1a
};
static const unsigned char fractus_legacy_graphic_header_latin1[14] = {
    'M', 'H', 'I', 'D', 'S', ' ', 'G', 'r', 0xe1, 'f', 'i', 'c', 'o', 0x1a
};

static uint8_t fractus_to_legacy_6bit(uint8_t value)
{
    return (uint8_t)((value * 63u + 127u) / 255u);
}

static uint8_t fractus_from_legacy_6bit(uint8_t value)
{
    return (uint8_t)((value * 255u + 31u) / 63u);
}

static uint8_t fractus_valid_drawing_video_mode(uint8_t value)
{
    return (value < FRACTUS_DRAWING_VIDEO_MODE_COUNT) ? value : (uint8_t)FRACTUS_DRAWING_VIDEO_MODE_WINDOW_640X480;
}

uint8_t fractus_graphic_output_format_normalize(uint8_t format)
{
    if (format == (uint8_t)FRACTUS_GRAPHIC_OUTPUT_PNG ||
        format == (uint8_t)FRACTUS_GRAPHIC_OUTPUT_BMP) {
        return format;
    }

    return (uint8_t)FRACTUS_GRAPHIC_OUTPUT_DRSG;
}

const char *fractus_graphic_output_extension(uint8_t format)
{
    format = fractus_graphic_output_format_normalize(format);
    if (format == (uint8_t)FRACTUS_GRAPHIC_OUTPUT_PNG) {
        return ".png";
    }
    if (format == (uint8_t)FRACTUS_GRAPHIC_OUTPUT_BMP) {
        return ".bmp";
    }

    return ".drsg";
}

static fractus_status fractus_read_u8(FILE *file, uint8_t *value)
{
    if (file == NULL || value == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    return (fread(value, sizeof(*value), 1u, file) == 1u) ? FRACTUS_STATUS_OK : FRACTUS_STATUS_ERROR;
}

static fractus_status fractus_write_u8(FILE *file, uint8_t value)
{
    if (file == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    return (fwrite(&value, sizeof(value), 1u, file) == 1u) ? FRACTUS_STATUS_OK : FRACTUS_STATUS_ERROR;
}

static fractus_status fractus_read_i16_le(FILE *file, int16_t *value)
{
    uint8_t bytes[2];

    if (file == NULL || value == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fread(bytes, sizeof(bytes), 1u, file) != 1u) {
        return FRACTUS_STATUS_ERROR;
    }

    *value = (int16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8u));
    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_write_i16_le(FILE *file, int16_t value)
{
    uint8_t bytes[2];

    if (file == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    bytes[0] = (uint8_t)((uint16_t)value & 0xffu);
    bytes[1] = (uint8_t)(((uint16_t)value >> 8u) & 0xffu);

    return (fwrite(bytes, sizeof(bytes), 1u, file) == 1u) ? FRACTUS_STATUS_OK : FRACTUS_STATUS_ERROR;
}

static fractus_status fractus_read_u16_le(FILE *file, uint16_t *value)
{
    uint8_t bytes[2];

    if (file == NULL || value == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fread(bytes, sizeof(bytes), 1u, file) != 1u) {
        return FRACTUS_STATUS_ERROR;
    }

    *value = (uint16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8u));
    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_write_u16_le(FILE *file, uint16_t value)
{
    uint8_t bytes[2];

    if (file == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    bytes[0] = (uint8_t)(value & 0xffu);
    bytes[1] = (uint8_t)((value >> 8u) & 0xffu);

    return (fwrite(bytes, sizeof(bytes), 1u, file) == 1u) ? FRACTUS_STATUS_OK : FRACTUS_STATUS_ERROR;
}

static fractus_status fractus_read_u32_le(FILE *file, uint32_t *value)
{
    uint8_t bytes[4];

    if (file == NULL || value == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fread(bytes, sizeof(bytes), 1u, file) != 1u) {
        return FRACTUS_STATUS_ERROR;
    }

    *value = (uint32_t)bytes[0] |
             ((uint32_t)bytes[1] << 8u) |
             ((uint32_t)bytes[2] << 16u) |
             ((uint32_t)bytes[3] << 24u);
    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_write_u32_le(FILE *file, uint32_t value)
{
    uint8_t bytes[4];

    if (file == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    bytes[0] = (uint8_t)(value & 0xffu);
    bytes[1] = (uint8_t)((value >> 8u) & 0xffu);
    bytes[2] = (uint8_t)((value >> 16u) & 0xffu);
    bytes[3] = (uint8_t)((value >> 24u) & 0xffu);

    return (fwrite(bytes, sizeof(bytes), 1u, file) == 1u) ? FRACTUS_STATUS_OK : FRACTUS_STATUS_ERROR;
}

/* Header word [3] of a .drsg. Ids 0..11 are fixed so older files still load.
   New sizes are appended. This numbering is not drawing_video_mode. */
#define FRACTUS_LEGACY_GRAPHIC_VIDEO_MODE_LAST 14u

static fractus_status fractus_legacy_video_mode_info(
    uint16_t video_mode,
    fractus_size_u32 *size,
    uint16_t *chunk_count)
{
    fractus_size_u32 resolved_size;
    uint16_t resolved_chunk_count;

    switch (video_mode) {
    case 0u:
        resolved_size.width = 320u;
        resolved_size.height = 200u;
        resolved_chunk_count = 2u;
        break;
    case 1u:
        resolved_size.width = 640u;
        resolved_size.height = 400u;
        resolved_chunk_count = 8u;
        break;
    case 2u:
        resolved_size.width = 640u;
        resolved_size.height = 480u;
        resolved_chunk_count = 10u;
        break;
    case 3u:
        resolved_size.width = 800u;
        resolved_size.height = 600u;
        resolved_chunk_count = 16u;
        break;
    case 4u:
        resolved_size.width = 1024u;
        resolved_size.height = 768u;
        resolved_chunk_count = 32u;
        break;
    case 5u:
        resolved_size.width = 640u;
        resolved_size.height = 350u;
        resolved_chunk_count = 8u;
        break;
    case 6u:
        resolved_size.width = 1280u;
        resolved_size.height = 1024u;
        resolved_chunk_count = 64u;
        break;
    case 7u:
        resolved_size.width = 1280u;
        resolved_size.height = 960u;
        resolved_chunk_count = 1u;
        break;
    case 8u:
        resolved_size.width = 1280u;
        resolved_size.height = 720u;
        resolved_chunk_count = 1u;
        break;
    case 9u:
        resolved_size.width = 1920u;
        resolved_size.height = 1080u;
        resolved_chunk_count = 1u;
        break;
    case 10u:
        resolved_size.width = 2560u;
        resolved_size.height = 1440u;
        resolved_chunk_count = 1u;
        break;
    case 11u:
        resolved_size.width = 3840u;
        resolved_size.height = 2160u;
        resolved_chunk_count = 1u;
        break;
    case 12u:
        resolved_size.width = 1440u;
        resolved_size.height = 900u;
        resolved_chunk_count = 1u;
        break;
    case 13u:
        resolved_size.width = 1920u;
        resolved_size.height = 1200u;
        resolved_chunk_count = 1u;
        break;
    case 14u:
        resolved_size.width = 2880u;
        resolved_size.height = 1800u;
        resolved_chunk_count = 1u;
        break;
    default:
        return FRACTUS_STATUS_UNSUPPORTED;
    }

    if (size != NULL) {
        *size = resolved_size;
    }

    if (chunk_count != NULL) {
        *chunk_count = resolved_chunk_count;
    }

    return FRACTUS_STATUS_OK;
}

static const fractus_color_rgba8 fractus_default_fractal_palette[FRACTUS_LEGACY_PALETTE_DATA_COUNT] = {
    {121,  61, 121, 255u}, {117,  61, 121, 255u}, {117,  65, 130, 255u}, {117,  69, 134, 255u},
    {113,  69, 138, 255u}, {113,  73, 142, 255u}, {113,  77, 142, 255u}, {109,  77, 146, 255u},
    {109,  81, 150, 255u}, {109,  85, 154, 255u}, {105,  85, 158, 255u}, {105,  85, 158, 255u},
    {105,  89, 162, 255u}, {101,  89, 166, 255u}, {101,  93, 170, 255u}, {101,  97, 170, 255u},
    {101, 101, 174, 255u}, { 97,  97, 174, 255u}, { 97,  97, 178, 255u}, { 93,  93, 178, 255u},
    { 93,  93, 182, 255u}, { 89,  89, 182, 255u}, { 89,  89, 186, 255u}, { 85,  85, 186, 255u},
    { 85,  85, 190, 255u}, { 85,  85, 190, 255u}, { 85,  85, 194, 255u}, { 81,  81, 194, 255u},
    { 81,  81, 198, 255u}, { 77,  77, 198, 255u}, { 77,  77, 202, 255u}, { 73,  73, 202, 255u},
    { 73,  73, 206, 255u}, { 69,  69, 206, 255u}, { 69,  69, 215, 255u}, { 65,  65, 215, 255u},
    { 65,  65, 219, 255u}, { 61,  61, 219, 255u}, { 61,  61, 223, 255u}, { 57,  57, 223, 255u},
    { 57,  57, 227, 255u}, { 53,  53, 227, 255u}, { 53,  53, 231, 255u}, { 49,  49, 231, 255u},
    { 49,  49, 235, 255u}, { 45,  45, 235, 255u}, { 45,  45, 239, 255u}, { 36,  36, 239, 255u},
    { 36,  36, 243, 255u}, { 36,  36, 239, 255u}, { 36,  45, 235, 255u}, { 36,  45, 235, 255u},
    { 36,  49, 231, 255u}, { 36,  49, 231, 255u}, { 36,  53, 227, 255u}, { 36,  53, 227, 255u},
    { 36,  57, 223, 255u}, { 36,  61, 219, 255u}, { 36,  61, 219, 255u}, { 36,  65, 215, 255u},
    { 36,  65, 215, 255u}, { 36,  69, 206, 255u}, { 36,  69, 206, 255u}, { 36,  73, 202, 255u},
    { 36,  77, 202, 255u}, { 36,  77, 198, 255u}, { 36,  81, 194, 255u}, { 36,  81, 194, 255u},
    { 36,  85, 190, 255u}, { 36,  85, 190, 255u}, { 36,  85, 186, 255u}, { 36,  85, 186, 255u},
    { 36,  89, 182, 255u}, { 36,  93, 178, 255u}, { 36,  93, 178, 255u}, { 36,  97, 174, 255u},
    { 36,  97, 174, 255u}, { 36, 101, 170, 255u}, { 36, 101, 170, 255u}, { 36, 105, 170, 255u},
    { 36, 109, 170, 255u}, { 36, 113, 166, 255u}, { 36, 117, 162, 255u}, { 36, 121, 158, 255u},
    { 36, 130, 154, 255u}, { 36, 134, 150, 255u}, { 36, 138, 146, 255u}, { 36, 142, 142, 255u},
    { 36, 146, 138, 255u}, { 36, 150, 134, 255u}, { 36, 154, 130, 255u}, { 36, 158, 121, 255u},
    { 36, 162, 117, 255u}, { 36, 166, 113, 255u}, { 36, 170, 109, 255u}, { 36, 170, 105, 255u},
    { 36, 174, 101, 255u}, { 36, 178,  97, 255u}, { 36, 182,  93, 255u}, { 36, 186,  89, 255u},
    { 36, 190,  85, 255u}, { 36, 194,  85, 255u}, { 36, 198,  81, 255u}, { 36, 202,  77, 255u},
    { 36, 206,  73, 255u}, { 36, 215,  69, 255u}, { 36, 219,  65, 255u}, { 36, 223,  61, 255u},
    { 36, 227,  57, 255u}, { 36, 231,  53, 255u}, { 36, 235,  49, 255u}, { 36, 239,  45, 255u},
    { 36, 243,  36, 255u}, { 45, 243,  36, 255u}, { 53, 243,  36, 255u}, { 57, 243,  36, 255u},
    { 65, 243,  36, 255u}, { 69, 243,  36, 255u}, { 77, 243,  36, 255u}, { 81, 243,  36, 255u},
    { 85, 243,  36, 255u}, { 93, 243,  36, 255u}, { 97, 243,  36, 255u}, {105, 243,  36, 255u},
    {109, 243,  36, 255u}, {117, 243,  36, 255u}, {121, 243,  36, 255u}, {134, 243,  36, 255u},
    {142, 243,  36, 255u}, {146, 243,  36, 255u}, {154, 243,  36, 255u}, {158, 243,  36, 255u},
    {166, 243,  36, 255u}, {170, 243,  36, 255u}, {174, 243,  36, 255u}, {178, 243,  36, 255u},
    {186, 243,  36, 255u}, {194, 243,  36, 255u}, {198, 243,  36, 255u}, {206, 243,  36, 255u},
    {215, 243,  36, 255u}, {223, 243,  36, 255u}, {227, 243,  36, 255u}, {235, 243,  36, 255u},
    {243, 243,  36, 255u}, {239, 239,  36, 255u}, {239, 235,  36, 255u}, {239, 235,  36, 255u},
    {239, 231,  36, 255u}, {239, 231,  45, 255u}, {239, 227,  45, 255u}, {239, 227,  45, 255u},
    {239, 223,  45, 255u}, {239, 223,  45, 255u}, {239, 219,  49, 255u}, {239, 219,  49, 255u},
    {239, 215,  49, 255u}, {239, 215,  49, 255u}, {239, 206,  53, 255u}, {239, 206,  53, 255u},
    {239, 202,  53, 255u}, {239, 198,  53, 255u}, {239, 198,  53, 255u}, {239, 194,  57, 255u},
    {239, 194,  57, 255u}, {239, 190,  57, 255u}, {239, 190,  57, 255u}, {239, 186,  61, 255u},
    {239, 186,  61, 255u}, {239, 182,  61, 255u}, {239, 182,  61, 255u}, {239, 178,  61, 255u},
    {239, 178,  65, 255u}, {239, 174,  65, 255u}, {239, 174,  65, 255u}, {239, 170,  65, 255u},
    {239, 170,  69, 255u}, {239, 166,  65, 255u}, {239, 162,  65, 255u}, {239, 158,  65, 255u},
    {239, 154,  65, 255u}, {239, 150,  61, 255u}, {239, 146,  61, 255u}, {239, 142,  61, 255u},
    {239, 138,  61, 255u}, {239, 134,  61, 255u}, {239, 130,  57, 255u}, {239, 121,  57, 255u},
    {239, 117,  57, 255u}, {239, 113,  57, 255u}, {239, 109,  53, 255u}, {239, 105,  53, 255u},
    {239, 101,  53, 255u}, {239,  97,  53, 255u}, {239,  93,  53, 255u}, {239,  89,  49, 255u},
    {239,  85,  49, 255u}, {239,  85,  49, 255u}, {239,  81,  49, 255u}, {239,  77,  45, 255u},
    {239,  73,  45, 255u}, {239,  69,  45, 255u}, {239,  65,  45, 255u}, {239,  61,  45, 255u},
    {239,  57,  36, 255u}, {239,  53,  36, 255u}, {239,  49,  36, 255u}, {239,  45,  36, 255u},
    {243,  36,  36, 255u}, {239,  36,  36, 255u}, {235,  36,  40, 255u}, {231,  36,  45, 255u},
    {227,  36,  45, 255u}, {223,  36,  49, 255u}, {219,  40,  53, 255u}, {215,  40,  53, 255u},
    {210,  40,  57, 255u}, {206,  40,  61, 255u}, {202,  40,  61, 255u}, {198,  45,  65, 255u},
    {194,  45,  69, 255u}, {190,  45,  69, 255u}, {186,  45,  73, 255u}, {182,  45,  77, 255u},
    {178,  49,  77, 255u}, {174,  49,  81, 255u}, {170,  49,  85, 255u}, {166,  49,  85, 255u},
    {162,  49,  89, 255u}, {158,  53,  93, 255u}, {154,  53,  93, 255u}, {150,  53,  97, 255u},
    {146,  53, 101, 255u}, {142,  53, 101, 255u}, {138,  57, 105, 255u}, {134,  57, 109, 255u},
    {130,  57, 109, 255u}, {125,  57, 113, 255u}, {121,  57, 117, 255u}, {121,  61, 121, 255u}
};

fractus_status fractus_legacy_config_init_default(fractus_legacy_config *config)
{
    uint32_t i;

    if (config == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    memset(config, 0, sizeof(*config));
    for (i = 0u; i < FRACTUS_LEGACY_DETVIDEO_COUNT; ++i) {
        config->detvideo[i] = 1u;
    }
    config->drawing_video_mode = (uint8_t)FRACTUS_DRAWING_VIDEO_MODE_WINDOW_640X480;
    config->iterations = 250;
    config->escape_radius_squared = 4;
    config->biomorph_iterations = 250;
    config->biomorph_escape_radius_squared = 100;
    config->biomorph_cutoff = 10;
    config->plasma_rectangular_seed = 1337u;
    config->plasma_circular_seed = 7331u;
    config->graphic_output_format = (uint8_t)FRACTUS_GRAPHIC_OUTPUT_DRSG;

    for (i = 0u; i < FRACTUS_LEGACY_PALETTE_DATA_COUNT; ++i) {
        config->palette[i] = fractus_default_fractal_palette[i];
        config->default_palette[i] = fractus_default_fractal_palette[i];
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_legacy_config_load(
    const char *path,
    fractus_legacy_config *config)
{
    FILE *file;
    uint8_t drawing_video_mode;
    int16_t biomorph_cutoff;
    int16_t biomorph_iterations_val;
    uint32_t rect_seed;
    uint32_t circ_seed;
    uint8_t output_format;
    uint32_t i;

    if (path == NULL || config == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_legacy_config_init_default(config) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    file = fopen(path, "rb");
    if (file == NULL) {
        return FRACTUS_STATUS_ERROR;
    }

    for (i = 0u; i < FRACTUS_LEGACY_DETVIDEO_COUNT; ++i) {
        if (fractus_read_u8(file, &config->detvideo[i]) != FRACTUS_STATUS_OK) {
            fclose(file);
            return FRACTUS_STATUS_ERROR;
        }
    }

    if (fractus_read_i16_le(file, &config->iterations) != FRACTUS_STATUS_OK ||
        fractus_read_i16_le(file, &config->escape_radius_squared) != FRACTUS_STATUS_OK ||
        fractus_read_i16_le(file, &config->biomorph_escape_radius_squared) != FRACTUS_STATUS_OK) {
            fclose(file);
            return FRACTUS_STATUS_ERROR;
    }

    for (i = 0u; i < FRACTUS_LEGACY_PALETTE_DATA_COUNT; ++i) {
        uint8_t r;
        uint8_t g;
        uint8_t b;

        if (fractus_read_u8(file, &r) != FRACTUS_STATUS_OK ||
            fractus_read_u8(file, &g) != FRACTUS_STATUS_OK ||
            fractus_read_u8(file, &b) != FRACTUS_STATUS_OK) {
            fclose(file);
            return FRACTUS_STATUS_ERROR;
        }

        config->palette[i].r = fractus_from_legacy_6bit(r);
        config->palette[i].g = fractus_from_legacy_6bit(g);
        config->palette[i].b = fractus_from_legacy_6bit(b);
        config->palette[i].a = 255u;
    }

    if (fractus_read_u8(file, &drawing_video_mode) == FRACTUS_STATUS_OK) {
        config->drawing_video_mode = fractus_valid_drawing_video_mode(drawing_video_mode);
        if (fractus_read_i16_le(file, &biomorph_cutoff) == FRACTUS_STATUS_OK) {
            config->biomorph_cutoff = (biomorph_cutoff > 0) ? biomorph_cutoff : 1;
            if (fractus_read_i16_le(file, &biomorph_iterations_val) == FRACTUS_STATUS_OK) {
                config->biomorph_iterations = (biomorph_iterations_val > 0) ? biomorph_iterations_val : 250;
            }
            for (i = 0u; i < FRACTUS_LEGACY_PALETTE_DATA_COUNT; ++i) {
                uint8_t r, g, b;
                if (fractus_read_u8(file, &r) != FRACTUS_STATUS_OK ||
                    fractus_read_u8(file, &g) != FRACTUS_STATUS_OK ||
                    fractus_read_u8(file, &b) != FRACTUS_STATUS_OK) {
                    break;
                }
                config->default_palette[i].r = fractus_from_legacy_6bit(r);
                config->default_palette[i].g = fractus_from_legacy_6bit(g);
                config->default_palette[i].b = fractus_from_legacy_6bit(b);
                config->default_palette[i].a = 255u;
            }
            if (fractus_read_u32_le(file, &rect_seed) == FRACTUS_STATUS_OK) {
                config->plasma_rectangular_seed = (rect_seed > 0u) ? rect_seed : 1337u;
                if (fractus_read_u32_le(file, &circ_seed) == FRACTUS_STATUS_OK) {
                    config->plasma_circular_seed = (circ_seed > 0u) ? circ_seed : 7331u;
                    if (fractus_read_u8(file, &output_format) == FRACTUS_STATUS_OK) {
                        config->graphic_output_format = fractus_graphic_output_format_normalize(output_format);
                    }
                }
            }
        }
    } else if (ferror(file)) {
        fclose(file);
        return FRACTUS_STATUS_ERROR;
    }

    fclose(file);
    return FRACTUS_STATUS_OK;
}

fractus_status fractus_legacy_config_save(
    const char *path,
    const fractus_legacy_config *config)
{
    FILE *file;
    uint32_t i;

    if (path == NULL || config == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    file = fopen(path, "wb");
    if (file == NULL) {
        return FRACTUS_STATUS_ERROR;
    }

    for (i = 0u; i < FRACTUS_LEGACY_DETVIDEO_COUNT; ++i) {
        if (fractus_write_u8(file, config->detvideo[i]) != FRACTUS_STATUS_OK) {
            fclose(file);
            return FRACTUS_STATUS_ERROR;
        }
    }

    if (fractus_write_i16_le(file, config->iterations) != FRACTUS_STATUS_OK ||
        fractus_write_i16_le(file, config->escape_radius_squared) != FRACTUS_STATUS_OK ||
        fractus_write_i16_le(file, config->biomorph_escape_radius_squared) != FRACTUS_STATUS_OK) {
        fclose(file);
        return FRACTUS_STATUS_ERROR;
    }

    for (i = 0u; i < FRACTUS_LEGACY_PALETTE_DATA_COUNT; ++i) {
        if (fractus_write_u8(file, fractus_to_legacy_6bit(config->palette[i].r)) != FRACTUS_STATUS_OK ||
            fractus_write_u8(file, fractus_to_legacy_6bit(config->palette[i].g)) != FRACTUS_STATUS_OK ||
            fractus_write_u8(file, fractus_to_legacy_6bit(config->palette[i].b)) != FRACTUS_STATUS_OK) {
            fclose(file);
            return FRACTUS_STATUS_ERROR;
        }
    }

    if (fractus_write_u8(file, fractus_valid_drawing_video_mode(config->drawing_video_mode)) != FRACTUS_STATUS_OK ||
        fractus_write_i16_le(file, (config->biomorph_cutoff > 0) ? config->biomorph_cutoff : 1) != FRACTUS_STATUS_OK ||
        fractus_write_i16_le(file, (config->biomorph_iterations > 0) ? config->biomorph_iterations : 15) != FRACTUS_STATUS_OK) {
        fclose(file);
        return FRACTUS_STATUS_ERROR;
    }

    for (i = 0u; i < FRACTUS_LEGACY_PALETTE_DATA_COUNT; ++i) {
        if (fractus_write_u8(file, fractus_to_legacy_6bit(config->default_palette[i].r)) != FRACTUS_STATUS_OK ||
            fractus_write_u8(file, fractus_to_legacy_6bit(config->default_palette[i].g)) != FRACTUS_STATUS_OK ||
            fractus_write_u8(file, fractus_to_legacy_6bit(config->default_palette[i].b)) != FRACTUS_STATUS_OK) {
            fclose(file);
            return FRACTUS_STATUS_ERROR;
        }
    }

    if (fractus_write_u32_le(file, config->plasma_rectangular_seed > 0u ? config->plasma_rectangular_seed : 1337u) != FRACTUS_STATUS_OK ||
        fractus_write_u32_le(file, config->plasma_circular_seed > 0u ? config->plasma_circular_seed : 7331u) != FRACTUS_STATUS_OK ||
        fractus_write_u8(file, fractus_graphic_output_format_normalize(config->graphic_output_format)) != FRACTUS_STATUS_OK) {
        fclose(file);
        return FRACTUS_STATUS_ERROR;
    }

    fclose(file);
    return FRACTUS_STATUS_OK;
}

fractus_status fractus_legacy_palette_load(
    const char *path,
    fractus_palette *palette)
{
    FILE *file;
    uint8_t header[13];
    uint32_t i;

    if (path == NULL || palette == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    file = fopen(path, "rb");
    if (file == NULL) {
        return FRACTUS_STATUS_ERROR;
    }

    if (fread(header, sizeof(header), 1u, file) != 1u ||
        (memcmp(header, fractus_palette_header_drsp, sizeof(header)) != 0 &&
         memcmp(header, fractus_legacy_palette_header_mhp, sizeof(header)) != 0)) {
        fclose(file);
        return FRACTUS_STATUS_ERROR;
    }

    if (fractus_palette_init_default(palette) != FRACTUS_STATUS_OK) {
        fclose(file);
        return FRACTUS_STATUS_ERROR;
    }

    for (i = FRACTUS_PALETTE_OFFSET; i < FRACTUS_PALETTE_SIZE; ++i) {
        uint8_t r;
        uint8_t g;
        uint8_t b;

        if (fractus_read_u8(file, &r) != FRACTUS_STATUS_OK ||
            fractus_read_u8(file, &g) != FRACTUS_STATUS_OK ||
            fractus_read_u8(file, &b) != FRACTUS_STATUS_OK) {
            fclose(file);
            return FRACTUS_STATUS_ERROR;
        }

        palette->entries[i].r = fractus_from_legacy_6bit(r);
        palette->entries[i].g = fractus_from_legacy_6bit(g);
        palette->entries[i].b = fractus_from_legacy_6bit(b);
        palette->entries[i].a = 255u;
    }

    fclose(file);
    return FRACTUS_STATUS_OK;
}

fractus_status fractus_legacy_palette_save(
    const char *path,
    const fractus_palette *palette)
{
    FILE *file;
    uint32_t i;

    if (path == NULL || palette == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    file = fopen(path, "wb");
    if (file == NULL) {
        return FRACTUS_STATUS_ERROR;
    }

    if (fwrite(fractus_palette_header_drsp, sizeof(fractus_palette_header_drsp), 1u, file) != 1u) {
        fclose(file);
        return FRACTUS_STATUS_ERROR;
    }

    for (i = FRACTUS_PALETTE_OFFSET; i < FRACTUS_PALETTE_SIZE; ++i) {
        if (fractus_write_u8(file, fractus_to_legacy_6bit(palette->entries[i].r)) != FRACTUS_STATUS_OK ||
            fractus_write_u8(file, fractus_to_legacy_6bit(palette->entries[i].g)) != FRACTUS_STATUS_OK ||
            fractus_write_u8(file, fractus_to_legacy_6bit(palette->entries[i].b)) != FRACTUS_STATUS_OK) {
            fclose(file);
            return FRACTUS_STATUS_ERROR;
        }
    }

    fclose(file);
    return FRACTUS_STATUS_OK;
}

fractus_status fractus_legacy_graphic_load(
    const char *path,
    fractus_indexed_image *image,
    fractus_palette *palette,
    uint16_t *video_mode)
{
    FILE *file;
    uint8_t file_prefix[14];
    uint16_t header[FRACTUS_LEGACY_GRAPHIC_HEADER_WORD_COUNT];
    fractus_size_u32 expected_size;
    fractus_indexed_image loaded_image;
    uint16_t expected_chunk_count;
    uint32_t chunk;
    uint32_t rows_per_chunk;
    fractus_status status;

    if (path == NULL || image == NULL || palette == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    memset(&loaded_image, 0, sizeof(loaded_image));
    file = fopen(path, "rb");
    if (file == NULL) {
        return FRACTUS_STATUS_ERROR;
    }

    if (fread(file_prefix, sizeof(file_prefix), 1u, file) != 1u ||
        (memcmp(file_prefix, fractus_graphic_header_drsg, sizeof(fractus_graphic_header_drsg)) != 0 &&
         memcmp(file_prefix, fractus_legacy_graphic_header_dos, sizeof(fractus_legacy_graphic_header_dos)) != 0 &&
         memcmp(file_prefix, fractus_legacy_graphic_header_latin1, sizeof(fractus_legacy_graphic_header_latin1)) != 0)) {
        fclose(file);
        return FRACTUS_STATUS_ERROR;
    }

    for (chunk = 0u; chunk < FRACTUS_LEGACY_GRAPHIC_HEADER_WORD_COUNT; ++chunk) {
        if (fractus_read_u16_le(file, &header[chunk]) != FRACTUS_STATUS_OK) {
            fclose(file);
            return FRACTUS_STATUS_ERROR;
        }
    }

    if (header[0] != 1u) {
        fclose(file);
        return FRACTUS_STATUS_ERROR;
    }

    status = fractus_legacy_video_mode_info(header[3], &expected_size, &expected_chunk_count);
    if (status != FRACTUS_STATUS_OK) {
        fclose(file);
        return status;
    }

    if (header[1] != expected_size.width ||
        header[2] != expected_size.height ||
        header[4] != expected_chunk_count ||
        expected_chunk_count == 0u ||
        (expected_size.height % expected_chunk_count) != 0u) {
        fclose(file);
        return FRACTUS_STATUS_ERROR;
    }

    if (fractus_palette_init_default(palette) != FRACTUS_STATUS_OK) {
        fclose(file);
        return FRACTUS_STATUS_ERROR;
    }

    for (chunk = 0u; chunk < FRACTUS_PALETTE_SIZE; ++chunk) {
        uint8_t r;
        uint8_t g;
        uint8_t b;

        if (fractus_read_u8(file, &r) != FRACTUS_STATUS_OK ||
            fractus_read_u8(file, &g) != FRACTUS_STATUS_OK ||
            fractus_read_u8(file, &b) != FRACTUS_STATUS_OK) {
            fclose(file);
            return FRACTUS_STATUS_ERROR;
        }

        palette->entries[chunk].r = fractus_from_legacy_6bit(r);
        palette->entries[chunk].g = fractus_from_legacy_6bit(g);
        palette->entries[chunk].b = fractus_from_legacy_6bit(b);
        palette->entries[chunk].a = 255u;
    }

    if (fractus_indexed_image_init(&loaded_image, expected_size) != FRACTUS_STATUS_OK) {
        fclose(file);
        return FRACTUS_STATUS_ERROR;
    }

    rows_per_chunk = expected_size.height / expected_chunk_count;
    for (chunk = 0u; chunk < expected_chunk_count; ++chunk) {
        size_t chunk_size = (size_t)expected_size.width * rows_per_chunk;
        uint8_t *chunk_pixels = loaded_image.pixels + ((size_t)chunk * rows_per_chunk * loaded_image.pitch_pixels);

        if (fread(chunk_pixels, chunk_size, 1u, file) != 1u) {
            fractus_indexed_image_shutdown(&loaded_image);
            fclose(file);
            return FRACTUS_STATUS_ERROR;
        }
    }

    if (video_mode != NULL) {
        *video_mode = header[3];
    }

    *image = loaded_image;
    fclose(file);
    return FRACTUS_STATUS_OK;
}

fractus_status fractus_legacy_graphic_save(
    const char *path,
    const fractus_indexed_image *image,
    const fractus_palette *palette)
{
    FILE *file;
    uint16_t header[FRACTUS_LEGACY_GRAPHIC_HEADER_WORD_COUNT];
    uint16_t video_mode;
    uint16_t chunk_count;
    uint32_t i;
    uint32_t rows_per_chunk;
    fractus_status status;

    if (path == NULL || image == NULL || palette == NULL || !image->initialized) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    status = FRACTUS_STATUS_UNSUPPORTED;
    chunk_count = 0u;
    for (video_mode = 0u; video_mode <= FRACTUS_LEGACY_GRAPHIC_VIDEO_MODE_LAST; ++video_mode) {
        fractus_size_u32 known_size;
        uint16_t known_chunk_count;

        if (fractus_legacy_video_mode_info(video_mode, &known_size, &known_chunk_count) != FRACTUS_STATUS_OK) {
            continue;
        }

        if (known_size.width == image->size.width && known_size.height == image->size.height) {
            chunk_count = known_chunk_count;
            status = FRACTUS_STATUS_OK;
            break;
        }
    }

    if (status != FRACTUS_STATUS_OK || chunk_count == 0u || (image->size.height % chunk_count) != 0u) {
        return FRACTUS_STATUS_UNSUPPORTED;
    }

    memset(header, 0, sizeof(header));
    header[0] = 1u;
    header[1] = (uint16_t)image->size.width;
    header[2] = (uint16_t)image->size.height;
    header[3] = video_mode;
    header[4] = chunk_count;

    file = fopen(path, "wb");
    if (file == NULL) {
        return FRACTUS_STATUS_ERROR;
    }

    if (fwrite(fractus_graphic_header_drsg, sizeof(fractus_graphic_header_drsg), 1u, file) != 1u) {
        fclose(file);
        return FRACTUS_STATUS_ERROR;
    }

    for (i = 0u; i < FRACTUS_LEGACY_GRAPHIC_HEADER_WORD_COUNT; ++i) {
        if (fractus_write_u16_le(file, header[i]) != FRACTUS_STATUS_OK) {
            fclose(file);
            return FRACTUS_STATUS_ERROR;
        }
    }

    for (i = 0u; i < FRACTUS_PALETTE_SIZE; ++i) {
        if (fractus_write_u8(file, fractus_to_legacy_6bit(palette->entries[i].r)) != FRACTUS_STATUS_OK ||
            fractus_write_u8(file, fractus_to_legacy_6bit(palette->entries[i].g)) != FRACTUS_STATUS_OK ||
            fractus_write_u8(file, fractus_to_legacy_6bit(palette->entries[i].b)) != FRACTUS_STATUS_OK) {
            fclose(file);
            return FRACTUS_STATUS_ERROR;
        }
    }

    rows_per_chunk = image->size.height / chunk_count;
    for (i = 0u; i < chunk_count; ++i) {
        size_t chunk_size = (size_t)image->size.width * rows_per_chunk;
        const uint8_t *chunk_pixels = image->pixels + ((size_t)i * rows_per_chunk * image->pitch_pixels);

        if (fwrite(chunk_pixels, chunk_size, 1u, file) != 1u) {
            fclose(file);
            return FRACTUS_STATUS_ERROR;
        }
    }

    fclose(file);
    return FRACTUS_STATUS_OK;
}

fractus_status fractus_legacy_bmp_save(
    const char *path,
    const fractus_indexed_image *image,
    const fractus_palette *palette)
{
    FILE *file;
    uint32_t width;
    uint32_t height;
    uint32_t row_stride;
    uint32_t image_size;
    uint32_t file_header_size = 14u;
    uint32_t info_header_size = 40u;
    uint32_t palette_size_bytes = 256u * 4u;
    uint32_t data_offset = file_header_size + info_header_size + palette_size_bytes;
    uint32_t file_size;
    uint32_t i;
    int32_t y;
    static const uint8_t padding[4] = {0u, 0u, 0u, 0u};
    size_t pad_len;

    if (path == NULL || image == NULL || palette == NULL || !image->initialized) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    width = image->size.width;
    height = image->size.height;
    if (width == 0u || height == 0u) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    row_stride = (width + 3u) & ~3u;
    pad_len = (size_t)(row_stride - width);
    image_size = row_stride * height;
    file_size = data_offset + image_size;

    file = fopen(path, "wb");
    if (file == NULL) {
        return FRACTUS_STATUS_ERROR;
    }

    /* BITMAPFILEHEADER (14 bytes) */
    if (fractus_write_u8(file, 'B') != FRACTUS_STATUS_OK ||
        fractus_write_u8(file, 'M') != FRACTUS_STATUS_OK ||
        fractus_write_u32_le(file, file_size) != FRACTUS_STATUS_OK ||
        fractus_write_u16_le(file, 0u) != FRACTUS_STATUS_OK ||
        fractus_write_u16_le(file, 0u) != FRACTUS_STATUS_OK ||
        fractus_write_u32_le(file, data_offset) != FRACTUS_STATUS_OK) {
        fclose(file);
        return FRACTUS_STATUS_ERROR;
    }

    /* BITMAPINFOHEADER (40 bytes) */
    if (fractus_write_u32_le(file, info_header_size) != FRACTUS_STATUS_OK ||
        fractus_write_u32_le(file, width) != FRACTUS_STATUS_OK ||
        fractus_write_u32_le(file, height) != FRACTUS_STATUS_OK ||
        fractus_write_u16_le(file, 1u) != FRACTUS_STATUS_OK ||
        fractus_write_u16_le(file, 8u) != FRACTUS_STATUS_OK ||
        fractus_write_u32_le(file, 0u) != FRACTUS_STATUS_OK ||
        fractus_write_u32_le(file, image_size) != FRACTUS_STATUS_OK ||
        fractus_write_u32_le(file, 2835u) != FRACTUS_STATUS_OK ||
        fractus_write_u32_le(file, 2835u) != FRACTUS_STATUS_OK ||
        fractus_write_u32_le(file, 256u) != FRACTUS_STATUS_OK ||
        fractus_write_u32_le(file, 0u) != FRACTUS_STATUS_OK) {
        fclose(file);
        return FRACTUS_STATUS_ERROR;
    }

    /* RGBQUAD palette table (256 * 4 = 1024 bytes: Blue, Green, Red, Reserved) */
    for (i = 0u; i < FRACTUS_PALETTE_SIZE; ++i) {
        if (fractus_write_u8(file, palette->entries[i].b) != FRACTUS_STATUS_OK ||
            fractus_write_u8(file, palette->entries[i].g) != FRACTUS_STATUS_OK ||
            fractus_write_u8(file, palette->entries[i].r) != FRACTUS_STATUS_OK ||
            fractus_write_u8(file, 0u) != FRACTUS_STATUS_OK) {
            fclose(file);
            return FRACTUS_STATUS_ERROR;
        }
    }

    /* Bitmap data (bottom-to-top raster scanlines) */
    for (y = (int32_t)height - 1; y >= 0; --y) {
        const uint8_t *row = image->pixels + ((size_t)y * image->pitch_pixels);
        if (fwrite(row, (size_t)width, 1u, file) != 1u) {
            fclose(file);
            return FRACTUS_STATUS_ERROR;
        }
        if (pad_len > 0u) {
            if (fwrite(padding, pad_len, 1u, file) != 1u) {
                fclose(file);
                return FRACTUS_STATUS_ERROR;
            }
        }
    }

    fclose(file);
    return FRACTUS_STATUS_OK;
}

/* Indexed PNG uses zlib stored blocks so the writer needs no deflate compressor.
   The reader accepts that encoding and rejects compressed or non-indexed PNGs. */

static uint32_t fractus_crc32_table[256];
static int fractus_crc32_ready;

static void fractus_crc32_ensure_table(void)
{
    uint32_t i;

    if (fractus_crc32_ready) {
        return;
    }

    for (i = 0u; i < 256u; ++i) {
        uint32_t crc = i;
        int bit;

        for (bit = 0; bit < 8; ++bit) {
            if ((crc & 1u) != 0u) {
                crc = (crc >> 1) ^ 0xedb88320u;
            } else {
                crc >>= 1;
            }
        }
        fractus_crc32_table[i] = crc;
    }

    fractus_crc32_ready = 1;
}

static uint32_t fractus_crc32_type_and_data(const uint8_t type[4], const uint8_t *data, uint32_t length)
{
    uint32_t crc = 0xffffffffu;
    uint32_t i;

    fractus_crc32_ensure_table();
    for (i = 0u; i < 4u; ++i) {
        crc = fractus_crc32_table[(crc ^ type[i]) & 0xffu] ^ (crc >> 8);
    }
    for (i = 0u; i < length; ++i) {
        crc = fractus_crc32_table[(crc ^ data[i]) & 0xffu] ^ (crc >> 8);
    }

    return crc ^ 0xffffffffu;
}

static uint32_t fractus_adler32(const uint8_t *data, size_t length)
{
    uint64_t s1 = 1u;
    uint64_t s2 = 0u;
    size_t i = 0u;

    while (i < length) {
        size_t block = length - i;
        size_t j;

        if (block > 5552u) {
            block = 5552u;
        }
        for (j = 0u; j < block; ++j) {
            s1 += data[i + j];
            s2 += s1;
        }
        s1 %= 65521u;
        s2 %= 65521u;
        i += block;
    }

    return (uint32_t)((s2 << 16) | s1);
}

static fractus_status fractus_write_u32_be(FILE *file, uint32_t value)
{
    uint8_t bytes[4];

    if (file == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    bytes[0] = (uint8_t)((value >> 24) & 0xffu);
    bytes[1] = (uint8_t)((value >> 16) & 0xffu);
    bytes[2] = (uint8_t)((value >> 8) & 0xffu);
    bytes[3] = (uint8_t)(value & 0xffu);
    return (fwrite(bytes, sizeof(bytes), 1u, file) == 1u) ? FRACTUS_STATUS_OK : FRACTUS_STATUS_ERROR;
}

static fractus_status fractus_read_u32_be(FILE *file, uint32_t *value)
{
    uint8_t bytes[4];

    if (file == NULL || value == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }
    if (fread(bytes, sizeof(bytes), 1u, file) != 1u) {
        return FRACTUS_STATUS_ERROR;
    }

    *value = ((uint32_t)bytes[0] << 24) |
             ((uint32_t)bytes[1] << 16) |
             ((uint32_t)bytes[2] << 8) |
             (uint32_t)bytes[3];
    return FRACTUS_STATUS_OK;
}

static int fractus_indexed_dimensions_fit(uint32_t width, uint32_t height)
{
    if (width == 0u || height == 0u || width > 33554431u) {
        return 0;
    }

    return height <= (33554432u / (width + 1u));
}

static fractus_status fractus_png_write_chunk(FILE *file, const uint8_t type[4], const uint8_t *data, uint32_t length)
{
    uint32_t crc;

    if (fractus_write_u32_be(file, length) != FRACTUS_STATUS_OK ||
        fwrite(type, 4u, 1u, file) != 1u) {
        return FRACTUS_STATUS_ERROR;
    }
    if (length > 0u && fwrite(data, length, 1u, file) != 1u) {
        return FRACTUS_STATUS_ERROR;
    }

    crc = fractus_crc32_type_and_data(type, data, length);
    return fractus_write_u32_be(file, crc);
}

static fractus_status fractus_png_save(
    const char *path,
    const fractus_indexed_image *image,
    const fractus_palette *palette)
{
    static const uint8_t signature[8] = {137u, 80u, 78u, 71u, 13u, 10u, 26u, 10u};
    static const uint8_t ihdr_type[4] = {'I', 'H', 'D', 'R'};
    static const uint8_t plte_type[4] = {'P', 'L', 'T', 'E'};
    static const uint8_t idat_type[4] = {'I', 'D', 'A', 'T'};
    static const uint8_t iend_type[4] = {'I', 'E', 'N', 'D'};
    FILE *file = NULL;
    uint8_t *raw = NULL;
    uint8_t *zlib = NULL;
    uint8_t ihdr[13];
    uint8_t plte[FRACTUS_PALETTE_SIZE * 3u];
    uint32_t width;
    uint32_t height;
    uint32_t y;
    uint32_t i;
    size_t raw_size;
    size_t zlib_size;
    size_t offset;
    size_t source_offset;
    size_t remaining;
    uint32_t adler;
    fractus_status status = FRACTUS_STATUS_ERROR;

    if (path == NULL || image == NULL || palette == NULL || !image->initialized) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    width = image->size.width;
    height = image->size.height;
    if (!fractus_indexed_dimensions_fit(width, height) || image->pitch_pixels < width) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    raw_size = (size_t)height * ((size_t)width + 1u);
    raw = (uint8_t *)malloc(raw_size);
    if (raw == NULL) {
        return FRACTUS_STATUS_ERROR;
    }

    for (y = 0u; y < height; ++y) {
        uint8_t *row = raw + ((size_t)y * ((size_t)width + 1u));
        row[0] = 0u;
        memcpy(row + 1, image->pixels + ((size_t)y * image->pitch_pixels), width);
    }

    zlib_size = 2u + ((raw_size + 65534u) / 65535u) * 5u + raw_size + 4u;
    zlib = (uint8_t *)malloc(zlib_size);
    if (zlib == NULL) {
        free(raw);
        return FRACTUS_STATUS_ERROR;
    }

    zlib[0] = 0x78u;
    zlib[1] = 0x01u;
    offset = 2u;
    source_offset = 0u;
    remaining = raw_size;
    while (remaining > 0u) {
        uint32_t chunk_len = (remaining > 65535u) ? 65535u : (uint32_t)remaining;
        uint16_t nlen = (uint16_t)~(uint16_t)chunk_len;
        int final_block = (remaining == (size_t)chunk_len);

        zlib[offset++] = final_block ? 0x01u : 0x00u;
        zlib[offset++] = (uint8_t)(chunk_len & 0xffu);
        zlib[offset++] = (uint8_t)((chunk_len >> 8) & 0xffu);
        zlib[offset++] = (uint8_t)(nlen & 0xffu);
        zlib[offset++] = (uint8_t)((nlen >> 8) & 0xffu);
        memcpy(zlib + offset, raw + source_offset, chunk_len);
        offset += chunk_len;
        source_offset += chunk_len;
        remaining -= chunk_len;
    }

    if (offset + 4u > zlib_size) {
        free(zlib);
        free(raw);
        return FRACTUS_STATUS_ERROR;
    }

    adler = fractus_adler32(raw, raw_size);
    zlib[offset++] = (uint8_t)((adler >> 24) & 0xffu);
    zlib[offset++] = (uint8_t)((adler >> 16) & 0xffu);
    zlib[offset++] = (uint8_t)((adler >> 8) & 0xffu);
    zlib[offset++] = (uint8_t)(adler & 0xffu);

    ihdr[0] = (uint8_t)((width >> 24) & 0xffu);
    ihdr[1] = (uint8_t)((width >> 16) & 0xffu);
    ihdr[2] = (uint8_t)((width >> 8) & 0xffu);
    ihdr[3] = (uint8_t)(width & 0xffu);
    ihdr[4] = (uint8_t)((height >> 24) & 0xffu);
    ihdr[5] = (uint8_t)((height >> 16) & 0xffu);
    ihdr[6] = (uint8_t)((height >> 8) & 0xffu);
    ihdr[7] = (uint8_t)(height & 0xffu);
    ihdr[8] = 8u;
    ihdr[9] = 3u;
    ihdr[10] = 0u;
    ihdr[11] = 0u;
    ihdr[12] = 0u;

    for (i = 0u; i < FRACTUS_PALETTE_SIZE; ++i) {
        plte[i * 3u] = palette->entries[i].r;
        plte[i * 3u + 1u] = palette->entries[i].g;
        plte[i * 3u + 2u] = palette->entries[i].b;
    }

    file = fopen(path, "wb");
    if (file == NULL) {
        free(zlib);
        free(raw);
        return FRACTUS_STATUS_ERROR;
    }

    if (fwrite(signature, sizeof(signature), 1u, file) == 1u &&
        fractus_png_write_chunk(file, ihdr_type, ihdr, 13u) == FRACTUS_STATUS_OK &&
        fractus_png_write_chunk(file, plte_type, plte, (uint32_t)sizeof(plte)) == FRACTUS_STATUS_OK &&
        fractus_png_write_chunk(file, idat_type, zlib, (uint32_t)offset) == FRACTUS_STATUS_OK &&
        fractus_png_write_chunk(file, iend_type, NULL, 0u) == FRACTUS_STATUS_OK) {
        status = FRACTUS_STATUS_OK;
    }

    fclose(file);
    free(zlib);
    free(raw);
    return status;
}

static fractus_status fractus_buffer_append(
    uint8_t **buffer,
    size_t *length,
    size_t *capacity,
    const uint8_t *src,
    size_t src_len,
    size_t max_len)
{
    size_t needed;
    size_t next;
    uint8_t *grown;

    if (src_len > max_len || *length > max_len - src_len) {
        return FRACTUS_STATUS_ERROR;
    }

    needed = *length + src_len;
    if (needed > *capacity) {
        next = (*capacity == 0u) ? 4096u : *capacity;
        while (next < needed) {
            if (next > (SIZE_MAX / 2u)) {
                return FRACTUS_STATUS_ERROR;
            }
            next *= 2u;
        }
        grown = (uint8_t *)realloc(*buffer, next);
        if (grown == NULL) {
            return FRACTUS_STATUS_ERROR;
        }
        *buffer = grown;
        *capacity = next;
    }

    if (src_len > 0u) {
        memcpy(*buffer + *length, src, src_len);
        *length = needed;
    }
    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_png_read_chunk(
    FILE *file,
    uint8_t type[4],
    uint8_t **data,
    uint32_t *length)
{
    uint32_t len;
    uint32_t expected_crc;
    uint8_t *buffer = NULL;

    *data = NULL;
    *length = 0u;
    if (fractus_read_u32_be(file, &len) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }
    if (len > 32u * 1024u * 1024u) {
        return FRACTUS_STATUS_ERROR;
    }
    if (fread(type, 4u, 1u, file) != 1u) {
        return FRACTUS_STATUS_ERROR;
    }
    if (len > 0u) {
        buffer = (uint8_t *)malloc(len);
        if (buffer == NULL) {
            return FRACTUS_STATUS_ERROR;
        }
        if (fread(buffer, len, 1u, file) != 1u) {
            free(buffer);
            return FRACTUS_STATUS_ERROR;
        }
    }
    if (fractus_read_u32_be(file, &expected_crc) != FRACTUS_STATUS_OK ||
        fractus_crc32_type_and_data(type, buffer, len) != expected_crc) {
        free(buffer);
        return FRACTUS_STATUS_ERROR;
    }

    *data = buffer;
    *length = len;
    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_zlib_stored_inflate(
    const uint8_t *src,
    size_t src_size,
    uint8_t *dst,
    size_t dst_size,
    size_t *out_size)
{
    size_t in;
    size_t out = 0u;
    int final_block = 0;
    uint32_t stored_adler;
    uint32_t actual_adler;

    if (src == NULL || dst == NULL || out_size == NULL || src_size < 6u) {
        return FRACTUS_STATUS_ERROR;
    }
    if ((src[0] & 0x0fu) != 8u || (src[1] & 0x20u) != 0u) {
        return FRACTUS_STATUS_UNSUPPORTED;
    }
    if (((((uint32_t)src[0] << 8) | src[1]) % 31u) != 0u) {
        return FRACTUS_STATUS_ERROR;
    }

    in = 2u;
    while (!final_block) {
        uint8_t header;
        uint16_t len;
        uint16_t nlen;

        if (in >= src_size) {
            return FRACTUS_STATUS_ERROR;
        }
        header = src[in++];
        final_block = header & 1u;
        if (((header >> 1) & 0x03u) != 0u) {
            return FRACTUS_STATUS_UNSUPPORTED;
        }
        if (in + 4u > src_size) {
            return FRACTUS_STATUS_ERROR;
        }
        len = (uint16_t)src[in] | (uint16_t)((uint16_t)src[in + 1u] << 8);
        nlen = (uint16_t)src[in + 2u] | (uint16_t)((uint16_t)src[in + 3u] << 8);
        in += 4u;
        if ((uint16_t)(len ^ 0xffffu) != nlen) {
            return FRACTUS_STATUS_ERROR;
        }
        if (in + (size_t)len > src_size || out + (size_t)len > dst_size) {
            return FRACTUS_STATUS_ERROR;
        }
        if (len > 0u) {
            memcpy(dst + out, src + in, len);
        }
        out += len;
        in += len;
    }

    if (in + 4u != src_size) {
        return FRACTUS_STATUS_ERROR;
    }

    stored_adler = ((uint32_t)src[in] << 24) |
                   ((uint32_t)src[in + 1u] << 16) |
                   ((uint32_t)src[in + 2u] << 8) |
                   (uint32_t)src[in + 3u];
    actual_adler = fractus_adler32(dst, out);
    if (stored_adler != actual_adler) {
        return FRACTUS_STATUS_ERROR;
    }

    *out_size = out;
    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_png_load(
    const char *path,
    fractus_indexed_image *image,
    fractus_palette *palette,
    uint16_t *video_mode)
{
    static const uint8_t signature[8] = {137u, 80u, 78u, 71u, 13u, 10u, 26u, 10u};
    FILE *file;
    uint8_t file_signature[8];
    uint8_t *idat = NULL;
    uint8_t *raw = NULL;
    uint8_t *chunk = NULL;
    fractus_indexed_image loaded;
    fractus_palette loaded_palette;
    uint32_t width = 0u;
    uint32_t height = 0u;
    size_t idat_len = 0u;
    size_t idat_cap = 0u;
    size_t raw_size = 0u;
    size_t inflated = 0u;
    int saw_ihdr = 0;
    int saw_plte = 0;
    int saw_idat = 0;
    int idat_closed = 0;
    int saw_iend = 0;
    fractus_status status = FRACTUS_STATUS_OK;
    uint32_t chunks = 0u;

    if (path == NULL || image == NULL || palette == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    memset(&loaded, 0, sizeof(loaded));
    file = fopen(path, "rb");
    if (file == NULL) {
        return FRACTUS_STATUS_ERROR;
    }
    if (fread(file_signature, sizeof(file_signature), 1u, file) != 1u ||
        memcmp(file_signature, signature, sizeof(signature)) != 0) {
        fclose(file);
        return FRACTUS_STATUS_ERROR;
    }
    if (fractus_palette_init_default(&loaded_palette) != FRACTUS_STATUS_OK) {
        fclose(file);
        return FRACTUS_STATUS_ERROR;
    }

    while (status == FRACTUS_STATUS_OK && !saw_iend && chunks < 64u) {
        uint8_t type[4];
        uint32_t length = 0u;

        ++chunks;
        status = fractus_png_read_chunk(file, type, &chunk, &length);
        if (status != FRACTUS_STATUS_OK) {
            break;
        }

        if (memcmp(type, "IHDR", 4) == 0) {
            if (saw_ihdr || length != 13u) {
                status = FRACTUS_STATUS_ERROR;
            } else {
                width = ((uint32_t)chunk[0] << 24) | ((uint32_t)chunk[1] << 16) |
                        ((uint32_t)chunk[2] << 8) | (uint32_t)chunk[3];
                height = ((uint32_t)chunk[4] << 24) | ((uint32_t)chunk[5] << 16) |
                         ((uint32_t)chunk[6] << 8) | (uint32_t)chunk[7];
                if (chunk[8] != 8u || chunk[9] != 3u || chunk[12] != 0u) {
                    status = FRACTUS_STATUS_UNSUPPORTED;
                } else if (chunk[10] != 0u || chunk[11] != 0u || !fractus_indexed_dimensions_fit(width, height)) {
                    status = FRACTUS_STATUS_ERROR;
                } else {
                    saw_ihdr = 1;
                    raw_size = (size_t)height * ((size_t)width + 1u);
                }
            }
        } else if (!saw_ihdr) {
            status = FRACTUS_STATUS_ERROR;
        } else if (memcmp(type, "PLTE", 4) == 0) {
            uint32_t colors;
            uint32_t i;

            if (saw_plte || saw_idat || length == 0u || (length % 3u) != 0u || length > 768u) {
                status = FRACTUS_STATUS_ERROR;
            } else {
                colors = length / 3u;
                for (i = 0u; i < colors; ++i) {
                    loaded_palette.entries[i].r = chunk[i * 3u];
                    loaded_palette.entries[i].g = chunk[i * 3u + 1u];
                    loaded_palette.entries[i].b = chunk[i * 3u + 2u];
                    loaded_palette.entries[i].a = 255u;
                }
                saw_plte = 1;
            }
        } else if (memcmp(type, "IDAT", 4) == 0) {
            if (!saw_plte || idat_closed) {
                status = FRACTUS_STATUS_ERROR;
            } else {
                status = fractus_buffer_append(&idat, &idat_len, &idat_cap, chunk, length, raw_size + 1048576u);
                saw_idat = 1;
            }
        } else if (memcmp(type, "IEND", 4) == 0) {
            if (length != 0u) {
                status = FRACTUS_STATUS_ERROR;
            } else {
                saw_iend = 1;
            }
        } else if ((type[0] & 0x20u) == 0u) {
            status = FRACTUS_STATUS_UNSUPPORTED;
        } else if (saw_idat) {
            idat_closed = 1;
        }

        free(chunk);
        chunk = NULL;
    }

    if (status == FRACTUS_STATUS_OK && (!saw_iend || !saw_idat || idat == NULL)) {
        status = FRACTUS_STATUS_ERROR;
    }

    if (status == FRACTUS_STATUS_OK) {
        raw = (uint8_t *)malloc(raw_size);
        if (raw == NULL) {
            status = FRACTUS_STATUS_ERROR;
        }
    }

    if (status == FRACTUS_STATUS_OK) {
        status = fractus_zlib_stored_inflate(idat, idat_len, raw, raw_size, &inflated);
        if (status == FRACTUS_STATUS_OK && inflated != raw_size) {
            status = FRACTUS_STATUS_ERROR;
        }
    }

    if (status == FRACTUS_STATUS_OK) {
        fractus_size_u32 size;
        uint32_t y;
        size_t stride = (size_t)width + 1u;

        size.width = width;
        size.height = height;
        status = fractus_indexed_image_init(&loaded, size);
        if (status == FRACTUS_STATUS_OK) {
            for (y = 0u; y < height; ++y) {
                if (raw[y * stride] != 0u) {
                    status = FRACTUS_STATUS_UNSUPPORTED;
                    break;
                }
                memcpy(
                    loaded.pixels + ((size_t)y * loaded.pitch_pixels),
                    raw + (y * stride) + 1u,
                    width);
            }
            if (status != FRACTUS_STATUS_OK) {
                fractus_indexed_image_shutdown(&loaded);
            }
        }
    }

    fclose(file);
    free(chunk);
    free(idat);
    free(raw);
    if (status != FRACTUS_STATUS_OK) {
        return status;
    }

    if (video_mode != NULL) {
        *video_mode = 0u;
    }
    *palette = loaded_palette;
    *image = loaded;
    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_legacy_bmp_load(
    const char *path,
    fractus_indexed_image *image,
    fractus_palette *palette,
    uint16_t *video_mode)
{
    FILE *file;
    fractus_indexed_image loaded;
    fractus_palette loaded_palette;
    fractus_size_u32 size;
    uint32_t file_size;
    uint32_t data_offset;
    uint32_t header_size;
    uint32_t compression;
    uint32_t colors_used;
    uint32_t width;
    uint32_t height;
    uint32_t row_stride;
    uint32_t y;
    uint16_t planes;
    uint16_t bit_count;
    int32_t width_s;
    int32_t height_s;
    int top_down = 0;
    long palette_pos;
    uint8_t magic[2];

    if (path == NULL || image == NULL || palette == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    memset(&loaded, 0, sizeof(loaded));
    file = fopen(path, "rb");
    if (file == NULL) {
        return FRACTUS_STATUS_ERROR;
    }

    if (fread(magic, 2u, 1u, file) != 1u || magic[0] != (uint8_t)'B' || magic[1] != (uint8_t)'M' ||
        fractus_read_u32_le(file, &file_size) != FRACTUS_STATUS_OK ||
        fractus_read_u16_le(file, &planes) != FRACTUS_STATUS_OK ||
        fractus_read_u16_le(file, &bit_count) != FRACTUS_STATUS_OK ||
        fractus_read_u32_le(file, &data_offset) != FRACTUS_STATUS_OK ||
        fractus_read_u32_le(file, &header_size) != FRACTUS_STATUS_OK) {
        fclose(file);
        return FRACTUS_STATUS_ERROR;
    }

    if (header_size != 40u) {
        fclose(file);
        return FRACTUS_STATUS_UNSUPPORTED;
    }

    if (fractus_read_u32_le(file, &file_size) != FRACTUS_STATUS_OK) {
        fclose(file);
        return FRACTUS_STATUS_ERROR;
    }
    width_s = (int32_t)file_size;
    if (fractus_read_u32_le(file, &file_size) != FRACTUS_STATUS_OK) {
        fclose(file);
        return FRACTUS_STATUS_ERROR;
    }
    height_s = (int32_t)file_size;
    if (fractus_read_u16_le(file, &planes) != FRACTUS_STATUS_OK ||
        fractus_read_u16_le(file, &bit_count) != FRACTUS_STATUS_OK ||
        fractus_read_u32_le(file, &compression) != FRACTUS_STATUS_OK) {
        fclose(file);
        return FRACTUS_STATUS_ERROR;
    }

    if (width_s <= 0 || height_s == 0 || planes != 1u || bit_count != 8u || compression != 0u) {
        fclose(file);
        return FRACTUS_STATUS_UNSUPPORTED;
    }

    width = (uint32_t)width_s;
    if (height_s < 0) {
        height = (uint32_t)(-(height_s + 1)) + 1u;
        top_down = 1;
    } else {
        height = (uint32_t)height_s;
    }
    if (!fractus_indexed_dimensions_fit(width, height)) {
        fclose(file);
        return FRACTUS_STATUS_ERROR;
    }

    if (fractus_read_u32_le(file, &file_size) != FRACTUS_STATUS_OK ||
        fractus_read_u32_le(file, &file_size) != FRACTUS_STATUS_OK ||
        fractus_read_u32_le(file, &file_size) != FRACTUS_STATUS_OK ||
        fractus_read_u32_le(file, &colors_used) != FRACTUS_STATUS_OK ||
        fractus_read_u32_le(file, &file_size) != FRACTUS_STATUS_OK) {
        fclose(file);
        return FRACTUS_STATUS_ERROR;
    }

    if (colors_used == 0u) {
        colors_used = 256u;
    }
    if (colors_used > 256u) {
        fclose(file);
        return FRACTUS_STATUS_UNSUPPORTED;
    }

    if (fractus_palette_init_default(&loaded_palette) != FRACTUS_STATUS_OK) {
        fclose(file);
        return FRACTUS_STATUS_ERROR;
    }

    for (y = 0u; y < colors_used; ++y) {
        uint8_t b;
        uint8_t g;
        uint8_t r;
        uint8_t reserved;

        if (fractus_read_u8(file, &b) != FRACTUS_STATUS_OK ||
            fractus_read_u8(file, &g) != FRACTUS_STATUS_OK ||
            fractus_read_u8(file, &r) != FRACTUS_STATUS_OK ||
            fractus_read_u8(file, &reserved) != FRACTUS_STATUS_OK) {
            fclose(file);
            return FRACTUS_STATUS_ERROR;
        }
        (void)reserved;
        loaded_palette.entries[y].r = r;
        loaded_palette.entries[y].g = g;
        loaded_palette.entries[y].b = b;
        loaded_palette.entries[y].a = 255u;
    }

    palette_pos = ftell(file);
    if (palette_pos < 0 || (uint32_t)palette_pos > data_offset ||
        fseek(file, (long)data_offset, SEEK_SET) != 0) {
        fclose(file);
        return FRACTUS_STATUS_ERROR;
    }

    size.width = width;
    size.height = height;
    if (fractus_indexed_image_init(&loaded, size) != FRACTUS_STATUS_OK) {
        fclose(file);
        return FRACTUS_STATUS_ERROR;
    }

    row_stride = (width + 3u) & ~3u;
    for (y = 0u; y < height; ++y) {
        uint32_t dest_y = top_down ? y : (height - 1u - y);
        uint8_t *row = loaded.pixels + ((size_t)dest_y * loaded.pitch_pixels);
        uint8_t padding[3];
        size_t pad_len = (size_t)(row_stride - width);

        if (fread(row, width, 1u, file) != 1u ||
            (pad_len > 0u && fread(padding, pad_len, 1u, file) != 1u)) {
            fractus_indexed_image_shutdown(&loaded);
            fclose(file);
            return FRACTUS_STATUS_ERROR;
        }
    }

    fclose(file);
    if (video_mode != NULL) {
        *video_mode = 0u;
    }
    *palette = loaded_palette;
    *image = loaded;
    return FRACTUS_STATUS_OK;
}

fractus_status fractus_indexed_graphic_load(
    const char *path,
    fractus_indexed_image *image,
    fractus_palette *palette,
    uint16_t *video_mode)
{
    if (fractus_formats_has_extension(path, ".png")) {
        return fractus_png_load(path, image, palette, video_mode);
    }
    if (fractus_formats_has_extension(path, ".bmp")) {
        return fractus_legacy_bmp_load(path, image, palette, video_mode);
    }
    return fractus_legacy_graphic_load(path, image, palette, video_mode);
}

fractus_status fractus_indexed_graphic_save(
    const char *path,
    const fractus_indexed_image *image,
    const fractus_palette *palette,
    uint8_t format)
{
    format = fractus_graphic_output_format_normalize(format);
    if (format == (uint8_t)FRACTUS_GRAPHIC_OUTPUT_PNG) {
        return fractus_png_save(path, image, palette);
    }
    if (format == (uint8_t)FRACTUS_GRAPHIC_OUTPUT_BMP) {
        return fractus_legacy_bmp_save(path, image, palette);
    }
    return fractus_legacy_graphic_save(path, image, palette);
}

int fractus_formats_file_exists(const char *path)
{
    FILE *file;

    if (path == NULL) {
        return 0;
    }

    file = fopen(path, "rb");
    if (file == NULL) {
        return 0;
    }

    fclose(file);
    return 1;
}

int fractus_formats_directory_exists(const char *path)
{
    if (path == NULL) {
        return 0;
    }

#if defined(_WIN32)
    {
        DWORD attributes = GetFileAttributesA(path);
        return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    }
#else
    {
        struct stat path_stat;
        return stat(path, &path_stat) == 0 && S_ISDIR(path_stat.st_mode);
    }
#endif
}

char fractus_formats_ascii_lower(char value)
{
    if (value >= 'A' && value <= 'Z') {
        return (char)(value - 'A' + 'a');
    }

    return value;
}

int fractus_formats_has_extension(const char *name, const char *extension)
{
    size_t name_length;
    size_t extension_length;
    size_t i;

    if (name == NULL || extension == NULL) {
        return 0;
    }

    name_length = strlen(name);
    extension_length = strlen(extension);
    if (name_length < extension_length) {
        return 0;
    }

    for (i = 0u; i < extension_length; ++i) {
        if (fractus_formats_ascii_lower(name[name_length - extension_length + i]) !=
            fractus_formats_ascii_lower(extension[i])) {
            return 0;
        }
    }

    return 1;
}

fractus_status fractus_formats_copy_path(
    const char *source,
    char *destination,
    size_t destination_size)
{
    size_t length;

    if (source == NULL || destination == NULL || destination_size == 0u) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    length = strlen(source);
    if (length + 1u > destination_size) {
        return FRACTUS_STATUS_ERROR;
    }

    memcpy(destination, source, length + 1u);
    return FRACTUS_STATUS_OK;
}

fractus_status fractus_formats_join_directory_path(
    const char *directory,
    const char *file_name,
    char separator,
    char *buffer,
    size_t buffer_size)
{
    size_t directory_length;
    size_t file_name_length;
    size_t required_length;

    if (directory == NULL || file_name == NULL || buffer == NULL || buffer_size == 0u) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    directory_length = strlen(directory);
    file_name_length = strlen(file_name);
    required_length = directory_length + 1u + file_name_length + 1u;
    if (required_length > buffer_size) {
        return FRACTUS_STATUS_ERROR;
    }

    memcpy(buffer, directory, directory_length);
    buffer[directory_length] = separator;
    memcpy(buffer + directory_length + 1u, file_name, file_name_length + 1u);
    return FRACTUS_STATUS_OK;
}

fractus_status fractus_formats_join_search_path(
    const char *prefix,
    const char *relative_path,
    char *buffer,
    size_t buffer_size)
{
    int written;

    if (prefix == NULL || relative_path == NULL || buffer == NULL || buffer_size == 0u) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    written = snprintf(buffer, buffer_size, "%s%s", prefix, relative_path);
    if (written < 0 || (size_t)written >= buffer_size) {
        return FRACTUS_STATUS_ERROR;
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_formats_get_parent_relative_path(
    const char *relative_path,
    char *buffer,
    size_t buffer_size)
{
    const char *last_slash;
    const char *last_backslash;
    const char *separator;
    size_t length;

    if (relative_path == NULL || buffer == NULL || buffer_size == 0u) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    last_slash = strrchr(relative_path, '/');
    last_backslash = strrchr(relative_path, '\\');
    separator = last_slash;
    if (last_backslash != NULL && (separator == NULL || last_backslash > separator)) {
        separator = last_backslash;
    }
    if (separator == NULL) {
        return fractus_formats_copy_path("", buffer, buffer_size);
    }

    length = (size_t)(separator - relative_path);
    if (length + 1u > buffer_size) {
        return FRACTUS_STATUS_ERROR;
    }

    memcpy(buffer, relative_path, length);
    buffer[length] = '\0';
    return FRACTUS_STATUS_OK;
}

fractus_status fractus_formats_resolve_legacy_path(
    const fractus_platform_context *platform,
    const char *relative_path,
    char *buffer,
    size_t buffer_size)
{
    size_t i;
    char candidate[512];

    if (platform == NULL || relative_path == NULL || buffer == NULL || buffer_size == 0u) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    for (i = 0u; i < sizeof(fractus_formats_search_prefixes) / sizeof(fractus_formats_search_prefixes[0]); ++i) {
        if (fractus_formats_join_search_path(fractus_formats_search_prefixes[i], relative_path, candidate, sizeof(candidate)) !=
            FRACTUS_STATUS_OK) {
            continue;
        }

        if (fractus_platform_get_resource_path(platform, candidate, buffer, buffer_size) == FRACTUS_STATUS_OK &&
            fractus_formats_file_exists(buffer)) {
            return FRACTUS_STATUS_OK;
        }
    }

    return FRACTUS_STATUS_ERROR;
}

fractus_status fractus_formats_resolve_legacy_directory(
    const fractus_platform_context *platform,
    const char *relative_path,
    char *buffer,
    size_t buffer_size)
{
    size_t i;
    char candidate[512];

    if (platform == NULL || relative_path == NULL || buffer == NULL || buffer_size == 0u) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    for (i = 0u; i < sizeof(fractus_formats_search_prefixes) / sizeof(fractus_formats_search_prefixes[0]); ++i) {
        if (fractus_formats_join_search_path(fractus_formats_search_prefixes[i], relative_path, candidate, sizeof(candidate)) !=
            FRACTUS_STATUS_OK) {
            continue;
        }

        if (fractus_platform_get_resource_path(platform, candidate, buffer, buffer_size) == FRACTUS_STATUS_OK &&
            fractus_formats_directory_exists(buffer)) {
            return FRACTUS_STATUS_OK;
        }
    }

    return FRACTUS_STATUS_ERROR;
}

fractus_status fractus_formats_resolve_legacy_write_path(
    const fractus_platform_context *platform,
    const char *relative_path,
    char *buffer,
    size_t buffer_size)
{
    char parent_relative_path[512];
    char candidate[512];
    char parent_candidate[512];
    char parent_path[512];
    size_t i;

    if (platform == NULL || relative_path == NULL || buffer == NULL || buffer_size == 0u) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_formats_resolve_legacy_path(platform, relative_path, buffer, buffer_size) == FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_OK;
    }

    if (fractus_formats_get_parent_relative_path(relative_path, parent_relative_path, sizeof(parent_relative_path)) !=
        FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    for (i = 0u; i < sizeof(fractus_formats_search_prefixes) / sizeof(fractus_formats_search_prefixes[0]); ++i) {
        if (fractus_formats_join_search_path(fractus_formats_search_prefixes[i], relative_path, candidate, sizeof(candidate)) !=
            FRACTUS_STATUS_OK) {
            continue;
        }

        if (parent_relative_path[0] == '\0') {
            if (fractus_formats_copy_path(fractus_formats_search_prefixes[i], parent_candidate, sizeof(parent_candidate)) !=
                FRACTUS_STATUS_OK) {
                continue;
            }
        } else if (fractus_formats_join_search_path(
                fractus_formats_search_prefixes[i],
                parent_relative_path,
                parent_candidate,
                sizeof(parent_candidate)) != FRACTUS_STATUS_OK) {
            continue;
        }

        if (fractus_platform_get_resource_path(platform, parent_candidate, parent_path, sizeof(parent_path)) == FRACTUS_STATUS_OK &&
            fractus_formats_directory_exists(parent_path) &&
            fractus_platform_get_resource_path(platform, candidate, buffer, buffer_size) == FRACTUS_STATUS_OK) {
            return FRACTUS_STATUS_OK;
        }
    }

    return FRACTUS_STATUS_ERROR;
}

fractus_graphic_metadata fractus_graphic_metadata_from_mandelbrot(
    const fractus_mandelbrot_params *params,
    uint32_t width,
    uint32_t height)
{
    fractus_graphic_metadata meta;
    memset(&meta, 0, sizeof(meta));
    meta.kind = FRACTUS_GRAPHIC_KIND_MANDELBROT;
    meta.width = width;
    meta.height = height;
    if (params != NULL) {
        meta.params.mandelbrot = *params;
    }
    return meta;
}

fractus_graphic_metadata fractus_graphic_metadata_from_mandelbrot_dem(
    const fractus_mandelbrot_dem_params *params,
    uint32_t width,
    uint32_t height)
{
    fractus_graphic_metadata meta;
    memset(&meta, 0, sizeof(meta));
    meta.kind = FRACTUS_GRAPHIC_KIND_MANDELBROT_DEM;
    meta.width = width;
    meta.height = height;
    if (params != NULL) {
        meta.params.mandelbrot_dem = *params;
    }
    return meta;
}

fractus_graphic_metadata fractus_graphic_metadata_from_julia(
    const fractus_julia_params *params,
    uint32_t width,
    uint32_t height)
{
    fractus_graphic_metadata meta;
    memset(&meta, 0, sizeof(meta));
    meta.kind = FRACTUS_GRAPHIC_KIND_JULIA;
    meta.width = width;
    meta.height = height;
    if (params != NULL) {
        meta.params.julia = *params;
    }
    return meta;
}

fractus_graphic_metadata fractus_graphic_metadata_from_julia_dem(
    const fractus_julia_dem_params *params,
    uint32_t width,
    uint32_t height)
{
    fractus_graphic_metadata meta;
    memset(&meta, 0, sizeof(meta));
    meta.kind = FRACTUS_GRAPHIC_KIND_JULIA_DEM;
    meta.width = width;
    meta.height = height;
    if (params != NULL) {
        meta.params.julia_dem = *params;
    }
    return meta;
}

fractus_graphic_metadata fractus_graphic_metadata_from_biomorph(
    const fractus_biomorph_params *params,
    uint32_t width,
    uint32_t height)
{
    fractus_graphic_metadata meta;
    memset(&meta, 0, sizeof(meta));
    meta.kind = FRACTUS_GRAPHIC_KIND_BIOMORPH;
    meta.width = width;
    meta.height = height;
    if (params != NULL) {
        meta.params.biomorph = *params;
    }
    return meta;
}

fractus_graphic_metadata fractus_graphic_metadata_from_plasma_rectangular(
    const fractus_plasma_params *params,
    uint32_t width,
    uint32_t height)
{
    fractus_graphic_metadata meta;
    memset(&meta, 0, sizeof(meta));
    meta.kind = FRACTUS_GRAPHIC_KIND_PLASMA_RECTANGULAR;
    meta.width = width;
    meta.height = height;
    if (params != NULL) {
        meta.params.plasma_rectangular = *params;
    }
    return meta;
}

fractus_graphic_metadata fractus_graphic_metadata_from_plasma_circular(
    const fractus_plasma_circular_params *params,
    uint32_t width,
    uint32_t height)
{
    fractus_graphic_metadata meta;
    memset(&meta, 0, sizeof(meta));
    meta.kind = FRACTUS_GRAPHIC_KIND_PLASMA_CIRCULAR;
    meta.width = width;
    meta.height = height;
    if (params != NULL) {
        meta.params.plasma_circular = *params;
    }
    return meta;
}

fractus_graphic_metadata fractus_graphic_metadata_from_lorenz(
    const fractus_lorenz_params *params,
    uint32_t width,
    uint32_t height)
{
    fractus_graphic_metadata meta;
    memset(&meta, 0, sizeof(meta));
    meta.kind = FRACTUS_GRAPHIC_KIND_LORENZ;
    meta.width = width;
    meta.height = height;
    if (params != NULL) {
        meta.params.lorenz = *params;
    }
    return meta;
}

fractus_status fractus_graphic_metadata_save_json(
    const char *path,
    const fractus_graphic_metadata *metadata)
{
    FILE *file;

    if (path == NULL || metadata == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    file = fopen(path, "w");
    if (file == NULL) {
        return FRACTUS_STATUS_ERROR;
    }

    fprintf(file, "{\n");
    fprintf(file, "  \"application\": \"Fractus-x64\",\n");
    fprintf(file, "  \"version\": 1,\n");

    switch (metadata->kind) {
    case FRACTUS_GRAPHIC_KIND_MANDELBROT:
        fprintf(file, "  \"fractal_type\": \"mandelbrot\",\n");
        fprintf(file, "  \"resolution\": {\n");
        fprintf(file, "    \"width\": %u,\n", (unsigned)metadata->width);
        fprintf(file, "    \"height\": %u\n", (unsigned)metadata->height);
        fprintf(file, "  },\n");
        fprintf(file, "  \"coordinates\": {\n");
        fprintf(file, "    \"xmin\": %.16g,\n", metadata->params.mandelbrot.xmin);
        fprintf(file, "    \"xmax\": %.16g,\n", metadata->params.mandelbrot.xmax);
        fprintf(file, "    \"ymin\": %.16g,\n", metadata->params.mandelbrot.ymin);
        fprintf(file, "    \"ymax\": %.16g\n", metadata->params.mandelbrot.ymax);
        fprintf(file, "  },\n");
        fprintf(file, "  \"parameters\": {\n");
        fprintf(file, "    \"max_iterations\": %u,\n", (unsigned)metadata->params.mandelbrot.max_iterations);
        fprintf(file, "    \"escape_radius_squared\": %.16g,\n", metadata->params.mandelbrot.escape_radius_squared);
        fprintf(file, "    \"inside_color_index\": %u,\n", (unsigned)metadata->params.mandelbrot.inside_color_index);
        fprintf(file, "    \"palette_offset\": %u,\n", (unsigned)metadata->params.mandelbrot.palette_offset);
        fprintf(file, "    \"palette_span\": %u,\n", (unsigned)metadata->params.mandelbrot.palette_span);
        fprintf(file, "    \"color_mode\": \"%s\"\n",
            (metadata->params.mandelbrot.color_mode == FRACTUS_MANDELBROT_COLOR_SMOOTH) ? "smooth" : "escape");
        fprintf(file, "  }\n");
        break;

    case FRACTUS_GRAPHIC_KIND_MANDELBROT_DEM:
        fprintf(file, "  \"fractal_type\": \"mandelbrot_dem\",\n");
        fprintf(file, "  \"resolution\": {\n");
        fprintf(file, "    \"width\": %u,\n", (unsigned)metadata->width);
        fprintf(file, "    \"height\": %u\n", (unsigned)metadata->height);
        fprintf(file, "  },\n");
        fprintf(file, "  \"coordinates\": {\n");
        fprintf(file, "    \"xmin\": %.16g,\n", metadata->params.mandelbrot_dem.xmin);
        fprintf(file, "    \"xmax\": %.16g,\n", metadata->params.mandelbrot_dem.xmax);
        fprintf(file, "    \"ymin\": %.16g,\n", metadata->params.mandelbrot_dem.ymin);
        fprintf(file, "    \"ymax\": %.16g\n", metadata->params.mandelbrot_dem.ymax);
        fprintf(file, "  },\n");
        fprintf(file, "  \"parameters\": {\n");
        fprintf(file, "    \"max_iterations\": %u,\n", (unsigned)metadata->params.mandelbrot_dem.max_iterations);
        fprintf(file, "    \"escape_radius_squared\": %.16g,\n", metadata->params.mandelbrot_dem.escape_radius_squared);
        fprintf(file, "    \"inside_color_index\": %u,\n", (unsigned)metadata->params.mandelbrot_dem.inside_color_index);
        fprintf(file, "    \"palette_offset\": %u,\n", (unsigned)metadata->params.mandelbrot_dem.palette_offset);
        fprintf(file, "    \"palette_span\": %u,\n", (unsigned)metadata->params.mandelbrot_dem.palette_span);
        fprintf(file, "    \"color_mode\": \"%s\"\n",
            (metadata->params.mandelbrot_dem.color_mode == FRACTUS_MANDELBROT_DEM_COLOR_GRADIENT) ? "gradient" : "boundary");
        fprintf(file, "  }\n");
        break;

    case FRACTUS_GRAPHIC_KIND_JULIA:
        fprintf(file, "  \"fractal_type\": \"julia\",\n");
        fprintf(file, "  \"resolution\": {\n");
        fprintf(file, "    \"width\": %u,\n", (unsigned)metadata->width);
        fprintf(file, "    \"height\": %u\n", (unsigned)metadata->height);
        fprintf(file, "  },\n");
        fprintf(file, "  \"coordinates\": {\n");
        fprintf(file, "    \"xmin\": %.16g,\n", metadata->params.julia.xmin);
        fprintf(file, "    \"xmax\": %.16g,\n", metadata->params.julia.xmax);
        fprintf(file, "    \"ymin\": %.16g,\n", metadata->params.julia.ymin);
        fprintf(file, "    \"ymax\": %.16g\n", metadata->params.julia.ymax);
        fprintf(file, "  },\n");
        fprintf(file, "  \"parameters\": {\n");
        fprintf(file, "    \"constant_real\": %.16g,\n", metadata->params.julia.constant_real);
        fprintf(file, "    \"constant_imag\": %.16g,\n", metadata->params.julia.constant_imag);
        fprintf(file, "    \"max_iterations\": %u,\n", (unsigned)metadata->params.julia.max_iterations);
        fprintf(file, "    \"escape_radius_squared\": %.16g,\n", metadata->params.julia.escape_radius_squared);
        fprintf(file, "    \"inside_color_index\": %u,\n", (unsigned)metadata->params.julia.inside_color_index);
        fprintf(file, "    \"palette_offset\": %u,\n", (unsigned)metadata->params.julia.palette_offset);
        fprintf(file, "    \"palette_span\": %u,\n", (unsigned)metadata->params.julia.palette_span);
        fprintf(file, "    \"color_mode\": \"%s\"\n",
            (metadata->params.julia.color_mode == FRACTUS_JULIA_COLOR_SMOOTH) ? "smooth" : "escape");
        fprintf(file, "  }\n");
        break;

    case FRACTUS_GRAPHIC_KIND_JULIA_DEM:
        fprintf(file, "  \"fractal_type\": \"julia_dem\",\n");
        fprintf(file, "  \"resolution\": {\n");
        fprintf(file, "    \"width\": %u,\n", (unsigned)metadata->width);
        fprintf(file, "    \"height\": %u\n", (unsigned)metadata->height);
        fprintf(file, "  },\n");
        fprintf(file, "  \"coordinates\": {\n");
        fprintf(file, "    \"xmin\": %.16g,\n", metadata->params.julia_dem.xmin);
        fprintf(file, "    \"xmax\": %.16g,\n", metadata->params.julia_dem.xmax);
        fprintf(file, "    \"ymin\": %.16g,\n", metadata->params.julia_dem.ymin);
        fprintf(file, "    \"ymax\": %.16g\n", metadata->params.julia_dem.ymax);
        fprintf(file, "  },\n");
        fprintf(file, "  \"parameters\": {\n");
        fprintf(file, "    \"constant_real\": %.16g,\n", metadata->params.julia_dem.constant_real);
        fprintf(file, "    \"constant_imag\": %.16g,\n", metadata->params.julia_dem.constant_imag);
        fprintf(file, "    \"max_iterations\": %u,\n", (unsigned)metadata->params.julia_dem.max_iterations);
        fprintf(file, "    \"escape_radius_squared\": %.16g,\n", metadata->params.julia_dem.escape_radius_squared);
        fprintf(file, "    \"inside_color_index\": %u,\n", (unsigned)metadata->params.julia_dem.inside_color_index);
        fprintf(file, "    \"palette_offset\": %u,\n", (unsigned)metadata->params.julia_dem.palette_offset);
        fprintf(file, "    \"palette_span\": %u,\n", (unsigned)metadata->params.julia_dem.palette_span);
        fprintf(file, "    \"color_mode\": \"%s\"\n",
            (metadata->params.julia_dem.color_mode == FRACTUS_JULIA_DEM_COLOR_GRADIENT) ? "gradient" : "boundary");
        fprintf(file, "  }\n");
        break;

    case FRACTUS_GRAPHIC_KIND_BIOMORPH:
        fprintf(file, "  \"fractal_type\": \"biomorph\",\n");
        fprintf(file, "  \"resolution\": {\n");
        fprintf(file, "    \"width\": %u,\n", (unsigned)metadata->width);
        fprintf(file, "    \"height\": %u\n", (unsigned)metadata->height);
        fprintf(file, "  },\n");
        fprintf(file, "  \"coordinates\": {\n");
        fprintf(file, "    \"xmin\": %.16g,\n", metadata->params.biomorph.xmin);
        fprintf(file, "    \"xmax\": %.16g,\n", metadata->params.biomorph.xmax);
        fprintf(file, "    \"ymin\": %.16g,\n", metadata->params.biomorph.ymin);
        fprintf(file, "    \"ymax\": %.16g\n", metadata->params.biomorph.ymax);
        fprintf(file, "  },\n");
        fprintf(file, "  \"parameters\": {\n");
        fprintf(file, "    \"constant_real\": %.16g,\n", metadata->params.biomorph.constant_real);
        fprintf(file, "    \"constant_imag\": %.16g,\n", metadata->params.biomorph.constant_imag);
        fprintf(file, "    \"max_iterations\": %u,\n", (unsigned)metadata->params.biomorph.max_iterations);
        fprintf(file, "    \"escape_radius_squared\": %.16g,\n", metadata->params.biomorph.escape_radius_squared);
        fprintf(file, "    \"cutoff\": %.16g,\n", metadata->params.biomorph.cutoff);
        fprintf(file, "    \"background_color_index\": %u,\n", (unsigned)metadata->params.biomorph.background_color_index);
        fprintf(file, "    \"palette_offset\": %u,\n", (unsigned)metadata->params.biomorph.palette_offset);
        fprintf(file, "    \"palette_span\": %u,\n", (unsigned)metadata->params.biomorph.palette_span);
        {
            const char *eq_str = "z^2+c";
            const char *trap_str = "re_or_im";
            switch (metadata->params.biomorph.equation) {
            case FRACTUS_BIOMORPH_EQ_Z2: eq_str = "z^2+c"; break;
            case FRACTUS_BIOMORPH_EQ_Z3: eq_str = "z^3+c"; break;
            case FRACTUS_BIOMORPH_EQ_Z4: eq_str = "z^4+c"; break;
            case FRACTUS_BIOMORPH_EQ_Z5: eq_str = "z^5+c"; break;
            case FRACTUS_BIOMORPH_EQ_SIN_Z: eq_str = "sin(z)+c"; break;
            case FRACTUS_BIOMORPH_EQ_EXP_Z: eq_str = "exp(z)+c"; break;
            }
            switch (metadata->params.biomorph.trap_mode) {
            case FRACTUS_BIOMORPH_TRAP_RE_OR_IM: trap_str = "re_or_im"; break;
            case FRACTUS_BIOMORPH_TRAP_RE_AND_IM: trap_str = "re_and_im"; break;
            case FRACTUS_BIOMORPH_TRAP_SOLO_RE: trap_str = "solo_re"; break;
            case FRACTUS_BIOMORPH_TRAP_SOLO_IM: trap_str = "solo_im"; break;
            }
            fprintf(file, "    \"equation\": \"%s\",\n", eq_str);
            fprintf(file, "    \"trap_mode\": \"%s\"\n", trap_str);
        }
        fprintf(file, "  }\n");
        break;

    case FRACTUS_GRAPHIC_KIND_PLASMA_RECTANGULAR:
        fprintf(file, "  \"fractal_type\": \"plasma_rectangular\",\n");
        fprintf(file, "  \"resolution\": {\n");
        fprintf(file, "    \"width\": %u,\n", (unsigned)metadata->width);
        fprintf(file, "    \"height\": %u\n", (unsigned)metadata->height);
        fprintf(file, "  },\n");
        fprintf(file, "  \"parameters\": {\n");
        fprintf(file, "    \"seed\": %u,\n", (unsigned)metadata->params.plasma_rectangular.seed);
        fprintf(file, "    \"dispersion\": %d,\n", (int)metadata->params.plasma_rectangular.dispersion);
        fprintf(file, "    \"palette_offset\": %u,\n", (unsigned)metadata->params.plasma_rectangular.palette_offset);
        fprintf(file, "    \"palette_span\": %u\n", (unsigned)metadata->params.plasma_rectangular.palette_span);
        fprintf(file, "  }\n");
        break;

    case FRACTUS_GRAPHIC_KIND_PLASMA_CIRCULAR:
        fprintf(file, "  \"fractal_type\": \"plasma_circular\",\n");
        fprintf(file, "  \"resolution\": {\n");
        fprintf(file, "    \"width\": %u,\n", (unsigned)metadata->width);
        fprintf(file, "    \"height\": %u\n", (unsigned)metadata->height);
        fprintf(file, "  },\n");
        fprintf(file, "  \"parameters\": {\n");
        fprintf(file, "    \"seed\": %u,\n", (unsigned)metadata->params.plasma_circular.seed);
        fprintf(file, "    \"circle_count\": %d,\n", (int)metadata->params.plasma_circular.circle_count);
        fprintf(file, "    \"max_radius\": %d,\n", (int)metadata->params.plasma_circular.max_radius);
        fprintf(file, "    \"palette_offset\": %u,\n", (unsigned)metadata->params.plasma_circular.palette_offset);
        fprintf(file, "    \"palette_span\": %u\n", (unsigned)metadata->params.plasma_circular.palette_span);
        fprintf(file, "  }\n");
        break;

    case FRACTUS_GRAPHIC_KIND_LORENZ:
        fprintf(file, "  \"fractal_type\": \"lorenz\",\n");
        fprintf(file, "  \"resolution\": {\n");
        fprintf(file, "    \"width\": %u,\n", (unsigned)metadata->width);
        fprintf(file, "    \"height\": %u\n", (unsigned)metadata->height);
        fprintf(file, "  },\n");
        fprintf(file, "  \"parameters\": {\n");
        fprintf(file, "    \"sigma\": %.16g,\n", metadata->params.lorenz.sigma);
        fprintf(file, "    \"rho\": %.16g,\n", metadata->params.lorenz.rho);
        fprintf(file, "    \"beta\": %.16g,\n", metadata->params.lorenz.beta);
        fprintf(file, "    \"dt\": %.16g,\n", metadata->params.lorenz.dt);
        fprintf(file, "    \"iterations\": %u,\n", (unsigned)metadata->params.lorenz.iterations);
        {
            const char *proj_str = "xz";
            switch (metadata->params.lorenz.projection) {
            case FRACTUS_LORENZ_PROJECTION_XZ: proj_str = "xz"; break;
            case FRACTUS_LORENZ_PROJECTION_XY: proj_str = "xy"; break;
            case FRACTUS_LORENZ_PROJECTION_YZ: proj_str = "yz"; break;
            case FRACTUS_LORENZ_PROJECTION_CUSTOM: proj_str = "custom"; break;
            }
            fprintf(file, "    \"projection\": \"%s\",\n", proj_str);
        }
        fprintf(file, "    \"rot_x\": %.16g,\n", metadata->params.lorenz.rot_x);
        fprintf(file, "    \"rot_y\": %.16g,\n", metadata->params.lorenz.rot_y);
        fprintf(file, "    \"rot_z\": %.16g,\n", metadata->params.lorenz.rot_z);
        fprintf(file, "    \"palette_offset\": %u,\n", (unsigned)metadata->params.lorenz.palette_offset);
        fprintf(file, "    \"palette_span\": %u\n", (unsigned)metadata->params.lorenz.palette_span);
        fprintf(file, "  }\n");
        break;

    case FRACTUS_GRAPHIC_KIND_UNKNOWN:
    default:
        fprintf(file, "  \"fractal_type\": \"unknown\",\n");
        fprintf(file, "  \"resolution\": {\n");
        fprintf(file, "    \"width\": %u,\n", (unsigned)metadata->width);
        fprintf(file, "    \"height\": %u\n", (unsigned)metadata->height);
        fprintf(file, "  }\n");
        break;
    }

    fprintf(file, "}\n");
    fclose(file);
    return FRACTUS_STATUS_OK;
}

static int fractus_json_parse_string(const char *json, const char *key, char *out, size_t out_size)
{
    char search_key[128];
    const char *pos;
    const char *start;
    const char *end;
    size_t len;

    if (json == NULL || key == NULL || out == NULL || out_size == 0u) {
        return 0;
    }

    (void)snprintf(search_key, sizeof(search_key), "\"%s\"", key);
    pos = strstr(json, search_key);
    if (pos == NULL) {
        return 0;
    }
    pos += strlen(search_key);
    while (*pos == ' ' || *pos == '\t' || *pos == '\r' || *pos == '\n' || *pos == ':') {
        ++pos;
    }
    if (*pos != '"') {
        return 0;
    }
    start = pos + 1;
    end = strchr(start, '"');
    if (end == NULL) {
        return 0;
    }
    len = (size_t)(end - start);
    if (len + 1u > out_size) {
        len = out_size - 1u;
    }
    memcpy(out, start, len);
    out[len] = '\0';
    return 1;
}

static int fractus_json_parse_double(const char *json, const char *key, double *out)
{
    char search_key[128];
    const char *pos;
    char *endptr;

    if (json == NULL || key == NULL || out == NULL) {
        return 0;
    }

    (void)snprintf(search_key, sizeof(search_key), "\"%s\"", key);
    pos = strstr(json, search_key);
    if (pos == NULL) {
        return 0;
    }
    pos += strlen(search_key);
    while (*pos == ' ' || *pos == '\t' || *pos == '\r' || *pos == '\n' || *pos == ':') {
        ++pos;
    }
    *out = strtod(pos, &endptr);
    return (endptr != pos);
}

static int fractus_json_parse_uint32(const char *json, const char *key, uint32_t *out)
{
    double val;
    if (fractus_json_parse_double(json, key, &val)) {
        *out = (uint32_t)val;
        return 1;
    }
    return 0;
}

static int fractus_json_parse_int32(const char *json, const char *key, int32_t *out)
{
    double val;
    if (fractus_json_parse_double(json, key, &val)) {
        *out = (int32_t)val;
        return 1;
    }
    return 0;
}

static int fractus_json_parse_uint8(const char *json, const char *key, uint8_t *out)
{
    double val;
    if (fractus_json_parse_double(json, key, &val)) {
        *out = (uint8_t)val;
        return 1;
    }
    return 0;
}

fractus_status fractus_graphic_metadata_load_json(
    const char *path,
    fractus_graphic_metadata *metadata)
{
    FILE *file;
    char buffer[4096];
    size_t bytes_read;
    char type_str[64];
    char mode_str[64];
    char eq_str[64];
    char trap_str[64];
    char proj_str[64];

    if (path == NULL || metadata == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    file = fopen(path, "r");
    if (file == NULL) {
        return FRACTUS_STATUS_ERROR;
    }

    bytes_read = fread(buffer, 1, sizeof(buffer) - 1, file);
    fclose(file);
    if (bytes_read == 0u) {
        return FRACTUS_STATUS_ERROR;
    }
    buffer[bytes_read] = '\0';

    memset(metadata, 0, sizeof(*metadata));
    metadata->kind = FRACTUS_GRAPHIC_KIND_UNKNOWN;

    if (!fractus_json_parse_string(buffer, "fractal_type", type_str, sizeof(type_str))) {
        return FRACTUS_STATUS_ERROR;
    }

    fractus_json_parse_uint32(buffer, "width", &metadata->width);
    fractus_json_parse_uint32(buffer, "height", &metadata->height);

    if (strcmp(type_str, "mandelbrot") == 0) {
        metadata->kind = FRACTUS_GRAPHIC_KIND_MANDELBROT;
        fractus_json_parse_double(buffer, "xmin", &metadata->params.mandelbrot.xmin);
        fractus_json_parse_double(buffer, "xmax", &metadata->params.mandelbrot.xmax);
        fractus_json_parse_double(buffer, "ymin", &metadata->params.mandelbrot.ymin);
        fractus_json_parse_double(buffer, "ymax", &metadata->params.mandelbrot.ymax);
        fractus_json_parse_uint32(buffer, "max_iterations", &metadata->params.mandelbrot.max_iterations);
        fractus_json_parse_double(buffer, "escape_radius_squared", &metadata->params.mandelbrot.escape_radius_squared);
        fractus_json_parse_uint8(buffer, "inside_color_index", &metadata->params.mandelbrot.inside_color_index);
        fractus_json_parse_uint8(buffer, "palette_offset", &metadata->params.mandelbrot.palette_offset);
        fractus_json_parse_uint8(buffer, "palette_span", &metadata->params.mandelbrot.palette_span);
        if (fractus_json_parse_string(buffer, "color_mode", mode_str, sizeof(mode_str))) {
            if (strcmp(mode_str, "smooth") == 0) {
                metadata->params.mandelbrot.color_mode = FRACTUS_MANDELBROT_COLOR_SMOOTH;
            } else {
                metadata->params.mandelbrot.color_mode = FRACTUS_MANDELBROT_COLOR_ESCAPE;
            }
        }
    } else if (strcmp(type_str, "mandelbrot_dem") == 0) {
        metadata->kind = FRACTUS_GRAPHIC_KIND_MANDELBROT_DEM;
        fractus_json_parse_double(buffer, "xmin", &metadata->params.mandelbrot_dem.xmin);
        fractus_json_parse_double(buffer, "xmax", &metadata->params.mandelbrot_dem.xmax);
        fractus_json_parse_double(buffer, "ymin", &metadata->params.mandelbrot_dem.ymin);
        fractus_json_parse_double(buffer, "ymax", &metadata->params.mandelbrot_dem.ymax);
        fractus_json_parse_uint32(buffer, "max_iterations", &metadata->params.mandelbrot_dem.max_iterations);
        fractus_json_parse_double(buffer, "escape_radius_squared", &metadata->params.mandelbrot_dem.escape_radius_squared);
        fractus_json_parse_uint8(buffer, "inside_color_index", &metadata->params.mandelbrot_dem.inside_color_index);
        fractus_json_parse_uint8(buffer, "palette_offset", &metadata->params.mandelbrot_dem.palette_offset);
        fractus_json_parse_uint8(buffer, "palette_span", &metadata->params.mandelbrot_dem.palette_span);
        if (fractus_json_parse_string(buffer, "color_mode", mode_str, sizeof(mode_str))) {
            if (strcmp(mode_str, "gradient") == 0) {
                metadata->params.mandelbrot_dem.color_mode = FRACTUS_MANDELBROT_DEM_COLOR_GRADIENT;
            } else {
                metadata->params.mandelbrot_dem.color_mode = FRACTUS_MANDELBROT_DEM_COLOR_BOUNDARY;
            }
        }
    } else if (strcmp(type_str, "julia") == 0) {
        metadata->kind = FRACTUS_GRAPHIC_KIND_JULIA;
        fractus_json_parse_double(buffer, "xmin", &metadata->params.julia.xmin);
        fractus_json_parse_double(buffer, "xmax", &metadata->params.julia.xmax);
        fractus_json_parse_double(buffer, "ymin", &metadata->params.julia.ymin);
        fractus_json_parse_double(buffer, "ymax", &metadata->params.julia.ymax);
        fractus_json_parse_double(buffer, "constant_real", &metadata->params.julia.constant_real);
        fractus_json_parse_double(buffer, "constant_imag", &metadata->params.julia.constant_imag);
        fractus_json_parse_uint32(buffer, "max_iterations", &metadata->params.julia.max_iterations);
        fractus_json_parse_double(buffer, "escape_radius_squared", &metadata->params.julia.escape_radius_squared);
        fractus_json_parse_uint8(buffer, "inside_color_index", &metadata->params.julia.inside_color_index);
        fractus_json_parse_uint8(buffer, "palette_offset", &metadata->params.julia.palette_offset);
        fractus_json_parse_uint8(buffer, "palette_span", &metadata->params.julia.palette_span);
        if (fractus_json_parse_string(buffer, "color_mode", mode_str, sizeof(mode_str))) {
            if (strcmp(mode_str, "smooth") == 0) {
                metadata->params.julia.color_mode = FRACTUS_JULIA_COLOR_SMOOTH;
            } else {
                metadata->params.julia.color_mode = FRACTUS_JULIA_COLOR_ESCAPE;
            }
        }
    } else if (strcmp(type_str, "julia_dem") == 0) {
        metadata->kind = FRACTUS_GRAPHIC_KIND_JULIA_DEM;
        fractus_json_parse_double(buffer, "xmin", &metadata->params.julia_dem.xmin);
        fractus_json_parse_double(buffer, "xmax", &metadata->params.julia_dem.xmax);
        fractus_json_parse_double(buffer, "ymin", &metadata->params.julia_dem.ymin);
        fractus_json_parse_double(buffer, "ymax", &metadata->params.julia_dem.ymax);
        fractus_json_parse_double(buffer, "constant_real", &metadata->params.julia_dem.constant_real);
        fractus_json_parse_double(buffer, "constant_imag", &metadata->params.julia_dem.constant_imag);
        fractus_json_parse_uint32(buffer, "max_iterations", &metadata->params.julia_dem.max_iterations);
        fractus_json_parse_double(buffer, "escape_radius_squared", &metadata->params.julia_dem.escape_radius_squared);
        fractus_json_parse_uint8(buffer, "inside_color_index", &metadata->params.julia_dem.inside_color_index);
        fractus_json_parse_uint8(buffer, "palette_offset", &metadata->params.julia_dem.palette_offset);
        fractus_json_parse_uint8(buffer, "palette_span", &metadata->params.julia_dem.palette_span);
        if (fractus_json_parse_string(buffer, "color_mode", mode_str, sizeof(mode_str))) {
            if (strcmp(mode_str, "gradient") == 0) {
                metadata->params.julia_dem.color_mode = FRACTUS_JULIA_DEM_COLOR_GRADIENT;
            } else {
                metadata->params.julia_dem.color_mode = FRACTUS_JULIA_DEM_COLOR_BOUNDARY;
            }
        }
    } else if (strcmp(type_str, "biomorph") == 0) {
        metadata->kind = FRACTUS_GRAPHIC_KIND_BIOMORPH;
        fractus_json_parse_double(buffer, "xmin", &metadata->params.biomorph.xmin);
        fractus_json_parse_double(buffer, "xmax", &metadata->params.biomorph.xmax);
        fractus_json_parse_double(buffer, "ymin", &metadata->params.biomorph.ymin);
        fractus_json_parse_double(buffer, "ymax", &metadata->params.biomorph.ymax);
        fractus_json_parse_double(buffer, "constant_real", &metadata->params.biomorph.constant_real);
        fractus_json_parse_double(buffer, "constant_imag", &metadata->params.biomorph.constant_imag);
        fractus_json_parse_uint32(buffer, "max_iterations", &metadata->params.biomorph.max_iterations);
        fractus_json_parse_double(buffer, "escape_radius_squared", &metadata->params.biomorph.escape_radius_squared);
        fractus_json_parse_double(buffer, "cutoff", &metadata->params.biomorph.cutoff);
        fractus_json_parse_uint8(buffer, "background_color_index", &metadata->params.biomorph.background_color_index);
        fractus_json_parse_uint8(buffer, "palette_offset", &metadata->params.biomorph.palette_offset);
        fractus_json_parse_uint8(buffer, "palette_span", &metadata->params.biomorph.palette_span);
        if (fractus_json_parse_string(buffer, "equation", eq_str, sizeof(eq_str))) {
            if (strcmp(eq_str, "z^3+c") == 0) {
                metadata->params.biomorph.equation = FRACTUS_BIOMORPH_EQ_Z3;
            } else if (strcmp(eq_str, "z^4+c") == 0) {
                metadata->params.biomorph.equation = FRACTUS_BIOMORPH_EQ_Z4;
            } else if (strcmp(eq_str, "z^5+c") == 0) {
                metadata->params.biomorph.equation = FRACTUS_BIOMORPH_EQ_Z5;
            } else if (strcmp(eq_str, "sin(z)+c") == 0) {
                metadata->params.biomorph.equation = FRACTUS_BIOMORPH_EQ_SIN_Z;
            } else if (strcmp(eq_str, "exp(z)+c") == 0) {
                metadata->params.biomorph.equation = FRACTUS_BIOMORPH_EQ_EXP_Z;
            } else {
                metadata->params.biomorph.equation = FRACTUS_BIOMORPH_EQ_Z2;
            }
        }
        if (fractus_json_parse_string(buffer, "trap_mode", trap_str, sizeof(trap_str))) {
            if (strcmp(trap_str, "re_and_im") == 0) {
                metadata->params.biomorph.trap_mode = FRACTUS_BIOMORPH_TRAP_RE_AND_IM;
            } else if (strcmp(trap_str, "solo_re") == 0) {
                metadata->params.biomorph.trap_mode = FRACTUS_BIOMORPH_TRAP_SOLO_RE;
            } else if (strcmp(trap_str, "solo_im") == 0) {
                metadata->params.biomorph.trap_mode = FRACTUS_BIOMORPH_TRAP_SOLO_IM;
            } else {
                metadata->params.biomorph.trap_mode = FRACTUS_BIOMORPH_TRAP_RE_OR_IM;
            }
        }
    } else if (strcmp(type_str, "plasma_rectangular") == 0) {
        metadata->kind = FRACTUS_GRAPHIC_KIND_PLASMA_RECTANGULAR;
        fractus_json_parse_uint32(buffer, "seed", &metadata->params.plasma_rectangular.seed);
        fractus_json_parse_int32(buffer, "dispersion", &metadata->params.plasma_rectangular.dispersion);
        fractus_json_parse_uint8(buffer, "palette_offset", &metadata->params.plasma_rectangular.palette_offset);
        fractus_json_parse_uint8(buffer, "palette_span", &metadata->params.plasma_rectangular.palette_span);
    } else if (strcmp(type_str, "plasma_circular") == 0) {
        metadata->kind = FRACTUS_GRAPHIC_KIND_PLASMA_CIRCULAR;
        fractus_json_parse_uint32(buffer, "seed", &metadata->params.plasma_circular.seed);
        fractus_json_parse_int32(buffer, "circle_count", &metadata->params.plasma_circular.circle_count);
        fractus_json_parse_int32(buffer, "max_radius", &metadata->params.plasma_circular.max_radius);
        fractus_json_parse_uint8(buffer, "palette_offset", &metadata->params.plasma_circular.palette_offset);
        fractus_json_parse_uint8(buffer, "palette_span", &metadata->params.plasma_circular.palette_span);
    } else if (strcmp(type_str, "lorenz") == 0) {
        metadata->kind = FRACTUS_GRAPHIC_KIND_LORENZ;
        fractus_json_parse_double(buffer, "sigma", &metadata->params.lorenz.sigma);
        fractus_json_parse_double(buffer, "rho", &metadata->params.lorenz.rho);
        fractus_json_parse_double(buffer, "beta", &metadata->params.lorenz.beta);
        fractus_json_parse_double(buffer, "dt", &metadata->params.lorenz.dt);
        fractus_json_parse_uint32(buffer, "iterations", &metadata->params.lorenz.iterations);
        if (fractus_json_parse_string(buffer, "projection", proj_str, sizeof(proj_str))) {
            if (strcmp(proj_str, "xy") == 0) {
                metadata->params.lorenz.projection = FRACTUS_LORENZ_PROJECTION_XY;
            } else if (strcmp(proj_str, "yz") == 0) {
                metadata->params.lorenz.projection = FRACTUS_LORENZ_PROJECTION_YZ;
            } else if (strcmp(proj_str, "custom") == 0) {
                metadata->params.lorenz.projection = FRACTUS_LORENZ_PROJECTION_CUSTOM;
            } else {
                metadata->params.lorenz.projection = FRACTUS_LORENZ_PROJECTION_XZ;
            }
        }
        fractus_json_parse_double(buffer, "rot_x", &metadata->params.lorenz.rot_x);
        fractus_json_parse_double(buffer, "rot_y", &metadata->params.lorenz.rot_y);
        fractus_json_parse_double(buffer, "rot_z", &metadata->params.lorenz.rot_z);
        fractus_json_parse_uint8(buffer, "palette_offset", &metadata->params.lorenz.palette_offset);
        fractus_json_parse_uint8(buffer, "palette_span", &metadata->params.lorenz.palette_span);
    }

    return FRACTUS_STATUS_OK;
}
