#include "core/core.h"

fractus_status fractus_core_init(fractus_core *core, fractus_size_u32 logical_size)
{
    if (core == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_framebuffer_init(&core->ui_framebuffer, logical_size) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    if (fractus_framebuffer_init(&core->drawing_framebuffer, logical_size) != FRACTUS_STATUS_OK) {
        fractus_framebuffer_shutdown(&core->ui_framebuffer);
        return FRACTUS_STATUS_ERROR;
    }

    core->initialized = 1;
    return FRACTUS_STATUS_OK;
}

void fractus_core_shutdown(fractus_core *core)
{
    if (core == NULL) {
        return;
    }

    fractus_framebuffer_shutdown(&core->drawing_framebuffer);
    fractus_framebuffer_shutdown(&core->ui_framebuffer);
    core->initialized = 0;
}
