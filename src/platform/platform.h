#ifndef FRACTUS_X64_PLATFORM_H
#define FRACTUS_X64_PLATFORM_H

#include "platform/types.h"

typedef enum fractus_platform_event_type {
    FRACTUS_PLATFORM_EVENT_NONE = 0,
    FRACTUS_PLATFORM_EVENT_QUIT,
    FRACTUS_PLATFORM_EVENT_WINDOW_RESIZED,
    FRACTUS_PLATFORM_EVENT_MOUSE_MOVED,
    FRACTUS_PLATFORM_EVENT_MOUSE_BUTTON_DOWN,
    FRACTUS_PLATFORM_EVENT_MOUSE_BUTTON_UP,
    FRACTUS_PLATFORM_EVENT_KEY_DOWN,
    FRACTUS_PLATFORM_EVENT_KEY_UP,
    FRACTUS_PLATFORM_EVENT_TEXT_INPUT
} fractus_platform_event_type;

typedef enum fractus_platform_key {
    FRACTUS_PLATFORM_KEY_BACKSPACE = 8u,
    FRACTUS_PLATFORM_KEY_RETURN = 13u,
    FRACTUS_PLATFORM_KEY_ESCAPE = 27u,
    FRACTUS_PLATFORM_KEY_DELETE = 127u,
    FRACTUS_PLATFORM_KEY_LEFT = 0x40000050u,
    FRACTUS_PLATFORM_KEY_RIGHT = 0x4000004fu,
    FRACTUS_PLATFORM_KEY_HOME = 0x4000004au,
    FRACTUS_PLATFORM_KEY_END = 0x4000004du
} fractus_platform_key;

typedef struct fractus_platform_config {
    const char *application_name;
    const char *organization_name;
    fractus_size_u32 logical_size;
} fractus_platform_config;

typedef enum fractus_platform_pixel_format {
    FRACTUS_PLATFORM_PIXEL_FORMAT_RGBA8888 = 1
} fractus_platform_pixel_format;

typedef enum fractus_platform_present_mode {
    FRACTUS_PLATFORM_PRESENT_MODE_UI = 0,
    FRACTUS_PLATFORM_PRESENT_MODE_DRAWING
} fractus_platform_present_mode;

typedef struct fractus_platform_surface {
    uint32_t width;
    uint32_t height;
    uint32_t pitch_bytes;
    fractus_platform_pixel_format pixel_format;
    fractus_platform_present_mode present_mode;
    const void *pixels;
} fractus_platform_surface;

typedef struct fractus_platform_event {
    fractus_platform_event_type type;
    union {
        struct {
            fractus_size_u32 size;
        } window_resized;
        struct {
            fractus_point_i32 position;
            uint32_t button;
        } mouse_button;
        struct {
            fractus_point_i32 position;
        } mouse_move;
        struct {
            uint32_t keycode;
        } key;
        struct {
            char text[32];
        } text_input;
    } data;
} fractus_platform_event;

typedef struct fractus_platform_context {
    fractus_platform_config config;
    uint64_t started_at_ms;
    void *backend_state;
    int initialized;
} fractus_platform_context;

fractus_status fractus_platform_init(
    fractus_platform_context *platform,
    const fractus_platform_config *config);
void fractus_platform_shutdown(fractus_platform_context *platform);

fractus_status fractus_platform_poll_event(
    fractus_platform_context *platform,
    fractus_platform_event *event,
    int *has_event);

uint64_t fractus_platform_get_ticks_ms(const fractus_platform_context *platform);
void fractus_platform_sleep_ms(uint32_t duration_ms);
fractus_status fractus_platform_get_output_size(
    const fractus_platform_context *platform,
    fractus_size_u32 *size);

fractus_status fractus_platform_present(
    fractus_platform_context *platform,
    const fractus_platform_surface *surface);
fractus_status fractus_platform_set_video_mode(
    fractus_platform_context *platform,
    fractus_size_u32 size,
    int fullscreen);

fractus_status fractus_platform_get_resource_path(
    const fractus_platform_context *platform,
    const char *relative_path,
    char *buffer,
    size_t buffer_size);

#endif
