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
    uint32_t video_width;
    uint32_t video_height;
    int video_fullscreen;
} fractus_sdl2_backend;

static fractus_sdl2_backend *fractus_platform_backend(
    const fractus_platform_context *platform)
{
    return (fractus_sdl2_backend *)platform->backend_state;
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
    uint32_t window_width;
    uint32_t window_height;
    Uint32 renderer_flags;

    if (platform == NULL || config == NULL || config->application_name == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (config->logical_size.width == 0u || config->logical_size.height == 0u) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    SDL_SetMainReady();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
        return FRACTUS_STATUS_ERROR;
    }

    backend = (fractus_sdl2_backend *)calloc(1u, sizeof(*backend));
    if (backend == NULL) {
        SDL_Quit();
        return FRACTUS_STATUS_ERROR;
    }

    window_width = config->logical_size.width * 2u;
    window_height = config->logical_size.height * 2u;

    if (window_width < config->logical_size.width) {
        window_width = config->logical_size.width;
    }

    if (window_height < config->logical_size.height) {
        window_height = config->logical_size.height;
    }

    backend->window = SDL_CreateWindow(
        config->application_name,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        (int)window_width,
        (int)window_height,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

    if (backend->window == NULL) {
        free(backend);
        SDL_Quit();
        return FRACTUS_STATUS_ERROR;
    }
    backend->video_width = window_width;
    backend->video_height = window_height;
    backend->video_fullscreen = 0;

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
        SDL_RenderSetIntegerScale(backend->renderer, SDL_TRUE);
        SDL_RenderSetLogicalSize(
            backend->renderer,
            (int)platform->config.logical_size.width,
            (int)platform->config.logical_size.height);
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
    int fullscreen)
{
    fractus_sdl2_backend *backend;

    if (platform == NULL || size.width == 0u || size.height == 0u || !platform->initialized) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    backend = fractus_platform_backend(platform);
    if (backend == NULL || backend->window == NULL) {
        return FRACTUS_STATUS_ERROR;
    }

    fullscreen = fullscreen ? 1 : 0;
    if (backend->video_width == size.width &&
        backend->video_height == size.height &&
        backend->video_fullscreen == fullscreen) {
        return FRACTUS_STATUS_OK;
    }

    if (fullscreen) {
        if (SDL_SetWindowFullscreen(backend->window, SDL_WINDOW_FULLSCREEN_DESKTOP) != 0) {
            SDL_ClearError();
            (void)SDL_SetWindowFullscreen(backend->window, 0);
            SDL_SetWindowSize(
                backend->window,
                (int)(platform->config.logical_size.width * 2u),
                (int)(platform->config.logical_size.height * 2u));
            SDL_SetWindowPosition(backend->window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        }
    } else {
        if (backend->video_fullscreen &&
            SDL_SetWindowFullscreen(backend->window, 0) != 0) {
            return FRACTUS_STATUS_ERROR;
        }
        SDL_SetWindowSize(backend->window, (int)size.width, (int)size.height);
        SDL_SetWindowPosition(backend->window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    }

    backend->video_width = size.width;
    backend->video_height = size.height;
    backend->video_fullscreen = fullscreen;
    return FRACTUS_STATUS_OK;
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
