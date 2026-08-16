#include "platform/graphics.h"

#include <stdlib.h>

static int32_t fractus_min_i32(int32_t a, int32_t b)
{
    return (a < b) ? a : b;
}

static int32_t fractus_max_i32(int32_t a, int32_t b)
{
    return (a > b) ? a : b;
}

static int fractus_graphics_clip_rect(
    const fractus_framebuffer *framebuffer,
    fractus_rect_i32 rect,
    fractus_rect_i32 *clipped)
{
    if (framebuffer == NULL || clipped == NULL) {
        return 0;
    }

    clipped->x = fractus_max_i32(0, rect.x);
    clipped->y = fractus_max_i32(0, rect.y);
    clipped->width = fractus_min_i32(rect.x + rect.width, (int32_t)framebuffer->size.width) - clipped->x;
    clipped->height = fractus_min_i32(rect.y + rect.height, (int32_t)framebuffer->size.height) - clipped->y;

    return clipped->width > 0 && clipped->height > 0;
}

fractus_status fractus_indexed_image_init(
    fractus_indexed_image *image,
    fractus_size_u32 size)
{
    size_t pixel_count;

    if (image == NULL || size.width == 0u || size.height == 0u) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    pixel_count = (size_t)size.width * (size_t)size.height;
    image->size = size;
    image->pitch_pixels = size.width;
    image->pixels = (uint8_t *)calloc(pixel_count, sizeof(uint8_t));
    if (image->pixels == NULL) {
        image->size.width = 0u;
        image->size.height = 0u;
        image->pitch_pixels = 0u;
        return FRACTUS_STATUS_ERROR;
    }

    image->initialized = 1;
    return FRACTUS_STATUS_OK;
}

void fractus_indexed_image_shutdown(fractus_indexed_image *image)
{
    if (image == NULL) {
        return;
    }

    free(image->pixels);
    image->pixels = NULL;
    image->size.width = 0u;
    image->size.height = 0u;
    image->pitch_pixels = 0u;
    image->initialized = 0;
}

fractus_status fractus_graphics_put_pixel(
    fractus_framebuffer *framebuffer,
    int32_t x,
    int32_t y,
    uint8_t color_index)
{
    if (framebuffer == NULL || !framebuffer->initialized) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (x < 0 || y < 0 ||
        x >= (int32_t)framebuffer->size.width ||
        y >= (int32_t)framebuffer->size.height) {
        return FRACTUS_STATUS_OK;
    }

    return fractus_framebuffer_set_pixel(framebuffer, (uint32_t)x, (uint32_t)y, color_index);
}

fractus_status fractus_graphics_get_pixel(
    const fractus_framebuffer *framebuffer,
    int32_t x,
    int32_t y,
    uint8_t *color_index)
{
    if (framebuffer == NULL || color_index == NULL || !framebuffer->initialized) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (x < 0 || y < 0 ||
        x >= (int32_t)framebuffer->size.width ||
        y >= (int32_t)framebuffer->size.height) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    return fractus_framebuffer_get_pixel(framebuffer, (uint32_t)x, (uint32_t)y, color_index);
}

fractus_status fractus_graphics_line(
    fractus_framebuffer *framebuffer,
    int32_t x0,
    int32_t y0,
    int32_t x1,
    int32_t y1,
    uint8_t color_index)
{
    int32_t dx;
    int32_t sx;
    int32_t dy;
    int32_t sy;
    int32_t err;
    int32_t e2;

    if (framebuffer == NULL || !framebuffer->initialized) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    dx = fractus_max_i32(x1 - x0, x0 - x1);
    sx = (x0 < x1) ? 1 : -1;
    dy = -fractus_max_i32(y1 - y0, y0 - y1);
    sy = (y0 < y1) ? 1 : -1;
    err = dx + dy;

    for (;;) {
        if (fractus_graphics_put_pixel(framebuffer, x0, y0, color_index) != FRACTUS_STATUS_OK) {
            return FRACTUS_STATUS_ERROR;
        }

        if (x0 == x1 && y0 == y1) {
            break;
        }

        e2 = 2 * err;

        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }

        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_graphics_rect(
    fractus_framebuffer *framebuffer,
    fractus_rect_i32 rect,
    uint8_t color_index)
{
    if (framebuffer == NULL || !framebuffer->initialized || rect.width <= 0 || rect.height <= 0) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_graphics_line(framebuffer, rect.x, rect.y, rect.x + rect.width - 1, rect.y, color_index) != FRACTUS_STATUS_OK ||
        fractus_graphics_line(framebuffer, rect.x, rect.y, rect.x, rect.y + rect.height - 1, color_index) != FRACTUS_STATUS_OK ||
        fractus_graphics_line(framebuffer, rect.x + rect.width - 1, rect.y, rect.x + rect.width - 1, rect.y + rect.height - 1, color_index) != FRACTUS_STATUS_OK ||
        fractus_graphics_line(framebuffer, rect.x, rect.y + rect.height - 1, rect.x + rect.width - 1, rect.y + rect.height - 1, color_index) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_graphics_fill_rect(
    fractus_framebuffer *framebuffer,
    fractus_rect_i32 rect,
    uint8_t color_index)
{
    fractus_rect_i32 clipped;
    int32_t y;

    if (framebuffer == NULL || !framebuffer->initialized || rect.width <= 0 || rect.height <= 0) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (!fractus_graphics_clip_rect(framebuffer, rect, &clipped)) {
        return FRACTUS_STATUS_OK;
    }

    for (y = clipped.y; y < clipped.y + clipped.height; ++y) {
        if (fractus_graphics_line(
                framebuffer,
                clipped.x,
                y,
                clipped.x + clipped.width - 1,
                y,
                color_index) != FRACTUS_STATUS_OK) {
            return FRACTUS_STATUS_ERROR;
        }
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_graphics_capture_region(
    const fractus_framebuffer *framebuffer,
    fractus_rect_i32 rect,
    fractus_indexed_image *image)
{
    fractus_rect_i32 clipped;
    int32_t x;
    int32_t y;
    uint8_t color_index;

    if (framebuffer == NULL || image == NULL || !framebuffer->initialized) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (!fractus_graphics_clip_rect(framebuffer, rect, &clipped)) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_indexed_image_init(
            image,
            (fractus_size_u32){(uint32_t)clipped.width, (uint32_t)clipped.height}) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    for (y = 0; y < clipped.height; ++y) {
        for (x = 0; x < clipped.width; ++x) {
            if (fractus_graphics_get_pixel(
                    framebuffer,
                    clipped.x + x,
                    clipped.y + y,
                    &color_index) != FRACTUS_STATUS_OK) {
                fractus_indexed_image_shutdown(image);
                return FRACTUS_STATUS_ERROR;
            }

            image->pixels[(size_t)y * image->pitch_pixels + (uint32_t)x] = color_index;
        }
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_graphics_blit(
    fractus_framebuffer *framebuffer,
    fractus_point_i32 destination,
    const fractus_indexed_image *image)
{
    uint32_t x;
    uint32_t y;

    if (framebuffer == NULL || image == NULL || !framebuffer->initialized || !image->initialized) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    for (y = 0u; y < image->size.height; ++y) {
        for (x = 0u; x < image->size.width; ++x) {
            if (fractus_graphics_put_pixel(
                    framebuffer,
                    destination.x + (int32_t)x,
                    destination.y + (int32_t)y,
                    image->pixels[(size_t)y * image->pitch_pixels + x]) != FRACTUS_STATUS_OK) {
                return FRACTUS_STATUS_ERROR;
            }
        }
    }

    return FRACTUS_STATUS_OK;
}
