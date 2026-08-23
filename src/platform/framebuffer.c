#include "platform/framebuffer.h"

#include <stdlib.h>
#include <string.h>

static uint8_t fractus_scale_6bit_to_8bit(uint8_t value)
{
    return (uint8_t)((value * 255u) / 63u);
}

static fractus_color_rgba8 fractus_rgba8(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    fractus_color_rgba8 color;

    color.r = r;
    color.g = g;
    color.b = b;
    color.a = a;
    return color;
}

static void fractus_palette_set_original_base(fractus_palette *palette)
{
    palette->entries[0] = fractus_rgba8(0, 0, 0, 255);
    palette->entries[1] = fractus_rgba8(fractus_scale_6bit_to_8bit(40), 0, 0, 255);
    palette->entries[2] = fractus_rgba8(fractus_scale_6bit_to_8bit(60), fractus_scale_6bit_to_8bit(10), fractus_scale_6bit_to_8bit(10), 255);
    palette->entries[3] = fractus_rgba8(0, fractus_scale_6bit_to_8bit(32), fractus_scale_6bit_to_8bit(16), 255);
    palette->entries[4] = fractus_rgba8(fractus_scale_6bit_to_8bit(10), fractus_scale_6bit_to_8bit(60), fractus_scale_6bit_to_8bit(10), 255);
    palette->entries[5] = fractus_rgba8(0, fractus_scale_6bit_to_8bit(16), fractus_scale_6bit_to_8bit(32), 255);
    palette->entries[6] = fractus_rgba8(fractus_scale_6bit_to_8bit(10), fractus_scale_6bit_to_8bit(10), fractus_scale_6bit_to_8bit(60), 255);
    palette->entries[7] = fractus_rgba8(fractus_scale_6bit_to_8bit(32), fractus_scale_6bit_to_8bit(32), fractus_scale_6bit_to_8bit(32), 255);
    palette->entries[8] = fractus_rgba8(fractus_scale_6bit_to_8bit(48), fractus_scale_6bit_to_8bit(48), fractus_scale_6bit_to_8bit(48), 255);
    palette->entries[9] = fractus_rgba8(fractus_scale_6bit_to_8bit(40), fractus_scale_6bit_to_8bit(40), fractus_scale_6bit_to_8bit(16), 255);
    palette->entries[10] = fractus_rgba8(fractus_scale_6bit_to_8bit(60), fractus_scale_6bit_to_8bit(60), fractus_scale_6bit_to_8bit(10), 255);
    palette->entries[11] = fractus_rgba8(fractus_scale_6bit_to_8bit(40), fractus_scale_6bit_to_8bit(45), fractus_scale_6bit_to_8bit(60), 255);
    palette->entries[12] = fractus_rgba8(fractus_scale_6bit_to_8bit(25), fractus_scale_6bit_to_8bit(55), fractus_scale_6bit_to_8bit(60), 255);
    palette->entries[13] = fractus_rgba8(fractus_scale_6bit_to_8bit(40), fractus_scale_6bit_to_8bit(15), fractus_scale_6bit_to_8bit(15), 255);
    palette->entries[14] = fractus_rgba8(fractus_scale_6bit_to_8bit(50), fractus_scale_6bit_to_8bit(30), fractus_scale_6bit_to_8bit(15), 255);
    palette->entries[15] = fractus_rgba8(fractus_scale_6bit_to_8bit(60), fractus_scale_6bit_to_8bit(60), fractus_scale_6bit_to_8bit(60), 255);
}

fractus_status fractus_palette_init_default(fractus_palette *palette)
{
    uint32_t i;

    if (palette == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    fractus_palette_set_original_base(palette);

    for (i = FRACTUS_PALETTE_OFFSET; i < FRACTUS_PALETTE_SIZE; ++i) {
        const uint8_t t = (uint8_t)(i - FRACTUS_PALETTE_OFFSET);

        palette->entries[i] = fractus_rgba8(
            t,
            (uint8_t)((t * 5u) & 0xffu),
            (uint8_t)(255u - t),
            255);
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_palette_set_entry(
    fractus_palette *palette,
    uint32_t index,
    fractus_color_rgba8 color)
{
    if (palette == NULL || index >= FRACTUS_PALETTE_SIZE) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    palette->entries[index] = color;
    return FRACTUS_STATUS_OK;
}

fractus_status fractus_palette_get_entry(
    const fractus_palette *palette,
    uint32_t index,
    fractus_color_rgba8 *color)
{
    if (palette == NULL || color == NULL || index >= FRACTUS_PALETTE_SIZE) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    *color = palette->entries[index];
    return FRACTUS_STATUS_OK;
}

fractus_status fractus_framebuffer_init(
    fractus_framebuffer *framebuffer,
    fractus_size_u32 size)
{
    size_t index_count;
    size_t rgba_count;

    if (framebuffer == NULL || size.width == 0u || size.height == 0u) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    index_count = (size_t)size.width * (size_t)size.height;
    rgba_count = index_count * 4u;

    framebuffer->size = size;
    framebuffer->pitch_pixels = size.width;
    framebuffer->rgba_pitch_bytes = size.width * 4u;
    framebuffer->index_pixels = (uint8_t *)calloc(index_count, sizeof(uint8_t));
    framebuffer->rgba_pixels = (uint8_t *)calloc(rgba_count, sizeof(uint8_t));

    if (framebuffer->index_pixels == NULL || framebuffer->rgba_pixels == NULL) {
        fractus_framebuffer_shutdown(framebuffer);
        return FRACTUS_STATUS_ERROR;
    }

    if (fractus_palette_init_default(&framebuffer->palette) != FRACTUS_STATUS_OK) {
        fractus_framebuffer_shutdown(framebuffer);
        return FRACTUS_STATUS_ERROR;
    }

    framebuffer->pixels_dirty = 1;
    framebuffer->palette_dirty = 1;
    framebuffer->initialized = 1;
    return FRACTUS_STATUS_OK;
}

void fractus_framebuffer_shutdown(fractus_framebuffer *framebuffer)
{
    if (framebuffer == NULL) {
        return;
    }

    free(framebuffer->index_pixels);
    free(framebuffer->rgba_pixels);

    framebuffer->index_pixels = NULL;
    framebuffer->rgba_pixels = NULL;
    framebuffer->size.width = 0u;
    framebuffer->size.height = 0u;
    framebuffer->pitch_pixels = 0u;
    framebuffer->rgba_pitch_bytes = 0u;
    framebuffer->pixels_dirty = 0;
    framebuffer->palette_dirty = 0;
    framebuffer->initialized = 0;
}

fractus_status fractus_framebuffer_clear(
    fractus_framebuffer *framebuffer,
    uint8_t color_index)
{
    size_t count;

    if (framebuffer == NULL || !framebuffer->initialized || framebuffer->index_pixels == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    count = (size_t)framebuffer->size.width * (size_t)framebuffer->size.height;
    memset(framebuffer->index_pixels, color_index, count);
    framebuffer->pixels_dirty = 1;
    return FRACTUS_STATUS_OK;
}

fractus_status fractus_framebuffer_set_pixel(
    fractus_framebuffer *framebuffer,
    uint32_t x,
    uint32_t y,
    uint8_t color_index)
{
    size_t offset;

    if (framebuffer == NULL || !framebuffer->initialized ||
        x >= framebuffer->size.width || y >= framebuffer->size.height) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    offset = (size_t)y * (size_t)framebuffer->pitch_pixels + (size_t)x;
    framebuffer->index_pixels[offset] = color_index;
    framebuffer->pixels_dirty = 1;
    return FRACTUS_STATUS_OK;
}

fractus_status fractus_framebuffer_get_pixel(
    const fractus_framebuffer *framebuffer,
    uint32_t x,
    uint32_t y,
    uint8_t *color_index)
{
    size_t offset;

    if (framebuffer == NULL || !framebuffer->initialized || color_index == NULL ||
        x >= framebuffer->size.width || y >= framebuffer->size.height) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    offset = (size_t)y * (size_t)framebuffer->pitch_pixels + (size_t)x;
    *color_index = framebuffer->index_pixels[offset];
    return FRACTUS_STATUS_OK;
}

fractus_status fractus_framebuffer_sync_rgba(fractus_framebuffer *framebuffer)
{
    size_t total_pixels;
    size_t i;
    const uint8_t *src;
    uint8_t *dst;

    if (framebuffer == NULL || !framebuffer->initialized ||
        framebuffer->index_pixels == NULL || framebuffer->rgba_pixels == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (!framebuffer->pixels_dirty && !framebuffer->palette_dirty) {
        return FRACTUS_STATUS_OK;
    }

    total_pixels = (size_t)framebuffer->size.width * (size_t)framebuffer->size.height;
    src = framebuffer->index_pixels;
    dst = framebuffer->rgba_pixels;

    for (i = 0u; i < total_pixels; ++i) {
        const uint8_t index = src[i];
        const fractus_color_rgba8 color = framebuffer->palette.entries[index];
        const size_t out_index = i * 4u;

        dst[out_index + 0u] = color.r;
        dst[out_index + 1u] = color.g;
        dst[out_index + 2u] = color.b;
        dst[out_index + 3u] = color.a;
    }

    framebuffer->pixels_dirty = 0;
    framebuffer->palette_dirty = 0;
    return FRACTUS_STATUS_OK;
}

fractus_status fractus_palette_cycle(
    fractus_palette *palette,
    uint32_t offset,
    uint32_t span,
    int direction)
{
    fractus_color_rgba8 temp[FRACTUS_PALETTE_SIZE];
    uint32_t i;

    if (palette == NULL || span < 2u || offset >= FRACTUS_PALETTE_SIZE || (offset + span) > FRACTUS_PALETTE_SIZE) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (direction >= 0) {
        const fractus_color_rgba8 last = palette->entries[offset + span - 1u];
        for (i = 0u; i < span - 1u; ++i) {
            temp[offset + i + 1u] = palette->entries[offset + i];
        }
        temp[offset] = last;
    } else {
        const fractus_color_rgba8 first = palette->entries[offset];
        for (i = 0u; i < span - 1u; ++i) {
            temp[offset + i] = palette->entries[offset + i + 1u];
        }
        temp[offset + span - 1u] = first;
    }

    for (i = 0u; i < span; ++i) {
        palette->entries[offset + i] = temp[offset + i];
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_framebuffer_cycle_palette(
    fractus_framebuffer *framebuffer,
    uint32_t offset,
    uint32_t span,
    int direction)
{
    fractus_status status;

    if (framebuffer == NULL || !framebuffer->initialized) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    status = fractus_palette_cycle(&framebuffer->palette, offset, span, direction);
    if (status != FRACTUS_STATUS_OK) {
        return status;
    }

    framebuffer->palette_dirty = 1;
    return FRACTUS_STATUS_OK;
}
