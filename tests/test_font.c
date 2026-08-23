#include "test_common.h"
#include "ui/font.h"
#include "platform/framebuffer.h"

static int test_font_load_and_measure_all_glyphs(void)
{
    fractus_font_library library;
    int32_t width = 0;
    int32_t height = 0;
    const char *font_path = "fractus.fon";
    fractus_font_kind k;

    if (fractus_font_library_load_archive(&library, font_path) != FRACTUS_STATUS_OK) {
        font_path = "../fractus.fon";
        if (fractus_font_library_load_archive(&library, font_path) != FRACTUS_STATUS_OK) {
            font_path = "../../fractus.fon";
            TEST_ASSERT(fractus_font_library_load_archive(&library, font_path) == FRACTUS_STATUS_OK, "Font archive load failed");
        }
    }

    TEST_ASSERT(library.initialized, "Font library not initialized");
    for (k = 0; k < FRACTUS_FONT_COUNT; ++k) {
        TEST_ASSERT(library.faces[k].initialized, "Font face not initialized");
        TEST_ASSERT(library.faces[k].glyph_height > 0, "Glyph height must be > 0");
    }

    /* 1. Mayusculas con acento y dieresis */
    width = 0;
    TEST_ASSERT(fractus_font_measure_text(&library, FRACTUS_FONT_SMALL, "Á É Í Ó Ú Ñ Ü", &width, &height) == FRACTUS_STATUS_OK, "Measure uppercase accents failed");
    TEST_ASSERT(width > 0, "Uppercase accents width must be > 0");

    /* 2. Minusculas con acento y dieresis */
    width = 0;
    TEST_ASSERT(fractus_font_measure_text(&library, FRACTUS_FONT_ARIAL, "á é í ó ú ñ ü", &width, &height) == FRACTUS_STATUS_OK, "Measure lowercase accents failed");
    TEST_ASSERT(width > 0, "Lowercase accents width must be > 0");

    /* 3. Simbolos del ASCII 127 y puntuacion espanola */
    width = 0;
    TEST_ASSERT(fractus_font_measure_text(&library, FRACTUS_FONT_COURIER, "^ _ ` ~ ¿ ¡ º ª", &width, &height) == FRACTUS_STATUS_OK, "Measure symbols failed");
    TEST_ASSERT(width > 0, "Symbols width must be > 0");

    /* 4. Superindices (1 2 3 4 5 x y z) */
    width = 0;
    TEST_ASSERT(fractus_font_measure_text(&library, FRACTUS_FONT_FRANCE, "¹ ² ³ ⁴ ⁵ ˣ ʸ ᶻ", &width, &height) == FRACTUS_STATUS_OK, "Measure superscripts failed");
    TEST_ASSERT(width > 0, "Superscripts width must be > 0");

    /* 5. Formulas tipicas de fractales */
    width = 0;
    TEST_ASSERT(fractus_font_measure_text(&library, FRACTUS_FONT_SMALL, "z² + c, x² + y² < 4, z⁴ - z²", &width, &height) == FRACTUS_STATUS_OK, "Measure fractal formula failed");
    TEST_ASSERT(width > 0, "Fractal formula width must be > 0");

    fractus_font_library_shutdown(&library);
    return 1;
}

static int test_font_draw_all_new_glyphs(void)
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

    TEST_ASSERT(fractus_framebuffer_init(&framebuffer, (fractus_size_u32){640u, 480u}) == FRACTUS_STATUS_OK, "Framebuffer init failed");
    TEST_ASSERT(fractus_framebuffer_clear(&framebuffer, 0u) == FRACTUS_STATUS_OK, "Clear failed");

    /* Renderizar textos con todas las nuevas capacidades */
    TEST_ASSERT(fractus_font_draw_text(
        &framebuffer,
        &library,
        FRACTUS_FONT_ARIAL,
        10, 10, 15u,
        "¿Configuración de Parámetros? ¡ÉXITO TOTAL EN EL AÑO 2026!") == FRACTUS_STATUS_OK, "Draw Arial text failed");

    TEST_ASSERT(fractus_font_draw_text(
        &framebuffer,
        &library,
        FRACTUS_FONT_SMALL,
        10, 40, 14u,
        "Fórmulas: z² + c, |z| > R², f(z) = z³ + z⁴ + z⁵ + xˣ + yʸ + zᶻ") == FRACTUS_STATUS_OK, "Draw Small formulas failed");

    TEST_ASSERT(fractus_font_draw_text(
        &framebuffer,
        &library,
        FRACTUS_FONT_COURIER,
        10, 70, 11u,
        "Caracteres ASCII: ^ _ ` ~ | Puntuación: 1º puesto, 2ª opción, ¿Qué tal?") == FRACTUS_STATUS_OK, "Draw Courier text failed");

    TEST_ASSERT(fractus_font_draw_text(
        &framebuffer,
        &library,
        FRACTUS_FONT_FRANCE,
        10, 100, 12u,
        "ÁÉÍÓÚ Ü Ñ áéíóú ü ñ") == FRACTUS_STATUS_OK, "Draw France accented text failed");

    total_pixels = (size_t)framebuffer.size.width * framebuffer.size.height;
    for (i = 0; i < total_pixels; ++i) {
        if (framebuffer.index_pixels[i] != 0u) {
            ++non_zero_pixels;
        }
    }
    TEST_ASSERT(non_zero_pixels > 500u, "Sufficient pixels should have been drawn for all glyphs");

    fractus_framebuffer_shutdown(&framebuffer);
    fractus_font_library_shutdown(&library);
    return 1;
}

int main(void)
{
    TEST_RUN(test_font_load_and_measure_all_glyphs);
    TEST_RUN(test_font_draw_all_new_glyphs);
    return 0;
}
