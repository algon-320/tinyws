#include <stdio.h>
#include <assert.h>

#include "query.h"

void print_query(const struct Query *query) {
    switch (query->type) {
        case TINYWS_QUERY_DRAW_RECT:
            printf("TINYWS_QUERY_DRAW_RECT(x=%d, y=%d, w=%d, h=%d)\n",
                    query->param.draw_rect.x,
                    query->param.draw_rect.y,
                    query->param.draw_rect.w,
                    query->param.draw_rect.h
                    );
            break;
        case TINYWS_QUERY_DRAW_CIRCLE:
            printf("TINYWS_QUERY_DRAW_CIRCLE(x=%d, y=%d, radius=%d, filled=%d)\n",
                    query->param.draw_circle.x,
                    query->param.draw_circle.y,
                    query->param.draw_circle.radius,
                    query->param.draw_circle.filled
                    );
            break;
        case TINYWS_QUERY_DRAW_LINE:
            printf("TINYWS_QUERY_DRAW_LINE(x1=%d, y1=%d, x2=%d, y2=%d)\n",
                    query->param.draw_line.x1,
                    query->param.draw_line.y1,
                    query->param.draw_line.x2,
                    query->param.draw_line.y2
                    );
            break;  
        case TINYWS_QUERY_DRAW_PIXEL:
            printf("TINYWS_QUERY_DRAW_PIXEL(x=%d, y=%d)\n",
                    query->param.draw_rect.x,
                    query->param.draw_rect.y
                    );
            break;
        case TINYWS_QUERY_CLEAR_SCREEN:
            printf("TINYWS_QUERY_CLEAR_SCREEN\n");
            break;
        case TINYWS_QUERY_REFRESH:
            printf("TINYWS_QUERY_REFRESH\n");
            break;
        case TINYWS_QUERY_CREATE_WINDOW:
            printf("TINYWS_QUERY_CREATE_WINDOW(pwid=%d, width=%d, height=%d, pos_x=%d, pos_y=%d)\n",
                    query->param.create_window.parent_window_id,
                    query->param.create_window.width,
                    query->param.create_window.height,
                    query->param.create_window.pos_x,
                    query->param.create_window.pos_y
                    );
            break;
        case TINYWS_QUERY_SET_WINDOW_POS:
            printf("TINYWS_QUERY_SET_WINDOW_POS(wid=%d, pos_x=%d, pos_y=%d)\n",
                    query->param.set_window_pos.window_id,
                    query->param.set_window_pos.pos_x,
                    query->param.set_window_pos.pos_y
                    );
            break;
        case TINYWS_QUERY_SET_WINDOW_VISIBILITY:
            printf("TINYWS_QUERY_SET_WINDOW_VISIBILITY(wid=%d, visible=%d)\n",
                    query->param.set_window_visibility.window_id,
                    query->param.set_window_visibility.visible
                    );
            break;
        case TINYWS_QUERY_INVALID:
            printf("TINYWS_QUERY_INVALID\n");
            break;
    }
}


#define WRITE_INT_LE(value, dst)\
    do {\
        for (size_t i = 0; i < sizeof(value); i++) {\
            dst[i] = (value >> (i * 8)) & ((1 << 8) - 1);\
        }\
        dst += sizeof(value);\
    } while (0)

size_t encode_query(const struct Query *query, uint8_t *out, size_t size) {
    
    uint8_t *nxt = out;
    WRITE_INT_LE((int32_t)query->type, nxt);

    switch (query->type) {
        case TINYWS_QUERY_DRAW_RECT:
        {
            WRITE_INT_LE(query->param.draw_rect.x, nxt);
            WRITE_INT_LE(query->param.draw_rect.y, nxt);
            WRITE_INT_LE(query->param.draw_rect.w, nxt);
            WRITE_INT_LE(query->param.draw_rect.h, nxt);
            break;
        }
        case TINYWS_QUERY_DRAW_CIRCLE:
        {
            WRITE_INT_LE(query->param.draw_circle.x, nxt);
            WRITE_INT_LE(query->param.draw_circle.y, nxt);
            WRITE_INT_LE(query->param.draw_circle.radius, nxt);
            WRITE_INT_LE(query->param.draw_circle.filled, nxt);
            break;
        }
        case TINYWS_QUERY_DRAW_LINE:
        {
            WRITE_INT_LE(query->param.draw_line.x1, nxt);
            WRITE_INT_LE(query->param.draw_line.y1, nxt);
            WRITE_INT_LE(query->param.draw_line.x2, nxt);
            WRITE_INT_LE(query->param.draw_line.y2, nxt);
            break;
        }
        case TINYWS_QUERY_DRAW_PIXEL:
        {
            WRITE_INT_LE(query->param.draw_pixel.x, nxt);
            WRITE_INT_LE(query->param.draw_pixel.y, nxt);
            break;
        }
        case TINYWS_QUERY_CLEAR_SCREEN:
        {
            break;
        }
        case TINYWS_QUERY_REFRESH:
        {
            break;
        }

        // Window management
        case TINYWS_QUERY_CREATE_WINDOW:
        {
            WRITE_INT_LE(query->param.create_window.parent_window_id, nxt);
            WRITE_INT_LE(query->param.create_window.width, nxt);
            WRITE_INT_LE(query->param.create_window.height, nxt);
            WRITE_INT_LE(query->param.create_window.pos_x, nxt);
            WRITE_INT_LE(query->param.create_window.pos_y, nxt);
            break;
        }
        case TINYWS_QUERY_SET_WINDOW_POS:
        {
            WRITE_INT_LE(query->param.set_window_pos.window_id, nxt);
            WRITE_INT_LE(query->param.set_window_pos.pos_x, nxt);
            WRITE_INT_LE(query->param.set_window_pos.pos_y, nxt);
            break;
        }
        case TINYWS_QUERY_SET_WINDOW_VISIBILITY:
        {
            WRITE_INT_LE(query->param.set_window_visibility.window_id, nxt);
            WRITE_INT_LE(query->param.set_window_visibility.visible, nxt);
            break;
        }

        default:
        {
            fprintf(stderr, "invalid query type\n");
            assert(0);
            break;
        }
    }
    return nxt - out;
}



#define READ_INT_LE(src, dst)\
    do {\
        *dst = 0;\
        for (size_t i = 0; i < sizeof(*dst); i++) {\
            *dst |= src[i] << (i * 8);\
        }\
        src += sizeof(*dst);\
    } while (0)

struct Query decode_query(const uint8_t *buf, size_t size) {
    struct Query ret;
    
    {
        int32_t tmp_query_type;
        READ_INT_LE(buf, &tmp_query_type);
        ret.type = tmp_query_type;
    }

    switch (ret.type) {
        case TINYWS_QUERY_DRAW_RECT:
        {
            ret.type = TINYWS_QUERY_DRAW_RECT;
            READ_INT_LE(buf, &ret.param.draw_rect.x);
            READ_INT_LE(buf, &ret.param.draw_rect.y);
            READ_INT_LE(buf, &ret.param.draw_rect.w);
            READ_INT_LE(buf, &ret.param.draw_rect.h);
            break;
        }
        case TINYWS_QUERY_DRAW_CIRCLE:
        {
            ret.type = TINYWS_QUERY_DRAW_CIRCLE;
            READ_INT_LE(buf, &ret.param.draw_circle.x);
            READ_INT_LE(buf, &ret.param.draw_circle.y);
            READ_INT_LE(buf, &ret.param.draw_circle.radius);
            READ_INT_LE(buf, &ret.param.draw_circle.filled);
            break;
        }
        case TINYWS_QUERY_DRAW_LINE:
        {
            ret.type = TINYWS_QUERY_DRAW_LINE;
            READ_INT_LE(buf, &ret.param.draw_line.x1);
            READ_INT_LE(buf, &ret.param.draw_line.y1);
            READ_INT_LE(buf, &ret.param.draw_line.x2);
            READ_INT_LE(buf, &ret.param.draw_line.y2);
            break;
        }
        case TINYWS_QUERY_DRAW_PIXEL:
        {
            ret.type = TINYWS_QUERY_DRAW_PIXEL;
            READ_INT_LE(buf, &ret.param.draw_pixel.x);
            READ_INT_LE(buf, &ret.param.draw_pixel.y);
            break;
        }
        case TINYWS_QUERY_CLEAR_SCREEN:
        {
            ret.type = TINYWS_QUERY_CLEAR_SCREEN;
            break;
        }
        case TINYWS_QUERY_REFRESH:
        {
            ret.type = TINYWS_QUERY_REFRESH;
            break;
        }

        // window management
        case TINYWS_QUERY_CREATE_WINDOW:
        {
            ret.type = TINYWS_QUERY_CREATE_WINDOW;
            READ_INT_LE(buf, &ret.param.create_window.parent_window_id);
            READ_INT_LE(buf, &ret.param.create_window.width);
            READ_INT_LE(buf, &ret.param.create_window.height);
            READ_INT_LE(buf, &ret.param.create_window.pos_x);
            READ_INT_LE(buf, &ret.param.create_window.pos_y);
            break;
        }
        case TINYWS_QUERY_SET_WINDOW_POS:
        {
            ret.type = TINYWS_QUERY_SET_WINDOW_POS;
            READ_INT_LE(buf, &ret.param.set_window_pos.window_id);
            READ_INT_LE(buf, &ret.param.set_window_pos.pos_x);
            READ_INT_LE(buf, &ret.param.set_window_pos.pos_y);
            break;
        }
        case TINYWS_QUERY_SET_WINDOW_VISIBILITY:
        {
            ret.type = TINYWS_QUERY_SET_WINDOW_VISIBILITY;
            READ_INT_LE(buf, &ret.param.set_window_visibility.window_id);
            READ_INT_LE(buf, &ret.param.set_window_visibility.visible);
            break;
        }
        default:
        {
            ret.type = TINYWS_QUERY_INVALID;
            break;
        }
    }
    return ret;
}
