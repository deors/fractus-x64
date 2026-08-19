#include "test_common.h"
#include "core/formats.h"
#include "platform/framebuffer.h"

static int test_legacy_config_defaults(void)
{
    fractus_legacy_config config;

    TEST_ASSERT(fractus_legacy_config_init_default(&config) == FRACTUS_STATUS_OK, "Default config init failed");
    TEST_ASSERT_EQUAL_INT(240, config.iterations, "Default iterations mismatch");
    TEST_ASSERT_EQUAL_INT(4, config.escape_radius_squared, "Default escape radius mismatch");
    TEST_ASSERT_EQUAL_INT(1000, config.biomorph_escape_radius_squared, "Default biomorph radius mismatch");
    TEST_ASSERT_EQUAL_INT(1, config.biomorph_cutoff, "Default biomorph cutoff mismatch");

    return 1;
}

static int test_legacy_config_save_and_load(void)
{
    fractus_legacy_config original;
    fractus_legacy_config loaded;
    const char *test_path = "test_fractus_temp.cfg";
    uint32_t i;

    TEST_ASSERT(fractus_legacy_config_init_default(&original) == FRACTUS_STATUS_OK, "Default config init failed");
    original.iterations = 500;
    original.escape_radius_squared = 250;
    original.biomorph_escape_radius_squared = 2000;
    original.biomorph_cutoff = 5;
    original.drawing_video_mode = 2;

    for (i = 0; i < FRACTUS_LEGACY_PALETTE_DATA_COUNT; ++i) {
        original.palette[i].r = (uint8_t)(i & 0xFF);
        original.palette[i].g = (uint8_t)((i * 2u) & 0xFF);
        original.palette[i].b = (uint8_t)((i * 3u) & 0xFF);
        original.palette[i].a = 255u;
    }

    TEST_ASSERT(fractus_legacy_config_save(test_path, &original) == FRACTUS_STATUS_OK, "Config save failed");
    TEST_ASSERT(fractus_legacy_config_load(test_path, &loaded) == FRACTUS_STATUS_OK, "Config load failed");

    TEST_ASSERT_EQUAL_INT(original.iterations, loaded.iterations, "Saved iterations mismatch");
    TEST_ASSERT_EQUAL_INT(original.escape_radius_squared, loaded.escape_radius_squared, "Saved radius mismatch");
    TEST_ASSERT_EQUAL_INT(original.biomorph_escape_radius_squared, loaded.biomorph_escape_radius_squared, "Saved biomorph radius mismatch");
    TEST_ASSERT_EQUAL_INT(original.biomorph_cutoff, loaded.biomorph_cutoff, "Saved biomorph cutoff mismatch");
    TEST_ASSERT_EQUAL_INT(original.drawing_video_mode, loaded.drawing_video_mode, "Saved video mode mismatch");

    /* Palette round-trip within 6-bit VGA DAC quantization tolerance (+-5 levels) */
    for (i = 0; i < FRACTUS_LEGACY_PALETTE_DATA_COUNT; ++i) {
        TEST_ASSERT(abs((int)original.palette[i].r - (int)loaded.palette[i].r) <= 5, "Palette R tolerance mismatch");
        TEST_ASSERT(abs((int)original.palette[i].g - (int)loaded.palette[i].g) <= 5, "Palette G tolerance mismatch");
        TEST_ASSERT(abs((int)original.palette[i].b - (int)loaded.palette[i].b) <= 5, "Palette B tolerance mismatch");
    }

    remove(test_path);
    return 1;
}

static int test_palette_operations(void)
{
    fractus_palette pal;
    fractus_color_rgba8 color = {255, 128, 64, 255};
    fractus_color_rgba8 got = {0, 0, 0, 0};

    TEST_ASSERT(fractus_palette_init_default(&pal) == FRACTUS_STATUS_OK, "Palette default init failed");
    TEST_ASSERT(fractus_palette_set_entry(&pal, 42, color) == FRACTUS_STATUS_OK, "Set palette entry failed");
    TEST_ASSERT(fractus_palette_get_entry(&pal, 42, &got) == FRACTUS_STATUS_OK, "Get palette entry failed");

    TEST_ASSERT_EQUAL_INT(255, got.r, "Got R mismatch");
    TEST_ASSERT_EQUAL_INT(128, got.g, "Got G mismatch");
    TEST_ASSERT_EQUAL_INT(64, got.b, "Got B mismatch");

    return 1;
}

int main(void)
{
    printf("=== RUNNING FORMATS & CONFIG TESTS ===\n");
    TEST_RUN(test_legacy_config_defaults);
    TEST_RUN(test_legacy_config_save_and_load);
    TEST_RUN(test_palette_operations);
    TEST_REPORT();
}
