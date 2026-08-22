#include "core/attractors.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

static void fractus_lorenz_rk4_step(
    double *x,
    double *y,
    double *z,
    double sigma,
    double rho,
    double beta,
    double dt)
{
    double k1x = sigma * (*y - *x);
    double k1y = *x * (rho - *z) - *y;
    double k1z = *x * *y - beta * *z;

    double x2 = *x + 0.5 * dt * k1x;
    double y2 = *y + 0.5 * dt * k1y;
    double z2 = *z + 0.5 * dt * k1z;
    double k2x = sigma * (y2 - x2);
    double k2y = x2 * (rho - z2) - y2;
    double k2z = x2 * y2 - beta * z2;

    double x3 = *x + 0.5 * dt * k2x;
    double y3 = *y + 0.5 * dt * k2y;
    double z3 = *z + 0.5 * dt * k2z;
    double k3x = sigma * (y3 - x3);
    double k3y = x3 * (rho - z3) - y3;
    double k3z = x3 * y3 - beta * z3;

    double x4 = *x + dt * k3x;
    double y4 = *y + dt * k3y;
    double z4 = *z + dt * k3z;
    double k4x = sigma * (y4 - x4);
    double k4y = x4 * (rho - z4) - y4;
    double k4z = x4 * y4 - beta * z4;

    *x += (dt / 6.0) * (k1x + 2.0 * k2x + 2.0 * k3x + k4x);
    *y += (dt / 6.0) * (k1y + 2.0 * k2y + 2.0 * k3y + k4y);
    *z += (dt / 6.0) * (k1z + 2.0 * k2z + 2.0 * k3z + k4z);
}

static void fractus_lorenz_plot_line(
    uint8_t *pixels,
    int32_t width,
    int32_t height,
    int32_t pitch,
    int32_t x0,
    int32_t y0,
    int32_t x1,
    int32_t y1,
    uint8_t color)
{
    int32_t dx = abs(x1 - x0);
    int32_t dy = -abs(y1 - y0);
    int32_t sx = (x0 < x1) ? 1 : -1;
    int32_t sy = (y0 < y1) ? 1 : -1;
    int32_t err = dx + dy;

    while (1) {
        if (x0 >= 0 && x0 < width && y0 >= 0 && y0 < (height - 20)) {
            const size_t idx = (size_t)y0 * (size_t)pitch + (size_t)x0;
            pixels[idx] = color;
        }
        if (x0 == x1 && y0 == y1) {
            break;
        }
        {
            int32_t e2 = 2 * err;
            if (e2 >= dy) {
                err += dy;
                x0 += sx;
            }
            if (e2 <= dx) {
                err += dx;
                y0 += sy;
            }
        }
    }
}

fractus_status fractus_fractal_render_lorenz(
    fractus_framebuffer *framebuffer,
    const fractus_lorenz_params *params)
{
    int32_t width;
    int32_t height;
    int32_t pitch;
    uint32_t i;
    double x = 0.1;
    double y = 0.0;
    double z = 0.0;
    double x_min_s = 1e9, x_max_s = -1e9;
    double y_min_s = 1e9, y_max_s = -1e9;
    double z_min_s = 1e9, z_max_s = -1e9;
    double x_max_abs, y_max_abs;
    double z_center, z_span;
    double x_span, y_span;
    double h_span, v_span, v_center;
    double scale_h, scale_v, scale;
    double h_offset_px, v_offset_px;
    double c_focus, c_z, d_max;
    uint8_t pal_offset;
    uint8_t pal_span;

    if (framebuffer == NULL || !framebuffer->initialized || params == NULL) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    width = (int32_t)framebuffer->size.width;
    height = (int32_t)framebuffer->size.height;
    pitch = (int32_t)framebuffer->pitch_pixels;
    pal_offset = (params->palette_offset >= 16u) ? params->palette_offset : 16u;
    pal_span = (params->palette_span >= 2u) ? params->palette_span : 240u;

    if (width <= 0 || height <= 0 || params->iterations == 0u || params->dt <= 0.0) {
        return FRACTUS_STATUS_INVALID_ARGUMENT;
    }

    /* 1. Calentamiento inicial: 2000 pasos para situar el punto en el atractor */
    for (i = 0u; i < 2000u; ++i) {
        fractus_lorenz_rk4_step(&x, &y, &z, params->sigma, params->rho, params->beta, params->dt);
    }

    /* 2. Muestreo para determinar limites reales con simetria horizontal estricta */
    {
        double sx = x, sy = y, sz = z;
        const uint32_t sample_steps = (params->iterations > 20000u) ? 20000u : params->iterations;
        for (i = 0u; i < sample_steps; ++i) {
            fractus_lorenz_rk4_step(&sx, &sy, &sz, params->sigma, params->rho, params->beta, params->dt);
            if (sx < x_min_s) x_min_s = sx;
            if (sx > x_max_s) x_max_s = sx;
            if (sy < y_min_s) y_min_s = sy;
            if (sy > y_max_s) y_max_s = sy;
            if (sz < z_min_s) z_min_s = sz;
            if (sz > z_max_s) z_max_s = sz;
        }

        if (x_max_s - x_min_s < 1e-4 || z_max_s - z_min_s < 1e-4) {
            x_min_s = -25.0;
            x_max_s = 25.0;
            y_min_s = -25.0;
            y_max_s = 25.0;
            z_min_s = 0.0;
            z_max_s = 55.0;
        }
    }

    /* Margen de seguridad del 15% */
    x_max_abs = (fabs(x_min_s) > fabs(x_max_s) ? fabs(x_min_s) : fabs(x_max_s)) * 1.15;
    y_max_abs = (fabs(y_min_s) > fabs(y_max_s) ? fabs(y_min_s) : fabs(y_max_s)) * 1.15;
    z_center = (z_min_s + z_max_s) * 0.5;
    z_span = (z_max_s - z_min_s) * 1.15;
    x_span = 2.0 * x_max_abs;
    y_span = 2.0 * y_max_abs;

    /* Seleccionar dimensiones segun la proyeccion planar configurada */
    if (params->projection == FRACTUS_LORENZ_PROJECTION_XY) {
        h_span = x_span;
        v_span = y_span;
        v_center = 0.0;
    } else if (params->projection == FRACTUS_LORENZ_PROJECTION_YZ) {
        h_span = y_span;
        v_span = z_span;
        v_center = z_center;
    } else { /* FRACTUS_LORENZ_PROJECTION_XZ */
        h_span = x_span;
        v_span = z_span;
        v_center = z_center;
    }

    /* Escala 1:1 preservando aspecto geometrico real */
    scale_h = ((double)width - 24.0) / h_span;
    scale_v = ((double)height - 36.0) / v_span;
    scale = (scale_h < scale_v) ? scale_h : scale_v;

    /* Offsets de centrado horizontal y vertical */
    h_offset_px = (double)width * 0.5;
    v_offset_px = (((double)height - 20.0) * 0.5) + (v_center * scale);

    /* Focos fijos del atractor de Lorenz para calculo del gradiente de color */
    c_focus = (params->rho > 1.0) ? sqrt(params->beta * (params->rho - 1.0)) : 0.0;
    c_z = (params->rho > 1.0) ? (params->rho - 1.0) : 0.0;
    d_max = (c_focus > 0.0) ? (c_focus * 3.5) : 30.0;
    if (d_max < 1.0) {
        d_max = 1.0;
    }

    /* Limpiar framebuffer antes de dibujar */
    (void)memset(framebuffer->index_pixels, 0, (size_t)height * (size_t)pitch);

    /* 3. Trazado continuo con segmentos de linea (Bresenham) y doble orbita simetrica */
    {
        int32_t prev_px1, prev_py1, prev_px2, prev_py2;

        if (params->projection == FRACTUS_LORENZ_PROJECTION_XY) {
            prev_px1 = (int32_t)(h_offset_px + x * scale + 0.5);
            prev_py1 = (int32_t)(v_offset_px - y * scale + 0.5);
            prev_px2 = (int32_t)(h_offset_px - x * scale + 0.5);
            prev_py2 = (int32_t)(v_offset_px + y * scale + 0.5);
        } else if (params->projection == FRACTUS_LORENZ_PROJECTION_YZ) {
            prev_px1 = (int32_t)(h_offset_px + y * scale + 0.5);
            prev_py1 = (int32_t)(v_offset_px - z * scale + 0.5);
            prev_px2 = (int32_t)(h_offset_px - y * scale + 0.5);
            prev_py2 = prev_py1;
        } else { /* X-Z */
            prev_px1 = (int32_t)(h_offset_px + x * scale + 0.5);
            prev_py1 = (int32_t)(v_offset_px - z * scale + 0.5);
            prev_px2 = (int32_t)(h_offset_px - x * scale + 0.5);
            prev_py2 = prev_py1;
        }

        for (i = 0u; i < params->iterations; ++i) {
            int32_t px1, py1, px2, py2;
            double d1, d2, d, norm;
            uint32_t cidx;
            uint8_t color_val;

            fractus_lorenz_rk4_step(&x, &y, &z, params->sigma, params->rho, params->beta, params->dt);

            /* Coordenadas en pantalla segun la proyeccion elegida */
            if (params->projection == FRACTUS_LORENZ_PROJECTION_XY) {
                px1 = (int32_t)(h_offset_px + x * scale + 0.5);
                py1 = (int32_t)(v_offset_px - y * scale + 0.5);
                px2 = (int32_t)(h_offset_px - x * scale + 0.5);
                py2 = (int32_t)(v_offset_px + y * scale + 0.5);
            } else if (params->projection == FRACTUS_LORENZ_PROJECTION_YZ) {
                px1 = (int32_t)(h_offset_px + y * scale + 0.5);
                py1 = (int32_t)(v_offset_px - z * scale + 0.5);
                px2 = (int32_t)(h_offset_px - y * scale + 0.5);
                py2 = py1;
            } else { /* X-Z */
                px1 = (int32_t)(h_offset_px + x * scale + 0.5);
                py1 = (int32_t)(v_offset_px - z * scale + 0.5);
                px2 = (int32_t)(h_offset_px - x * scale + 0.5);
                py2 = py1;
            }

            /* Gradiente de color continuo segun la distancia euclidea 3D a los focos */
            d1 = sqrt((x - c_focus) * (x - c_focus) + (y - c_focus) * (y - c_focus) + (z - c_z) * (z - c_z));
            d2 = sqrt((x + c_focus) * (x + c_focus) + (y + c_focus) * (y + c_focus) + (z - c_z) * (z - c_z));
            d = (d1 < d2) ? d1 : d2;

            norm = d / d_max;
            if (norm > 1.0) norm = 1.0;
            if (norm < 0.0) norm = 0.0;

            /* Foco/nucleo -> indice maximo (calido/rojo); bucles exteriores -> indice menor (frio/azul) */
            cidx = (uint32_t)pal_offset + (uint32_t)((1.0 - norm) * (double)(pal_span - 1u));
            if (cidx >= FRACTUS_PALETTE_SIZE) {
                cidx = FRACTUS_PALETTE_SIZE - 1u;
            }
            color_val = (uint8_t)cidx;

            fractus_lorenz_plot_line(framebuffer->index_pixels, width, height, pitch, prev_px1, prev_py1, px1, py1, color_val);
            fractus_lorenz_plot_line(framebuffer->index_pixels, width, height, pitch, prev_px2, prev_py2, px2, py2, color_val);

            prev_px1 = px1;
            prev_py1 = py1;
            prev_px2 = px2;
            prev_py2 = py2;
        }
    }

    framebuffer->pixels_dirty = 1;
    return FRACTUS_STATUS_OK;
}
