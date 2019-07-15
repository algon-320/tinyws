#include <stdio.h>
#include <assert.h>

#include "query.h"

void print_drawing_query(struct DrawingQuery query) {
    switch (query.op) {
        case DrawRect:
            printf("DrawRect(x=%d, y=%d, w=%d, h=%d)\n",
                query.param.draw_rect_param.x,
                query.param.draw_rect_param.y,
                query.param.draw_rect_param.w,
                query.param.draw_rect_param.h);
            break;
        case DrawCircle:
            printf("DrawCircle(x=%d, y=%d, radius=%d, filled=%d)\n",
                query.param.draw_circle_param.x,
                query.param.draw_circle_param.y,
                query.param.draw_circle_param.radius,
                (int)query.param.draw_circle_param.filled);
            break;
        case DrawLine:
            printf("DrawLine(x1=%d, y1=%d, x2=%d, y2=%d)\n",
                query.param.draw_line_param.x1,
                query.param.draw_line_param.y1,
                query.param.draw_line_param.x2,
                query.param.draw_line_param.y2);
            break;  
        case DrawPixel:
            printf("DrawRect(x=%d, y=%d)\n",
                query.param.draw_rect_param.x,
                query.param.draw_rect_param.y);
            break;
        case ClearScreen:
            printf("ClearScreen\n");
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
ENCODE_INT_LITTLE_FUNC(encode_int32_little, int)
ENCODE_INT_LITTLE_FUNC(encode_int8_little, char)

int encode_query(struct DrawingQuery query, unsigned char *out, size_t size) {
    assert(size > 0);
    out[0] = (unsigned char)query.op;
    switch (query.op) {
        case DrawRect:
        {
            if (size < 17) {
                return -1;
            }

            int param[4] = {
                query.param.draw_rect_param.x,
                query.param.draw_rect_param.y,
                query.param.draw_rect_param.w,
                query.param.draw_rect_param.h
            };
            for (int i = 0; i < 4; i++) {
                encode_int32_little(param[i], out + 1 + i * 4);
            }
            break;
        }
        case DrawCircle:
        {
            if (size < 14) {
                return -1;
            }

            int param[3] = {
                query.param.draw_circle_param.x,
                query.param.draw_circle_param.y,
                query.param.draw_circle_param.radius
            };
            for (int i = 0; i < 3; i++) {
                encode_int32_little(param[i], out + 1 + i * 4);
            }
            fprintf(stderr, "filled = %d\n", query.param.draw_circle_param.filled);
            encode_int8_little(query.param.draw_circle_param.filled, out + 13);
            break;
        }
        case DrawLine:
        {
            if (size < 17) {
                return -1;
            }

            int param[4] = {
                query.param.draw_line_param.x1,
                query.param.draw_line_param.y1,
                query.param.draw_line_param.x2,
                query.param.draw_line_param.y2
            };
            for (int i = 0; i < 4; i++) {
                encode_int32_little(param[i], out + 1 + i * 4);
            }
            break;
        }
        case DrawPixel:
        {
            if (size < 9) {
                return -1;
            }

            int param[2] = {
                query.param.draw_pixel_param.x,
                query.param.draw_pixel_param.y,
            };
            for (int i = 0; i < 2; i++) {
                encode_int32_little(param[i], out + 1 + i * 4);
            }
            break;
        }
        case ClearScreen:
        {
            // no body
            break;
        }
        default:
        {
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
DECODE_INT_LITTLE_FUNC(decode_int32_little, int)
DECODE_INT_LITTLE_FUNC(decode_int8_little, char)

struct DrawingQuery decode_query(const unsigned char *buf, size_t size) {
    struct DrawingQuery ret;
    switch (buf[0]) {
        case DrawRect:
        {
            if (size < 17) {
                ret.op = Invalid;
                break;
            }

            int x, y, w, h;
            x = decode_int32_little(buf + 1);
            y = decode_int32_little(buf + 5);
            w = decode_int32_little(buf + 9);
            h = decode_int32_little(buf + 13);

            ret.op = DrawRect;
            ret.param.draw_rect_param.x = x;
            ret.param.draw_rect_param.y = y;
            ret.param.draw_rect_param.w = w;
            ret.param.draw_rect_param.h = h;
            break;
        }
        case DrawCircle:
        {
            if (size < 14) {
                ret.op = Invalid;
                break;
            }

            int x, y, radius;
            char filled;
            x = decode_int32_little(buf + 1);
            y = decode_int32_little(buf + 5);
            radius = decode_int32_little(buf + 9);
            filled = decode_int8_little(buf + 13);

            ret.op = DrawCircle;
            ret.param.draw_circle_param.x = x;
            ret.param.draw_circle_param.y = y;
            ret.param.draw_circle_param.radius = radius;
            ret.param.draw_circle_param.filled = filled;
            break;
        }
        case DrawLine:
        {
            if (size < 17) {
                ret.op = Invalid;
                break;
            }

            int x1, y1, x2, y2;
            x1 = decode_int32_little(buf + 1);
            y1 = decode_int32_little(buf + 5);
            x2 = decode_int32_little(buf + 9);
            y2 = decode_int32_little(buf + 13);

            ret.op = DrawLine;
            ret.param.draw_line_param.x1 = x1;
            ret.param.draw_line_param.y1 = y1;
            ret.param.draw_line_param.x2 = x2;
            ret.param.draw_line_param.y2 = y2;
            break;
        }
        case DrawPixel:
        {
            if (size < 9) {
                ret.op = Invalid;
                break;
            }

            int x, y;
            x = decode_int32_little(buf + 1);
            y = decode_int32_little(buf + 5);

            ret.op = DrawPixel;
            ret.param.draw_pixel_param.x = x;
            ret.param.draw_pixel_param.y = y;
            break;
        }
        case ClearScreen:
        {
            ret.op = ClearScreen;
            break;
        }
    }
    return ret;
}
