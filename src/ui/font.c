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

static int fractus_font_decode_next_glyph(
    const char *text,
    size_t *offset,
    uint32_t *index,
    int *is_space)
{
    uint8_t b0;

    if (text == NULL || offset == NULL || index == NULL || is_space == NULL) {
        return 0;
    }

    *is_space = 0;
    b0 = (uint8_t)text[*offset];
    if (b0 == 0u) {
        return 0;
    }

    /* 1. Secuencia UTF-8 de 2 bytes que comienza con 0xC3 (Latin-1 Supplement: Acentos, Ñ, etc.) */
    if (b0 == 0xC3u && text[*offset + 1] != '\0') {
        uint8_t b1 = (uint8_t)text[*offset + 1];
        *offset += 2u;

        /* Acentos minusculas y enes presentes en fractus.fon */
        if (b1 == 0xA1u) { *index = 92u; return 1; } /* á */
        if (b1 == 0xA9u) { *index = 93u; return 1; } /* é */
        if (b1 == 0xADu) { *index = 94u; return 1; } /* í */
        if (b1 == 0xB3u) { *index = 95u; return 1; } /* ó */
        if (b1 == 0xBAu) { *index = 96u; return 1; } /* ú */
        if (b1 == 0xB1u) { *index = 51u; return 1; } /* ñ */
        if (b1 == 0x91u) { *index = 14u; return 1; } /* Ñ */

        /* Fallbacks para mayusculas con tilde y dieresis */
        if (b1 == 0x81u) { *index = 0u;  return 1; } /* Á -> A */
        if (b1 == 0x89u) { *index = 4u;  return 1; } /* É -> E */
        if (b1 == 0x8Du) { *index = 8u;  return 1; } /* Í -> I */
        if (b1 == 0x93u) { *index = 15u; return 1; } /* Ó -> O */
        if (b1 == 0x9Au) { *index = 20u; return 1; } /* Ú -> U */
        if (b1 == 0x9Cu) { *index = 20u; return 1; } /* Ü -> U */
        if (b1 == 0xBCu) { *index = 58u; return 1; } /* ü -> u */

        return 0;
    }

    /* 2. Secuencia UTF-8 de 2 bytes que comienza con 0xC2 (Puntuacion espanola) */
    if (b0 == 0xC2u && text[*offset + 1] != '\0') {
        *offset += 2u;
        return 0;
    }

    /* 3. Secuencia UTF-8 multibyte de 3 o 4 bytes */
    if ((b0 & 0xE0u) == 0xE0u) {
        size_t skip = ((b0 & 0xF0u) == 0xF0u) ? 4u : 3u;
        while (skip > 0u && text[*offset] != '\0') {
            *offset += 1u;
            --skip;
        }
        return 0;
    }

    /* 4. Caracteres ASCII estandar de 1 byte */
    *offset += 1u;

    if (b0 == 32u) {
        *is_space = 1;
        return 1;
    }

    if (b0 >= 33u && b0 <= 47u) {
        *index = (uint32_t)(b0 - 33u + 64u);
        return 1;
    }

    if (b0 >= 48u && b0 <= 57u) {
        *index = (uint32_t)(b0 - 48u + 27u);
        return 1;
    }

    if (b0 >= 58u && b0 <= 64u) {
        *index = (uint32_t)(b0 - 58u + 79u);
        return 1;
    }

    if (b0 >= 65u && b0 <= 78u) {
        *index = (uint32_t)(b0 - 65u);
        return 1;
    }

    if (b0 >= 79u && b0 <= 90u) {
        *index = (uint32_t)(b0 - 79u + 15u);
        return 1;
    }

    if (b0 >= 91u && b0 <= 93u) {
        *index = (uint32_t)(b0 - 91u + 86u);
        return 1;
    }

    if (b0 >= 97u && b0 <= 110u) {
        *index = (uint32_t)(b0 - 97u + 37u);
        return 1;
    }

    if (b0 >= 111u && b0 <= 122u) {
        *index = (uint32_t)(b0 - 111u + 52u);
        return 1;
    }

    if (b0 >= 123u && b0 <= 125u) {
        *index = (uint32_t)(b0 - 123u + 89u);
        return 1;
    }

    /* 5. Fallback para caracteres de 1 solo byte en ISO-8859-1 / Windows-1252 */
    if (b0 == 0xE1u) { *index = 92u; return 1; } /* á */
    if (b0 == 0xE9u) { *index = 93u; return 1; } /* é */
    if (b0 == 0xEDu) { *index = 94u; return 1; } /* í */
    if (b0 == 0xF3u) { *index = 95u; return 1; } /* ó */
    if (b0 == 0xFAu) { *index = 96u; return 1; } /* ú */
    if (b0 == 0xF1u) { *index = 51u; return 1; } /* ñ */
    if (b0 == 0xD1u) { *index = 14u; return 1; } /* Ñ */
    if (b0 == 0xC1u) { *index = 0u;  return 1; } /* Á -> A */
    if (b0 == 0xC9u) { *index = 4u;  return 1; } /* É -> E */
    if (b0 == 0xCDu) { *index = 8u;  return 1; } /* Í -> I */
    if (b0 == 0xD3u) { *index = 15u; return 1; } /* Ó -> O */
    if (b0 == 0xDAu) { *index = 20u; return 1; } /* Ú -> U */
    if (b0 == 0xDCu) { *index = 20u; return 1; } /* Ü -> U */
    if (b0 == 0xFCu) { *index = 58u; return 1; } /* ü -> u */

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
    size_t offset;
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
    offset = 0u;
    while (text[offset] != '\0') {
        uint32_t glyph_index = 0u;
        int is_space = 0;

        if (!fractus_font_decode_next_glyph(text, &offset, &glyph_index, &is_space)) {
            continue;
        }

        if (is_space) {
            measured_width += (int32_t)face->glyph_widths[8] + ((kind == FRACTUS_FONT_COURIER) ? 0 : 1);
            continue;
        }

        measured_width += (int32_t)face->glyph_widths[glyph_index] + 1;
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
    size_t offset;

    if (framebuffer == NULL || library == NULL || text == NULL ||
        !framebuffer->initialized || !library->initialized ||
        kind < 0 || kind >= FRACTUS_FONT_COUNT) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    face = &library->faces[kind];
    if (!face->initialized) {
        return FRACTUS_STATUS_ERROR;
    }

    offset = 0u;
    while (text[offset] != '\0') {
        uint32_t glyph_index = 0u;
        uint16_t glyph_width;
        uint16_t row;
        uint16_t column;
        int is_space = 0;

        if (!fractus_font_decode_next_glyph(text, &offset, &glyph_index, &is_space)) {
            continue;
        }

        if (is_space) {
            x += (int32_t)face->glyph_widths[8] + ((kind == FRACTUS_FONT_COURIER) ? 0 : 1);
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
    size_t offset;

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

    offset = 0u;
    while (text[offset] != '\0') {
        uint32_t glyph_index = 0u;
        uint16_t glyph_width;
        uint16_t row;
        uint16_t column;
        int is_space = 0;

        if (!fractus_font_decode_next_glyph(text, &offset, &glyph_index, &is_space)) {
            continue;
        }

        if (is_space) {
            x += ((int32_t)face->glyph_widths[8] + ((kind == FRACTUS_FONT_COURIER) ? 0 : 1)) * (int32_t)scale;
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
