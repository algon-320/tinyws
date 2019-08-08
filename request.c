#include <stdio.h>
#include <assert.h>

#include "request.h"
#include "common.h"

void request_print(const struct Request *request) {
    switch (request->type) {
        case TINYWS_REQUEST_DRAW_RECT:
            printf("TINYWS_REQUEST_DRAW_RECT(target=%d, x=%d, y=%d, w=%d, h=%d, filled=%d, color=(%d, %d, %d, %d))\n",
                    request->target_window_id,
                    request->param.draw_rect.rect.x,
                    request->param.draw_rect.rect.y,
                    request->param.draw_rect.rect.width,
                    request->param.draw_rect.rect.height,
                    request->param.draw_rect.filled,
                    request->param.draw_rect.color.r,
                    request->param.draw_rect.color.g,
                    request->param.draw_rect.color.b,
                    request->param.draw_rect.color.a
                    );
            break;
        case TINYWS_REQUEST_DRAW_CIRCLE:
            printf("TINYWS_REQUEST_DRAW_CIRCLE(target=%d, x=%d, y=%d, radius=%d, filled=%d, color=(%d, %d, %d, %d))\n",
                    request->target_window_id,
                    request->param.draw_circle.center.x,
                    request->param.draw_circle.center.y,
                    request->param.draw_circle.radius,
                    request->param.draw_circle.filled,
                    request->param.draw_circle.color.r,
                    request->param.draw_circle.color.g,
                    request->param.draw_circle.color.b,
                    request->param.draw_circle.color.a
                    );
            break;
        case TINYWS_REQUEST_DRAW_LINE:
            printf("TINYWS_REQUEST_DRAW_LINE(target=%d, x1=%d, y1=%d, x2=%d, y2=%d, color=(%d, %d, %d, %d))\n",
                    request->target_window_id,
                    request->param.draw_line.p1.x,
                    request->param.draw_line.p1.y,
                    request->param.draw_line.p2.x,
                    request->param.draw_line.p2.y,
                    request->param.draw_line.color.r,
                    request->param.draw_line.color.g,
                    request->param.draw_line.color.b,
                    request->param.draw_line.color.a
                    );
            break;  
        case TINYWS_REQUEST_DRAW_PIXEL:
            printf("TINYWS_REQUEST_DRAW_PIXEL(target=%d, x=%d, y=%d, color=(%d, %d, %d, %d))\n",
                    request->target_window_id,
                    request->param.draw_pixel.p.x,
                    request->param.draw_pixel.p.y,
                    request->param.draw_pixel.color.r,
                    request->param.draw_pixel.color.g,
                    request->param.draw_pixel.color.b,
                    request->param.draw_pixel.color.a
                    );
            break;
        case TINYWS_REQUEST_CLEAR_WINDOW:
            printf("TINYWS_REQUEST_CLEAR_WINDOW(target=%d)\n",
                    request->target_window_id
                    );
            break;
        case TINYWS_REQUEST_REFRESH:
            printf("TINYWS_REQUEST_REFRESH(target=%d)\n",
                    request->target_window_id
                    );
            break;
        case TINYWS_REQUEST_CREATE_WINDOW:
            printf("TINYWS_REQUEST_CREATE_WINDOW(target=%d, width=%d, height=%d, pos_x=%d, pos_y=%d, bg_color=(%d, %d, %d, %d))\n",
                    request->target_window_id,
                    request->param.create_window.rect.width,
                    request->param.create_window.rect.height,
                    request->param.create_window.rect.x,
                    request->param.create_window.rect.y,
                    request->param.create_window.bg_color.r,
                    request->param.create_window.bg_color.g,
                    request->param.create_window.bg_color.b,
                    request->param.create_window.bg_color.a
                    );
            break;
        case TINYWS_REQUEST_CLOSE_WINDOW:
        {
            printf("TINYWS_REQUEST_CLOSE_WINDOW(target=%d)\n",
                    request->target_window_id
                    );
            break;
        }
        case TINYWS_REQUEST_SET_WINDOW_POS:
            printf("TINYWS_REQUEST_SET_WINDOW_POS(target=%d, pos.x=%d, pos.y=%d)\n",
                    request->target_window_id,
                    request->param.set_window_pos.pos.x,
                    request->param.set_window_pos.pos.y
                    );
            break;
        case TINYWS_REQUEST_SET_WINDOW_VISIBILITY:
            printf("TINYWS_REQUEST_SET_WINDOW_VISIBILITY(target=%d, visible=%d)\n",
                    request->target_window_id,
                    request->param.set_window_visibility.visible
                    );
            break;
        case TINYWS_REQUEST_MOVE_WINDOW_TOP:
            printf("TINYWS_REQUEST_MOVE_WINDOW_TOP(target=%d)\n",
                    request->target_window_id
                    );
            break;
        case TINYWS_REQUEST_SET_FOCUS:
            printf("TINYWS_REQUEST_FOCUS(target=%d)\n",
                    request->target_window_id
                    );
            break;
        case TINYWS_REQUEST_GET_WINDOW_INFO:
        {
            printf("TINYWS_REQUEST_GET_WINDOW_INFO(target=%d)\n",
                    request->target_window_id
                    );
            break;
        }
        case TINYWS_REQUEST_WINDOW_REPARENT:
            printf("TINYWS_REQUEST_WINDOW_REPARENT(target=%d, parent=%d)\n",
                    request->target_window_id,
                    request->param.reparent.parent_window_id
                    );
            break;
        case TINYWS_REQUEST_GET_TOPLEVEL_WINDOW:
        {
            printf("TINYWS_REQUEST_GET_TOPLEVEL_WINDOW(target=%d, root=%d)\n",
                    request->target_window_id,
                    request->param.get_toplevel_window.root_win_id
                    );
            break;
        }

        case TINYWS_REQUEST_APPLY_FOR_WM:
            printf("TINYWS_REQUEST_APPLY_FOR_WM(target=%d)\n",
                    request->target_window_id
                    );
            break;
        case TINYWS_REQUEST_GET_EVENT:
            printf("TINYWS_REQUEST_GET_EVENT(target=%d)\n",
                    request->target_window_id
                    );
            break;
        case TINYWS_REQUEST_INVALID:
            printf("TINYWS_REQUEST_INVALID\n"
                    );
            break;
    }
}


size_t request_encode(const struct Request *request, uint8_t *out, size_t size) {
    
    uint8_t *nxt = out;
    WRITE_ENUM_LE(request->type, &nxt);
    WRITE_INT_LE(request->target_window_id, &nxt);

    switch (request->type) {
        case TINYWS_REQUEST_DRAW_RECT:
        {
            rect_encode(&request->param.draw_rect.rect, &nxt);
            WRITE_INT_LE(request->param.draw_rect.filled, &nxt);
            color_encode(&request->param.draw_rect.color, &nxt);
            break;
        }
        case TINYWS_REQUEST_DRAW_CIRCLE:
        {
            point_encode(&request->param.draw_circle.center, &nxt);
            WRITE_INT_LE(request->param.draw_circle.radius, &nxt);
            WRITE_INT_LE(request->param.draw_circle.filled, &nxt);
            color_encode(&request->param.draw_circle.color, &nxt);
            break;
        }
        case TINYWS_REQUEST_DRAW_LINE:
        {
            point_encode(&request->param.draw_line.p1, &nxt);
            point_encode(&request->param.draw_line.p2, &nxt);
            color_encode(&request->param.draw_line.color, &nxt);
            break;
        }
        case TINYWS_REQUEST_DRAW_PIXEL:
        {
            point_encode(&request->param.draw_pixel.p, &nxt);
            color_encode(&request->param.draw_pixel.color, &nxt);
            break;
        }
        case TINYWS_REQUEST_CLEAR_WINDOW:
        {
            break;
        }
        case TINYWS_REQUEST_REFRESH:
        {
            break;
        }

        // Window management
        case TINYWS_REQUEST_CREATE_WINDOW:
        {
            rect_encode(&request->param.create_window.rect, &nxt);
            color_encode(&request->param.create_window.bg_color, &nxt);
            break;
        }
        case TINYWS_REQUEST_CLOSE_WINDOW:
        {
            break;
        }
        case TINYWS_REQUEST_SET_WINDOW_POS:
        {
            point_encode(&request->param.set_window_pos.pos, &nxt);
            break;
        }
        case TINYWS_REQUEST_SET_WINDOW_VISIBILITY:
        {
            WRITE_INT_LE(request->param.set_window_visibility.visible, &nxt);
            break;
        }
        case TINYWS_REQUEST_GET_WINDOW_INFO:
        {
            break;
        }
        case TINYWS_REQUEST_WINDOW_REPARENT:
        {
            WRITE_INT_LE(request->param.reparent.parent_window_id, &nxt);
            break;
        }
        case TINYWS_REQUEST_GET_TOPLEVEL_WINDOW:
        {
            WRITE_INT_LE(request->param.get_toplevel_window.root_win_id, &nxt);
            break;
        }
        case TINYWS_REQUEST_SET_FOCUS:
        {
            break;
        }
        case TINYWS_REQUEST_MOVE_WINDOW_TOP:
        {
            break;
        }
        case TINYWS_REQUEST_APPLY_FOR_WM:
        {
            break;
        }
        case TINYWS_REQUEST_GET_EVENT:
        {
            break;
        }

        default:
        {
            fprintf(stderr, "invalid request type\n");
            assert(0);
            break;
        }
    }
    return nxt - out;
}


struct Request request_decode(const uint8_t *buf, size_t size) {
    struct Request ret;
    
    READ_ENUM_LE(&buf, &ret.type);
    READ_INT_LE(&buf, &ret.target_window_id);

    switch (ret.type) {
        case TINYWS_REQUEST_DRAW_RECT:
        {
            rect_decode(&buf, &ret.param.draw_rect.rect);
            READ_INT_LE(&buf, &ret.param.draw_rect.filled);
            color_decode(&buf, &ret.param.draw_rect.color);
            break;
        }
        case TINYWS_REQUEST_DRAW_CIRCLE:
        {
            point_decode(&buf, &ret.param.draw_circle.center);
            READ_INT_LE(&buf, &ret.param.draw_circle.radius);
            READ_INT_LE(&buf, &ret.param.draw_circle.filled);
            color_decode(&buf, &ret.param.draw_circle.color);
            break;
        }
        case TINYWS_REQUEST_DRAW_LINE:
        {
            point_decode(&buf, &ret.param.draw_line.p1);
            point_decode(&buf, &ret.param.draw_line.p2);
            color_decode(&buf, &ret.param.draw_line.color);
            break;
        }
        case TINYWS_REQUEST_DRAW_PIXEL:
        {
            point_decode(&buf, &ret.param.draw_pixel.p);
            color_decode(&buf, &ret.param.draw_pixel.color);
            break;
        }
        case TINYWS_REQUEST_CLEAR_WINDOW:
        {
            break;
        }
        case TINYWS_REQUEST_REFRESH:
        {
            break;
        }

        // window management
        case TINYWS_REQUEST_CREATE_WINDOW:
        {
            rect_decode(&buf, &ret.param.create_window.rect);
            color_decode(&buf, &ret.param.create_window.bg_color);
            break;
        }
        case TINYWS_REQUEST_CLOSE_WINDOW:
        {
            break;
        }
        case TINYWS_REQUEST_SET_WINDOW_POS:
        {
            point_decode(&buf, &ret.param.set_window_pos.pos);
            break;
        }
        case TINYWS_REQUEST_SET_WINDOW_VISIBILITY:
        {
            READ_INT_LE(&buf, &ret.param.set_window_visibility.visible);
            break;
        }
        case TINYWS_REQUEST_GET_WINDOW_INFO:
        {
            break;
        }
        case TINYWS_REQUEST_WINDOW_REPARENT:
        {
            READ_INT_LE(&buf, &ret.param.reparent.parent_window_id);
            break;
        }
        case TINYWS_REQUEST_GET_TOPLEVEL_WINDOW:
        {
            READ_INT_LE(&buf, &ret.param.get_toplevel_window.root_win_id);
            break;
        }
        case TINYWS_REQUEST_SET_FOCUS:
        {
            break;
        }
        case TINYWS_REQUEST_MOVE_WINDOW_TOP:
        {
            break;
        }
        case TINYWS_REQUEST_APPLY_FOR_WM:
        {
            break;
        }
        case TINYWS_REQUEST_GET_EVENT:
        {
            break;
        }

        default:
        {
            ret.type = TINYWS_REQUEST_INVALID;
            break;
        }
    }
    return ret;
}

struct Request requeset_new_draw_rect(window_id_t target_window_id, Rect rect, uint8_t filled, Color color) {
    struct Request req;
    req.target_window_id = target_window_id;
    req.type = TINYWS_REQUEST_DRAW_RECT;
    req.param.draw_rect.rect = rect;
    req.param.draw_rect.filled = filled;
    req.param.draw_rect.color = color;
    return req;
}
struct Request requeset_new_draw_circle(window_id_t target_window_id, Point center, int32_t radius, uint8_t filled, Color color) {
    struct Request req;
    req.target_window_id = target_window_id;
    req.type = TINYWS_REQUEST_DRAW_CIRCLE;
    req.param.draw_circle.center = center;
    req.param.draw_circle.radius = radius;
    req.param.draw_circle.filled = filled;
    req.param.draw_circle.color = color;
    return req;
}
struct Request requeset_new_draw_line(window_id_t target_window_id, Point p1, Point p2, Color color) {
    struct Request req;
    req.target_window_id = target_window_id;
    req.type = TINYWS_REQUEST_DRAW_LINE;
    req.param.draw_line.p1 = p1;
    req.param.draw_line.p2 = p2;
    req.param.draw_line.color = color;
    return req;
}
struct Request requeset_new_draw_pixel(window_id_t target_window_id, Point p, Color color) {
    struct Request req;
    req.target_window_id = target_window_id;
    req.type = TINYWS_REQUEST_DRAW_PIXEL;
    req.param.draw_pixel.p = p;
    req.param.draw_line.color = color;
    return req;
}
struct Request requeset_new_clear_window(window_id_t target_window_id) {
    struct Request req;
    req.target_window_id = target_window_id;
    req.type = TINYWS_REQUEST_CLEAR_WINDOW;
    return req;
}
struct Request requeset_new_refresh() {
    struct Request req;
    req.type = TINYWS_REQUEST_REFRESH;
    return req;
}
struct Request requeset_new_create_window(window_id_t parent_window_id, Rect rect, Color bg_color) {
    struct Request req;
    req.target_window_id = parent_window_id;
    req.type = TINYWS_REQUEST_CREATE_WINDOW;
    req.param.create_window.rect = rect;
    req.param.create_window.bg_color = bg_color;
    return req;
}
struct Request requeset_new_close_window(window_id_t target_window_id) {
    struct Request req;
    req.target_window_id = target_window_id;
    req.type = TINYWS_REQUEST_CLOSE_WINDOW;
    return req;
}
struct Request requeset_new_set_window_pos(window_id_t target_window_id, Point pos) {
    struct Request req;
    req.target_window_id = target_window_id;
    req.type = TINYWS_REQUEST_SET_WINDOW_POS;
    req.param.set_window_pos.pos = pos;
    return req;
}
struct Request requeset_new_set_window_visibility(window_id_t target_window_id, uint8_t visible) {
    struct Request req;
    req.target_window_id = target_window_id;
    req.type = TINYWS_REQUEST_SET_WINDOW_VISIBILITY;
    req.param.set_window_visibility.visible = visible;
    return req;
}
struct Request requeset_new_set_focus(window_id_t target_window_id) {
    struct Request req;
    req.target_window_id = target_window_id;
    req.type = TINYWS_REQUEST_SET_FOCUS;
    return req;
}
struct Request requeset_new_get_window_info(window_id_t target_window_id) {
    struct Request req;
    req.target_window_id = target_window_id;
    req.type = TINYWS_REQUEST_GET_WINDOW_INFO;
    return req;
}
struct Request requeset_new_move_window_top(window_id_t target_window_id) {
    struct Request req;
    req.target_window_id = target_window_id;
    req.type = TINYWS_REQUEST_MOVE_WINDOW_TOP;
    return req;
}
struct Request requeset_new_window_reparent(window_id_t target_window_id, window_id_t parent_window_id) {
    struct Request req;
    req.target_window_id = target_window_id;
    req.type = TINYWS_REQUEST_WINDOW_REPARENT;
    req.param.reparent.parent_window_id = parent_window_id;
    return req;
}
struct Request requeset_new_get_toplevel_window(window_id_t target_window_id, window_id_t root_win_id) {
    struct Request req;
    req.target_window_id = target_window_id;
    req.type = TINYWS_REQUEST_GET_TOPLEVEL_WINDOW;
    req.param.get_toplevel_window.root_win_id = root_win_id;
    return req;
}
struct Request requeset_new_apply_for_wm(window_id_t target_window_id) {
    struct Request req;
    req.target_window_id = target_window_id;
    req.type = TINYWS_REQUEST_APPLY_FOR_WM;
    return req;
}
struct Request requeset_new_get_event(window_id_t target_window_id) {
    struct Request req;
    req.target_window_id = target_window_id;
    req.type = TINYWS_REQUEST_GET_EVENT;
    return req;
}