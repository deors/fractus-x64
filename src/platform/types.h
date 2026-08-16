#ifndef FRACTUS_X64_TYPES_H
#define FRACTUS_X64_TYPES_H

#include <stddef.h>
#include <stdint.h>

typedef enum fractus_status {
    FRACTUS_STATUS_OK = 0,
    FRACTUS_STATUS_ERROR = 1,
    FRACTUS_STATUS_INVALID_ARGUMENT = 2,
    FRACTUS_STATUS_UNSUPPORTED = 3
} fractus_status;

typedef struct fractus_size_u32 {
    uint32_t width;
    uint32_t height;
} fractus_size_u32;

typedef struct fractus_point_i32 {
    int32_t x;
    int32_t y;
} fractus_point_i32;

typedef struct fractus_rect_i32 {
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
} fractus_rect_i32;

#endif
