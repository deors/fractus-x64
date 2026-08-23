#include "test_common.h"
#include "core/fractal.h"
#include "platform/framebuffer.h"

static int test_biomorph_validation(void)
{
    fractus_framebuffer fb;
    fractus_size_u32 size = {64, 48};
    fractus_biomorph_params params;

    TEST_ASSERT(fractus_framebuffer_init(&fb, size) == FRACTUS_STATUS_OK, "Framebuffer init failed");

    params = (fractus_biomorph_params){
        -2.0, 2.0, -1.5, 1.5,
        -0.6, 0.55,
        250u, 1000.0, 1.0,
        0u, FRACTUS_PALETTE_OFFSET, FRACTUS_PALETTE_SPAN,
        FRACTUS_BIOMORPH_EQ_Z2,
        FRACTUS_BIOMORPH_TRAP_RE_OR_IM
    };

    /* Valid render */
    TEST_ASSERT(fractus_fractal_render_biomorph(&fb, &params) == FRACTUS_STATUS_OK, "Valid biomorph render failed");

    /* NULL parameters */
    TEST_ASSERT(fractus_fractal_render_biomorph(NULL, &params) == FRACTUS_STATUS_INVALID_ARGUMENT, "NULL framebuffer should fail");
    TEST_ASSERT(fractus_fractal_render_biomorph(&fb, NULL) == FRACTUS_STATUS_INVALID_ARGUMENT, "NULL params should fail");

    /* Invalid max_iterations */
    params.max_iterations = 0u;
    TEST_ASSERT(fractus_fractal_render_biomorph(&fb, &params) == FRACTUS_STATUS_INVALID_ARGUMENT, "0 iterations should fail");
    params.max_iterations = 250u;

    /* Invalid escape_radius_squared */
    params.escape_radius_squared = 0.0;
    TEST_ASSERT(fractus_fractal_render_biomorph(&fb, &params) == FRACTUS_STATUS_INVALID_ARGUMENT, "0 escape radius should fail");
    params.escape_radius_squared = -10.0;
    TEST_ASSERT(fractus_fractal_render_biomorph(&fb, &params) == FRACTUS_STATUS_INVALID_ARGUMENT, "Negative escape radius should fail");
    params.escape_radius_squared = 1000.0;

    /* Invalid palette_span */
    params.palette_span = 0u;
    TEST_ASSERT(fractus_fractal_render_biomorph(&fb, &params) == FRACTUS_STATUS_INVALID_ARGUMENT, "0 palette span should fail");

    fractus_framebuffer_shutdown(&fb);
    return 1;
}

static int test_biomorph_all_24_combinations(void)
{
    fractus_framebuffer fb;
    fractus_size_u32 size = {160, 120};
    int eq;
    int trap;

    TEST_ASSERT(fractus_framebuffer_init(&fb, size) == FRACTUS_STATUS_OK, "Framebuffer init failed");

    for (eq = 0; eq <= (int)FRACTUS_BIOMORPH_EQ_EXP_Z; ++eq) {
        for (trap = 0; trap <= (int)FRACTUS_BIOMORPH_TRAP_SOLO_IM; ++trap) {
            fractus_biomorph_params params = {
                -2.0, 2.0, -1.5, 1.5,
                -0.6, 0.55,
                100u, 1000.0, 1.0,
                0u, FRACTUS_PALETTE_OFFSET, FRACTUS_PALETTE_SPAN,
                (fractus_biomorph_equation)eq,
                (fractus_biomorph_trap_mode)trap
            };

            fractus_status status = fractus_fractal_render_biomorph(&fb, &params);
            TEST_ASSERT(status == FRACTUS_STATUS_OK, "Biomorph combination render failed");
        }
    }

    fractus_framebuffer_shutdown(&fb);
    return 1;
}

static int test_biomorph_numerical_stability(void)
{
    fractus_framebuffer fb;
    fractus_size_u32 size = {160, 120};
    fractus_biomorph_params params;

    TEST_ASSERT(fractus_framebuffer_init(&fb, size) == FRACTUS_STATUS_OK, "Framebuffer init failed");

    /* Stress test sin(z) with large coordinate domain */
    params = (fractus_biomorph_params){
        -10.0, 10.0, -10.0, 10.0,
        1.5, -0.8,
        200u, 10000.0, 2.0,
        0u, FRACTUS_PALETTE_OFFSET, FRACTUS_PALETTE_SPAN,
        FRACTUS_BIOMORPH_EQ_SIN_Z,
        FRACTUS_BIOMORPH_TRAP_RE_OR_IM
    };
    TEST_ASSERT(fractus_fractal_render_biomorph(&fb, &params) == FRACTUS_STATUS_OK, "sin(z) stress render failed");

    /* Stress test e^z with large coordinate domain */
    params.equation = FRACTUS_BIOMORPH_EQ_EXP_Z;
    TEST_ASSERT(fractus_fractal_render_biomorph(&fb, &params) == FRACTUS_STATUS_OK, "e^z stress render failed");

    /* Stress test z^5 with large coordinate domain */
    params.equation = FRACTUS_BIOMORPH_EQ_Z5;
    TEST_ASSERT(fractus_fractal_render_biomorph(&fb, &params) == FRACTUS_STATUS_OK, "z^5 stress render failed");

    fractus_framebuffer_shutdown(&fb);
    return 1;
}

static int test_biomorph_trap_logic(void)
{
    fractus_framebuffer fb;
    fractus_size_u32 size = {64, 64};
    uint8_t pixel_val = 0;
    fractus_biomorph_params params;

    TEST_ASSERT(fractus_framebuffer_init(&fb, size) == FRACTUS_STATUS_OK, "Framebuffer init failed");

    params = (fractus_biomorph_params){
        -2.0, 2.0, -2.0, 2.0,
        0.0, 0.0,
        50u, 100.0, 1.0,
        0u, FRACTUS_PALETTE_OFFSET, FRACTUS_PALETTE_SPAN,
        FRACTUS_BIOMORPH_EQ_Z2,
        FRACTUS_BIOMORPH_TRAP_RE_OR_IM
    };

    TEST_ASSERT(fractus_fractal_render_biomorph(&fb, &params) == FRACTUS_STATUS_OK, "Trap render failed");
    TEST_ASSERT(fractus_framebuffer_get_pixel(&fb, 0, 0, &pixel_val) == FRACTUS_STATUS_OK, "Get pixel failed");

    fractus_framebuffer_shutdown(&fb);
    return 1;
}

int main(void)
{
    printf("=== RUNNING BIOMORPH TESTS ===\n");
    TEST_RUN(test_biomorph_validation);
    TEST_RUN(test_biomorph_all_24_combinations);
    TEST_RUN(test_biomorph_numerical_stability);
    TEST_RUN(test_biomorph_trap_logic);
    TEST_REPORT();
}
