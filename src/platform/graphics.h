#ifndef FRACTUS_X64_GRAPHICS_H
#define FRACTUS_X64_GRAPHICS_H

#include "platform/framebuffer.h"
#include "platform/types.h"

typedef struct fractus_indexed_image {
    fractus_size_u32 size;
    uint32_t pitch_pixels;
    uint8_t *pixels;
    int initialized;
} fractus_indexed_image;

fractus_status fractus_indexed_image_init(
    fractus_indexed_image *image,
    fractus_size_u32 size);
void fractus_indexed_image_shutdown(fractus_indexed_image *image);

fractus_status fractus_graphics_put_pixel(
    fractus_framebuffer *framebuffer,
    int32_t x,
    int32_t y,
    uint8_t color_index);
fractus_status fractus_graphics_get_pixel(
    const fractus_framebuffer *framebuffer,
    int32_t x,
    int32_t y,
    uint8_t *color_index);
fractus_status fractus_graphics_line(
    fractus_framebuffer *framebuffer,
    int32_t x0,
    int32_t y0,
    int32_t x1,
    int32_t y1,
    uint8_t color_index);
fractus_status fractus_graphics_rect(
    fractus_framebuffer *framebuffer,
    fractus_rect_i32 rect,
    uint8_t color_index);
fractus_status fractus_graphics_fill_rect(
    fractus_framebuffer *framebuffer,
    fractus_rect_i32 rect,
    uint8_t color_index);

fractus_status fractus_graphics_capture_region(
    const fractus_framebuffer *framebuffer,
    fractus_rect_i32 rect,
    fractus_indexed_image *image);
fractus_status fractus_graphics_blit(
    fractus_framebuffer *framebuffer,
    fractus_point_i32 destination,
    const fractus_indexed_image *image);

#endif
