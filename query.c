#include <stdio.h>
#include <assert.h>

#include "query.h"

void print_query(struct Query query) {
    switch (query.type) {
        case TINYWS_QUERY_DRAW_RECT:
            printf("TINYWS_QUERY_DRAW_RECT(x=%d, y=%d, w=%d, h=%d)\n",
                    query.param.draw_rect.x,
                    query.param.draw_rect.y,
                    query.param.draw_rect.w,
                    query.param.draw_rect.h
                    );
            break;
        case TINYWS_QUERY_DRAW_CIRCLE:
            printf("TINYWS_QUERY_DRAW_CIRCLE(x=%d, y=%d, radius=%d, filled=%d)\n",
                    query.param.draw_circle.x,
                    query.param.draw_circle.y,
                    query.param.draw_circle.radius,
                    query.param.draw_circle.filled
                    );
            break;
        case TINYWS_QUERY_DRAW_LINE:
            printf("TINYWS_QUERY_DRAW_LINE(x1=%d, y1=%d, x2=%d, y2=%d)\n",
                    query.param.draw_line.x1,
                    query.param.draw_line.y1,
                    query.param.draw_line.x2,
                    query.param.draw_line.y2
                    );
            break;  
        case TINYWS_QUERY_DRAW_PIXEL:
            printf("TINYWS_QUERY_DRAW_PIXEL(x=%d, y=%d)\n",
                    query.param.draw_rect.x,
                    query.param.draw_rect.y
                    );
            break;
        case TINYWS_QUERY_CLEAR_SCREEN:
            printf("TINYWS_QUERY_CLEAR_SCREEN\n");
            break;
        case TINYWS_QUERY_CREATE_WINDOW:
            printf("TINYWS_QUERY_CREATE_WINDOW(pwid=%d, width=%d, height=%d, pos_x=%d, pos_y=%d)\n",
                    query.param.create_window.parent_window_id,
                    query.param.create_window.width,
                    query.param.create_window.height,
                    query.param.create_window.pos_x,
                    query.param.create_window.pos_y
                    );
            break;
        case TINYWS_QUERY_SET_WINDOW_POS:
            printf("TINYWS_QUERY_SET_WINDOW_POS(wid=%d, pos_x=%d, pos_y=%d)\n",
                    query.param.set_window_pos.window_id,
                    query.param.set_window_pos.pos_x,
                    query.param.set_window_pos.pos_y
                    );
            break;
        case TINYWS_QUERY_SET_WINDOW_VISIBILITY:
            printf("TINYWS_QUERY_SET_WINDOW_VISIBILITY(wid=%d, visible=%d)\n",
                    query.param.set_window_visibility.window_id,
                    query.param.set_window_visibility.visible
                    );
            break;
        default:
            printf("invlid operator\n");
            break;
    }
}



#define ENCODE_INT_LITTLE_FUNC(func_name, type)\
    void func_name(type value, unsigned char *out) {\
        for (size_t i = 0; i < sizeof(type); i++) {\
            out[i] = (unsigned char)(value & ((1 << 8) - 1));\
            value >>= 8;\
        }\
    }
ENCODE_INT_LITTLE_FUNC(encode_int32_little, int32_t)
ENCODE_INT_LITTLE_FUNC(encode_uint8_little, unsigned char)

int encode_query(struct Query query, unsigned char *out, size_t size) {
    assert(size > 0);
    out[0] = (unsigned char)query.type;
    switch (query.type) {
        // TODO check out buffer size

        case TINYWS_QUERY_DRAW_RECT:
        {
            int32_t param[4] = {
                query.param.draw_rect.x,
                query.param.draw_rect.y,
                query.param.draw_rect.w,
                query.param.draw_rect.h
            };
            for (int i = 0; i < 4; i++) {
                encode_int32_little(param[i], out + 1 + i * sizeof(int32_t));
            }
            break;
        }
        case TINYWS_QUERY_DRAW_CIRCLE:
        {
            int32_t param[3] = {
                query.param.draw_circle.x,
                query.param.draw_circle.y,
                query.param.draw_circle.radius
            };
            for (int i = 0; i < 3; i++) {
                encode_int32_little(param[i], out + 1 + i * sizeof(int32_t));
            }
            encode_uint8_little(query.param.draw_circle.filled, out + 1 + 3 * sizeof(int32_t));
            break;
        }
        case TINYWS_QUERY_DRAW_LINE:
        {
            int32_t param[4] = {
                query.param.draw_line.x1,
                query.param.draw_line.y1,
                query.param.draw_line.x2,
                query.param.draw_line.y2
            };
            for (int i = 0; i < 4; i++) {
                encode_int32_little(param[i], out + 1 + i * sizeof(int32_t));
            }
            break;
        }
        case TINYWS_QUERY_DRAW_PIXEL:
        {
            int32_t param[2] = {
                query.param.draw_pixel.x,
                query.param.draw_pixel.y,
            };
            for (int i = 0; i < 2; i++) {
                encode_int32_little(param[i], out + 1 + i * sizeof(int32_t));
            }
            break;
        }
        case TINYWS_QUERY_CLEAR_SCREEN:
        {
            break;
        }

        // Window management
        case TINYWS_QUERY_CREATE_WINDOW:
        {
            int32_t param[5] = {
                query.param.create_window.parent_window_id,
                query.param.create_window.width,
                query.param.create_window.height,
                query.param.create_window.pos_x,
                query.param.create_window.pos_y,
            };
            for (int i = 0; i < 5; i++) {
                encode_int32_little(param[i], out + 1 + i * sizeof(int32_t));
            }
            break;
        }
        case TINYWS_QUERY_SET_WINDOW_POS:
        {
            int32_t param[3] = {
                query.param.set_window_pos.window_id,
                query.param.set_window_pos.pos_x,
                query.param.set_window_pos.pos_y,
            };
            for (int i = 0; i < 3; i++) {
                encode_int32_little(param[i], out + 1 + i * sizeof(int32_t));
            }
            break;
        }
        case TINYWS_QUERY_SET_WINDOW_VISIBILITY:
        {
            encode_int32_little(query.param.set_window_visibility.window_id,
                    out + 1 + 0 * sizeof(int32_t));
            encode_uint8_little(query.param.set_window_visibility.visible,
                    out + 1 + 1 * sizeof(int32_t));
            break;
        }

        default:
        {
            fprintf(stderr, "invalid query type\n");
            assert(0);
            break;
        }
    }
    return 0;
}



#define DECODE_INT_LITTLE_FUNC(func_name, type)\
    type func_name(const unsigned char *buf) {\
        type ret = 0;\
        for (size_t i = 0; i < sizeof(type); i++) {\
            int x = buf[i];\
            ret |= (x << (i * 8));\
        }\
        return ret;\
    }
DECODE_INT_LITTLE_FUNC(decode_int32_little, int32_t)
DECODE_INT_LITTLE_FUNC(decode_uint8_little, unsigned char)

struct Query decode_query(const unsigned char *buf, size_t size) {
    struct Query ret;
    switch (buf[0]) {
        // TODO check buf size
        case TINYWS_QUERY_DRAW_RECT:
        {
            int x, y, w, h;
            x = decode_int32_little(buf + 1 + sizeof(int32_t) * 0);
            y = decode_int32_little(buf + 1 + sizeof(int32_t) * 1);
            w = decode_int32_little(buf + 1 + sizeof(int32_t) * 2);
            h = decode_int32_little(buf + 1 + sizeof(int32_t) * 3);

            ret.type = TINYWS_QUERY_DRAW_RECT;
            ret.param.draw_rect.x = x;
            ret.param.draw_rect.y = y;
            ret.param.draw_rect.w = w;
            ret.param.draw_rect.h = h;
            break;
        }
        case TINYWS_QUERY_DRAW_CIRCLE:
        {
            int x, y, r;
            char f;
            x = decode_int32_little(buf + 1 + sizeof(int32_t) * 0);
            y = decode_int32_little(buf + 1 + sizeof(int32_t) * 1);
            r = decode_int32_little(buf + 1 + sizeof(int32_t) * 2);
            f = decode_uint8_little(buf + 1 + sizeof(int32_t) * 3);

            ret.type = TINYWS_QUERY_DRAW_CIRCLE;
            ret.param.draw_circle.x = x;
            ret.param.draw_circle.y = y;
            ret.param.draw_circle.radius = r;
            ret.param.draw_circle.filled = f;
            break;
        }
        case TINYWS_QUERY_DRAW_LINE:
        {
            int x1, y1, x2, y2;
            x1 = decode_int32_little(buf + 1 + sizeof(int32_t) * 0);
            y1 = decode_int32_little(buf + 1 + sizeof(int32_t) * 1);
            x2 = decode_int32_little(buf + 1 + sizeof(int32_t) * 2);
            y2 = decode_int32_little(buf + 1 + sizeof(int32_t) * 3);

            ret.type = TINYWS_QUERY_DRAW_LINE;
            ret.param.draw_line.x1 = x1;
            ret.param.draw_line.y1 = y1;
            ret.param.draw_line.x2 = x2;
            ret.param.draw_line.y2 = y2;
            break;
        }
        case TINYWS_QUERY_DRAW_PIXEL:
        {
            int x, y;
            x = decode_int32_little(buf + 1 + sizeof(int32_t) * 0);
            y = decode_int32_little(buf + 1 + sizeof(int32_t) * 1);

            ret.type = TINYWS_QUERY_DRAW_PIXEL;
            ret.param.draw_pixel.x = x;
            ret.param.draw_pixel.y = y;
            break;
        }
        case TINYWS_QUERY_CLEAR_SCREEN:
        {
            ret.type = TINYWS_QUERY_CLEAR_SCREEN;
            break;
        }

        // window management
        case TINYWS_QUERY_CREATE_WINDOW:
        {
            ret.type = TINYWS_QUERY_CREATE_WINDOW;
            ret.param.create_window.parent_window_id
                = decode_int32_little(buf + 1 + sizeof(int32_t) * 0);
            ret.param.create_window.width
                = decode_int32_little(buf + 1 + sizeof(int32_t) * 1);
            ret.param.create_window.height
                = decode_int32_little(buf + 1 + sizeof(int32_t) * 2);
            ret.param.create_window.pos_x
                = decode_int32_little(buf + 1 + sizeof(int32_t) * 3);
            ret.param.create_window.pos_y
                = decode_int32_little(buf + 1 + sizeof(int32_t) * 4);
            break;
        }
        case TINYWS_QUERY_SET_WINDOW_POS:
        {
            ret.type = TINYWS_QUERY_SET_WINDOW_POS;
            ret.param.set_window_pos.window_id
                = decode_int32_little(buf + 1 + sizeof(int32_t) * 0);
            ret.param.set_window_pos.pos_x
                = decode_int32_little(buf + 1 + sizeof(int32_t) * 1);
            ret.param.set_window_pos.pos_y
                = decode_int32_little(buf + 1 + sizeof(int32_t) * 2);
            break;
        }
        case TINYWS_QUERY_SET_WINDOW_VISIBILITY:
        {
            ret.type = TINYWS_QUERY_SET_WINDOW_VISIBILITY;
            ret.param.set_window_visibility.window_id
                = decode_int32_little(buf + 1 + sizeof(int32_t) * 0);
            ret.param.set_window_visibility.visible
                = decode_uint8_little(buf + 1 + sizeof(int32_t) * 1);
            break;
        }
    }
    return ret;
}
