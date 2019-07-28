#ifndef INCLUDE_GUARD_QUERY_H
#define INCLUDE_GUARD_QUERY_H

#include <stddef.h>
#include <stdint.h>

#include "basic_structures.h"

typedef enum {
    // drawing
    TINYWS_QUERY_DRAW_RECT,
    TINYWS_QUERY_DRAW_CIRCLE,
    TINYWS_QUERY_DRAW_LINE,
    TINYWS_QUERY_DRAW_PIXEL,
    TINYWS_QUERY_CLEAR_WINDOW,
    TINYWS_QUERY_REFRESH,

    // window management
    TINYWS_QUERY_CREATE_WINDOW,
    TINYWS_QUERY_SET_WINDOW_POS,
    TINYWS_QUERY_SET_WINDOW_VISIBILITY,

    TINYWS_QUERY_INVALID,
} QueryType;

struct Query {
    QueryType type;
    int32_t target_window_id;
    union {
        struct {
            int32_t x, y, w, h;
            Color color;
        } draw_rect;
        struct {
            int32_t x, y, radius;
            uint8_t filled;
            Color color;
        } draw_circle;
        struct {
            int32_t x1, y1, x2, y2;
            Color color;
        } draw_line;
        struct {
            int32_t x, y;
            Color color;
        } draw_pixel;
        struct {
            int32_t parent_window_id;
            int32_t width, height, pos_x, pos_y;
            Color bg_color;
        } create_window;
        struct {
            int32_t pos_x, pos_y;
        } set_window_pos;
        struct {
            uint8_t visible;
        } set_window_visibility;
    } param;
};

void query_print(const struct Query *query);

size_t query_encode(const struct Query *query, uint8_t *out, size_t size);
struct Query query_decode(const uint8_t *buf, size_t size);

#endif