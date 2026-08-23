#include "test_common.h"
#include "core/formats.h"
#include "platform/framebuffer.h"
#include <string.h>

static int test_legacy_config_defaults(void)
{
    fractus_legacy_config config;
    uint32_t i;

    TEST_ASSERT(fractus_legacy_config_init_default(&config) == FRACTUS_STATUS_OK, "Default config init failed");
    TEST_ASSERT_EQUAL_INT(250, config.iterations, "Default iterations mismatch");
    TEST_ASSERT_EQUAL_INT(4, config.escape_radius_squared, "Default escape radius mismatch");
    TEST_ASSERT_EQUAL_INT(250, config.biomorph_iterations, "Default biomorph iterations mismatch");
    TEST_ASSERT_EQUAL_INT(100, config.biomorph_escape_radius_squared, "Default biomorph radius mismatch");
    TEST_ASSERT_EQUAL_INT(10, config.biomorph_cutoff, "Default biomorph cutoff mismatch");
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
    original.biomorph_iterations = 25;
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
    TEST_ASSERT_EQUAL_INT(original.biomorph_iterations, loaded.biomorph_iterations, "Saved biomorph iterations mismatch");
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

static int test_graphic_metadata_json_save(void)
{
    fractus_mandelbrot_params mandel = {
        .xmin = -2.1,
        .xmax = 0.8,
        .ymin = -1.2,
        .ymax = 1.2,
        .max_iterations = 250,
        .escape_radius_squared = 4.0,
        .inside_color_index = 0,
        .palette_offset = 16,
        .palette_span = 240,
        .color_mode = FRACTUS_MANDELBROT_COLOR_ESCAPE
    };
    fractus_biomorph_params bio = {
        .xmin = -2.0,
        .xmax = 2.0,
        .ymin = -2.0,
        .ymax = 2.0,
        .constant_real = 0.5,
        .constant_imag = 0.0,
        .max_iterations = 250,
        .escape_radius_squared = 100.0,
        .cutoff = 10.0,
        .background_color_index = 0,
        .palette_offset = 16,
        .palette_span = 240,
        .equation = FRACTUS_BIOMORPH_EQ_Z3,
        .trap_mode = FRACTUS_BIOMORPH_TRAP_RE_OR_IM
    };
    fractus_lorenz_params lorenz = {
        .sigma = 10.0,
        .rho = 28.0,
        .beta = 8.0 / 3.0,
        .dt = 0.01,
        .iterations = 10000,
        .projection = FRACTUS_LORENZ_PROJECTION_XZ,
        .rot_x = 0.0,
        .rot_y = 0.0,
        .rot_z = 0.0,
        .palette_offset = 16,
        .palette_span = 240
    };
    const char *test_json = "test_metadata_temp.json";
    char buffer[2048];
    FILE *f;
    size_t bytes_read;

    fractus_graphic_metadata meta = fractus_graphic_metadata_from_mandelbrot(&mandel, 640, 480);
    TEST_ASSERT(fractus_graphic_metadata_save_json(test_json, &meta) == FRACTUS_STATUS_OK, "Mandelbrot JSON save failed");

    f = fopen(test_json, "r");
    TEST_ASSERT(f != NULL, "Failed to open saved Mandelbrot JSON");
    bytes_read = fread(buffer, 1, sizeof(buffer) - 1, f);
    buffer[bytes_read] = '\0';
    fclose(f);
    remove(test_json);

    TEST_ASSERT(strstr(buffer, "\"application\": \"Fractus-x64\"") != NULL, "JSON application missing");
    TEST_ASSERT(strstr(buffer, "\"fractal_type\": \"mandelbrot\"") != NULL, "JSON fractal_type missing");
    TEST_ASSERT(strstr(buffer, "\"max_iterations\": 250") != NULL, "JSON max_iterations missing");
    TEST_ASSERT(strstr(buffer, "\"color_mode\": \"escape\"") != NULL, "JSON color_mode missing");

    meta = fractus_graphic_metadata_from_biomorph(&bio, 800, 600);
    TEST_ASSERT(fractus_graphic_metadata_save_json(test_json, &meta) == FRACTUS_STATUS_OK, "Biomorph JSON save failed");
    f = fopen(test_json, "r");
    TEST_ASSERT(f != NULL, "Failed to open saved Biomorph JSON");
    bytes_read = fread(buffer, 1, sizeof(buffer) - 1, f);
    buffer[bytes_read] = '\0';
    fclose(f);
    remove(test_json);

    TEST_ASSERT(strstr(buffer, "\"fractal_type\": \"biomorph\"") != NULL, "JSON biomorph missing");
    TEST_ASSERT(strstr(buffer, "\"equation\": \"z^3+c\"") != NULL, "JSON equation missing");
    TEST_ASSERT(strstr(buffer, "\"trap_mode\": \"re_or_im\"") != NULL, "JSON trap_mode missing");

    meta = fractus_graphic_metadata_from_lorenz(&lorenz, 1024, 768);
    TEST_ASSERT(fractus_graphic_metadata_save_json(test_json, &meta) == FRACTUS_STATUS_OK, "Lorenz JSON save failed");
    f = fopen(test_json, "r");
    TEST_ASSERT(f != NULL, "Failed to open saved Lorenz JSON");
    bytes_read = fread(buffer, 1, sizeof(buffer) - 1, f);
    buffer[bytes_read] = '\0';
    fclose(f);
    remove(test_json);

    TEST_ASSERT(strstr(buffer, "\"fractal_type\": \"lorenz\"") != NULL, "JSON lorenz missing");
    TEST_ASSERT(strstr(buffer, "\"sigma\": 10") != NULL, "JSON sigma missing");
    TEST_ASSERT(strstr(buffer, "\"projection\": \"xz\"") != NULL, "JSON projection missing");

    return 1;
}

static int test_graphic_metadata_json_load(void)
{
    fractus_biomorph_params bio = {
        .xmin = -2.5,
        .xmax = 2.5,
        .ymin = -1.8,
        .ymax = 1.8,
        .constant_real = 0.35,
        .constant_imag = -0.42,
        .max_iterations = 300,
        .escape_radius_squared = 50.0,
        .cutoff = 8.5,
        .background_color_index = 0,
        .palette_offset = 16,
        .palette_span = 240,
        .equation = FRACTUS_BIOMORPH_EQ_SIN_Z,
        .trap_mode = FRACTUS_BIOMORPH_TRAP_RE_AND_IM
    };
    const char *test_json = "test_metadata_load_temp.json";
    fractus_graphic_metadata saved_meta = fractus_graphic_metadata_from_biomorph(&bio, 800, 600);
    fractus_graphic_metadata loaded_meta;

    TEST_ASSERT(fractus_graphic_metadata_save_json(test_json, &saved_meta) == FRACTUS_STATUS_OK, "JSON save failed");
    TEST_ASSERT(fractus_graphic_metadata_load_json(test_json, &loaded_meta) == FRACTUS_STATUS_OK, "JSON load failed");
    remove(test_json);

    TEST_ASSERT_EQUAL_INT(FRACTUS_GRAPHIC_KIND_BIOMORPH, loaded_meta.kind, "Loaded kind mismatch");
    TEST_ASSERT_EQUAL_INT(800, (int)loaded_meta.width, "Loaded width mismatch");
    TEST_ASSERT_EQUAL_INT(600, (int)loaded_meta.height, "Loaded height mismatch");
    TEST_ASSERT(fabs(loaded_meta.params.biomorph.xmin - (-2.5)) < 1e-6, "xmin mismatch");
    TEST_ASSERT(fabs(loaded_meta.params.biomorph.xmax - 2.5) < 1e-6, "xmax mismatch");
    TEST_ASSERT(fabs(loaded_meta.params.biomorph.constant_real - 0.35) < 1e-6, "constant_real mismatch");
    TEST_ASSERT(fabs(loaded_meta.params.biomorph.constant_imag - (-0.42)) < 1e-6, "constant_imag mismatch");
    TEST_ASSERT_EQUAL_INT(300, (int)loaded_meta.params.biomorph.max_iterations, "max_iterations mismatch");
    TEST_ASSERT(fabs(loaded_meta.params.biomorph.cutoff - 8.5) < 1e-6, "cutoff mismatch");
    TEST_ASSERT_EQUAL_INT(FRACTUS_BIOMORPH_EQ_SIN_Z, loaded_meta.params.biomorph.equation, "equation mismatch");
    TEST_ASSERT_EQUAL_INT(FRACTUS_BIOMORPH_TRAP_RE_AND_IM, loaded_meta.params.biomorph.trap_mode, "trap_mode mismatch");

    return 1;
}

int main(void)
{
    printf("=== RUNNING FORMATS & CONFIG TESTS ===\n");
    TEST_RUN(test_legacy_config_defaults);
    TEST_RUN(test_legacy_config_save_and_load);
    TEST_RUN(test_palette_operations);
    TEST_RUN(test_bmp_save_and_formats_roundtrip);
    TEST_RUN(test_graphic_metadata_json_save);
    TEST_RUN(test_graphic_metadata_json_load);
    TEST_REPORT();
}
