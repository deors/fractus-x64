#include "ui/font.h"

#include "platform/graphics.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FRACTUS_FONT_GLYPH_COUNT 97u

typedef struct fractus_font_archive_slice {
    size_t offset;
    size_t size;
} fractus_font_archive_slice;

static const uint8_t fractus_font_magic[13] = {
    'M', 'H', 'I', 'D', 'S', ' ', 'F', 'u', 'e', 'n', 't', 'e', 0x1a
};
static const fractus_font_archive_slice fractus_font_archive_layout[FRACTUS_FONT_COUNT] = {
    {13685u, 33891u},
    {0u, 13685u},
    {47576u, 4746u},
    {52322u, 11948u}
};
static const char *fractus_font_face_names[FRACTUS_FONT_COUNT] = {
    "france",
    "arial",
    "small",
    "courier"
};

static void fractus_font_log(const char *message)
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

static uint16_t fractus_read_u16_le_from_memory(const uint8_t *data)
{
    return (uint16_t)((uint16_t)data[0] | ((uint16_t)data[1] << 8u));
}

static uint32_t fractus_read_u32_le_from_memory(const uint8_t *data)
{
    return (uint32_t)data[0] |
        ((uint32_t)data[1] << 8u) |
        ((uint32_t)data[2] << 16u) |
        ((uint32_t)data[3] << 24u);
}

static int fractus_font_glyph_index(char c, uint32_t *index)
{
    if (index == NULL) {
        return 0;
    }

    if (c >= 33 && c <= 47) {
        *index = (uint32_t)(c - 33 + 64);
        return 1;
    }

    if (c >= 48 && c <= 57) {
        *index = (uint32_t)(c - 48 + 27);
        return 1;
    }

    if (c >= 58 && c <= 64) {
        *index = (uint32_t)(c - 58 + 79);
        return 1;
    }

    if (c >= 65 && c <= 78) {
        *index = (uint32_t)(c - 65);
        return 1;
    }

    if (c >= 79 && c <= 90) {
        *index = (uint32_t)(c - 79 + 15);
        return 1;
    }

    if (c >= 91 && c <= 93) {
        *index = (uint32_t)(c - 91 + 86);
        return 1;
    }

    if (c >= 97 && c <= 110) {
        *index = (uint32_t)(c - 97 + 37);
        return 1;
    }

    if (c >= 111 && c <= 122) {
        *index = (uint32_t)(c - 111 + 52);
        return 1;
    }

    if (c >= 123 && c <= 125) {
        *index = (uint32_t)(c - 123 + 89);
        return 1;
    }

    return 0;
}

static fractus_status fractus_font_face_load_memory(
    fractus_font_face *face,
    const uint8_t *data,
    size_t data_size)
{
    size_t cursor;
    uint32_t i;
    char log_buffer[160];

    if (face == NULL || data == NULL || data_size < 13u + 2u + 2u) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (memcmp(data, fractus_font_magic, sizeof(fractus_font_magic)) != 0) {
        fractus_font_log("font: magic mismatch");
        return FRACTUS_STATUS_ERROR;
    }

    face->data = (uint8_t *)malloc(data_size);
    if (face->data == NULL) {
        return FRACTUS_STATUS_ERROR;
    }

    memcpy(face->data, data, data_size);
    face->data_size = data_size;

    cursor = sizeof(fractus_font_magic);
    face->file_type = fractus_read_u16_le_from_memory(face->data + cursor);
    cursor += 2u;
    face->glyph_height = fractus_read_u16_le_from_memory(face->data + cursor);
    cursor += 2u;

    if (face->file_type > 1u || face->glyph_height == 0u) {
        snprintf(
            log_buffer,
            sizeof(log_buffer),
            "font: invalid header type=%u height=%u",
            face->file_type,
            face->glyph_height);
        fractus_font_log(log_buffer);
        free(face->data);
        face->data = NULL;
        face->data_size = 0u;
        return FRACTUS_STATUS_ERROR;
    }

    if (cursor + FRACTUS_FONT_GLYPH_COUNT * 2u + FRACTUS_FONT_GLYPH_COUNT * 4u > data_size) {
        fractus_font_log("font: table area exceeds slice size");
        free(face->data);
        face->data = NULL;
        face->data_size = 0u;
        return FRACTUS_STATUS_ERROR;
    }

    for (i = 0u; i < FRACTUS_FONT_GLYPH_COUNT; ++i) {
        face->glyph_widths[i] = fractus_read_u16_le_from_memory(face->data + cursor);
        cursor += 2u;
        face->glyph_offsets[i] = fractus_read_u32_le_from_memory(face->data + cursor);
        cursor += 4u;
    }

    for (i = 0u; i < FRACTUS_FONT_GLYPH_COUNT; ++i) {
        size_t glyph_size = (size_t)face->glyph_widths[i] * face->glyph_height;

        if (glyph_size == 0u) {
            continue;
        }

        if (face->glyph_offsets[i] > data_size ||
            glyph_size > data_size ||
            face->glyph_offsets[i] + glyph_size > data_size) {
            snprintf(
                log_buffer,
                sizeof(log_buffer),
                "font: glyph bounds invalid index=%u width=%u height=%u offset=%u size=%zu slice=%zu",
                i,
                face->glyph_widths[i],
                face->glyph_height,
                face->glyph_offsets[i],
                glyph_size,
                data_size);
            fractus_font_log(log_buffer);
            free(face->data);
            face->data = NULL;
            face->data_size = 0u;
            return FRACTUS_STATUS_ERROR;
        }
    }

    face->initialized = 1;
    return FRACTUS_STATUS_OK;
}

fractus_status fractus_font_library_load_archive(
    fractus_font_library *library,
    const char *path)
{
    FILE *file;
    size_t archive_size;
    uint8_t *archive;
    uint32_t i;
    uint32_t loaded_faces;
    char log_buffer[160];

    if (library == NULL || path == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    memset(library, 0, sizeof(*library));

    file = fopen(path, "rb");
    if (file == NULL) {
        return FRACTUS_STATUS_ERROR;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return FRACTUS_STATUS_ERROR;
    }

    archive_size = (size_t)ftell(file);
    if (fseek(file, 0, SEEK_SET) != 0 || archive_size == 0u) {
        fclose(file);
        return FRACTUS_STATUS_ERROR;
    }

    archive = (uint8_t *)malloc(archive_size);
    if (archive == NULL) {
        fclose(file);
        return FRACTUS_STATUS_ERROR;
    }

    if (fread(archive, archive_size, 1u, file) != 1u) {
        free(archive);
        fclose(file);
        return FRACTUS_STATUS_ERROR;
    }

    fclose(file);

    loaded_faces = 0u;
    for (i = 0u; i < FRACTUS_FONT_COUNT; ++i) {
        const fractus_font_archive_slice slice = fractus_font_archive_layout[i];

        if (slice.offset + slice.size > archive_size) {
            snprintf(
                log_buffer,
                sizeof(log_buffer),
                "font: slice out of range for %s (offset=%zu size=%zu archive=%zu)",
                fractus_font_face_names[i],
                slice.offset,
                slice.size,
                archive_size);
            fractus_font_log(log_buffer);
            continue;
        }

        if (fractus_font_face_load_memory(&library->faces[i], archive + slice.offset, slice.size) == FRACTUS_STATUS_OK) {
            ++loaded_faces;
            snprintf(
                log_buffer,
                sizeof(log_buffer),
                "font: loaded %s (size=%zu height=%u type=%u)",
                fractus_font_face_names[i],
                slice.size,
                library->faces[i].glyph_height,
                library->faces[i].file_type);
            fractus_font_log(log_buffer);
        } else {
            snprintf(
                log_buffer,
                sizeof(log_buffer),
                "font: failed %s slice parse (offset=%zu size=%zu)",
                fractus_font_face_names[i],
                slice.offset,
                slice.size);
            fractus_font_log(log_buffer);
        }
    }

    free(archive);
    library->initialized = (loaded_faces > 0u);
    return (loaded_faces > 0u) ? FRACTUS_STATUS_OK : FRACTUS_STATUS_ERROR;
}

void fractus_font_library_shutdown(fractus_font_library *library)
{
    uint32_t i;

    if (library == NULL) {
        return;
    }

    for (i = 0u; i < FRACTUS_FONT_COUNT; ++i) {
        free(library->faces[i].data);
        library->faces[i].data = NULL;
        library->faces[i].data_size = 0u;
        library->faces[i].initialized = 0;
        library->faces[i].file_type = 0u;
        library->faces[i].glyph_height = 0u;
    }

    library->initialized = 0;
}

fractus_status fractus_font_measure_text(
    const fractus_font_library *library,
    fractus_font_kind kind,
    const char *text,
    int32_t *width,
    int32_t *height)
{
    const fractus_font_face *face;
    size_t i;
    int32_t measured_width;

    if (library == NULL || !library->initialized || text == NULL ||
        kind < 0 || kind >= FRACTUS_FONT_COUNT) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    face = &library->faces[kind];
    if (!face->initialized) {
        return FRACTUS_STATUS_ERROR;
    }

    measured_width = 0;
    for (i = 0u; text[i] != '\0'; ++i) {
        uint32_t glyph_index;

        if (text[i] == ' ') {
            measured_width += (int32_t)face->glyph_widths[8] + ((kind == FRACTUS_FONT_COURIER) ? 0 : 1);
            continue;
        }

        if (fractus_font_glyph_index(text[i], &glyph_index)) {
            measured_width += (int32_t)face->glyph_widths[glyph_index] + 1;
        }
    }

    if (measured_width > 0) {
        measured_width -= 1;
    }

    if (width != NULL) {
        *width = measured_width;
    }

    if (height != NULL) {
        *height = (int32_t)face->glyph_height;
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_font_draw_text(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *library,
    fractus_font_kind kind,
    int32_t x,
    int32_t y,
    uint8_t color_index,
    const char *text)
{
    const fractus_font_face *face;
    size_t n;

    if (framebuffer == NULL || library == NULL || text == NULL ||
        !framebuffer->initialized || !library->initialized ||
        kind < 0 || kind >= FRACTUS_FONT_COUNT) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    face = &library->faces[kind];
    if (!face->initialized) {
        return FRACTUS_STATUS_ERROR;
    }

    for (n = 0u; text[n] != '\0'; ++n) {
        uint32_t glyph_index;
        uint16_t glyph_width;
        uint16_t row;
        uint16_t column;

        if (text[n] == ' ') {
            x += (int32_t)face->glyph_widths[8] + ((kind == FRACTUS_FONT_COURIER) ? 0 : 1);
            continue;
        }

        if (!fractus_font_glyph_index(text[n], &glyph_index)) {
            continue;
        }

        glyph_width = face->glyph_widths[glyph_index];
        for (row = 0u; row < face->glyph_height; ++row) {
            for (column = 0u; column < glyph_width; ++column) {
                const size_t pixel_offset =
                    (size_t)face->glyph_offsets[glyph_index] + (size_t)row * glyph_width + column;
                const uint8_t pixel = face->data[pixel_offset];

                if (face->file_type == 0u) {
                    if (pixel != 0u) {
                        (void)fractus_graphics_put_pixel(framebuffer, x + column, y + row, color_index);
                    }
                } else {
                    (void)fractus_graphics_put_pixel(framebuffer, x + column, y + row, pixel);
                }
            }
        }

        x += glyph_width + 1;
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_font_draw_text_scaled(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *library,
    fractus_font_kind kind,
    int32_t x,
    int32_t y,
    uint8_t color_index,
    const char *text,
    uint32_t scale)
{
    const fractus_font_face *face;
    size_t n;

    if (framebuffer == NULL || library == NULL || text == NULL ||
        !framebuffer->initialized || !library->initialized ||
        kind < 0 || kind >= FRACTUS_FONT_COUNT || scale == 0u) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (scale == 1u) {
        return fractus_font_draw_text(framebuffer, library, kind, x, y, color_index, text);
    }

    face = &library->faces[kind];
    if (!face->initialized) {
        return FRACTUS_STATUS_ERROR;
    }

    for (n = 0u; text[n] != '\0'; ++n) {
        uint32_t glyph_index;
        uint16_t glyph_width;
        uint16_t row;
        uint16_t column;

        if (text[n] == ' ') {
            x += ((int32_t)face->glyph_widths[8] + ((kind == FRACTUS_FONT_COURIER) ? 0 : 1)) * (int32_t)scale;
            continue;
        }

        if (!fractus_font_glyph_index(text[n], &glyph_index)) {
            continue;
        }

        glyph_width = face->glyph_widths[glyph_index];
        for (row = 0u; row < face->glyph_height; ++row) {
            for (column = 0u; column < glyph_width; ++column) {
                const size_t pixel_offset =
                    (size_t)face->glyph_offsets[glyph_index] + (size_t)row * glyph_width + column;
                const uint8_t pixel = face->data[pixel_offset];

                if (face->file_type == 0u) {
                    if (pixel != 0u &&
                        fractus_graphics_fill_rect(
                            framebuffer,
                            (fractus_rect_i32){
                                x + (int32_t)column * (int32_t)scale,
                                y + (int32_t)row * (int32_t)scale,
                                (int32_t)scale,
                                (int32_t)scale},
                            color_index) != FRACTUS_STATUS_OK) {
                        return FRACTUS_STATUS_ERROR;
                    }
                } else if (fractus_graphics_fill_rect(
                               framebuffer,
                               (fractus_rect_i32){
                                   x + (int32_t)column * (int32_t)scale,
                                   y + (int32_t)row * (int32_t)scale,
                                   (int32_t)scale,
                                   (int32_t)scale},
                               pixel) != FRACTUS_STATUS_OK) {
                    return FRACTUS_STATUS_ERROR;
                }
            }
        }

        x += ((int32_t)glyph_width + 1) * (int32_t)scale;
    }

    return FRACTUS_STATUS_OK;
}
