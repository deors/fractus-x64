#include "test_common.h"
#include "core/fractal.h"
#include "platform/framebuffer.h"

static int test_mandelbrot_rendering(void)
{
    fractus_framebuffer fb;
    fractus_size_u32 size = {80, 60};
    fractus_mandelbrot_params mandel_params;
    fractus_mandelbrot_dem_params dem_params;

    TEST_ASSERT(fractus_framebuffer_init(&fb, size) == FRACTUS_STATUS_OK, "Framebuffer init failed");

    /* Mandelbrot escape-time (Color mode 0 and 1) */
    mandel_params = (fractus_mandelbrot_params){
        -2.0, 0.5, -1.25, 1.25,
        100u, 4.0, 0u, 16u, 240u,
        FRACTUS_MANDELBROT_COLOR_ESCAPE
    };
    TEST_ASSERT(fractus_fractal_render_mandelbrot(&fb, &mandel_params) == FRACTUS_STATUS_OK, "Mandelbrot color mode 0 failed");

    mandel_params.color_mode = FRACTUS_MANDELBROT_COLOR_SMOOTH;
    TEST_ASSERT(fractus_fractal_render_mandelbrot(&fb, &mandel_params) == FRACTUS_STATUS_OK, "Mandelbrot color mode 1 failed");

    /* Mandelbrot DEM (Distance Estimation Method) */
    dem_params = (fractus_mandelbrot_dem_params){
        -2.0, 0.5, -1.25, 1.25,
        100u, 100.0, 0u, 16u, 240u,
        FRACTUS_MANDELBROT_DEM_COLOR_BOUNDARY
    };
    TEST_ASSERT(fractus_fractal_render_mandelbrot_dem(&fb, &dem_params) == FRACTUS_STATUS_OK, "Mandelbrot DEM color mode 0 failed");

    dem_params.color_mode = FRACTUS_MANDELBROT_DEM_COLOR_GRADIENT;
    TEST_ASSERT(fractus_fractal_render_mandelbrot_dem(&fb, &dem_params) == FRACTUS_STATUS_OK, "Mandelbrot DEM color mode 1 failed");

    fractus_framebuffer_shutdown(&fb);
    return 1;
}

static int test_julia_rendering(void)
{
    fractus_framebuffer fb;
    fractus_size_u32 size = {80, 60};
    fractus_julia_params julia_params;
    fractus_julia_dem_params dem_params;

    TEST_ASSERT(fractus_framebuffer_init(&fb, size) == FRACTUS_STATUS_OK, "Framebuffer init failed");

    /* Julia escape-time */
    julia_params = (fractus_julia_params){
        -1.5, 1.5, -1.2, 1.2,
        -0.7, 0.27015,
        100u, 4.0, 0u, 16u, 240u,
        FRACTUS_JULIA_COLOR_ESCAPE
    };
    TEST_ASSERT(fractus_fractal_render_julia(&fb, &julia_params) == FRACTUS_STATUS_OK, "Julia color mode 0 failed");

    julia_params.color_mode = FRACTUS_JULIA_COLOR_SMOOTH;
    TEST_ASSERT(fractus_fractal_render_julia(&fb, &julia_params) == FRACTUS_STATUS_OK, "Julia color mode 1 failed");

    /* Julia DEM */
    dem_params = (fractus_julia_dem_params){
        -1.5, 1.5, -1.2, 1.2,
        -0.7, 0.27015,
        100u, 100.0, 0u, 16u, 240u,
        FRACTUS_JULIA_DEM_COLOR_BOUNDARY
    };
    TEST_ASSERT(fractus_fractal_render_julia_dem(&fb, &dem_params) == FRACTUS_STATUS_OK, "Julia DEM color mode 0 failed");

    dem_params.color_mode = FRACTUS_JULIA_DEM_COLOR_GRADIENT;
    TEST_ASSERT(fractus_fractal_render_julia_dem(&fb, &dem_params) == FRACTUS_STATUS_OK, "Julia DEM color mode 1 failed");

    fractus_framebuffer_shutdown(&fb);
    return 1;
}

static int test_plasma_rendering(void)
{
    fractus_framebuffer fb1;
    fractus_framebuffer fb2;
    fractus_size_u32 size = {80, 60};
    fractus_plasma_params rect_params;
    fractus_plasma_circular_params circ_params;
    uint32_t x, y;

    TEST_ASSERT(fractus_framebuffer_init(&fb1, size) == FRACTUS_STATUS_OK, "Framebuffer 1 init failed");
    TEST_ASSERT(fractus_framebuffer_init(&fb2, size) == FRACTUS_STATUS_OK, "Framebuffer 2 init failed");

    /* Rectangular plasma */
    rect_params = (fractus_plasma_params){
        1337u, 25, 16u, 240u
    };
    TEST_ASSERT(fractus_fractal_render_plasma(&fb1, &rect_params) == FRACTUS_STATUS_OK, "Plasma rectangular render failed");
    TEST_ASSERT(fractus_fractal_render_plasma(&fb2, &rect_params) == FRACTUS_STATUS_OK, "Plasma rectangular render 2 failed");

    /* Verify determinism: same seed produces identical pixels */
    for (y = 0; y < size.height; ++y) {
        for (x = 0; x < size.width; ++x) {
            uint8_t p1 = 0, p2 = 0;
            TEST_ASSERT(fractus_framebuffer_get_pixel(&fb1, x, y, &p1) == FRACTUS_STATUS_OK, "Get pixel fb1 failed");
            TEST_ASSERT(fractus_framebuffer_get_pixel(&fb2, x, y, &p2) == FRACTUS_STATUS_OK, "Get pixel fb2 failed");
            TEST_ASSERT_EQUAL_INT(p1, p2, "Plasma deterministic pixel mismatch");
        }
    }

    /* Circular plasma */
    circ_params = (fractus_plasma_circular_params){
        1337u, 50, 40, 16u, 240u
    };
    TEST_ASSERT(fractus_fractal_render_plasma_circular(&fb1, &circ_params) == FRACTUS_STATUS_OK, "Plasma circular render failed");
    TEST_ASSERT(fractus_fractal_render_plasma_circular(&fb2, &circ_params) == FRACTUS_STATUS_OK, "Plasma circular render 2 failed");

    for (y = 0; y < size.height; ++y) {
        for (x = 0; x < size.width; ++x) {
            uint8_t p1 = 0, p2 = 0;
            TEST_ASSERT(fractus_framebuffer_get_pixel(&fb1, x, y, &p1) == FRACTUS_STATUS_OK, "Get pixel fb1 failed");
            TEST_ASSERT(fractus_framebuffer_get_pixel(&fb2, x, y, &p2) == FRACTUS_STATUS_OK, "Get pixel fb2 failed");
            TEST_ASSERT_EQUAL_INT(p1, p2, "Plasma circular deterministic pixel mismatch");
        }
    }

    /* Legacy circular plasma */
    TEST_ASSERT(fractus_fractal_render_plasma_circular_legacy(&fb1, &circ_params) == FRACTUS_STATUS_OK, "Plasma circular legacy render failed");

    fractus_framebuffer_shutdown(&fb1);
    fractus_framebuffer_shutdown(&fb2);
    return 1;
}

int main(void)
{
    printf("=== RUNNING FRACTAL TESTS ===\n");
    TEST_RUN(test_mandelbrot_rendering);
    TEST_RUN(test_julia_rendering);
    TEST_RUN(test_plasma_rendering);
    TEST_REPORT();
}
