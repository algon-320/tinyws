#include <stdio.h>
#include <assert.h>

#include "request.h"
#include "common.h"

void request_print(const struct Request *request) {
    switch (request->type) {
        case TINYWS_REQUEST_DRAW_RECT:
            printf("TINYWS_REQUEST_DRAW_RECT(target=%d, x=%d, y=%d, w=%d, h=%d, color=(%d, %d, %d, %d))\n",
                    request->target_window_id,
                    request->param.draw_rect.x,
                    request->param.draw_rect.y,
                    request->param.draw_rect.w,
                    request->param.draw_rect.h,
                    request->param.draw_rect.color.r,
                    request->param.draw_rect.color.g,
                    request->param.draw_rect.color.b,
                    request->param.draw_rect.color.a
                    );
            break;
        case TINYWS_REQUEST_DRAW_CIRCLE:
            printf("TINYWS_REQUEST_DRAW_CIRCLE(target=%d, x=%d, y=%d, radius=%d, filled=%d, color=(%d, %d, %d, %d))\n",
                    request->target_window_id,
                    request->param.draw_circle.x,
                    request->param.draw_circle.y,
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
                    request->param.draw_line.x1,
                    request->param.draw_line.y1,
                    request->param.draw_line.x2,
                    request->param.draw_line.y2,
                    request->param.draw_line.color.r,
                    request->param.draw_line.color.g,
                    request->param.draw_line.color.b,
                    request->param.draw_line.color.a
                    );
            break;  
        case TINYWS_REQUEST_DRAW_PIXEL:
            printf("TINYWS_REQUEST_DRAW_PIXEL(target=%d, x=%d, y=%d, color=(%d, %d, %d, %d))\n",
                    request->target_window_id,
                    request->param.draw_pixel.x,
                    request->param.draw_pixel.y,
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
            printf("TINYWS_REQUEST_REFRESH\n");
            break;
        case TINYWS_REQUEST_CREATE_WINDOW:
            printf("TINYWS_REQUEST_CREATE_WINDOW(pwid=%d, width=%d, height=%d, pos_x=%d, pos_y=%d, bg_color=(%d, %d, %d, %d))\n",
                    request->param.create_window.parent_window_id,
                    request->param.create_window.width,
                    request->param.create_window.height,
                    request->param.create_window.pos_x,
                    request->param.create_window.pos_y,
                    request->param.create_window.bg_color.r,
                    request->param.create_window.bg_color.g,
                    request->param.create_window.bg_color.b,
                    request->param.create_window.bg_color.a
                    );
            break;
        case TINYWS_REQUEST_SET_WINDOW_POS:
            printf("TINYWS_REQUEST_SET_WINDOW_POS(target=%d, pos_x=%d, pos_y=%d)\n",
                    request->target_window_id,
                    request->param.set_window_pos.pos_x,
                    request->param.set_window_pos.pos_y
                    );
            break;
        case TINYWS_REQUEST_SET_WINDOW_VISIBILITY:
            printf("TINYWS_REQUEST_SET_WINDOW_VISIBILITY(target=%d, visible=%d)\n",
                    request->target_window_id,
                    request->param.set_window_visibility.visible
                    );
            break;
        case TINYWS_REQUEST_INVALID:
            printf("TINYWS_REQUEST_INVALID\n");
            break;
    }
}


size_t request_encode(const struct Request *request, uint8_t *out, size_t size) {
    
    uint8_t *nxt = out;
    WRITE_ENUM_LE(request->type, nxt);
    WRITE_INT_LE(request->target_window_id, nxt);

    switch (request->type) {
        case TINYWS_REQUEST_DRAW_RECT:
        {
            WRITE_INT_LE(request->param.draw_rect.x, nxt);
            WRITE_INT_LE(request->param.draw_rect.y, nxt);
            WRITE_INT_LE(request->param.draw_rect.w, nxt);
            WRITE_INT_LE(request->param.draw_rect.h, nxt);
            WRITE_INT_LE(request->param.draw_rect.color.r, nxt);
            WRITE_INT_LE(request->param.draw_rect.color.g, nxt);
            WRITE_INT_LE(request->param.draw_rect.color.b, nxt);
            WRITE_INT_LE(request->param.draw_rect.color.a, nxt);
            break;
        }
        case TINYWS_REQUEST_DRAW_CIRCLE:
        {
            WRITE_INT_LE(request->param.draw_circle.x, nxt);
            WRITE_INT_LE(request->param.draw_circle.y, nxt);
            WRITE_INT_LE(request->param.draw_circle.radius, nxt);
            WRITE_INT_LE(request->param.draw_circle.filled, nxt);
            WRITE_INT_LE(request->param.draw_circle.color.r, nxt);
            WRITE_INT_LE(request->param.draw_circle.color.g, nxt);
            WRITE_INT_LE(request->param.draw_circle.color.b, nxt);
            WRITE_INT_LE(request->param.draw_circle.color.a, nxt);
            break;
        }
        case TINYWS_REQUEST_DRAW_LINE:
        {
            WRITE_INT_LE(request->param.draw_line.x1, nxt);
            WRITE_INT_LE(request->param.draw_line.y1, nxt);
            WRITE_INT_LE(request->param.draw_line.x2, nxt);
            WRITE_INT_LE(request->param.draw_line.y2, nxt);
            WRITE_INT_LE(request->param.draw_line.color.r, nxt);
            WRITE_INT_LE(request->param.draw_line.color.g, nxt);
            WRITE_INT_LE(request->param.draw_line.color.b, nxt);
            WRITE_INT_LE(request->param.draw_line.color.a, nxt);
            break;
        }
        case TINYWS_REQUEST_DRAW_PIXEL:
        {
            WRITE_INT_LE(request->param.draw_pixel.x, nxt);
            WRITE_INT_LE(request->param.draw_pixel.y, nxt);
            WRITE_INT_LE(request->param.draw_pixel.color.r, nxt);
            WRITE_INT_LE(request->param.draw_pixel.color.g, nxt);
            WRITE_INT_LE(request->param.draw_pixel.color.b, nxt);
            WRITE_INT_LE(request->param.draw_pixel.color.a, nxt);
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
            WRITE_INT_LE(request->param.create_window.parent_window_id, nxt);
            WRITE_INT_LE(request->param.create_window.width, nxt);
            WRITE_INT_LE(request->param.create_window.height, nxt);
            WRITE_INT_LE(request->param.create_window.pos_x, nxt);
            WRITE_INT_LE(request->param.create_window.pos_y, nxt);
            WRITE_INT_LE(request->param.create_window.bg_color.r, nxt);
            WRITE_INT_LE(request->param.create_window.bg_color.g, nxt);
            WRITE_INT_LE(request->param.create_window.bg_color.b, nxt);
            WRITE_INT_LE(request->param.create_window.bg_color.a, nxt);
            break;
        }
        case TINYWS_REQUEST_SET_WINDOW_POS:
        {
            WRITE_INT_LE(request->param.set_window_pos.pos_x, nxt);
            WRITE_INT_LE(request->param.set_window_pos.pos_y, nxt);
            break;
        }
        case TINYWS_REQUEST_SET_WINDOW_VISIBILITY:
        {
            WRITE_INT_LE(request->param.set_window_visibility.visible, nxt);
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
    
    READ_ENUM_LE(buf, &ret.type);
    READ_INT_LE(buf, &ret.target_window_id);

    switch (ret.type) {
        case TINYWS_REQUEST_DRAW_RECT:
        {
            ret.type = TINYWS_REQUEST_DRAW_RECT;
            READ_INT_LE(buf, &ret.param.draw_rect.x);
            READ_INT_LE(buf, &ret.param.draw_rect.y);
            READ_INT_LE(buf, &ret.param.draw_rect.w);
            READ_INT_LE(buf, &ret.param.draw_rect.h);
            READ_INT_LE(buf, &ret.param.draw_rect.color.r);
            READ_INT_LE(buf, &ret.param.draw_rect.color.g);
            READ_INT_LE(buf, &ret.param.draw_rect.color.b);
            READ_INT_LE(buf, &ret.param.draw_rect.color.a);
            break;
        }
        case TINYWS_REQUEST_DRAW_CIRCLE:
        {
            ret.type = TINYWS_REQUEST_DRAW_CIRCLE;
            READ_INT_LE(buf, &ret.param.draw_circle.x);
            READ_INT_LE(buf, &ret.param.draw_circle.y);
            READ_INT_LE(buf, &ret.param.draw_circle.radius);
            READ_INT_LE(buf, &ret.param.draw_circle.filled);
            READ_INT_LE(buf, &ret.param.draw_circle.color.r);
            READ_INT_LE(buf, &ret.param.draw_circle.color.g);
            READ_INT_LE(buf, &ret.param.draw_circle.color.b);
            READ_INT_LE(buf, &ret.param.draw_circle.color.a);
            break;
        }
        case TINYWS_REQUEST_DRAW_LINE:
        {
            ret.type = TINYWS_REQUEST_DRAW_LINE;
            READ_INT_LE(buf, &ret.param.draw_line.x1);
            READ_INT_LE(buf, &ret.param.draw_line.y1);
            READ_INT_LE(buf, &ret.param.draw_line.x2);
            READ_INT_LE(buf, &ret.param.draw_line.y2);
            READ_INT_LE(buf, &ret.param.draw_line.color.r);
            READ_INT_LE(buf, &ret.param.draw_line.color.g);
            READ_INT_LE(buf, &ret.param.draw_line.color.b);
            READ_INT_LE(buf, &ret.param.draw_line.color.a);
            break;
        }
        case TINYWS_REQUEST_DRAW_PIXEL:
        {
            ret.type = TINYWS_REQUEST_DRAW_PIXEL;
            READ_INT_LE(buf, &ret.param.draw_pixel.x);
            READ_INT_LE(buf, &ret.param.draw_pixel.y);
            READ_INT_LE(buf, &ret.param.draw_pixel.color.r);
            READ_INT_LE(buf, &ret.param.draw_pixel.color.g);
            READ_INT_LE(buf, &ret.param.draw_pixel.color.b);
            READ_INT_LE(buf, &ret.param.draw_pixel.color.a);
            break;
        }
        case TINYWS_REQUEST_CLEAR_WINDOW:
        {
            ret.type = TINYWS_REQUEST_CLEAR_WINDOW;
            break;
        }
        case TINYWS_REQUEST_REFRESH:
        {
            ret.type = TINYWS_REQUEST_REFRESH;
            break;
        }

        // window management
        case TINYWS_REQUEST_CREATE_WINDOW:
        {
            ret.type = TINYWS_REQUEST_CREATE_WINDOW;
            READ_INT_LE(buf, &ret.param.create_window.parent_window_id);
            READ_INT_LE(buf, &ret.param.create_window.width);
            READ_INT_LE(buf, &ret.param.create_window.height);
            READ_INT_LE(buf, &ret.param.create_window.pos_x);
            READ_INT_LE(buf, &ret.param.create_window.pos_y);
            READ_INT_LE(buf, &ret.param.create_window.bg_color.r);
            READ_INT_LE(buf, &ret.param.create_window.bg_color.g);
            READ_INT_LE(buf, &ret.param.create_window.bg_color.b);
            READ_INT_LE(buf, &ret.param.create_window.bg_color.a);
            break;
        }
        case TINYWS_REQUEST_SET_WINDOW_POS:
        {
            ret.type = TINYWS_REQUEST_SET_WINDOW_POS;
            READ_INT_LE(buf, &ret.param.set_window_pos.pos_x);
            READ_INT_LE(buf, &ret.param.set_window_pos.pos_y);
            break;
        }
        case TINYWS_REQUEST_SET_WINDOW_VISIBILITY:
        {
            ret.type = TINYWS_REQUEST_SET_WINDOW_VISIBILITY;
            READ_INT_LE(buf, &ret.param.set_window_visibility.visible);
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
