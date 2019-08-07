#ifndef INCLUDE_GUARD_REQUEST_H
#define INCLUDE_GUARD_REQUEST_H

#include <stddef.h>
#include <stdint.h>

#include "common.h"
#include "basic_structures.h"

typedef enum {
    // drawing
    TINYWS_REQUEST_DRAW_RECT,
    TINYWS_REQUEST_DRAW_CIRCLE,
    TINYWS_REQUEST_DRAW_LINE,
    TINYWS_REQUEST_DRAW_PIXEL,
    TINYWS_REQUEST_CLEAR_WINDOW,
    TINYWS_REQUEST_REFRESH,

    // window management
    TINYWS_REQUEST_CREATE_WINDOW,
    TINYWS_REQUEST_CLOSE_WINDOW,
    TINYWS_REQUEST_SET_WINDOW_POS,
    TINYWS_REQUEST_SET_WINDOW_VISIBILITY,
    TINYWS_REQUEST_SET_FOCUS,
    TINYWS_REQUEST_GET_WINDOW_INFO,
    TINYWS_REQUEST_MOVE_WINDOW_TOP,
    TINYWS_REQUEST_WINDOW_REPARENT,
    TINYWS_REQUEST_GET_TOPLEVEL_WINDOW,

    TINYWS_REQUEST_APPLY_FOR_WM,

    // event
    TINYWS_REQUEST_GET_EVENT,

    TINYWS_REQUEST_INVALID,
} RequestType;

struct Request {
    RequestType type;
    window_id_t target_window_id;
    client_id_t source;
    union {
        struct {
            Rect rect;
            Color color;
        } draw_rect;
        struct {
            Point center;
            int32_t radius;
            uint8_t filled;
            Color color;
        } draw_circle;
        struct {
            Point p1;
            Point p2;
            Color color;
        } draw_line;
        struct {
            Point p;
            Color color;
        } draw_pixel;
        struct {
            Rect rect;
            Color bg_color;
        } create_window;
        struct {
            Point pos;
        } set_window_pos;
        struct {
            uint8_t visible;
        } set_window_visibility;
        struct {
            window_id_t root_win_id;
        } get_toplevel_window;
        struct {
            window_id_t parent_window_id;
        } reparent;
    } param;
};

void request_print(const struct Request *request);

size_t request_encode(const struct Request *request, uint8_t *out, size_t size);
struct Request request_decode(const uint8_t *buf, size_t size);

#endif