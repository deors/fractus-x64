#ifndef FRACTUS_X64_APP_H
#define FRACTUS_X64_APP_H

#include "app/commons.h"

int fractus_app_run(void);

fractus_status fractus_app_render_main_menu(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    int active_index);

#endif
