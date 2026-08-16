#ifndef FRACTUS_X64_CORE_H
#define FRACTUS_X64_CORE_H

#include "core/fractal.h"
#include "core/formats.h"
#include "platform/framebuffer.h"
#include "platform/types.h"

typedef struct fractus_core {
    fractus_framebuffer ui_framebuffer;
    fractus_framebuffer drawing_framebuffer;
    int initialized;
} fractus_core;

fractus_status fractus_core_init(fractus_core *core, fractus_size_u32 logical_size);
void fractus_core_shutdown(fractus_core *core);

#endif
