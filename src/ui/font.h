#ifndef FRACTUS_X64_FONT_H
#define FRACTUS_X64_FONT_H

#include "platform/framebuffer.h"
#include "platform/types.h"

#include <stddef.h>

typedef enum fractus_font_kind {
    FRACTUS_FONT_FRANCE = 0,
    FRACTUS_FONT_ARIAL = 1,
    FRACTUS_FONT_SMALL = 2,
    FRACTUS_FONT_COURIER = 3,
    FRACTUS_FONT_COUNT = 4
} fractus_font_kind;

typedef struct fractus_font_face {
    uint16_t file_type;
    uint16_t glyph_height;
    uint16_t glyph_widths[97];
    uint32_t glyph_offsets[97];
    uint8_t *data;
    size_t data_size;
    int initialized;
} fractus_font_face;

typedef struct fractus_font_library {
    fractus_font_face faces[FRACTUS_FONT_COUNT];
    int initialized;
} fractus_font_library;

fractus_status fractus_font_library_load_archive(
    fractus_font_library *library,
    const char *path);
void fractus_font_library_shutdown(fractus_font_library *library);

fractus_status fractus_font_measure_text(
    const fractus_font_library *library,
    fractus_font_kind kind,
    const char *text,
    int32_t *width,
    int32_t *height);
fractus_status fractus_font_draw_text(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *library,
    fractus_font_kind kind,
    int32_t x,
    int32_t y,
    uint8_t color_index,
    const char *text);
fractus_status fractus_font_draw_text_scaled(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *library,
    fractus_font_kind kind,
    int32_t x,
    int32_t y,
    uint8_t color_index,
    const char *text,
    uint32_t scale);

#endif
