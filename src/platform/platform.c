#define SDL_MAIN_HANDLED
#include <SDL.h>

#include "platform/platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct fractus_sdl2_backend {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    uint32_t texture_width;
    uint32_t texture_height;
    uint32_t mode_width;
    uint32_t mode_height;
    int mode_fullscreen;
    int mode_integer_upscale;
    int mode_borders_known;
    int mode_valid;
} fractus_sdl2_backend;

static fractus_sdl2_backend *fractus_platform_backend(
    const fractus_platform_context *platform)
{
    return (fractus_sdl2_backend *)platform->backend_state;
}

static void fractus_platform_window_borders(
    SDL_Window *window,
    int *top,
    int *left,
    int *bottom,
    int *right)
{
    *top = 0;
    *left = 0;
    *bottom = 0;
    *right = 0;
    if (window == NULL || SDL_GetWindowBordersSize(window, top, left, bottom, right) != 0) {
        *top = 0;
        *left = 0;
        *bottom = 0;
        *right = 0;
        return;
    }
    if (*top < 0) {
        *top = 0;
    }
    if (*left < 0) {
        *left = 0;
    }
    if (*bottom < 0) {
        *bottom = 0;
    }
    if (*right < 0) {
        *right = 0;
    }
}

static int fractus_platform_usable_bounds(SDL_Window *window, SDL_Rect *usable)
{
    int display_index = 0;
    SDL_DisplayMode mode;

    if (window != NULL) {
        display_index = SDL_GetWindowDisplayIndex(window);
        if (display_index < 0) {
            display_index = 0;
        }
    }

    if (SDL_GetDisplayUsableBounds(display_index, usable) == 0 && usable->w > 0 && usable->h > 0) {
        return 1;
    }

    if (SDL_GetCurrentDisplayMode(display_index, &mode) != 0 || mode.w <= 0 || mode.h <= 0) {
        return 0;
    }

    usable->x = 0;
    usable->y = 0;
    usable->w = mode.w;
    usable->h = mode.h;
    return 1;
}

static void fractus_platform_fit_size(
    int budget_w,
    int budget_h,
    uint32_t content_w,
    uint32_t content_h,
    int integer_upscale,
    uint32_t *out_w,
    uint32_t *out_h)
{
    int scale;
    int scale_y;

    if (budget_w < 1) {
        budget_w = 1;
    }
    if (budget_h < 1) {
        budget_h = 1;
    }

    if (integer_upscale) {
        scale = budget_w / (int)content_w;
        scale_y = budget_h / (int)content_h;
        if (scale_y < scale) {
            scale = scale_y;
        }
        if (scale >= 1) {
            *out_w = content_w * (uint32_t)scale;
            *out_h = content_h * (uint32_t)scale;
            return;
        }
    } else if (content_w <= (uint32_t)budget_w && content_h <= (uint32_t)budget_h) {
        *out_w = content_w;
        *out_h = content_h;
        return;
    }

    if ((int64_t)budget_w * (int64_t)content_h <= (int64_t)budget_h * (int64_t)content_w) {
        *out_w = (uint32_t)budget_w;
        *out_h = (uint32_t)(((int64_t)budget_w * (int64_t)content_h) / (int64_t)content_w);
    } else {
        *out_h = (uint32_t)budget_h;
        *out_w = (uint32_t)(((int64_t)budget_h * (int64_t)content_w) / (int64_t)content_h);
    }
    if (*out_w < 1u) {
        *out_w = 1u;
    }
    if (*out_h < 1u) {
        *out_h = 1u;
    }
}

static void fractus_platform_place_window(SDL_Window *window, uint32_t width, uint32_t height)
{
    SDL_SetWindowSize(window, (int)width, (int)height);
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

static fractus_status fractus_platform_apply_window_mode(
    fractus_sdl2_backend *backend,
    fractus_size_u32 content,
    int fullscreen,
    int integer_upscale)
{
    int top;
    int left;
    int bottom;
    int right;
    int borders_known;
    int requested_fullscreen;
    int requested_integer_upscale;
    SDL_Rect usable;
    int budget_w;
    int budget_h;
    uint32_t fitted_w;
    uint32_t fitted_h;

    fullscreen = fullscreen ? 1 : 0;
    integer_upscale = integer_upscale ? 1 : 0;
    requested_fullscreen = fullscreen;
    requested_integer_upscale = integer_upscale;
    fractus_platform_window_borders(backend->window, &top, &left, &bottom, &right);
    borders_known = (top + left + bottom + right) > 0;

    if (backend->mode_valid &&
        backend->mode_width == content.width &&
        backend->mode_height == content.height &&
        backend->mode_fullscreen == requested_fullscreen &&
        backend->mode_integer_upscale == integer_upscale &&
        (backend->mode_borders_known || !borders_known)) {
        return FRACTUS_STATUS_OK;
    }

    if (requested_fullscreen) {
        if (SDL_SetWindowFullscreen(backend->window, SDL_WINDOW_FULLSCREEN_DESKTOP) != 0) {
            SDL_ClearError();
            (void)SDL_SetWindowFullscreen(backend->window, 0);
            fullscreen = 0;
            integer_upscale = 0;
        }
    }

    if (!fullscreen) {
        if (backend->mode_valid && backend->mode_fullscreen &&
            SDL_SetWindowFullscreen(backend->window, 0) != 0) {
            return FRACTUS_STATUS_ERROR;
        }
        fractus_platform_window_borders(backend->window, &top, &left, &bottom, &right);
        borders_known = (top + left + bottom + right) > 0;
        if (!fractus_platform_usable_bounds(backend->window, &usable)) {
            fitted_w = content.width;
            fitted_h = content.height;
        } else {
            budget_w = usable.w - left - right;
            budget_h = usable.h - top - bottom;
            fractus_platform_fit_size(
                budget_w,
                budget_h,
                content.width,
                content.height,
                integer_upscale,
                &fitted_w,
                &fitted_h);
        }
        fractus_platform_place_window(backend->window, fitted_w, fitted_h);
        if (!borders_known) {
            fractus_platform_window_borders(backend->window, &top, &left, &bottom, &right);
            borders_known = (top + left + bottom + right) > 0;
            if (borders_known && fractus_platform_usable_bounds(backend->window, &usable)) {
                budget_w = usable.w - left - right;
                budget_h = usable.h - top - bottom;
                fractus_platform_fit_size(
                    budget_w,
                    budget_h,
                    content.width,
                    content.height,
                    integer_upscale,
                    &fitted_w,
                    &fitted_h);
                fractus_platform_place_window(backend->window, fitted_w, fitted_h);
            }
        }
    }

    backend->mode_width = content.width;
    backend->mode_height = content.height;
    backend->mode_fullscreen = requested_fullscreen;
    backend->mode_integer_upscale = requested_integer_upscale;
    backend->mode_borders_known = borders_known;
    backend->mode_valid = 1;
    return FRACTUS_STATUS_OK;
}

static int fractus_platform_translate_event(
    const SDL_Event *sdl_event,
    fractus_platform_event *event)
{
    event->type = FRACTUS_PLATFORM_EVENT_NONE;
    memset(&event->data, 0, sizeof(event->data));

    switch (sdl_event->type) {
    case SDL_QUIT:
        event->type = FRACTUS_PLATFORM_EVENT_QUIT;
        return 1;
    case SDL_WINDOWEVENT:
        if (sdl_event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED ||
            sdl_event->window.event == SDL_WINDOWEVENT_RESIZED) {
            event->type = FRACTUS_PLATFORM_EVENT_WINDOW_RESIZED;
            event->data.window_resized.size.width = (uint32_t)sdl_event->window.data1;
            event->data.window_resized.size.height = (uint32_t)sdl_event->window.data2;
            return 1;
        }
        return 0;
    case SDL_MOUSEMOTION:
        event->type = FRACTUS_PLATFORM_EVENT_MOUSE_MOVED;
        event->data.mouse_move.position.x = sdl_event->motion.x;
        event->data.mouse_move.position.y = sdl_event->motion.y;
        return 1;
    case SDL_MOUSEBUTTONDOWN:
        event->type = FRACTUS_PLATFORM_EVENT_MOUSE_BUTTON_DOWN;
        event->data.mouse_button.position.x = sdl_event->button.x;
        event->data.mouse_button.position.y = sdl_event->button.y;
        event->data.mouse_button.button = sdl_event->button.button;
        return 1;
    case SDL_MOUSEBUTTONUP:
        event->type = FRACTUS_PLATFORM_EVENT_MOUSE_BUTTON_UP;
        event->data.mouse_button.position.x = sdl_event->button.x;
        event->data.mouse_button.position.y = sdl_event->button.y;
        event->data.mouse_button.button = sdl_event->button.button;
        return 1;
    case SDL_KEYDOWN:
        event->type = FRACTUS_PLATFORM_EVENT_KEY_DOWN;
        event->data.key.keycode = (uint32_t)sdl_event->key.keysym.sym;
        return 1;
    case SDL_KEYUP:
        event->type = FRACTUS_PLATFORM_EVENT_KEY_UP;
        event->data.key.keycode = (uint32_t)sdl_event->key.keysym.sym;
        return 1;
    case SDL_TEXTINPUT:
        event->type = FRACTUS_PLATFORM_EVENT_TEXT_INPUT;
        snprintf(event->data.text_input.text, sizeof(event->data.text_input.text), "%s", sdl_event->text.text);
        return 1;
    default:
        return 0;
    }
}

static fractus_status fractus_platform_ensure_texture(
    fractus_platform_context *platform,
    const fractus_platform_surface *surface)
{
    fractus_sdl2_backend *backend = fractus_platform_backend(platform);
    Uint32 sdl_pixel_format;

    if (backend->texture != NULL &&
        backend->texture_width == surface->width &&
        backend->texture_height == surface->height) {
        return FRACTUS_STATUS_OK;
    }

    if (backend->texture != NULL) {
        SDL_DestroyTexture(backend->texture);
        backend->texture = NULL;
    }

    switch (surface->pixel_format) {
    case FRACTUS_PLATFORM_PIXEL_FORMAT_RGBA8888:
        sdl_pixel_format = SDL_PIXELFORMAT_RGBA32;
        break;
    default:
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    backend->texture = SDL_CreateTexture(
        backend->renderer,
        sdl_pixel_format,
        SDL_TEXTUREACCESS_STREAMING,
        (int)surface->width,
        (int)surface->height);

    if (backend->texture == NULL) {
        return FRACTUS_STATUS_ERROR;
    }

    backend->texture_width = surface->width;
    backend->texture_height = surface->height;
    return FRACTUS_STATUS_OK;
}

fractus_status fractus_platform_init(
    fractus_platform_context *platform,
    const fractus_platform_config *config)
{
    fractus_sdl2_backend *backend;
    Uint32 renderer_flags;

    if (platform == NULL || config == NULL || config->application_name == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (config->logical_size.width == 0u || config->logical_size.height == 0u) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    SDL_SetMainReady();
    SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, "permonitorv2");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
        return FRACTUS_STATUS_ERROR;
    }

    backend = (fractus_sdl2_backend *)calloc(1u, sizeof(*backend));
    if (backend == NULL) {
        SDL_Quit();
        return FRACTUS_STATUS_ERROR;
    }

    backend->window = SDL_CreateWindow(
        config->application_name,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        (int)config->logical_size.width,
        (int)config->logical_size.height,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

    if (backend->window == NULL) {
        free(backend);
        SDL_Quit();
        return FRACTUS_STATUS_ERROR;
    }

    if (fractus_platform_apply_window_mode(
            backend,
            config->logical_size,
            0,
            FRACTUS_PLATFORM_WINDOW_FIT_INTEGER) != FRACTUS_STATUS_OK) {
        SDL_DestroyWindow(backend->window);
        free(backend);
        SDL_Quit();
        return FRACTUS_STATUS_ERROR;
    }

    renderer_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
    backend->renderer = SDL_CreateRenderer(backend->window, -1, renderer_flags);
    if (backend->renderer == NULL) {
        backend->renderer = SDL_CreateRenderer(backend->window, -1, SDL_RENDERER_SOFTWARE);
    }

    if (backend->renderer == NULL) {
        SDL_DestroyWindow(backend->window);
        free(backend);
        SDL_Quit();
        return FRACTUS_STATUS_ERROR;
    }

    SDL_RenderSetLogicalSize(
        backend->renderer,
        (int)config->logical_size.width,
        (int)config->logical_size.height);
    SDL_RenderSetIntegerScale(backend->renderer, SDL_TRUE);
    SDL_StartTextInput();

    platform->config = *config;
    platform->started_at_ms = SDL_GetTicks64();
    platform->backend_state = backend;
    platform->initialized = 1;
    return FRACTUS_STATUS_OK;
}

void fractus_platform_shutdown(fractus_platform_context *platform)
{
    if (platform == NULL) {
        return;
    }

    if (platform->backend_state != NULL) {
        fractus_sdl2_backend *backend = fractus_platform_backend(platform);

        SDL_StopTextInput();

        if (backend->texture != NULL) {
            SDL_DestroyTexture(backend->texture);
        }

        if (backend->renderer != NULL) {
            SDL_DestroyRenderer(backend->renderer);
        }

        if (backend->window != NULL) {
            SDL_DestroyWindow(backend->window);
        }

        free(backend);
    }

    SDL_Quit();

    platform->backend_state = NULL;
    platform->initialized = 0;
    platform->started_at_ms = 0u;
}

fractus_status fractus_platform_poll_event(
    fractus_platform_context *platform,
    fractus_platform_event *event,
    int *has_event)
{
    SDL_Event sdl_event;

    if (platform == NULL || event == NULL || has_event == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (!platform->initialized) {
        return FRACTUS_STATUS_ERROR;
    }

    *has_event = 0;

    while (SDL_PollEvent(&sdl_event) != 0) {
        if (fractus_platform_translate_event(&sdl_event, event)) {
            *has_event = 1;
            return FRACTUS_STATUS_OK;
        }
    }

    event->type = FRACTUS_PLATFORM_EVENT_NONE;
    return FRACTUS_STATUS_OK;
}

uint64_t fractus_platform_get_ticks_ms(const fractus_platform_context *platform)
{
    if (platform == NULL || !platform->initialized) {
        return 0u;
    }

    return SDL_GetTicks64() - platform->started_at_ms;
}

void fractus_platform_sleep_ms(uint32_t duration_ms)
{
    SDL_Delay(duration_ms);
}

fractus_status fractus_platform_present(
    fractus_platform_context *platform,
    const fractus_platform_surface *surface)
{
    fractus_sdl2_backend *backend;
    SDL_Rect destination;

    if (platform == NULL || surface == NULL || surface->pixels == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (!platform->initialized) {
        return FRACTUS_STATUS_ERROR;
    }

    if (surface->width == 0u || surface->height == 0u || surface->pitch_bytes == 0u) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (surface->pixel_format != FRACTUS_PLATFORM_PIXEL_FORMAT_RGBA8888) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    backend = fractus_platform_backend(platform);
    if (backend == NULL) {
        return FRACTUS_STATUS_ERROR;
    }

    if (fractus_platform_ensure_texture(platform, surface) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    if (SDL_UpdateTexture(
            backend->texture,
            NULL,
            surface->pixels,
            (int)surface->pitch_bytes) != 0) {
        return FRACTUS_STATUS_ERROR;
    }

    if (surface->present_mode == FRACTUS_PLATFORM_PRESENT_MODE_DRAWING) {
        int output_width;
        int output_height;
        double scale_x;
        double scale_y;
        double scale;
        int width;
        int height;

        SDL_RenderSetIntegerScale(backend->renderer, SDL_FALSE);
        SDL_RenderSetLogicalSize(backend->renderer, 0, 0);
        SDL_RenderSetViewport(backend->renderer, NULL);
        if (SDL_RenderClear(backend->renderer) != 0) {
            return FRACTUS_STATUS_ERROR;
        }
        if (SDL_GetRendererOutputSize(backend->renderer, &output_width, &output_height) != 0 ||
            output_width <= 0 || output_height <= 0) {
            return FRACTUS_STATUS_ERROR;
        }

        scale_x = (double)output_width / (double)surface->width;
        scale_y = (double)output_height / (double)surface->height;
        scale = (scale_x < scale_y) ? scale_x : scale_y;
        width = (int)((double)surface->width * scale);
        height = (int)((double)surface->height * scale);
        destination.x = (output_width - width) / 2;
        destination.y = (output_height - height) / 2;
        destination.w = width;
        destination.h = height;

        if (SDL_RenderCopy(backend->renderer, backend->texture, NULL, &destination) != 0) {
            return FRACTUS_STATUS_ERROR;
        }
    } else {
        int window_w = 0;
        int window_h = 0;
        const int logical_w = (int)platform->config.logical_size.width;
        const int logical_h = (int)platform->config.logical_size.height;
        int use_integer = 0;

        SDL_GetWindowSize(backend->window, &window_w, &window_h);
        if (logical_w > 0 && logical_h > 0 &&
            window_w >= logical_w && window_h >= logical_h &&
            window_w % logical_w == 0 && window_h % logical_h == 0 &&
            (window_w / logical_w) == (window_h / logical_h)) {
            use_integer = 1;
        }
        SDL_RenderSetIntegerScale(backend->renderer, use_integer ? SDL_TRUE : SDL_FALSE);
        SDL_RenderSetLogicalSize(
            backend->renderer,
            logical_w,
            logical_h);
        SDL_RenderSetViewport(backend->renderer, NULL);
        if (SDL_RenderClear(backend->renderer) != 0) {
            return FRACTUS_STATUS_ERROR;
        }
        if (SDL_RenderCopy(backend->renderer, backend->texture, NULL, NULL) != 0) {
            return FRACTUS_STATUS_ERROR;
        }
    }

    SDL_RenderPresent(backend->renderer);
    return FRACTUS_STATUS_OK;
}

fractus_status fractus_platform_set_video_mode(
    fractus_platform_context *platform,
    fractus_size_u32 size,
    int fullscreen,
    fractus_platform_window_fit fit)
{
    fractus_sdl2_backend *backend;

    if (platform == NULL || size.width == 0u || size.height == 0u || !platform->initialized) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    backend = fractus_platform_backend(platform);
    if (backend == NULL || backend->window == NULL) {
        return FRACTUS_STATUS_ERROR;
    }

    return fractus_platform_apply_window_mode(
        backend,
        size,
        fullscreen,
        fit == FRACTUS_PLATFORM_WINDOW_FIT_INTEGER);
}

fractus_status fractus_platform_get_output_size(
    const fractus_platform_context *platform,
    fractus_size_u32 *size)
{
    fractus_sdl2_backend *backend;
    int width;
    int height;

    if (platform == NULL || size == NULL || !platform->initialized) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    backend = fractus_platform_backend(platform);
    if (backend == NULL || backend->renderer == NULL) {
        return FRACTUS_STATUS_ERROR;
    }

    if (SDL_GetRendererOutputSize(backend->renderer, &width, &height) != 0 ||
        width <= 0 || height <= 0) {
        return FRACTUS_STATUS_ERROR;
    }

    size->width = (uint32_t)width;
    size->height = (uint32_t)height;
    return FRACTUS_STATUS_OK;
}

fractus_status fractus_platform_get_resource_path(
    const fractus_platform_context *platform,
    const char *relative_path,
    char *buffer,
    size_t buffer_size)
{
    char *base_path;
    int written;

    size_t required_length;

    if (platform == NULL || !platform->initialized || relative_path == NULL || buffer == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    base_path = SDL_GetBasePath();
    if (base_path == NULL) {
        required_length = strlen(relative_path) + 1u;
        if (required_length > buffer_size) {
            return FRACTUS_STATUS_ERROR;
        }

        memcpy(buffer, relative_path, required_length);
        return FRACTUS_STATUS_OK;
    }

    written = snprintf(buffer, buffer_size, "%s%s", base_path, relative_path);
    SDL_free(base_path);

    if (written < 0 || (size_t)written >= buffer_size) {
        return FRACTUS_STATUS_ERROR;
    }

    return FRACTUS_STATUS_OK;
}
