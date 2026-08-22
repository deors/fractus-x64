#include "test_common.h"
#include "core/formats.h"
#include "platform/framebuffer.h"

static int test_legacy_config_defaults(void)
{
    fractus_legacy_config config;
    uint32_t i;

    TEST_ASSERT(fractus_legacy_config_init_default(&config) == FRACTUS_STATUS_OK, "Default config init failed");
    TEST_ASSERT_EQUAL_INT(240, config.iterations, "Default iterations mismatch");
    TEST_ASSERT_EQUAL_INT(4, config.escape_radius_squared, "Default escape radius mismatch");
    TEST_ASSERT_EQUAL_INT(1000, config.biomorph_escape_radius_squared, "Default biomorph radius mismatch");
    TEST_ASSERT_EQUAL_INT(1, config.biomorph_cutoff, "Default biomorph cutoff mismatch");
    TEST_ASSERT_EQUAL_INT(1337, (int)config.plasma_rectangular_seed, "Default rectangular plasma seed mismatch");
    TEST_ASSERT_EQUAL_INT(7331, (int)config.plasma_circular_seed, "Default circular plasma seed mismatch");

    TEST_ASSERT_EQUAL_INT(121, config.default_palette[0].r, "Default palette entry 0 R mismatch");
    TEST_ASSERT_EQUAL_INT(61, config.default_palette[0].g, "Default palette entry 0 G mismatch");
    TEST_ASSERT_EQUAL_INT(121, config.default_palette[0].b, "Default palette entry 0 B mismatch");

    for (i = 0; i < FRACTUS_LEGACY_PALETTE_DATA_COUNT; ++i) {
        TEST_ASSERT_EQUAL_INT(config.palette[i].r, config.default_palette[i].r, "Palette R mismatch with default");
        TEST_ASSERT_EQUAL_INT(config.palette[i].g, config.default_palette[i].g, "Palette G mismatch with default");
        TEST_ASSERT_EQUAL_INT(config.palette[i].b, config.default_palette[i].b, "Palette B mismatch with default");
    }

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
    original.plasma_rectangular_seed = 4242u;
    original.plasma_circular_seed = 9999u;
    original.drawing_video_mode = 2;

    for (i = 0; i < FRACTUS_LEGACY_PALETTE_DATA_COUNT; ++i) {
        original.palette[i].r = (uint8_t)(i & 0xFF);
        original.palette[i].g = (uint8_t)((i * 2u) & 0xFF);
        original.palette[i].b = (uint8_t)((i * 3u) & 0xFF);
        original.palette[i].a = 255u;
        original.default_palette[i].r = (uint8_t)((i * 4u) & 0xFF);
        original.default_palette[i].g = (uint8_t)((i * 5u) & 0xFF);
        original.default_palette[i].b = (uint8_t)((i * 6u) & 0xFF);
        original.default_palette[i].a = 255u;
    }

    TEST_ASSERT(fractus_legacy_config_save(test_path, &original) == FRACTUS_STATUS_OK, "Config save failed");
    TEST_ASSERT(fractus_legacy_config_load(test_path, &loaded) == FRACTUS_STATUS_OK, "Config load failed");

    TEST_ASSERT_EQUAL_INT(original.iterations, loaded.iterations, "Saved iterations mismatch");
    TEST_ASSERT_EQUAL_INT(original.escape_radius_squared, loaded.escape_radius_squared, "Saved radius mismatch");
    TEST_ASSERT_EQUAL_INT(original.biomorph_escape_radius_squared, loaded.biomorph_escape_radius_squared, "Saved biomorph radius mismatch");
    TEST_ASSERT_EQUAL_INT(original.biomorph_cutoff, loaded.biomorph_cutoff, "Saved biomorph cutoff mismatch");
    TEST_ASSERT_EQUAL_INT((int)original.plasma_rectangular_seed, (int)loaded.plasma_rectangular_seed, "Saved rectangular seed mismatch");
    TEST_ASSERT_EQUAL_INT((int)original.plasma_circular_seed, (int)loaded.plasma_circular_seed, "Saved circular seed mismatch");
    TEST_ASSERT_EQUAL_INT(original.drawing_video_mode, loaded.drawing_video_mode, "Saved video mode mismatch");

    /* Palette round-trip within 6-bit VGA DAC quantization tolerance (+-5 levels) */
    for (i = 0; i < FRACTUS_LEGACY_PALETTE_DATA_COUNT; ++i) {
        TEST_ASSERT(abs((int)original.palette[i].r - (int)loaded.palette[i].r) <= 5, "Palette R tolerance mismatch");
        TEST_ASSERT(abs((int)original.palette[i].g - (int)loaded.palette[i].g) <= 5, "Palette G tolerance mismatch");
        TEST_ASSERT(abs((int)original.palette[i].b - (int)loaded.palette[i].b) <= 5, "Palette B tolerance mismatch");
        TEST_ASSERT(abs((int)original.default_palette[i].r - (int)loaded.default_palette[i].r) <= 5, "Default palette R tolerance mismatch");
        TEST_ASSERT(abs((int)original.default_palette[i].g - (int)loaded.default_palette[i].g) <= 5, "Default palette G tolerance mismatch");
        TEST_ASSERT(abs((int)original.default_palette[i].b - (int)loaded.default_palette[i].b) <= 5, "Default palette B tolerance mismatch");
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

static int test_bmp_save_and_formats_roundtrip(void)
{
    fractus_indexed_image img;
    fractus_palette pal;
    fractus_size_u32 size = {80, 60};
    const char *bmp_path = "test_temp.bmp";
    FILE *f;
    uint8_t header[54];
    uint32_t i;

    TEST_ASSERT(fractus_indexed_image_init(&img, size) == FRACTUS_STATUS_OK, "Image init failed");
    TEST_ASSERT(fractus_palette_init_default(&pal) == FRACTUS_STATUS_OK, "Palette init failed");

    for (i = 0u; i < 80u * 60u; ++i) {
        img.pixels[i] = (uint8_t)(i % 256u);
    }

    TEST_ASSERT(fractus_legacy_bmp_save(bmp_path, &img, &pal) == FRACTUS_STATUS_OK, "BMP save failed");

    f = fopen(bmp_path, "rb");
    TEST_ASSERT(f != NULL, "Failed to open saved BMP");
    TEST_ASSERT(fread(header, sizeof(header), 1u, f) == 1u, "Failed to read BMP header");
    fclose(f);

    TEST_ASSERT(header[0] == 'B' && header[1] == 'M', "BMP magic mismatch");

    fractus_indexed_image_shutdown(&img);
    remove(bmp_path);
    return 1;
}

int main(void)
{
    printf("=== RUNNING FORMATS & CONFIG TESTS ===\n");
    TEST_RUN(test_legacy_config_defaults);
    TEST_RUN(test_legacy_config_save_and_load);
    TEST_RUN(test_palette_operations);
    TEST_RUN(test_bmp_save_and_formats_roundtrip);
    TEST_REPORT();
}
