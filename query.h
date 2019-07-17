#ifndef INCLUDE_GUARD_QUERY_H
#define INCLUDE_GUARD_QUERY_H

#include <stddef.h>
#include "query.h"

typedef enum {
    DrawRect,
    DrawCircle,
    DrawLine,
    DrawPixel,
    ClearScreen,
    Invalid,
} Operation;

struct Query {
    Operation op;
    union {
        struct {
            int x, y, w, h;
        } draw_rect_param;
        struct {
            int x, y, radius;
            char filled;
        } draw_circle_param;
        struct {
            int x1, y1, x2, y2;
        } draw_line_param;
        struct {
            int x, y;
        } draw_pixel_param;
    } param;
};

void print_query(struct Query query);

int encode_query(struct Query query, unsigned char *out, size_t size);
struct Query decode_query(const unsigned char *buf, size_t size);

#endif