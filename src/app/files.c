#include "app/files.h"
#include "app/config.h"
#include "core/formats.h"

#if defined(_WIN32)
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

#include <stdio.h>
#include <string.h>

void fractus_app_sort_palette_files(
    fractus_app_palette_file *files,
    size_t count)
{
    size_t i;
    size_t j;

    if (files == NULL) {
        return;
    }

    for (i = 0u; i < count; ++i) {
        for (j = i + 1u; j < count; ++j) {
            size_t k;
            int swap = 0;

            for (k = 0u; files[i].label[k] != '\0' || files[j].label[k] != '\0'; ++k) {
                char left = fractus_formats_ascii_lower(files[i].label[k]);
                char right = fractus_formats_ascii_lower(files[j].label[k]);

                if (left < right) {
                    break;
                }

                if (left > right) {
                    swap = 1;
                    break;
                }
            }

            if (swap) {
                fractus_app_palette_file temporary = files[i];
                files[i] = files[j];
                files[j] = temporary;
            }
        }
    }
}

void fractus_app_sort_graphic_files(
    fractus_app_graphic_file *files,
    size_t count)
{
    size_t i;
    size_t j;

    if (files == NULL) {
        return;
    }

    for (i = 0u; i < count; ++i) {
        for (j = i + 1u; j < count; ++j) {
            size_t k;
            int swap = 0;

            for (k = 0u; files[i].label[k] != '\0' || files[j].label[k] != '\0'; ++k) {
                char left = fractus_formats_ascii_lower(files[i].label[k]);
                char right = fractus_formats_ascii_lower(files[j].label[k]);

                if (left < right) {
                    break;
                }

                if (left > right) {
                    swap = 1;
                    break;
                }
            }

            if (swap) {
                fractus_app_graphic_file temporary = files[i];
                files[i] = files[j];
                files[j] = temporary;
            }
        }
    }
}

static fractus_status fractus_app_set_palette_entry(
    fractus_framebuffer *framebuffer,
    uint32_t index,
    fractus_color_rgba8 color)
{
    if (framebuffer == NULL || index < 16u || index >= FRACTUS_PALETTE_SIZE) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_palette_set_entry(&framebuffer->palette, index, color) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    framebuffer->palette_dirty = 1;
    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_app_apply_palette_edit(
    const char *cfg_path,
    fractus_framebuffer *framebuffer,
    fractus_legacy_config *config,
    uint32_t index,
    fractus_color_rgba8 color)
{
    if (fractus_app_set_palette_entry(framebuffer, index, color) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return fractus_app_persist_current_palette(cfg_path, framebuffer, config);
}

static fractus_status fractus_app_apply_palette_gradient(
    const char *cfg_path,
    fractus_framebuffer *framebuffer,
    fractus_legacy_config *config,
    uint32_t first_index,
    uint32_t second_index)
{
    fractus_color_rgba8 first_color;
    fractus_color_rgba8 second_color;
    int32_t first_red;
    int32_t first_green;
    int32_t first_blue;
    int32_t second_red;
    int32_t second_green;
    int32_t second_blue;
    uint32_t start;
    uint32_t end;
    uint32_t span;
    uint32_t i;

    if (framebuffer == NULL || config == NULL ||
        first_index < 16u || first_index >= FRACTUS_PALETTE_SIZE ||
        second_index < 16u || second_index >= FRACTUS_PALETTE_SIZE) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (first_index == second_index) {
        return FRACTUS_STATUS_OK;
    }

    if (fractus_palette_get_entry(&framebuffer->palette, first_index, &first_color) != FRACTUS_STATUS_OK ||
        fractus_palette_get_entry(&framebuffer->palette, second_index, &second_color) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    fractus_app_vga_channels_from_color(first_color, &first_red, &first_green, &first_blue);
    fractus_app_vga_channels_from_color(second_color, &second_red, &second_green, &second_blue);
    start = first_index < second_index ? first_index : second_index;
    end = first_index < second_index ? second_index : first_index;
    span = end - start;

    for (i = start; i <= end; ++i) {
        uint32_t offset = i - start;
        int32_t red = (int32_t)((((uint32_t)first_red * (span - offset)) + ((uint32_t)second_red * offset)) / span);
        int32_t green = (int32_t)((((uint32_t)first_green * (span - offset)) + ((uint32_t)second_green * offset)) / span);
        int32_t blue = (int32_t)((((uint32_t)first_blue * (span - offset)) + ((uint32_t)second_blue * offset)) / span);

        if (fractus_app_set_palette_entry(framebuffer, i, fractus_app_vga_color(red, green, blue)) != FRACTUS_STATUS_OK) {
            return FRACTUS_STATUS_ERROR;
        }
    }

    return fractus_app_persist_current_palette(cfg_path, framebuffer, config);
}

fractus_status fractus_app_load_palette_into_state(
    const char *palette_path,
    fractus_framebuffer *framebuffer,
    fractus_legacy_config *config,
    const char *cfg_path)
{
    fractus_palette palette;

    if (palette_path == NULL || framebuffer == NULL || config == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_legacy_palette_load(palette_path, &palette) != FRACTUS_STATUS_OK ||
        fractus_app_apply_palette(framebuffer, &palette) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return fractus_app_persist_current_palette(cfg_path, framebuffer, config);
}

fractus_status fractus_app_load_graphic_palette_into_state(
    const char *graphic_path,
    fractus_framebuffer *framebuffer,
    fractus_legacy_config *config,
    const char *cfg_path)
{
    fractus_indexed_image image;
    fractus_palette palette;
    uint16_t video_mode;
    fractus_status status;

    if (graphic_path == NULL || framebuffer == NULL || config == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    memset(&image, 0, sizeof(image));
    status = fractus_legacy_graphic_load(graphic_path, &image, &palette, &video_mode);
    if (status != FRACTUS_STATUS_OK) {
        return status;
    }

    fractus_indexed_image_shutdown(&image);
    if (fractus_app_apply_palette(framebuffer, &palette) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return fractus_app_persist_current_palette(cfg_path, framebuffer, config);
}

fractus_status fractus_app_restore_default_palette(
    const fractus_platform_context *platform,
    fractus_framebuffer *framebuffer,
    fractus_legacy_config *config,
    const char *cfg_path)
{
    uint32_t i;
    (void)platform;

    if (framebuffer == NULL || config == NULL || !framebuffer->initialized) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    for (i = 0u; i < FRACTUS_LEGACY_PALETTE_DATA_COUNT; ++i) {
        framebuffer->palette.entries[i + 16u] = config->default_palette[i];
    }
    framebuffer->palette_dirty = 1;

    return fractus_app_persist_current_palette(cfg_path, framebuffer, config);
}

fractus_status fractus_app_save_current_palette_file(
    const fractus_platform_context *platform,
    const fractus_framebuffer *framebuffer,
    fractus_legacy_config *config,
    const char *cfg_path)
{
    char palette_path[512];

    if (platform == NULL || framebuffer == NULL || config == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_app_resolve_numbered_write_path(platform, "paletas", ".drsp", palette_path, sizeof(palette_path)) != FRACTUS_STATUS_OK) {
        fractus_app_log("palette: could not resolve a free numbered .drsp write path");
        return FRACTUS_STATUS_UNSUPPORTED;
    }

    fractus_app_log(palette_path);
    if (fractus_legacy_palette_save(palette_path, &framebuffer->palette) != FRACTUS_STATUS_OK) {
        fractus_app_log("palette: saving numbered .drsp failed");
        return FRACTUS_STATUS_ERROR;
    }

    return fractus_app_persist_current_palette(cfg_path, framebuffer, config);
}

fractus_status fractus_app_load_graphic_into_state(
    const char *graphic_path,
    fractus_framebuffer *framebuffer,
    fractus_legacy_config *config,
    const char *cfg_path)
{
    fractus_indexed_image image;
    fractus_palette palette;
    uint16_t video_mode;
    fractus_status status;

    if (graphic_path == NULL || framebuffer == NULL || config == NULL || !framebuffer->initialized) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    memset(&image, 0, sizeof(image));
    status = fractus_legacy_graphic_load(graphic_path, &image, &palette, &video_mode);
    if (status != FRACTUS_STATUS_OK) {
        return status;
    }

    if (framebuffer->size.width != image.size.width || framebuffer->size.height != image.size.height) {
        fractus_framebuffer_shutdown(framebuffer);
        if (fractus_framebuffer_init(framebuffer, image.size) != FRACTUS_STATUS_OK) {
            fractus_indexed_image_shutdown(&image);
            return FRACTUS_STATUS_ERROR;
        }
    }

    if (fractus_app_apply_palette(framebuffer, &palette) != FRACTUS_STATUS_OK ||
        fractus_framebuffer_clear(framebuffer, 0u) != FRACTUS_STATUS_OK ||
        fractus_graphics_blit(framebuffer, (fractus_point_i32){0, 0}, &image) != FRACTUS_STATUS_OK) {
        fractus_indexed_image_shutdown(&image);
        return FRACTUS_STATUS_ERROR;
    }

    fractus_indexed_image_shutdown(&image);
    return fractus_app_persist_current_palette(cfg_path, framebuffer, config);
}

fractus_status fractus_app_change_graphic_palette_into_state(
    const fractus_platform_context *platform,
    const char *graphic_path,
    const char *palette_path,
    fractus_framebuffer *framebuffer,
    fractus_legacy_config *config,
    const char *cfg_path)
{
    fractus_indexed_image image;
    fractus_palette loaded_graphic_palette;
    fractus_palette new_palette;
    uint16_t video_mode;
    char output_path[512];
    fractus_status status;

    if (platform == NULL || graphic_path == NULL || palette_path == NULL ||
        framebuffer == NULL || config == NULL || !framebuffer->initialized) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    memset(&image, 0, sizeof(image));
    status = fractus_legacy_graphic_load(graphic_path, &image, &loaded_graphic_palette, &video_mode);
    if (status != FRACTUS_STATUS_OK) {
        return status;
    }

    status = fractus_legacy_palette_load(palette_path, &new_palette);
    if (status != FRACTUS_STATUS_OK) {
        fractus_indexed_image_shutdown(&image);
        return status;
    }

    status = fractus_app_resolve_numbered_write_path(platform, "dibujos", ".drsg", output_path, sizeof(output_path));
    if (status != FRACTUS_STATUS_OK) {
        fractus_indexed_image_shutdown(&image);
        return status;
    }

    fractus_app_log(output_path);
    status = fractus_legacy_graphic_save(output_path, &image, &new_palette);
    if (status != FRACTUS_STATUS_OK) {
        fractus_indexed_image_shutdown(&image);
        return status;
    }

    if (framebuffer->size.width != image.size.width || framebuffer->size.height != image.size.height) {
        fractus_framebuffer_shutdown(framebuffer);
        if (fractus_framebuffer_init(framebuffer, image.size) != FRACTUS_STATUS_OK) {
            fractus_indexed_image_shutdown(&image);
            return FRACTUS_STATUS_ERROR;
        }
    }

    if (fractus_app_apply_palette(framebuffer, &new_palette) != FRACTUS_STATUS_OK ||
        fractus_framebuffer_clear(framebuffer, 0u) != FRACTUS_STATUS_OK ||
        fractus_graphics_blit(framebuffer, (fractus_point_i32){0, 0}, &image) != FRACTUS_STATUS_OK) {
        fractus_indexed_image_shutdown(&image);
        return FRACTUS_STATUS_ERROR;
    }

    fractus_indexed_image_shutdown(&image);
    return fractus_app_persist_current_palette(cfg_path, framebuffer, config);
}

fractus_status fractus_app_save_current_graphic_file(
    const fractus_platform_context *platform,
    const fractus_framebuffer *framebuffer)
{
    fractus_indexed_image image;
    char graphic_path[512];
    fractus_status status;

    if (platform == NULL || framebuffer == NULL || !framebuffer->initialized) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    memset(&image, 0, sizeof(image));
    if (fractus_app_resolve_numbered_write_path(platform, "dibujos", ".drsg", graphic_path, sizeof(graphic_path)) != FRACTUS_STATUS_OK) {
        fractus_app_log("graphic: could not resolve a free numbered .drsg write path");
        return FRACTUS_STATUS_UNSUPPORTED;
    }

    if (fractus_graphics_capture_region(
            framebuffer,
            (fractus_rect_i32){0, 0, (int32_t)framebuffer->size.width, (int32_t)framebuffer->size.height},
            &image) != FRACTUS_STATUS_OK) {
        fractus_app_log("graphic: capture failed");
        return FRACTUS_STATUS_ERROR;
    }

    fractus_app_log(graphic_path);
    status = fractus_legacy_graphic_save(graphic_path, &image, &framebuffer->palette);
    fractus_indexed_image_shutdown(&image);
    if (status != FRACTUS_STATUS_OK) {
        fractus_app_log("graphic: saving numbered .drsg failed");
        return status;
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_app_save_next_graphic_if_requested(
    const fractus_platform_context *platform,
    const fractus_framebuffer *framebuffer,
    int *save_next_graphic,
    char *error_message,
    size_t error_message_size)
{
    fractus_status status;

    if (save_next_graphic == NULL || *save_next_graphic == 0) {
        return FRACTUS_STATUS_OK;
    }

    *save_next_graphic = 0;
    status = fractus_app_save_current_graphic_file(platform, framebuffer);
    if (status != FRACTUS_STATUS_OK) {
        fractus_app_log("runtime: saving next graphic failed");
        if (status == FRACTUS_STATUS_UNSUPPORTED) {
            (void)fractus_app_set_message(
                error_message,
                error_message_size,
                "No hay nombres libres de fractus0001.drsg a fractus9999.drsg.");
        }
    }

    return FRACTUS_STATUS_OK;
}

size_t fractus_app_list_graphic_files(
    const fractus_platform_context *platform,
    fractus_app_graphic_file *files,
    size_t capacity)
{
    char graphic_directory[512];
    size_t count = 0u;

    if (platform == NULL || files == NULL || capacity == 0u) {
        return 0u;
    }

    if (fractus_formats_resolve_legacy_directory(platform, "dibujos", graphic_directory, sizeof(graphic_directory)) != FRACTUS_STATUS_OK) {
        fractus_app_log("graphic: dibujos directory not found");
        return 0u;
    }

#if defined(_WIN32)
    {
        WIN32_FIND_DATAA find_data;
        HANDLE find_handle;
        char search_path[512];

        if (fractus_formats_join_directory_path(
                graphic_directory,
                "*.*",
                '\\',
                search_path,
                sizeof(search_path)) != FRACTUS_STATUS_OK) {
            return 0u;
        }

        find_handle = FindFirstFileA(search_path, &find_data);
        if (find_handle == INVALID_HANDLE_VALUE) {
            return 0u;
        }

        do {
            if ((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
                continue;
            }

            if (!fractus_formats_has_extension(find_data.cFileName, ".drsg")) {
                continue;
            }

            if (count >= capacity) {
                break;
            }

            if (fractus_formats_copy_path(find_data.cFileName, files[count].label, sizeof(files[count].label)) != FRACTUS_STATUS_OK ||
                fractus_formats_join_directory_path(
                    graphic_directory,
                    find_data.cFileName,
                    '/',
                    files[count].path,
                    sizeof(files[count].path)) != FRACTUS_STATUS_OK) {
                continue;
            }
            ++count;
        } while (FindNextFileA(find_handle, &find_data) != 0);

        FindClose(find_handle);
    }
#else
    {
        DIR *directory = opendir(graphic_directory);
        struct dirent *entry;

        if (directory == NULL) {
            return 0u;
        }

        while ((entry = readdir(directory)) != NULL) {
            if (count >= capacity) {
                break;
            }

            if (!fractus_formats_has_extension(entry->d_name, ".drsg")) {
                continue;
            }

            if (fractus_formats_copy_path(entry->d_name, files[count].label, sizeof(files[count].label)) != FRACTUS_STATUS_OK ||
                fractus_formats_join_directory_path(
                    graphic_directory,
                    entry->d_name,
                    '/',
                    files[count].path,
                    sizeof(files[count].path)) != FRACTUS_STATUS_OK) {
                continue;
            }
            ++count;
        }

        closedir(directory);
    }
#endif

    fractus_app_sort_graphic_files(files, count);
    return count;
}

size_t fractus_app_list_palette_files(
    const fractus_platform_context *platform,
    fractus_app_palette_file *files,
    size_t capacity)
{
    char palette_directory[512];
    size_t count = 0u;

    if (platform == NULL || files == NULL || capacity == 0u) {
        return 0u;
    }

    if (fractus_formats_resolve_legacy_directory(platform, "paletas", palette_directory, sizeof(palette_directory)) != FRACTUS_STATUS_OK) {
        fractus_app_log("palette: paletas directory not found");
        return 0u;
    }

#if defined(_WIN32)
    {
        WIN32_FIND_DATAA find_data;
        HANDLE find_handle;
        char search_path[512];

        if (fractus_formats_join_directory_path(
                palette_directory,
                "*.drsp",
                '\\',
                search_path,
                sizeof(search_path)) != FRACTUS_STATUS_OK) {
            return 0u;
        }

        find_handle = FindFirstFileA(search_path, &find_data);
        if (find_handle == INVALID_HANDLE_VALUE) {
            return 0u;
        }

        do {
            if ((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
                continue;
            }

            if (count >= capacity) {
                break;
            }

            if (fractus_formats_copy_path(find_data.cFileName, files[count].label, sizeof(files[count].label)) != FRACTUS_STATUS_OK ||
                fractus_formats_join_directory_path(
                    palette_directory,
                    find_data.cFileName,
                    '/',
                    files[count].path,
                    sizeof(files[count].path)) != FRACTUS_STATUS_OK) {
                continue;
            }
            ++count;
        } while (FindNextFileA(find_handle, &find_data) != 0);

        FindClose(find_handle);
    }
#else
    {
        DIR *directory = opendir(palette_directory);
        struct dirent *entry;

        if (directory == NULL) {
            return 0u;
        }

        while ((entry = readdir(directory)) != NULL) {
            if (count >= capacity) {
                break;
            }

            if (!fractus_formats_has_extension(entry->d_name, ".drsp")) {
                continue;
            }

            if (fractus_formats_copy_path(entry->d_name, files[count].label, sizeof(files[count].label)) != FRACTUS_STATUS_OK ||
                fractus_formats_join_directory_path(
                    palette_directory,
                    entry->d_name,
                    '/',
                    files[count].path,
                    sizeof(files[count].path)) != FRACTUS_STATUS_OK) {
                continue;
            }
            ++count;
        }

        closedir(directory);
    }
#endif

    fractus_app_sort_palette_files(files, count);
    return count;
}

static size_t fractus_app_build_palette_load_entries(
    fractus_app_menu_entry *entries,
    size_t capacity,
    const fractus_app_palette_file *files,
    size_t file_count,
    size_t page,
    size_t *visible_file_count)
{
    size_t i;
    size_t first_file;
    size_t visible_count;
    int has_previous;
    int has_next;
    int32_t y;

    if (entries == NULL || files == NULL || visible_file_count == NULL || capacity < FRACTUS_APP_PALETTE_FILE_PAGE_SIZE + 3u) {
        return 0u;
    }

    first_file = page * FRACTUS_APP_PALETTE_FILE_PAGE_SIZE;
    if (first_file > file_count) {
        first_file = file_count;
    }

    visible_count = file_count - first_file;
    if (visible_count > FRACTUS_APP_PALETTE_FILE_PAGE_SIZE) {
        visible_count = FRACTUS_APP_PALETTE_FILE_PAGE_SIZE;
    }

    y = 124;
    for (i = 0u; i < visible_count; ++i) {
        fractus_app_set_button(&entries[i], 185, y, 455, y + 20, 8u, 0u, files[first_file + i].label);
        y += 24;
    }

    has_previous = page > 0u;
    has_next = first_file + visible_count < file_count;
    fractus_app_set_button(&entries[visible_count], 185, 374, 225, 394, 8u, has_previous ? 0u : 7u, "<");
    fractus_app_set_button(&entries[visible_count + 1u], 270, 374, 370, 394, 0u, 15u, "Cancelar");
    fractus_app_set_button(&entries[visible_count + 2u], 415, 374, 455, 394, 8u, has_next ? 0u : 7u, ">");
    *visible_file_count = visible_count;
    return visible_count + 3u;
}

static size_t fractus_app_build_graphic_load_entries(
    fractus_app_menu_entry *entries,
    size_t capacity,
    const fractus_app_graphic_file *files,
    size_t file_count,
    size_t page,
    size_t *visible_file_count)
{
    size_t i;
    size_t first_file;
    size_t visible_count;
    int has_previous;
    int has_next;
    int32_t y;

    if (entries == NULL || files == NULL || visible_file_count == NULL || capacity < FRACTUS_APP_GRAPHIC_FILE_PAGE_SIZE + 3u) {
        return 0u;
    }

    first_file = page * FRACTUS_APP_GRAPHIC_FILE_PAGE_SIZE;
    if (first_file > file_count) {
        first_file = file_count;
    }

    visible_count = file_count - first_file;
    if (visible_count > FRACTUS_APP_GRAPHIC_FILE_PAGE_SIZE) {
        visible_count = FRACTUS_APP_GRAPHIC_FILE_PAGE_SIZE;
    }

    y = 124;
    for (i = 0u; i < visible_count; ++i) {
        fractus_app_set_button(&entries[i], 185, y, 455, y + 20, 8u, 0u, files[first_file + i].label);
        y += 24;
    }

    has_previous = page > 0u;
    has_next = first_file + visible_count < file_count;
    fractus_app_set_button(&entries[visible_count], 185, 374, 225, 394, 8u, has_previous ? 0u : 7u, "<");
    fractus_app_set_button(&entries[visible_count + 1u], 270, 374, 370, 394, 0u, 15u, "Cancelar");
    fractus_app_set_button(&entries[visible_count + 2u], 415, 374, 455, 394, 8u, has_next ? 0u : 7u, ">");
    *visible_file_count = visible_count;
    return visible_count + 3u;
}

static fractus_status fractus_app_render_file_load(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const char *title,
    const char *help,
    const char *empty_message,
    const fractus_app_menu_entry *entries,
    size_t entry_count,
    size_t file_count,
    int active_index)
{
    if (framebuffer == NULL || fonts == NULL || title == NULL || help == NULL || empty_message == NULL || entries == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_app_render_main_menu(framebuffer, fonts, -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_window(framebuffer, 170, 76, 469, 404) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, 320, 80, 15u, title) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, 320, 102, 0u, help) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_button_list(framebuffer, fonts, entries, entry_count, active_index) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    if (file_count == 0u) {
        return fractus_ui_draw_text_centered(
            framebuffer,
            fonts,
            FRACTUS_FONT_SMALL,
            320,
            240,
            0u,
            empty_message);
    }

    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_app_render_palette_load(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const char *title,
    const char *help,
    const fractus_app_menu_entry *entries,
    size_t entry_count,
    size_t file_count,
    int active_index)
{
    return fractus_app_render_file_load(
        framebuffer,
        fonts,
        title,
        help,
        "No hay ficheros .drsp disponibles.",
        entries,
        entry_count,
        file_count,
        active_index);
}

static fractus_status fractus_app_render_graphic_load(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const char *title,
    const char *help,
    const fractus_app_menu_entry *entries,
    size_t entry_count,
    size_t file_count,
    int active_index)
{
    return fractus_app_render_file_load(
        framebuffer,
        fonts,
        title,
        help,
        "No hay ficheros .drsg disponibles.",
        entries,
        entry_count,
        file_count,
        active_index);
}

fractus_status fractus_app_run_load_graphic_view(
    fractus_framebuffer *ui_framebuffer,
    fractus_framebuffer *drawing_framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    const fractus_app_graphic_file *graphic_files,
    size_t graphic_file_count,
    size_t *graphic_file_page,
    fractus_legacy_config *legacy_config,
    const char *cfg_path,
    fractus_app_view *view)
{
    fractus_app_menu_entry dialog_entries[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    fractus_ui_menu_option dialog_options[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    size_t dialog_entry_count;
    size_t visible_file_count;
    size_t first_file;
    int selected_menu = -1;
    int cancelled = 0;

    if (ui_framebuffer == NULL || drawing_framebuffer == NULL || fonts == NULL || ui == NULL ||
        graphic_files == NULL || graphic_file_page == NULL || legacy_config == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (graphic_file_count == 0u) {
        *graphic_file_page = 0u;
    } else if (*graphic_file_page * FRACTUS_APP_GRAPHIC_FILE_PAGE_SIZE >= graphic_file_count) {
        *graphic_file_page = (graphic_file_count - 1u) / FRACTUS_APP_GRAPHIC_FILE_PAGE_SIZE;
    }

    dialog_entry_count = fractus_app_build_graphic_load_entries(
        dialog_entries,
        FRACTUS_APP_ARRAY_COUNT(dialog_entries),
        graphic_files,
        graphic_file_count,
        *graphic_file_page,
        &visible_file_count);
    fractus_app_build_options_from_entries(dialog_options, dialog_entries, dialog_entry_count);
    first_file = *graphic_file_page * FRACTUS_APP_GRAPHIC_FILE_PAGE_SIZE;

    /* 1. Contenedor exterior. */
    /* 2. Textos y controles. */
    if (fractus_app_render_graphic_load(
            ui_framebuffer,
            fonts,
            "Cargar un dibujo",
            "Seleccione un fichero .drsg de la carpeta dibujos.",
            dialog_entries,
            dialog_entry_count,
            visible_file_count,
            fractus_ui_active_menu_index(ui, dialog_options, dialog_entry_count)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones de botones. */
    if (fractus_ui_menu(ui, dialog_options, dialog_entry_count, &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == (int)visible_file_count + 1) {
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (selected_menu == (int)visible_file_count) {
            if (*graphic_file_page > 0u) {
                --(*graphic_file_page);
            }
        } else if (selected_menu == (int)visible_file_count + 2) {
            if (first_file + visible_file_count < graphic_file_count) {
                ++(*graphic_file_page);
            }
        } else if (selected_menu >= 0 && (size_t)selected_menu < visible_file_count) {
            if (fractus_app_load_graphic_into_state(
                    graphic_files[first_file + (size_t)selected_menu].path,
                    drawing_framebuffer,
                    legacy_config,
                    cfg_path) != FRACTUS_STATUS_OK ||
                fractus_app_sync_framebuffer_palette(ui_framebuffer, drawing_framebuffer) != FRACTUS_STATUS_OK) {
                fractus_app_log("runtime: loading selected graphic failed");
                *view = FRACTUS_APP_VIEW_MAIN_MENU;
            } else {
                *view = FRACTUS_APP_VIEW_GRAPHIC;
            }
        }
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_app_run_load_palette_view(
    fractus_framebuffer *ui_framebuffer,
    fractus_framebuffer *drawing_framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    const fractus_app_palette_file *palette_files,
    size_t palette_file_count,
    size_t *palette_file_page,
    fractus_legacy_config *legacy_config,
    const char *cfg_path,
    fractus_app_view *view)
{
    fractus_app_menu_entry dialog_entries[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    fractus_ui_menu_option dialog_options[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    size_t dialog_entry_count;
    size_t visible_file_count;
    size_t first_file;
    int selected_menu = -1;
    int cancelled = 0;

    if (ui_framebuffer == NULL || drawing_framebuffer == NULL || fonts == NULL || ui == NULL ||
        palette_files == NULL || palette_file_page == NULL || legacy_config == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (palette_file_count == 0u) {
        *palette_file_page = 0u;
    } else if (*palette_file_page * FRACTUS_APP_PALETTE_FILE_PAGE_SIZE >= palette_file_count) {
        *palette_file_page = (palette_file_count - 1u) / FRACTUS_APP_PALETTE_FILE_PAGE_SIZE;
    }

    dialog_entry_count = fractus_app_build_palette_load_entries(
        dialog_entries,
        FRACTUS_APP_ARRAY_COUNT(dialog_entries),
        palette_files,
        palette_file_count,
        *palette_file_page,
        &visible_file_count);
    fractus_app_build_options_from_entries(dialog_options, dialog_entries, dialog_entry_count);
    first_file = *palette_file_page * FRACTUS_APP_PALETTE_FILE_PAGE_SIZE;

    /* 1. Contenedor exterior. */
    /* 2. Textos y controles. */
    if (fractus_app_render_palette_load(
            ui_framebuffer,
            fonts,
            "Cargar una paleta",
            "Seleccione un fichero .drsp de la carpeta paletas.",
            dialog_entries,
            dialog_entry_count,
            visible_file_count,
            fractus_ui_active_menu_index(ui, dialog_options, dialog_entry_count)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones de botones. */
    if (fractus_ui_menu(ui, dialog_options, dialog_entry_count, &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == (int)visible_file_count + 1) {
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (selected_menu == (int)visible_file_count) {
            if (*palette_file_page > 0u) {
                --(*palette_file_page);
            }
        } else if (selected_menu == (int)visible_file_count + 2) {
            if (first_file + visible_file_count < palette_file_count) {
                ++(*palette_file_page);
            }
        } else if (selected_menu >= 0 && (size_t)selected_menu < visible_file_count) {
            if (fractus_app_load_palette_into_state(
                    palette_files[first_file + (size_t)selected_menu].path,
                    ui_framebuffer,
                    legacy_config,
                    cfg_path) != FRACTUS_STATUS_OK ||
                fractus_app_sync_framebuffer_palette(drawing_framebuffer, ui_framebuffer) != FRACTUS_STATUS_OK) {
                fractus_app_log("runtime: loading selected palette failed");
            }

            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        }
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_app_run_load_graphic_palette_view(
    fractus_framebuffer *ui_framebuffer,
    fractus_framebuffer *drawing_framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    const fractus_app_graphic_file *graphic_files,
    size_t graphic_file_count,
    size_t *graphic_file_page,
    fractus_legacy_config *legacy_config,
    const char *cfg_path,
    fractus_app_view *view)
{
    fractus_app_menu_entry dialog_entries[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    fractus_ui_menu_option dialog_options[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    size_t dialog_entry_count;
    size_t visible_file_count;
    size_t first_file;
    int selected_menu = -1;
    int cancelled = 0;

    if (ui_framebuffer == NULL || drawing_framebuffer == NULL || fonts == NULL || ui == NULL ||
        graphic_files == NULL || graphic_file_page == NULL || legacy_config == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (graphic_file_count == 0u) {
        *graphic_file_page = 0u;
    } else if (*graphic_file_page * FRACTUS_APP_GRAPHIC_FILE_PAGE_SIZE >= graphic_file_count) {
        *graphic_file_page = (graphic_file_count - 1u) / FRACTUS_APP_GRAPHIC_FILE_PAGE_SIZE;
    }

    dialog_entry_count = fractus_app_build_graphic_load_entries(
        dialog_entries,
        FRACTUS_APP_ARRAY_COUNT(dialog_entries),
        graphic_files,
        graphic_file_count,
        *graphic_file_page,
        &visible_file_count);
    fractus_app_build_options_from_entries(dialog_options, dialog_entries, dialog_entry_count);
    first_file = *graphic_file_page * FRACTUS_APP_GRAPHIC_FILE_PAGE_SIZE;

    /* 1. Contenedor exterior. */
    /* 2. Textos y controles. */
    if (fractus_app_render_graphic_load(
            ui_framebuffer,
            fonts,
            "Cargar la paleta de un dibujo",
            "Seleccione un dibujo .drsg para usar su paleta.",
            dialog_entries,
            dialog_entry_count,
            visible_file_count,
            fractus_ui_active_menu_index(ui, dialog_options, dialog_entry_count)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones de botones. */
    if (fractus_ui_menu(ui, dialog_options, dialog_entry_count, &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == (int)visible_file_count + 1) {
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (selected_menu == (int)visible_file_count) {
            if (*graphic_file_page > 0u) {
                --(*graphic_file_page);
            }
        } else if (selected_menu == (int)visible_file_count + 2) {
            if (first_file + visible_file_count < graphic_file_count) {
                ++(*graphic_file_page);
            }
        } else if (selected_menu >= 0 && (size_t)selected_menu < visible_file_count) {
            if (fractus_app_load_graphic_palette_into_state(
                    graphic_files[first_file + (size_t)selected_menu].path,
                    ui_framebuffer,
                    legacy_config,
                    cfg_path) != FRACTUS_STATUS_OK ||
                fractus_app_sync_framebuffer_palette(drawing_framebuffer, ui_framebuffer) != FRACTUS_STATUS_OK) {
                fractus_app_log("runtime: loading palette from selected graphic failed");
            }

            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        }
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_app_run_change_graphic_palette_graphic_view(
    fractus_platform_context *platform,
    fractus_framebuffer *ui_framebuffer,
    fractus_framebuffer *drawing_framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    const fractus_app_graphic_file *graphic_files,
    size_t graphic_file_count,
    size_t *graphic_file_page,
    fractus_app_palette_file *palette_files,
    size_t *palette_file_count,
    size_t *palette_file_page,
    char *selected_graphic_path,
    size_t selected_graphic_path_size,
    fractus_legacy_config *legacy_config,
    const char *cfg_path,
    fractus_app_view *view)
{
    fractus_app_menu_entry dialog_entries[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    fractus_ui_menu_option dialog_options[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    size_t dialog_entry_count;
    size_t visible_file_count;
    size_t first_file;
    int selected_menu = -1;
    int cancelled = 0;

    if (platform == NULL || ui_framebuffer == NULL || drawing_framebuffer == NULL ||
        fonts == NULL || ui == NULL || graphic_files == NULL || palette_files == NULL ||
        graphic_file_page == NULL || palette_file_count == NULL || palette_file_page == NULL || selected_graphic_path == NULL || legacy_config == NULL ||
        view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (graphic_file_count == 0u) {
        *graphic_file_page = 0u;
    } else if (*graphic_file_page * FRACTUS_APP_GRAPHIC_FILE_PAGE_SIZE >= graphic_file_count) {
        *graphic_file_page = (graphic_file_count - 1u) / FRACTUS_APP_GRAPHIC_FILE_PAGE_SIZE;
    }

    dialog_entry_count = fractus_app_build_graphic_load_entries(
        dialog_entries,
        FRACTUS_APP_ARRAY_COUNT(dialog_entries),
        graphic_files,
        graphic_file_count,
        *graphic_file_page,
        &visible_file_count);
    fractus_app_build_options_from_entries(dialog_options, dialog_entries, dialog_entry_count);
    first_file = *graphic_file_page * FRACTUS_APP_GRAPHIC_FILE_PAGE_SIZE;

    /* 1. Contenedor exterior. */
    /* 2. Textos y controles. */
    if (fractus_app_render_graphic_load(
            ui_framebuffer,
            fonts,
            "Cambiar la paleta de un dibujo",
            "Primero seleccione el dibujo .drsg.",
            dialog_entries,
            dialog_entry_count,
            visible_file_count,
            fractus_ui_active_menu_index(ui, dialog_options, dialog_entry_count)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones de botones. */
    if (fractus_ui_menu(ui, dialog_options, dialog_entry_count, &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == (int)visible_file_count + 1) {
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (selected_menu == (int)visible_file_count) {
            if (*graphic_file_page > 0u) {
                --(*graphic_file_page);
            }
        } else if (selected_menu == (int)visible_file_count + 2) {
            if (first_file + visible_file_count < graphic_file_count) {
                ++(*graphic_file_page);
            }
        } else if (selected_menu >= 0 && (size_t)selected_menu < visible_file_count) {
            if (fractus_formats_copy_path(
                    graphic_files[first_file + (size_t)selected_menu].path,
                    selected_graphic_path,
                    selected_graphic_path_size) != FRACTUS_STATUS_OK ||
                fractus_app_load_graphic_into_state(
                    selected_graphic_path,
                    drawing_framebuffer,
                    legacy_config,
                    cfg_path) != FRACTUS_STATUS_OK ||
                fractus_app_sync_framebuffer_palette(ui_framebuffer, drawing_framebuffer) != FRACTUS_STATUS_OK) {
                fractus_app_log("runtime: loading graphic before palette change failed");
                selected_graphic_path[0] = '\0';
                *view = FRACTUS_APP_VIEW_MAIN_MENU;
            } else {
                *palette_file_count = fractus_app_list_palette_files(platform, palette_files, FRACTUS_APP_PALETTE_FILE_CAPACITY);
                *palette_file_page = 0u;
                *view = FRACTUS_APP_VIEW_CHANGE_GRAPHIC_PALETTE_PALETTE;
            }
        }
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_app_run_change_graphic_palette_palette_view(
    fractus_platform_context *platform,
    fractus_framebuffer *ui_framebuffer,
    fractus_framebuffer *drawing_framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    const fractus_app_palette_file *palette_files,
    size_t palette_file_count,
    size_t *palette_file_page,
    char *selected_graphic_path,
    fractus_legacy_config *legacy_config,
    const char *cfg_path,
    char *error_message,
    size_t error_message_size,
    fractus_app_view *view)
{
    fractus_app_menu_entry dialog_entries[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    fractus_ui_menu_option dialog_options[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    size_t dialog_entry_count;
    size_t visible_file_count;
    size_t first_file;
    int selected_menu = -1;
    int cancelled = 0;

    if (platform == NULL || ui_framebuffer == NULL || drawing_framebuffer == NULL ||
        fonts == NULL || ui == NULL || palette_files == NULL || selected_graphic_path == NULL ||
        palette_file_page == NULL || legacy_config == NULL || error_message == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (palette_file_count == 0u) {
        *palette_file_page = 0u;
    } else if (*palette_file_page * FRACTUS_APP_PALETTE_FILE_PAGE_SIZE >= palette_file_count) {
        *palette_file_page = (palette_file_count - 1u) / FRACTUS_APP_PALETTE_FILE_PAGE_SIZE;
    }

    dialog_entry_count = fractus_app_build_palette_load_entries(
        dialog_entries,
        FRACTUS_APP_ARRAY_COUNT(dialog_entries),
        palette_files,
        palette_file_count,
        *palette_file_page,
        &visible_file_count);
    fractus_app_build_options_from_entries(dialog_options, dialog_entries, dialog_entry_count);
    first_file = *palette_file_page * FRACTUS_APP_PALETTE_FILE_PAGE_SIZE;

    /* 1. Contenedor exterior. */
    /* 2. Textos y controles. */
    if (fractus_app_render_palette_load(
            ui_framebuffer,
            fonts,
            "Cambiar la paleta de un dibujo",
            "Ahora seleccione la nueva paleta .drsp.",
            dialog_entries,
            dialog_entry_count,
            visible_file_count,
            fractus_ui_active_menu_index(ui, dialog_options, dialog_entry_count)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones de botones. */
    if (fractus_ui_menu(ui, dialog_options, dialog_entry_count, &selected_menu, &cancelled)) {
        if (cancelled || selected_menu == (int)visible_file_count + 1) {
            selected_graphic_path[0] = '\0';
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (selected_menu == (int)visible_file_count) {
            if (*palette_file_page > 0u) {
                --(*palette_file_page);
            }
        } else if (selected_menu == (int)visible_file_count + 2) {
            if (first_file + visible_file_count < palette_file_count) {
                ++(*palette_file_page);
            }
        } else if (selected_menu >= 0 && (size_t)selected_menu < visible_file_count) {
            fractus_status change_status = fractus_app_change_graphic_palette_into_state(
                platform,
                selected_graphic_path,
                palette_files[first_file + (size_t)selected_menu].path,
                drawing_framebuffer,
                legacy_config,
                cfg_path);
            if (change_status != FRACTUS_STATUS_OK) {
                fractus_app_log("runtime: changing selected graphic palette failed");
                if (change_status == FRACTUS_STATUS_UNSUPPORTED) {
                    (void)fractus_app_set_message(
                        error_message,
                        error_message_size,
                        "No hay nombres libres de fractus0001.drsg a fractus9999.drsg.");
                    selected_graphic_path[0] = '\0';
                    *view = FRACTUS_APP_VIEW_ERROR;
                } else {
                    selected_graphic_path[0] = '\0';
                    *view = FRACTUS_APP_VIEW_MAIN_MENU;
                }
            } else {
                if (fractus_app_sync_framebuffer_palette(ui_framebuffer, drawing_framebuffer) != FRACTUS_STATUS_OK) {
                    return FRACTUS_STATUS_ERROR;
                }
                selected_graphic_path[0] = '\0';
                *view = FRACTUS_APP_VIEW_GRAPHIC;
            }
        }
    }

    return FRACTUS_STATUS_OK;
}

static fractus_status fractus_app_draw_palette_grid(fractus_framebuffer *framebuffer)
{
    uint32_t color_index = 16u;
    int32_t row;
    int32_t column;

    if (framebuffer == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    for (row = 0; row < FRACTUS_APP_PALETTE_GRID_ROWS; ++row) {
        for (column = 0; column < FRACTUS_APP_PALETTE_GRID_COLUMNS; ++column) {
            fractus_rect_i32 cell = {
                FRACTUS_APP_PALETTE_GRID_X + column * FRACTUS_APP_PALETTE_CELL_STEP_X,
                FRACTUS_APP_PALETTE_GRID_Y + row * FRACTUS_APP_PALETTE_CELL_STEP_Y,
                FRACTUS_APP_PALETTE_CELL_WIDTH,
                FRACTUS_APP_PALETTE_CELL_HEIGHT};

            if (fractus_graphics_fill_rect(framebuffer, cell, (uint8_t)color_index) != FRACTUS_STATUS_OK ||
                fractus_graphics_rect(framebuffer, cell, 15u) != FRACTUS_STATUS_OK) {
                return FRACTUS_STATUS_ERROR;
            }

            ++color_index;
        }
    }

    return FRACTUS_STATUS_OK;
}

static int fractus_app_palette_index_at(fractus_point_i32 point, uint32_t *index)
{
    int32_t relative_x = point.x - FRACTUS_APP_PALETTE_GRID_X;
    int32_t relative_y = point.y - FRACTUS_APP_PALETTE_GRID_Y;
    int32_t column;
    int32_t row;
    int32_t cell_x;
    int32_t cell_y;

    if (index == NULL || relative_x < 0 || relative_y < 0) {
        return 0;
    }

    column = relative_x / FRACTUS_APP_PALETTE_CELL_STEP_X;
    row = relative_y / FRACTUS_APP_PALETTE_CELL_STEP_Y;
    cell_x = relative_x % FRACTUS_APP_PALETTE_CELL_STEP_X;
    cell_y = relative_y % FRACTUS_APP_PALETTE_CELL_STEP_Y;

    if (column < 0 ||
        column >= FRACTUS_APP_PALETTE_GRID_COLUMNS ||
        row < 0 ||
        row >= FRACTUS_APP_PALETTE_GRID_ROWS ||
        cell_x <= 0 ||
        cell_y <= 0 ||
        cell_x >= FRACTUS_APP_PALETTE_CELL_WIDTH ||
        cell_y >= FRACTUS_APP_PALETTE_CELL_HEIGHT ||
        cell_x == FRACTUS_APP_PALETTE_CELL_WIDTH - 1 ||
        cell_y == FRACTUS_APP_PALETTE_CELL_HEIGHT - 1) {
        return 0;
    }

    *index = 16u + (uint32_t)(row * FRACTUS_APP_PALETTE_GRID_COLUMNS + column);
    return 1;
}

static int fractus_app_palette_pick(
    const fractus_ui_context *ui,
    uint32_t *index,
    int *cancelled)
{
    if (cancelled != NULL) {
        *cancelled = 0;
    }

    if (ui == NULL || index == NULL) {
        return 0;
    }

    if ((ui->key_press_pending && ui->key_pressed == 27u) ||
        (ui->press_pending && ui->press_event.buttons.right)) {
        if (cancelled != NULL) {
            *cancelled = 1;
        }
        return 1;
    }

    if (ui->release_pending &&
        ui->release_event.buttons.left &&
        fractus_ui_point_in_rect(ui->release_event.position, (fractus_rect_i32)FRACTUS_APP_RECT(270, 401, 370, 420))) {
        if (cancelled != NULL) {
            *cancelled = 1;
        }
        return 1;
    }

    if (ui->release_pending &&
        ui->release_event.buttons.left &&
        fractus_app_palette_index_at(ui->release_event.position, index)) {
        return 1;
    }

    return 0;
}

static int fractus_app_palette_back_is_pressed(const fractus_ui_context *ui)
{
    return ui != NULL &&
           ui->buttons_down.left &&
           fractus_ui_point_in_rect(ui->pointer_position, (fractus_rect_i32)FRACTUS_APP_RECT(270, 401, 370, 420));
}

static int fractus_app_palette_back_was_clicked(const fractus_ui_context *ui)
{
    return ui != NULL &&
           ui->release_pending &&
           ui->release_event.buttons.left &&
           fractus_ui_point_in_rect(ui->release_event.position, (fractus_rect_i32)FRACTUS_APP_RECT(270, 401, 370, 420));
}

static fractus_status fractus_app_render_palette_screen(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    const char *title,
    const char *line1,
    int back_pressed)
{
    const fractus_app_menu_entry back_button = {
        FRACTUS_APP_RECT(270, 401, 370, 420), 0u, 15u, "Volver"
    };

    if (framebuffer == NULL || fonts == NULL || title == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    if (fractus_app_render_main_menu(framebuffer, fonts, -1) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_window(framebuffer, 36, 48, 603, 430) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, 320, 52, 15u, title) != FRACTUS_STATUS_OK ||
        fractus_app_draw_palette_grid(framebuffer) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    if (line1 != NULL &&
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, 320, 383, 0u, line1) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return fractus_ui_draw_button(
        framebuffer,
        fonts,
        &back_button,
        back_pressed);
}

static fractus_status fractus_app_render_palette_view_current(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    int back_pressed)
{
    return fractus_app_render_palette_screen(
        framebuffer,
        fonts,
        "Paleta actual",
        "La paleta .drsp usa trios de bytes RGB con valores VGA 0-63 por canal, no 0-255.",
        back_pressed);
}

static fractus_status fractus_app_render_palette_color_edit(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    uint32_t palette_index,
    const fractus_ui_numeric_field *red_field,
    const fractus_ui_numeric_field *green_field,
    const fractus_ui_numeric_field *blue_field,
    const fractus_app_menu_entry *entries,
    size_t entry_count,
    int active_index)
{
    char buffer[48];
    fractus_rect_i32 preview = {345, 189, 145, 58};

    if (framebuffer == NULL || fonts == NULL || red_field == NULL || green_field == NULL ||
        blue_field == NULL || entries == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    snprintf(buffer, sizeof(buffer), "Indice de la paleta a modificar %u", (unsigned)palette_index);

    if (fractus_app_render_palette_screen(
            framebuffer,
            fonts,
            "Modificar un color de la paleta",
            "",
            0) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_window(framebuffer, 135, 142, 504, 336) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_ARIAL, 320, 146, 15u, "Modificar color") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_group_box(framebuffer, fonts, 140, 174, 499, 280, 8u, 0u, buffer) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, 320, 288, 0u, "Ajuste los canales RGB con valores VGA entre 0 y 63 y pulse Guardar.") != FRACTUS_STATUS_OK ||
        fractus_graphics_fill_rect(framebuffer, preview, (uint8_t)palette_index) != FRACTUS_STATUS_OK ||
        fractus_graphics_rect(framebuffer, preview, 15u) != FRACTUS_STATUS_OK ||
        fractus_ui_draw_text_centered(framebuffer, fonts, FRACTUS_FONT_SMALL, 417, 254, 0u, "Previsualizacion") != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    if (fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 150, 194, 0u, "Rojo") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, red_field) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    if (fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 150, 221, 0u, "Verde") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, green_field) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    if (fractus_ui_draw_text_left(framebuffer, fonts, FRACTUS_FONT_SMALL, 150, 248, 0u, "Azul") != FRACTUS_STATUS_OK ||
        fractus_ui_draw_numeric_field(framebuffer, fonts, blue_field) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    return fractus_ui_draw_button_list(framebuffer, fonts, entries, entry_count, active_index);
}

fractus_status fractus_app_run_palette_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_app_view *view)
{
    if (framebuffer == NULL || fonts == NULL || ui == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    /* 1. Contenedor exterior. */
    /* 2. Textos y controles. */
    if (fractus_app_render_palette_view_current(
            framebuffer,
            fonts,
            fractus_app_palette_back_is_pressed(ui)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones de botones. */
    if (ui->release_pending &&
        ui->release_event.buttons.left &&
        fractus_ui_point_in_rect(ui->release_event.position, (fractus_rect_i32)FRACTUS_APP_RECT(270, 401, 370, 420))) {
        *view = FRACTUS_APP_VIEW_MAIN_MENU;
    }

    return FRACTUS_STATUS_OK;
}

static size_t fractus_app_build_palette_color_entries(
    fractus_app_menu_entry *entries,
    size_t capacity)
{
    static const fractus_app_menu_entry controls[] = {
        {FRACTUS_APP_RECT(244, 189, 284, 209), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(289, 189, 329, 209), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(244, 216, 284, 236), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(289, 216, 329, 236), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(244, 243, 284, 263), 8u, 0u, "-"},
        {FRACTUS_APP_RECT(289, 243, 329, 263), 8u, 0u, "+"},
        {FRACTUS_APP_RECT(210, 306, 310, 326), 8u, 0u, "Guardar"},
        {FRACTUS_APP_RECT(330, 306, 430, 326), 0u, 15u, "Cancelar"}
    };

    return fractus_app_copy_control_entries(
        entries,
        capacity,
        controls,
        FRACTUS_APP_ARRAY_COUNT(controls));
}

fractus_status fractus_app_run_palette_edit_select_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    uint32_t *palette_selected_index,
    fractus_color_rgba8 *palette_original_color,
    fractus_color_rgba8 *palette_pending_color,
    fractus_ui_numeric_field *red_field,
    fractus_ui_numeric_field *green_field,
    fractus_ui_numeric_field *blue_field,
    fractus_app_view *view)
{
    uint32_t picked_index;
    int cancelled = 0;

    if (framebuffer == NULL || fonts == NULL || ui == NULL ||
        palette_selected_index == NULL || palette_original_color == NULL ||
        palette_pending_color == NULL || red_field == NULL || green_field == NULL ||
        blue_field == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    /* 1. Contenedor exterior. */
    /* 2. Textos y controles. */
    if (fractus_app_render_palette_screen(
            framebuffer,
            fonts,
            "Modificar un color de la paleta",
            "Seleccione el color que desea modificar.",
            fractus_app_palette_back_is_pressed(ui)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones. */
    if (fractus_app_palette_pick(ui, &picked_index, &cancelled)) {
        if (cancelled) {
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else {
            *palette_selected_index = picked_index;
            if (fractus_palette_get_entry(&framebuffer->palette, *palette_selected_index, palette_original_color) != FRACTUS_STATUS_OK) {
                fractus_app_log("runtime: reading selected palette color failed");
                *view = FRACTUS_APP_VIEW_MAIN_MENU;
            } else {
                *palette_pending_color = *palette_original_color;
                fractus_app_init_palette_color_fields(red_field, green_field, blue_field, *palette_pending_color);
                *view = FRACTUS_APP_VIEW_PALETTE_EDIT_COLOR;
            }
        }
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_app_run_palette_edit_color_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    const char *cfg_path,
    fractus_legacy_config *legacy_config,
    uint32_t palette_selected_index,
    fractus_color_rgba8 palette_original_color,
    fractus_color_rgba8 *palette_pending_color,
    fractus_ui_numeric_field *red_field,
    fractus_ui_numeric_field *green_field,
    fractus_ui_numeric_field *blue_field,
    fractus_app_view *view)
{
    fractus_app_menu_entry dialog_entries[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    fractus_ui_menu_option dialog_options[FRACTUS_APP_DIALOG_BUTTON_CAPACITY];
    size_t dialog_entry_count;
    int32_t red;
    int32_t green;
    int32_t blue;
    int selected_menu = -1;
    int cancelled = 0;
    int skip_palette_color_menu = 0;

    if (framebuffer == NULL || fonts == NULL || ui == NULL || legacy_config == NULL ||
        palette_pending_color == NULL || red_field == NULL || green_field == NULL ||
        blue_field == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    dialog_entry_count = fractus_app_build_palette_color_entries(dialog_entries, FRACTUS_APP_ARRAY_COUNT(dialog_entries));
    fractus_app_build_options_from_entries(dialog_options, dialog_entries, dialog_entry_count);
    fractus_app_set_palette_entry(framebuffer, palette_selected_index, *palette_pending_color);

    /* 1. Contenedor exterior. */
    /* 2. Textos y controles. */
    if (fractus_app_render_palette_color_edit(
            framebuffer,
            fonts,
            palette_selected_index,
            red_field,
            green_field,
            blue_field,
            dialog_entries,
            dialog_entry_count,
            fractus_ui_active_menu_index(ui, dialog_options, dialog_entry_count)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado y raton en los campos editables con foco exclusivo. */
    fractus_app_vga_channels_from_color(*palette_pending_color, &red, &green, &blue);

    if (ui->press_pending && ui->press_event.buttons.left) {
        fractus_point_i32 click_pos = ui->press_event.position;
        if (fractus_ui_point_in_rect(click_pos, red_field->bounds)) {
            int32_t val;
            if (green_field->editing && fractus_ui_numeric_field_get_int(green_field, &val) == FRACTUS_STATUS_OK) {
                green = val;
            }
            if (blue_field->editing && fractus_ui_numeric_field_get_int(blue_field, &val) == FRACTUS_STATUS_OK) {
                blue = val;
            }
            *palette_pending_color = fractus_app_vga_color(red, green, blue);
            fractus_app_init_palette_color_fields(red_field, green_field, blue_field, *palette_pending_color);
            (void)fractus_ui_numeric_field_begin_edit(red_field);
        } else if (fractus_ui_point_in_rect(click_pos, green_field->bounds)) {
            int32_t val;
            if (red_field->editing && fractus_ui_numeric_field_get_int(red_field, &val) == FRACTUS_STATUS_OK) {
                red = val;
            }
            if (blue_field->editing && fractus_ui_numeric_field_get_int(blue_field, &val) == FRACTUS_STATUS_OK) {
                blue = val;
            }
            *palette_pending_color = fractus_app_vga_color(red, green, blue);
            fractus_app_init_palette_color_fields(red_field, green_field, blue_field, *palette_pending_color);
            (void)fractus_ui_numeric_field_begin_edit(green_field);
        } else if (fractus_ui_point_in_rect(click_pos, blue_field->bounds)) {
            int32_t val;
            if (red_field->editing && fractus_ui_numeric_field_get_int(red_field, &val) == FRACTUS_STATUS_OK) {
                red = val;
            }
            if (green_field->editing && fractus_ui_numeric_field_get_int(green_field, &val) == FRACTUS_STATUS_OK) {
                green = val;
            }
            *palette_pending_color = fractus_app_vga_color(red, green, blue);
            fractus_app_init_palette_color_fields(red_field, green_field, blue_field, *palette_pending_color);
            (void)fractus_ui_numeric_field_begin_edit(blue_field);
        } else {
            int32_t val;
            int changed = 0;
            if (red_field->editing) {
                if (fractus_ui_numeric_field_get_int(red_field, &val) == FRACTUS_STATUS_OK) {
                    red = val;
                    changed = 1;
                }
            }
            if (green_field->editing) {
                if (fractus_ui_numeric_field_get_int(green_field, &val) == FRACTUS_STATUS_OK) {
                    green = val;
                    changed = 1;
                }
            }
            if (blue_field->editing) {
                if (fractus_ui_numeric_field_get_int(blue_field, &val) == FRACTUS_STATUS_OK) {
                    blue = val;
                    changed = 1;
                }
            }
            if (changed) {
                *palette_pending_color = fractus_app_vga_color(red, green, blue);
            }
            fractus_app_init_palette_color_fields(red_field, green_field, blue_field, *palette_pending_color);
        }
    }

    if (red_field->editing) {
        int edit_accepted = 0;
        int edit_cancelled = 0;
        if (fractus_ui_numeric_field_handle_input(red_field, ui, fonts, &edit_accepted, &edit_cancelled) == FRACTUS_STATUS_OK) {
            if (edit_accepted) {
                int32_t val;
                if (fractus_ui_numeric_field_get_int(red_field, &val) == FRACTUS_STATUS_OK) {
                    red = val;
                    *palette_pending_color = fractus_app_vga_color(red, green, blue);
                }
                fractus_app_init_palette_color_fields(red_field, green_field, blue_field, *palette_pending_color);
            } else if (edit_cancelled) {
                fractus_app_init_palette_color_fields(red_field, green_field, blue_field, *palette_pending_color);
            }
        }
        skip_palette_color_menu = 1;
    } else if (green_field->editing) {
        int edit_accepted = 0;
        int edit_cancelled = 0;
        if (fractus_ui_numeric_field_handle_input(green_field, ui, fonts, &edit_accepted, &edit_cancelled) == FRACTUS_STATUS_OK) {
            if (edit_accepted) {
                int32_t val;
                if (fractus_ui_numeric_field_get_int(green_field, &val) == FRACTUS_STATUS_OK) {
                    green = val;
                    *palette_pending_color = fractus_app_vga_color(red, green, blue);
                }
                fractus_app_init_palette_color_fields(red_field, green_field, blue_field, *palette_pending_color);
            } else if (edit_cancelled) {
                fractus_app_init_palette_color_fields(red_field, green_field, blue_field, *palette_pending_color);
            }
        }
        skip_palette_color_menu = 1;
    } else if (blue_field->editing) {
        int edit_accepted = 0;
        int edit_cancelled = 0;
        if (fractus_ui_numeric_field_handle_input(blue_field, ui, fonts, &edit_accepted, &edit_cancelled) == FRACTUS_STATUS_OK) {
            if (edit_accepted) {
                int32_t val;
                if (fractus_ui_numeric_field_get_int(blue_field, &val) == FRACTUS_STATUS_OK) {
                    blue = val;
                    *palette_pending_color = fractus_app_vga_color(red, green, blue);
                }
                fractus_app_init_palette_color_fields(red_field, green_field, blue_field, *palette_pending_color);
            } else if (edit_cancelled) {
                fractus_app_init_palette_color_fields(red_field, green_field, blue_field, *palette_pending_color);
            }
        }
        skip_palette_color_menu = 1;
    }

    /* 4. Raton y acciones de botones. */
    if (!skip_palette_color_menu &&
        fractus_ui_menu(ui, dialog_options, dialog_entry_count, &selected_menu, &cancelled)) {
        fractus_app_vga_channels_from_color(*palette_pending_color, &red, &green, &blue);
        if (cancelled || selected_menu == FRACTUS_APP_PALETTE_COLOR_CANCEL) {
            fractus_app_set_palette_entry(framebuffer, palette_selected_index, palette_original_color);
            *view = FRACTUS_APP_VIEW_PALETTE_EDIT_SELECT;
        } else if (selected_menu == FRACTUS_APP_PALETTE_COLOR_ACCEPT) {
            if (fractus_app_apply_palette_edit(
                    cfg_path,
                    framebuffer,
                    legacy_config,
                    palette_selected_index,
                    *palette_pending_color) != FRACTUS_STATUS_OK) {
                fractus_app_log("runtime: saving edited palette color failed");
            }
            *view = FRACTUS_APP_VIEW_PALETTE_EDIT_SELECT;
        } else if (selected_menu == FRACTUS_APP_PALETTE_RED_DEC) {
            *palette_pending_color = fractus_app_vga_color(red - 1, green, blue);
            fractus_app_init_palette_color_fields(red_field, green_field, blue_field, *palette_pending_color);
        } else if (selected_menu == FRACTUS_APP_PALETTE_RED_INC) {
            *palette_pending_color = fractus_app_vga_color(red + 1, green, blue);
            fractus_app_init_palette_color_fields(red_field, green_field, blue_field, *palette_pending_color);
        } else if (selected_menu == FRACTUS_APP_PALETTE_GREEN_DEC) {
            *palette_pending_color = fractus_app_vga_color(red, green - 1, blue);
            fractus_app_init_palette_color_fields(red_field, green_field, blue_field, *palette_pending_color);
        } else if (selected_menu == FRACTUS_APP_PALETTE_GREEN_INC) {
            *palette_pending_color = fractus_app_vga_color(red, green + 1, blue);
            fractus_app_init_palette_color_fields(red_field, green_field, blue_field, *palette_pending_color);
        } else if (selected_menu == FRACTUS_APP_PALETTE_BLUE_DEC) {
            *palette_pending_color = fractus_app_vga_color(red, green, blue - 1);
            fractus_app_init_palette_color_fields(red_field, green_field, blue_field, *palette_pending_color);
        } else if (selected_menu == FRACTUS_APP_PALETTE_BLUE_INC) {
            *palette_pending_color = fractus_app_vga_color(red, green, blue + 1);
            fractus_app_init_palette_color_fields(red_field, green_field, blue_field, *palette_pending_color);
        }
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_app_run_palette_copy_source_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    fractus_color_rgba8 *palette_copy_color,
    uint32_t *palette_copy_source_index,
    fractus_app_view *view)
{
    uint32_t picked_index;
    int cancelled = 0;

    if (framebuffer == NULL || fonts == NULL || ui == NULL ||
        palette_copy_color == NULL || palette_copy_source_index == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    /* 1. Contenedor exterior. */
    /* 2. Textos y controles. */
    if (fractus_app_render_palette_screen(
            framebuffer,
            fonts,
            "Copiar un color de la paleta",
            "Seleccione el color origen.",
            fractus_app_palette_back_is_pressed(ui)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones. */
    if (fractus_app_palette_pick(ui, &picked_index, &cancelled)) {
        if (cancelled) {
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else if (fractus_palette_get_entry(&framebuffer->palette, picked_index, palette_copy_color) == FRACTUS_STATUS_OK) {
            *palette_copy_source_index = picked_index;
            *view = FRACTUS_APP_VIEW_PALETTE_COPY_TARGETS;
        } else {
            fractus_app_log("runtime: reading source palette color failed");
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        }
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_app_run_palette_copy_targets_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    const char *cfg_path,
    fractus_legacy_config *legacy_config,
    uint32_t palette_copy_source_index,
    fractus_color_rgba8 palette_copy_color,
    fractus_app_view *view)
{
    uint32_t picked_index;
    int cancelled = 0;
    char source_text[128];

    if (framebuffer == NULL || fonts == NULL || ui == NULL || legacy_config == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    snprintf(source_text, sizeof(source_text), "Origen: indice %u. Seleccione destinos con el boton izquierdo. Pulse boton derecho para dejar de copiar.", (unsigned)palette_copy_source_index);

    /* 1. Contenedor exterior. */
    /* 2. Textos y controles. */
    if (fractus_app_render_palette_screen(
            framebuffer,
            fonts,
            "Copiar un color de la paleta",
            source_text,
            fractus_app_palette_back_is_pressed(ui)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones. */
    if (fractus_app_palette_pick(ui, &picked_index, &cancelled)) {
        if (cancelled) {
            if (fractus_app_persist_current_palette(cfg_path, framebuffer, legacy_config) != FRACTUS_STATUS_OK) {
                fractus_app_log("runtime: saving copied palette colors failed");
            }
            *view = fractus_app_palette_back_was_clicked(ui) ?
                FRACTUS_APP_VIEW_MAIN_MENU :
                FRACTUS_APP_VIEW_PALETTE_COPY_SOURCE;
        } else if (fractus_app_set_palette_entry(framebuffer, picked_index, palette_copy_color) != FRACTUS_STATUS_OK) {
            fractus_app_log("runtime: applying copied palette color failed");
        }
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_app_run_palette_gradient_first_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    uint32_t *palette_gradient_first_index,
    fractus_app_view *view)
{
    uint32_t picked_index;
    int cancelled = 0;

    if (framebuffer == NULL || fonts == NULL || ui == NULL ||
        palette_gradient_first_index == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    /* 1. Contenedor exterior. */
    /* 2. Textos y controles. */
    if (fractus_app_render_palette_screen(
            framebuffer,
            fonts,
            "Crear un gradiente",
            "Seleccione el primer color del gradiente.",
            fractus_app_palette_back_is_pressed(ui)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones. */
    if (fractus_app_palette_pick(ui, &picked_index, &cancelled)) {
        if (cancelled) {
            *view = FRACTUS_APP_VIEW_MAIN_MENU;
        } else {
            *palette_gradient_first_index = picked_index;
            *view = FRACTUS_APP_VIEW_PALETTE_GRADIENT_SECOND;
        }
    }

    return FRACTUS_STATUS_OK;
}

fractus_status fractus_app_run_palette_gradient_second_view(
    fractus_framebuffer *framebuffer,
    const fractus_font_library *fonts,
    fractus_ui_context *ui,
    const char *cfg_path,
    fractus_legacy_config *legacy_config,
    uint32_t palette_gradient_first_index,
    fractus_app_view *view)
{
    uint32_t picked_index;
    int cancelled = 0;
    char first_text[96];

    if (framebuffer == NULL || fonts == NULL || ui == NULL || legacy_config == NULL || view == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    snprintf(first_text, sizeof(first_text), "Primer color: indice %u. Seleccione el segundo color del gradiente. Boton derecho para cancelar.", (unsigned)palette_gradient_first_index);

    /* 1. Contenedor exterior. */
    /* 2. Textos y controles. */
    if (fractus_app_render_palette_screen(
            framebuffer,
            fonts,
            "Crear un gradiente",
            first_text,
            fractus_app_palette_back_is_pressed(ui)) != FRACTUS_STATUS_OK) {
        return FRACTUS_STATUS_ERROR;
    }

    /* 3. Teclado. No hay campos editables directos en esta ventana. */

    /* 4. Raton y acciones. */
    if (fractus_app_palette_pick(ui, &picked_index, &cancelled)) {
        if (cancelled) {
            *view = fractus_app_palette_back_was_clicked(ui) ?
                FRACTUS_APP_VIEW_MAIN_MENU :
                FRACTUS_APP_VIEW_PALETTE_GRADIENT_FIRST;
        } else {
            if (fractus_app_apply_palette_gradient(
                    cfg_path,
                    framebuffer,
                    legacy_config,
                    palette_gradient_first_index,
                    picked_index) != FRACTUS_STATUS_OK) {
                fractus_app_log("runtime: saving palette gradient failed");
            }
            *view = FRACTUS_APP_VIEW_PALETTE_GRADIENT_FIRST;
        }
    }

    return FRACTUS_STATUS_OK;
}
