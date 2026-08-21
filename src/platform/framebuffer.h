#ifndef FRACTUS_X64_FRAMEBUFFER_H
#define FRACTUS_X64_FRAMEBUFFER_H

#include "platform/types.h"

#define FRACTUS_PALETTE_SIZE 256u

typedef struct fractus_color_rgba8 {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} fractus_color_rgba8;

typedef struct fractus_palette {
    fractus_color_rgba8 entries[FRACTUS_PALETTE_SIZE];
} fractus_palette;

typedef struct fractus_framebuffer {
    fractus_size_u32 size;
    uint32_t pitch_pixels;
    uint32_t rgba_pitch_bytes;
    uint8_t *index_pixels;
    uint8_t *rgba_pixels;
    fractus_palette palette;
    int pixels_dirty;
    int palette_dirty;
    int initialized;
} fractus_framebuffer;

fractus_status fractus_palette_init_default(fractus_palette *palette);
fractus_status fractus_palette_set_entry(
    fractus_palette *palette,
    uint32_t index,
    fractus_color_rgba8 color);
fractus_status fractus_palette_get_entry(
    const fractus_palette *palette,
    uint32_t index,
    fractus_color_rgba8 *color);

fractus_status fractus_palette_cycle(
    fractus_palette *palette,
    uint32_t offset,
    uint32_t span,
    int direction);

fractus_status fractus_framebuffer_init(
    fractus_framebuffer *framebuffer,
    fractus_size_u32 size);
void fractus_framebuffer_shutdown(fractus_framebuffer *framebuffer);

fractus_status fractus_framebuffer_clear(
    fractus_framebuffer *framebuffer,
    uint8_t color_index);
fractus_status fractus_framebuffer_set_pixel(
    fractus_framebuffer *framebuffer,
    uint32_t x,
    uint32_t y,
    uint8_t color_index);
fractus_status fractus_framebuffer_get_pixel(
    const fractus_framebuffer *framebuffer,
    uint32_t x,
    uint32_t y,
    uint8_t *color_index);

fractus_status fractus_framebuffer_sync_rgba(fractus_framebuffer *framebuffer);
fractus_status fractus_framebuffer_cycle_palette(
    fractus_framebuffer *framebuffer,
    uint32_t offset,
    uint32_t span,
    int direction);

#endif
