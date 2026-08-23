#include "test_common.h"
#include "ui/font.h"
#include "platform/framebuffer.h"

static int test_font_load_and_measure(void)
{
    fractus_font_library library;
    int32_t width = 0;
    int32_t height = 0;
    const char *font_path = "fractus.fon";

    /* Intentar cargar la fuente desde la raiz del proyecto */
    if (fractus_font_library_load_archive(&library, font_path) != FRACTUS_STATUS_OK) {
        font_path = "../fractus.fon";
        if (fractus_font_library_load_archive(&library, font_path) != FRACTUS_STATUS_OK) {
            font_path = "../../fractus.fon";
            TEST_ASSERT(fractus_font_library_load_archive(&library, font_path) == FRACTUS_STATUS_OK, "Font archive load failed");
        }
    }

    TEST_ASSERT(library.initialized, "Font library not initialized");
    TEST_ASSERT(library.faces[FRACTUS_FONT_ARIAL].initialized, "Arial face not initialized");
    TEST_ASSERT(library.faces[FRACTUS_FONT_SMALL].initialized, "Small face not initialized");
    TEST_ASSERT(library.faces[FRACTUS_FONT_COURIER].initialized, "Courier face not initialized");
    TEST_ASSERT(library.faces[FRACTUS_FONT_FRANCE].initialized, "France face not initialized");

    /* Test medicion texto simple */
    TEST_ASSERT(fractus_font_measure_text(&library, FRACTUS_FONT_SMALL, "Hola", &width, &height) == FRACTUS_STATUS_OK, "Measure basic text failed");
    TEST_ASSERT(width > 0, "Width should be > 0");
    TEST_ASSERT_EQUAL_INT(11, height, "Small font height should be 11");

    /* Test medicion caracteres espanoles UTF-8 */
    width = 0;
    TEST_ASSERT(fractus_font_measure_text(&library, FRACTUS_FONT_SMALL, "á é í ó ú ñ Ñ", &width, &height) == FRACTUS_STATUS_OK, "Measure Spanish accents failed");
    TEST_ASSERT(width > 0, "Spanish accents width should be > 0");

    /* Test medicion texto completo con acentos */
    width = 0;
    TEST_ASSERT(fractus_font_measure_text(&library, FRACTUS_FONT_ARIAL, "Parámetros de configuración", &width, &height) == FRACTUS_STATUS_OK, "Measure full accented title failed");
    TEST_ASSERT(width > 0, "Accented title width should be > 0");
    TEST_ASSERT_EQUAL_INT(18, height, "Arial font height should be 18");

    fractus_font_library_shutdown(&library);
    return 1;
}

static int test_font_draw_spanish_accents(void)
{
    fractus_font_library library;
    fractus_framebuffer framebuffer;
    const char *font_path = "fractus.fon";
    uint32_t i;
    size_t total_pixels;
    size_t non_zero_pixels = 0;

    if (fractus_font_library_load_archive(&library, font_path) != FRACTUS_STATUS_OK) {
        font_path = "../fractus.fon";
        if (fractus_font_library_load_archive(&library, font_path) != FRACTUS_STATUS_OK) {
            font_path = "../../fractus.fon";
            TEST_ASSERT(fractus_font_library_load_archive(&library, font_path) == FRACTUS_STATUS_OK, "Font archive load failed");
        }
    }

    TEST_ASSERT(fractus_framebuffer_init(&framebuffer, (fractus_size_u32){320u, 200u}) == FRACTUS_STATUS_OK, "Framebuffer init failed");
    TEST_ASSERT(fractus_framebuffer_clear(&framebuffer, 0u) == FRACTUS_STATUS_OK, "Clear failed");

    /* Dibujar texto con acentos y Ñ */
    TEST_ASSERT(fractus_font_draw_text(
        &framebuffer,
        &library,
        FRACTUS_FONT_SMALL,
        10,
        10,
        15u,
        "Año 2026: Parámetros, resolución y método (Ñandú)") == FRACTUS_STATUS_OK, "Draw text with accents failed");

    total_pixels = (size_t)framebuffer.size.width * framebuffer.size.height;
    for (i = 0; i < total_pixels; ++i) {
        if (framebuffer.index_pixels[i] != 0u) {
            ++non_zero_pixels;
        }
    }
    TEST_ASSERT(non_zero_pixels > 0, "Pixels should have been drawn for accented text");

    /* Dibujar con escala */
    TEST_ASSERT(fractus_font_draw_text_scaled(
        &framebuffer,
        &library,
        FRACTUS_FONT_ARIAL,
        10,
        50,
        14u,
        "Configuración",
        2u) == FRACTUS_STATUS_OK, "Draw scaled text failed");

    fractus_framebuffer_shutdown(&framebuffer);
    fractus_font_library_shutdown(&library);
    return 1;
}

int main(void)
{
    TEST_RUN(test_font_load_and_measure);
    TEST_RUN(test_font_draw_spanish_accents);
    return 0;
}
