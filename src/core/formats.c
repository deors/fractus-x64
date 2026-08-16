#include "core/formats.h"

#if defined(_WIN32)
#include <windows.h>
#else
#include <sys/stat.h>
#endif

#include <stdio.h>
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
    return (uint8_t)((value * 63u) / 255u);
}

static uint8_t fractus_from_legacy_6bit(uint8_t value)
{
    return (uint8_t)((value * 255u) / 63u);
}

static uint8_t fractus_valid_drawing_video_mode(uint8_t value)
{
    return (value < FRACTUS_DRAWING_VIDEO_MODE_COUNT) ? value : (uint8_t)FRACTUS_DRAWING_VIDEO_MODE_WINDOW_640X480;
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

fractus_status fractus_legacy_config_init_default(fractus_legacy_config *config)
{
    fractus_palette palette;
    uint32_t i;

    if (config == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    memset(config, 0, sizeof(*config));
    for (i = 0u; i < FRACTUS_LEGACY_DETVIDEO_COUNT; ++i) {
        config->detvideo[i] = 1u;
    }
    config->drawing_video_mode = (uint8_t)FRACTUS_DRAWING_VIDEO_MODE_WINDOW_640X480;
    config->iterations = 240;
    config->escape_radius_squared = 4;
    config->biomorph_escape_radius_squared = 1000;

    if (fractus_palette_init_default(&palette) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    for (i = 0u; i < FRACTUS_LEGACY_PALETTE_DATA_COUNT; ++i) {
        config->palette[i] = palette.entries[i + 16u];
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_legacy_config_load(
    const char *path,
    fractus_legacy_config *config)
{
    FILE *file;
    uint8_t drawing_video_mode;
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

    if (fractus_write_u8(file, fractus_valid_drawing_video_mode(config->drawing_video_mode)) != FRACTUS_STATUS_OK) {
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

    for (i = 16u; i < FRACTUS_PALETTE_SIZE; ++i) {
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

    for (i = 16u; i < FRACTUS_PALETTE_SIZE; ++i) {
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
    for (video_mode = 0u; video_mode <= 11u; ++video_mode) {
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
