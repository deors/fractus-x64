#include "test_common.h"
#include "core/core.h"
#include "core/fractal.h"
#include "platform/framebuffer.h"

#include <string.h>

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
        100u, 4.0, 0u, FRACTUS_PALETTE_OFFSET, FRACTUS_PALETTE_SPAN,
        FRACTUS_MANDELBROT_COLOR_ESCAPE,
        FRACTUS_HP_WINDOW_ZERO
    };
    TEST_ASSERT(fractus_fractal_render_mandelbrot(&fb, &mandel_params) == FRACTUS_STATUS_OK, "Mandelbrot color mode 0 failed");

    mandel_params.color_mode = FRACTUS_MANDELBROT_COLOR_SMOOTH;
    TEST_ASSERT(fractus_fractal_render_mandelbrot(&fb, &mandel_params) == FRACTUS_STATUS_OK, "Mandelbrot color mode 1 failed");

    /* Mandelbrot DEM (Distance Estimation Method) */
    dem_params = (fractus_mandelbrot_dem_params){
        -2.0, 0.5, -1.25, 1.25,
        100u, 100.0, 0u, FRACTUS_PALETTE_OFFSET, FRACTUS_PALETTE_SPAN,
        FRACTUS_MANDELBROT_DEM_COLOR_BOUNDARY,
        FRACTUS_HP_WINDOW_ZERO
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
        100u, 4.0, 0u, FRACTUS_PALETTE_OFFSET, FRACTUS_PALETTE_SPAN,
        FRACTUS_JULIA_COLOR_ESCAPE,
        FRACTUS_HP_WINDOW_ZERO
    };
    TEST_ASSERT(fractus_fractal_render_julia(&fb, &julia_params) == FRACTUS_STATUS_OK, "Julia color mode 0 failed");

    julia_params.color_mode = FRACTUS_JULIA_COLOR_SMOOTH;
    TEST_ASSERT(fractus_fractal_render_julia(&fb, &julia_params) == FRACTUS_STATUS_OK, "Julia color mode 1 failed");

    /* Julia DEM */
    dem_params = (fractus_julia_dem_params){
        -1.5, 1.5, -1.2, 1.2,
        -0.7, 0.27015,
        100u, 100.0, 0u, FRACTUS_PALETTE_OFFSET, FRACTUS_PALETTE_SPAN,
        FRACTUS_JULIA_DEM_COLOR_BOUNDARY,
        FRACTUS_HP_WINDOW_ZERO
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
        1337u, 25, FRACTUS_PALETTE_OFFSET, FRACTUS_PALETTE_SPAN
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
        1337u, 50, 40, FRACTUS_PALETTE_OFFSET, FRACTUS_PALETTE_SPAN
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

static int test_palette_cycling(void)
{
    fractus_framebuffer fb;
    fractus_size_u32 size = {80, 60};
    fractus_color_rgba8 original_entry_16;
    fractus_color_rgba8 original_entry_255;
    fractus_color_rgba8 original_entry_0;
    fractus_color_rgba8 original_entry_15;
    uint32_t i;

    TEST_ASSERT(fractus_framebuffer_init(&fb, size) == FRACTUS_STATUS_OK, "Framebuffer init failed");

    original_entry_0 = fb.palette.entries[0];
    original_entry_15 = fb.palette.entries[15];
    original_entry_16 = fb.palette.entries[FRACTUS_PALETTE_OFFSET];
    original_entry_255 = fb.palette.entries[255];

    /* Cycle forward 1 step */
    TEST_ASSERT(fractus_framebuffer_cycle_palette(&fb, FRACTUS_PALETTE_OFFSET, FRACTUS_PALETTE_SPAN, 1) == FRACTUS_STATUS_OK, "Cycle palette forward failed");
    TEST_ASSERT_EQUAL_INT(original_entry_0.r, fb.palette.entries[0].r, "Entry 0 should be unchanged");
    TEST_ASSERT_EQUAL_INT(original_entry_15.r, fb.palette.entries[15].r, "Entry 15 should be unchanged");
    TEST_ASSERT_EQUAL_INT(original_entry_255.r, fb.palette.entries[FRACTUS_PALETTE_OFFSET].r, "Entry 16 should have old entry 255");
    TEST_ASSERT_EQUAL_INT(original_entry_16.r, fb.palette.entries[FRACTUS_PALETTE_OFFSET + 1u].r, "Entry 17 should have old entry 16");

    /* Cycle 239 more times to complete a full 240-step loop */
    for (i = 0; i < FRACTUS_PALETTE_SPAN - 1u; ++i) {
        TEST_ASSERT(fractus_framebuffer_cycle_palette(&fb, FRACTUS_PALETTE_OFFSET, FRACTUS_PALETTE_SPAN, 1) == FRACTUS_STATUS_OK, "Cycle palette step failed");
    }
    TEST_ASSERT_EQUAL_INT(original_entry_16.r, fb.palette.entries[FRACTUS_PALETTE_OFFSET].r, "Entry 16 should match original after full loop");
    TEST_ASSERT_EQUAL_INT(original_entry_255.r, fb.palette.entries[255].r, "Entry 255 should match original after full loop");

    /* Test invalid arguments */
    TEST_ASSERT(fractus_framebuffer_cycle_palette(NULL, FRACTUS_PALETTE_OFFSET, FRACTUS_PALETTE_SPAN, 1) == FRACTUS_STATUS_INVALID_ARGUMENT, "Null check failed");
    TEST_ASSERT(fractus_framebuffer_cycle_palette(&fb, FRACTUS_PALETTE_OFFSET, 1u, 1) == FRACTUS_STATUS_INVALID_ARGUMENT, "Span < 2 check failed");
    TEST_ASSERT(fractus_palette_cycle(NULL, FRACTUS_PALETTE_OFFSET, FRACTUS_PALETTE_SPAN, 1) == FRACTUS_STATUS_INVALID_ARGUMENT, "Palette null check failed");

    fractus_framebuffer_shutdown(&fb);
    return 1;
}

static int test_lorenz_rendering(void)
{
    fractus_framebuffer fb;
    fractus_size_u32 size = {80, 60};
    fractus_lorenz_params params;
    uint32_t i;
    int has_non_zero_pixel = 0;

    TEST_ASSERT(fractus_framebuffer_init(&fb, size) == FRACTUS_STATUS_OK, "Framebuffer init failed");

    params = (fractus_lorenz_params){
        10.0, 28.0, 8.0 / 3.0, 0.01,
        1000u, FRACTUS_LORENZ_PROJECTION_XZ,
        35.0, 45.0, 0.0,
        FRACTUS_PALETTE_OFFSET, FRACTUS_PALETTE_SPAN
    };

    TEST_ASSERT(fractus_fractal_render_lorenz(&fb, &params) == FRACTUS_STATUS_OK, "Lorenz XZ rendering failed");
    params.projection = FRACTUS_LORENZ_PROJECTION_XY;
    TEST_ASSERT(fractus_fractal_render_lorenz(&fb, &params) == FRACTUS_STATUS_OK, "Lorenz XY rendering failed");
    params.projection = FRACTUS_LORENZ_PROJECTION_YZ;
    TEST_ASSERT(fractus_fractal_render_lorenz(&fb, &params) == FRACTUS_STATUS_OK, "Lorenz YZ rendering failed");
    params.projection = FRACTUS_LORENZ_PROJECTION_CUSTOM;
    TEST_ASSERT(fractus_fractal_render_lorenz(&fb, &params) == FRACTUS_STATUS_OK, "Lorenz 3D rendering failed");

    for (i = 0; i < (uint32_t)(fb.size.height * fb.pitch_pixels); ++i) {
        if (fb.index_pixels[i] >= FRACTUS_PALETTE_OFFSET) {
            has_non_zero_pixel = 1;
            break;
        }
    }
    TEST_ASSERT(has_non_zero_pixel == 1, "Lorenz rendering should plot points in palette range 16-255");

    /* Invalid arguments checks */
    TEST_ASSERT(fractus_fractal_render_lorenz(NULL, &params) == FRACTUS_STATUS_INVALID_ARGUMENT, "Null framebuffer check failed");
    TEST_ASSERT(fractus_fractal_render_lorenz(&fb, NULL) == FRACTUS_STATUS_INVALID_ARGUMENT, "Null params check failed");

    fractus_framebuffer_shutdown(&fb);
    return 1;
}

static int test_plane_point_and_zoom(void)
{
    double real_part = 0.0;
    double imag_part = 0.0;

    TEST_ASSERT(
        fractus_plane_point_from_pixel(-2.4, 1.2, -1.2, 1.2, 3u, 3u, 0u, 0u, &real_part, &imag_part),
        "Origin pixel should map");
    TEST_ASSERT(fabs(real_part - (-2.4)) < 1e-12, "Origin real mismatch");
    TEST_ASSERT(fabs(imag_part - 1.2) < 1e-12, "Origin imag mismatch");

    TEST_ASSERT(
        fractus_plane_point_from_pixel(-2.4, 1.2, -1.2, 1.2, 3u, 3u, 2u, 2u, &real_part, &imag_part),
        "Far pixel should map");
    TEST_ASSERT(fabs(real_part - 1.2) < 1e-12, "Far real mismatch");
    TEST_ASSERT(fabs(imag_part - (-1.2)) < 1e-12, "Far imag mismatch");

    TEST_ASSERT(
        fractus_plane_point_from_pixel(-2.4, 1.2, -1.2, 1.2, 3u, 3u, 1u, 1u, &real_part, &imag_part),
        "Center pixel should map");
    TEST_ASSERT(fabs(real_part - (-0.6)) < 1e-12, "Center real mismatch");
    TEST_ASSERT(fabs(imag_part - 0.0) < 1e-12, "Center imag mismatch");

    TEST_ASSERT(!fractus_plane_point_from_pixel(-2.4, 1.2, -1.2, 1.2, 3u, 3u, 3u, 0u, &real_part, &imag_part),
        "Pixel outside the image should fail");
    TEST_ASSERT(fabs(fractus_plane_zoom_level(FRACTUS_PLANE_MANDELBROT_REFERENCE_SPAN, 1.8) - 2.0) < 1e-12,
        "Zoom should be the reference span divided by the current span");
    TEST_ASSERT(fractus_plane_zoom_level(0.0, 1.0) == 0.0, "Zero reference span should not zoom");

    {
        double xmin = -2.4;
        double xmax = 1.2;
        double ymin = -1.2;
        double ymax = 1.2;

        TEST_ASSERT(fractus_plane_pan(&xmin, &xmax, &ymin, &ymax, 3u, 3u, 1, 1, 2, 1), "Pan should move the plane");
        TEST_ASSERT(fabs(xmin - (-4.2)) < 1e-12, "Pan real minimum mismatch");
        TEST_ASSERT(fabs(xmax - (-0.6)) < 1e-12, "Pan real maximum mismatch");
        TEST_ASSERT(
            fractus_plane_point_from_pixel(xmin, xmax, ymin, ymax, 3u, 3u, 2u, 1u, &real_part, &imag_part),
            "Panned pixel should map");
        TEST_ASSERT(fabs(real_part - (-0.6)) < 1e-12, "Pan should keep the grabbed point under the cursor");
        TEST_ASSERT(fabs(imag_part - 0.0) < 1e-12, "Pan should keep the grabbed imag under the cursor");
        TEST_ASSERT(!fractus_plane_pan(&xmin, &xmax, &ymin, &ymax, 3u, 3u, 2, 1, 2, 1), "A zero pan should not change the plane");

        xmin = -2.4;
        xmax = 1.2;
        ymin = -1.2;
        ymax = 1.2;
        TEST_ASSERT(fractus_plane_zoom_at_pixel(&xmin, &xmax, &ymin, &ymax, 3u, 3u, 1u, 1u, 1.0),
            "Wheel zoom should shrink the window");
        TEST_ASSERT(fabs((xmax - xmin) - (3.6 / FRACTUS_PLANE_WHEEL_ZOOM_STEP)) < 1e-12, "Zoomed real span mismatch");
        TEST_ASSERT(fabs((ymax - ymin) - (2.4 / FRACTUS_PLANE_WHEEL_ZOOM_STEP)) < 1e-12, "Zoomed imag span mismatch");
        TEST_ASSERT(
            fractus_plane_point_from_pixel(xmin, xmax, ymin, ymax, 3u, 3u, 1u, 1u, &real_part, &imag_part),
            "Zoomed center should map");
        TEST_ASSERT(fabs(real_part - (-0.6)) < 1e-12, "Zoom should keep the cursor's real coordinate");
        TEST_ASSERT(fabs(imag_part - 0.0) < 1e-12, "Zoom should keep the cursor's imag coordinate");
        TEST_ASSERT(fractus_plane_zoom_at_pixel(&xmin, &xmax, &ymin, &ymax, 3u, 3u, 1u, 1u, -1.0),
            "Wheel zoom out should restore the span");
        TEST_ASSERT(fabs(xmin - (-2.4)) < 1e-9, "Zoom out real minimum mismatch");
        TEST_ASSERT(fabs(xmax - 1.2) < 1e-9, "Zoom out real maximum mismatch");
        TEST_ASSERT(fabs(ymin - (-1.2)) < 1e-9, "Zoom out imag minimum mismatch");
        TEST_ASSERT(fabs(ymax - 1.2) < 1e-9, "Zoom out imag maximum mismatch");

        {
            fractus_hp_window centered = {0};
            fractus_hp mid_x;
            fractus_hp mid_y;
            fractus_hp_window_adopt_doubles(&centered, -2.4, 1.2, -1.2, 1.2);
            mid_x = fractus_hp_div_double(fractus_hp_add(centered.xmin, centered.xmax), 2.0);
            mid_y = fractus_hp_div_double(fractus_hp_add(centered.ymin, centered.ymax), 2.0);
            TEST_ASSERT(fractus_hp_plane_zoom_centered(&centered, 1920u, 1080u, 3.0),
                "Centered wheel zoom should shrink an even frame");
            TEST_ASSERT(
                fabs(fractus_hp_to_double(fractus_hp_sub(
                    fractus_hp_div_double(fractus_hp_add(centered.xmax, centered.xmin), 2.0),
                    mid_x))) < 1e-12,
                "Centered zoom should keep the real midpoint");
            TEST_ASSERT(
                fabs(fractus_hp_to_double(fractus_hp_sub(
                    fractus_hp_div_double(fractus_hp_add(centered.ymax, centered.ymin), 2.0),
                    mid_y))) < 1e-12,
                "Centered zoom should keep the imag midpoint");
            TEST_ASSERT(
                fabs(fractus_hp_to_double(fractus_hp_sub(centered.xmax, centered.xmin)) -
                    (3.6 / (FRACTUS_PLANE_WHEEL_ZOOM_STEP * FRACTUS_PLANE_WHEEL_ZOOM_STEP * FRACTUS_PLANE_WHEEL_ZOOM_STEP))) < 1e-12,
                "Centered zoom real span mismatch");
        }
    }

    return 1;
}

static int test_extended_precision(void)
{
    double tiny = 1e-20;
    fractus_hp sum = fractus_hp_add(fractus_hp_from_double(1.0), fractus_hp_from_double(tiny));
    fractus_hp diff = fractus_hp_sub(sum, fractus_hp_from_double(1.0));
    fractus_hp_window normal = {0};
    fractus_hp_window deep = {0};
    fractus_hp_window zoomed = {0};
    fractus_hp c0;
    fractus_hp c1;
    fractus_hp i0;
    fractus_hp i1;
    fractus_framebuffer fb;
    fractus_size_u32 size = {8u, 8u};
    int i;
    uint32_t pixel;
    int escaped = 0;

    TEST_ASSERT((1.0 + tiny) - 1.0 == 0.0, "Double addition loses 1e-20");
    TEST_ASSERT(fabs(fractus_hp_to_double(diff) - tiny) <= tiny * 1e-6, "Extended addition keeps 1e-20");

    fractus_hp_window_adopt_doubles(&normal, -2.4, 1.2, -1.2, 1.2);
    TEST_ASSERT(!fractus_hp_window_needs_extended(&normal, 80u, 60u), "A normal window stays on double");
    TEST_ASSERT(fractus_hp_plane_pan(&normal, 3u, 3u, 1, 1, 2, 1), "Extended pan should move the plane");
    TEST_ASSERT(fabs(normal.xmin.hi - (-4.2)) < 1e-12, "Extended pan real minimum mismatch");
    TEST_ASSERT(fabs(normal.xmax.hi - (-0.6)) < 1e-12, "Extended pan real maximum mismatch");

    fractus_hp_window_adopt_doubles(&normal, -2.4, 1.2, -1.2, 1.2);
    TEST_ASSERT(fractus_hp_plane_zoom_at_pixel(&normal, 3u, 3u, 1u, 1u, 1.0), "Extended zoom should shrink the window");
    TEST_ASSERT(
        fabs(fractus_hp_to_double(fractus_hp_sub(normal.xmax, normal.xmin)) - (3.6 / FRACTUS_PLANE_WHEEL_ZOOM_STEP)) < 1e-12,
        "Extended zoom real span mismatch");
    TEST_ASSERT(fractus_hp_plane_pixel(&normal, 3u, 3u, 1u, 1u, &c0, &i0), "Zoomed extended center should map");
    TEST_ASSERT(fabs(fractus_hp_to_double(c0) - (-0.6)) < 1e-12, "Extended zoom should keep the cursor real coordinate");
    TEST_ASSERT(fabs(fractus_hp_to_double(i0) - 0.0) < 1e-12, "Extended zoom should keep the cursor imag coordinate");

    {
        double xmin = 0.0;
        double xmax = FRACTUS_PLANE_MIN_SPAN;
        double ymin = 0.0;
        double ymax = FRACTUS_PLANE_MIN_SPAN;
        fractus_hp_window past_floor = {0};
        TEST_ASSERT(!fractus_plane_zoom_at_pixel(&xmin, &xmax, &ymin, &ymax, 3u, 3u, 1u, 1u, 1.0),
            "Double zoom stops at its span floor");
        fractus_hp_window_adopt_doubles(&past_floor, 0.0, FRACTUS_PLANE_MIN_SPAN, 0.0, FRACTUS_PLANE_MIN_SPAN);
        TEST_ASSERT(fractus_hp_plane_zoom_at_pixel(&past_floor, 3u, 3u, 1u, 1u, 1.0),
            "Extended zoom continues past the double floor");
        TEST_ASSERT(
            fractus_hp_to_double(fractus_hp_sub(past_floor.xmax, past_floor.xmin)) < FRACTUS_PLANE_MIN_SPAN,
            "Extended zoom should pass the double floor");
    }

    deep.xmin = fractus_hp_from_double(0.25);
    deep.xmax = fractus_hp_add(deep.xmin, fractus_hp_from_double(1e-18));
    deep.ymin = fractus_hp_from_double(0.0);
    deep.ymax = fractus_hp_add(deep.ymin, fractus_hp_from_double(1e-18));
    deep.valid = 1;
    TEST_ASSERT(deep.xmin.hi == deep.xmax.hi, "Double cannot separate a 1e-18 window");
    TEST_ASSERT((deep.xmax.hi - deep.xmin.hi) == 0.0, "The double step collapses");
    TEST_ASSERT(fractus_hp_window_needs_extended(&deep, 3u, 3u), "A 1e-18 window needs extended precision");
    TEST_ASSERT(fractus_hp_plane_pixel(&deep, 3u, 3u, 0u, 0u, &c0, &i0), "Deep pixel 0 should map");
    TEST_ASSERT(fractus_hp_plane_pixel(&deep, 3u, 3u, 1u, 0u, &c1, &i1), "Deep pixel 1 should map");
    TEST_ASSERT(
        fabs(fractus_hp_to_double(fractus_hp_sub(c1, c0)) - (1e-18 / 2.0)) / (1e-18 / 2.0) < 1e-8,
        "Adjacent deep pixels keep distinct coordinates");

    fractus_hp_window_adopt_doubles(&zoomed, -2.4, 1.2, -1.2, 1.2);
    for (i = 0; i < 40; ++i) {
        if (!fractus_hp_plane_zoom_at_pixel(&zoomed, 3u, 3u, 1u, 1u, 16.0)) {
            break;
        }
    }
    TEST_ASSERT(
        fractus_hp_to_double(fractus_hp_sub(zoomed.xmax, zoomed.xmin)) < 1e-16,
        "Repeated wheel zoom passes the double resolution limit");
    TEST_ASSERT(fractus_hp_window_needs_extended(&zoomed, 3u, 3u), "A deep zoom uses extended precision");
    TEST_ASSERT(fractus_hp_plane_pixel(&zoomed, 3u, 3u, 0u, 1u, &c0, &i0), "Deep zoom pixel 0 should map");
    TEST_ASSERT(fractus_hp_plane_pixel(&zoomed, 3u, 3u, 1u, 1u, &c1, &i1), "Deep zoom pixel 1 should map");
    TEST_ASSERT(fractus_hp_to_double(fractus_hp_sub(c1, c0)) != 0.0, "Adjacent pixels stay distinct after deep zoom");

    TEST_ASSERT(fractus_framebuffer_init(&fb, size) == FRACTUS_STATUS_OK, "Framebuffer init failed");
    {
        fractus_mandelbrot_params mandel = {
            1.0, 1.0, 1.0, 1.0,
            24u, 4.0, 0u, FRACTUS_PALETTE_OFFSET, FRACTUS_PALETTE_SPAN,
            FRACTUS_MANDELBROT_COLOR_ESCAPE,
            FRACTUS_HP_WINDOW_ZERO
        };
        fractus_mandelbrot_dem_params mandel_dem = {
            1.0, 1.0, 1.0, 1.0,
            24u, 4.0, 0u, FRACTUS_PALETTE_OFFSET, FRACTUS_PALETTE_SPAN,
            FRACTUS_MANDELBROT_DEM_COLOR_BOUNDARY,
            FRACTUS_HP_WINDOW_ZERO
        };
        fractus_julia_params julia = {
            1.0, 1.0, 1.0, 1.0,
            -0.7, 0.27015,
            24u, 4.0, 0u, FRACTUS_PALETTE_OFFSET, FRACTUS_PALETTE_SPAN,
            FRACTUS_JULIA_COLOR_ESCAPE,
            FRACTUS_HP_WINDOW_ZERO
        };
        fractus_julia_dem_params julia_dem = {
            1.0, 1.0, 1.0, 1.0,
            -0.7, 0.27015,
            24u, 4.0, 0u, FRACTUS_PALETTE_OFFSET, FRACTUS_PALETTE_SPAN,
            FRACTUS_JULIA_DEM_COLOR_BOUNDARY,
            FRACTUS_HP_WINDOW_ZERO
        };
        fractus_biomorph_params biomorph = {
            1.0, 1.0, 1.0, 1.0,
            0.0, 0.0,
            16u, 100.0, 1.0,
            0u, FRACTUS_PALETTE_OFFSET, FRACTUS_PALETTE_SPAN,
            FRACTUS_BIOMORPH_EQ_Z2,
            FRACTUS_BIOMORPH_TRAP_RE_OR_IM,
            FRACTUS_HP_WINDOW_ZERO
        };
        fractus_hp_window window = {0};
        window.xmin = fractus_hp_from_double(1.0);
        window.xmax = fractus_hp_add(window.xmin, fractus_hp_from_double(1e-18));
        window.ymin = fractus_hp_from_double(1.0);
        window.ymax = fractus_hp_add(window.ymin, fractus_hp_from_double(1e-18));
        window.valid = 1;
        mandel.precise = window;
        mandel_dem.precise = window;
        julia.precise = window;
        julia_dem.precise = window;
        biomorph.precise = window;
        TEST_ASSERT(fractus_fractal_render_mandelbrot(&fb, &mandel) == FRACTUS_STATUS_OK, "Deep Mandelbrot render failed");
        for (pixel = 0u; pixel < (uint32_t)(fb.size.height * fb.pitch_pixels); ++pixel) {
            if (fb.index_pixels[pixel] != mandel.inside_color_index) {
                escaped = 1;
                break;
            }
        }
        TEST_ASSERT(escaped == 1, "Deep Mandelbrot should escape outside the set");
        TEST_ASSERT(fractus_fractal_render_mandelbrot_dem(&fb, &mandel_dem) == FRACTUS_STATUS_OK, "Deep Mandelbrot DEM render failed");
        TEST_ASSERT(fractus_fractal_render_julia(&fb, &julia) == FRACTUS_STATUS_OK, "Deep Julia render failed");
        TEST_ASSERT(fractus_fractal_render_julia_dem(&fb, &julia_dem) == FRACTUS_STATUS_OK, "Deep Julia DEM render failed");
        TEST_ASSERT(fractus_fractal_render_biomorph(&fb, &biomorph) == FRACTUS_STATUS_OK, "Deep biomorph render failed");
    }
    fractus_framebuffer_shutdown(&fb);
    return 1;
}

static int test_progress_continue(void *user, uint32_t rows_done, uint32_t rows_total)
{
    (void)user;
    (void)rows_done;
    (void)rows_total;
    return 0;
}

static int test_progress_stop_after_first_band(void *user, uint32_t rows_done, uint32_t rows_total)
{
    uint32_t *rows = (uint32_t *)user;

    (void)rows_total;
    if (rows != NULL) {
        *rows = rows_done;
    }
    return 1;
}

static int test_render_progress_stop(void)
{
    fractus_framebuffer fb;
    fractus_size_u32 size = {32u, 96u};
    fractus_mandelbrot_params params;
    fractus_render_progress progress;
    uint32_t rows_done = 0u;

    TEST_ASSERT(fractus_framebuffer_init(&fb, size) == FRACTUS_STATUS_OK, "Framebuffer init failed");
    params = (fractus_mandelbrot_params){
        -2.0, 0.5, -1.25, 1.25,
        8u, 4.0, 0u, FRACTUS_PALETTE_OFFSET, FRACTUS_PALETTE_SPAN,
        FRACTUS_MANDELBROT_COLOR_ESCAPE,
        FRACTUS_HP_WINDOW_ZERO
    };

    progress.report = test_progress_continue;
    progress.user = NULL;
    TEST_ASSERT(
        fractus_fractal_render_mandelbrot_with_progress(&fb, &params, &progress) == FRACTUS_STATUS_OK,
        "Progress callback that continues should finish");

    fractus_framebuffer_shutdown(&fb);
    TEST_ASSERT(fractus_framebuffer_init(&fb, size) == FRACTUS_STATUS_OK, "Framebuffer reinit failed");
    progress.report = test_progress_stop_after_first_band;
    progress.user = &rows_done;
    TEST_ASSERT(
        fractus_fractal_render_mandelbrot_with_progress(&fb, &params, &progress) == FRACTUS_STATUS_CANCELLED,
        "Progress callback that stops should cancel before the next band");
    TEST_ASSERT(rows_done > 0u && rows_done <= fb.size.height, "Stop callback should report the finished band");
    if (rows_done < fb.size.height) {
        uint32_t y;
        int clear = 1;

        for (y = rows_done; y < fb.size.height; ++y) {
            uint32_t x;

            for (x = 0u; x < fb.size.width; ++x) {
                if (fb.index_pixels[(size_t)y * fb.pitch_pixels + x] != 0u) {
                    clear = 0;
                }
            }
        }
        TEST_ASSERT(clear, "Rows after the stopped band should stay clear");
    }

    fractus_framebuffer_shutdown(&fb);
    return 1;
}

static int test_hp_format(void)
{
    char text[64];
    char rounded[64];
    fractus_hp tail = fractus_hp_add(fractus_hp_from_double(1.0), fractus_hp_from_double(1e-18));

    TEST_ASSERT(fractus_hp_format(fractus_hp_from_double(0.0), 6, text, sizeof(text)) == 0, "Zero format failed");
    TEST_ASSERT(strcmp(text, "0") == 0, "Zero should print 0");
    TEST_ASSERT(fractus_hp_format(fractus_hp_from_double(-1.25), 3, text, sizeof(text)) == 0, "Negative format failed");
    TEST_ASSERT(strcmp(text, "-1.25") == 0, "-1.25 format mismatch");
    TEST_ASSERT(
        fractus_hp_format(fractus_hp_from_double(fractus_hp_to_double(tail)), 20, rounded, sizeof(rounded)) == 0,
        "Rounded tail format failed");
    TEST_ASSERT(fractus_hp_format(tail, 20, text, sizeof(text)) == 0, "Extended tail format failed");
    TEST_ASSERT(strcmp(text, rounded) != 0, "Extended format should keep the low word");
    TEST_ASSERT(strncmp(text, "1.000000000000000", 17) == 0, "Extended format should still read as 1");
    TEST_ASSERT(fractus_hp_format(fractus_hp_from_double(1.25e-8), 3, text, sizeof(text)) == 0, "Scientific format failed");
    TEST_ASSERT(strcmp(text, "1.25e-8") == 0, "Scientific 1.25e-8 mismatch");
    return 1;
}

static int test_precision_paths_match(void)
{
    TEST_ASSERT(fractus_mandelbrot_precision_paths_match(), "Mandelbrot double and extended images diverge");
    TEST_ASSERT(fractus_julia_precision_paths_match(), "Julia double and extended images diverge");
    TEST_ASSERT(fractus_biomorph_precision_paths_match(), "Biomorph double and extended images diverge");
    return 1;
}

int main(void)
{
    printf("=== RUNNING FRACTAL TESTS ===\n");
    TEST_RUN(test_plane_point_and_zoom);
    TEST_RUN(test_extended_precision);
    TEST_RUN(test_hp_format);
    TEST_RUN(test_render_progress_stop);
    TEST_RUN(test_precision_paths_match);
    TEST_RUN(test_mandelbrot_rendering);
    TEST_RUN(test_julia_rendering);
    TEST_RUN(test_plasma_rendering);
    TEST_RUN(test_palette_cycling);
    TEST_RUN(test_lorenz_rendering);
    TEST_REPORT();
}
