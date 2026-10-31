#ifndef FRACTUS_X64_CORE_PROGRESS_H
#define FRACTUS_X64_CORE_PROGRESS_H

#include <stdint.h>

#ifdef _OPENMP
#include <omp.h>
#endif

/* Optional row callback. NULL keeps one parallel region and does not report.
   A non-zero return stops the renderer before the next row band. */
typedef struct fractus_render_progress {
    int (*report)(void *user, uint32_t rows_done, uint32_t rows_total);
    void *user;
} fractus_render_progress;

static inline int fractus_render_progress_report(
    const fractus_render_progress *progress,
    uint32_t rows_done,
    uint32_t rows_total)
{
    if (progress != NULL && progress->report != NULL) {
        return progress->report(progress->user, rows_done, rows_total);
    }
    return 0;
}

/* Band height equals the worker count so every core stays busy.
   A missing callback returns the full height: one region, today's schedule. */
static inline uint32_t fractus_render_band_rows(
    const fractus_render_progress *progress,
    uint32_t height)
{
    uint32_t threads;

    if (progress == NULL || progress->report == NULL || height == 0u) {
        return height;
    }

#ifdef _OPENMP
    threads = (uint32_t)omp_get_max_threads();
#else
    threads = 1u;
#endif
    if (threads < 4u) {
        threads = 4u;
    }
    if (threads > height) {
        return height;
    }
    return threads;
}

#endif
