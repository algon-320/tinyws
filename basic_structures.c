#include "basic_structures.h"
#include "common.h"

Size size_new(int width, int height) {
    Size ret;
    ret.width = width;
    ret.height = height;
    return ret;
}

size_t size_encode(const Size *size, uint8_t **out) {
    uint8_t *nxt = *out;
    WRITE_INT_LE(size->width, &nxt);
    WRITE_INT_LE(size->height, &nxt);
    *out = nxt;
    return nxt - *out;
}

void size_decode(const uint8_t **in, Size *size) {
    READ_INT_LE(in, &size->width);
    READ_INT_LE(in, &size->height);
}


Point point_new(int x, int y) {
    Point ret;
    ret.x = x;
    ret.y = y;
    return ret;
}

size_t point_encode(const Point *point, uint8_t **out) {
    uint8_t *nxt = *out;
    WRITE_INT_LE(point->x, &nxt);
    WRITE_INT_LE(point->y, &nxt);
    *out = nxt;
    return nxt - *out;
}

void point_decode(const uint8_t **in, Point *point) {
    READ_INT_LE(in, &point->x);
    READ_INT_LE(in, &point->y);
}


Rect rect_new(int x, int y, int width, int height) {
    Rect ret;
    ret.x = x;
    ret.y = y;
    ret.width = width;
    ret.height = height;
    return ret;
}

size_t rect_encode(const Rect *rect, uint8_t **out) {
    uint8_t *nxt = *out;
    WRITE_INT_LE(rect->x, &nxt);
    WRITE_INT_LE(rect->y, &nxt);
    WRITE_INT_LE(rect->width, &nxt);
    WRITE_INT_LE(rect->height, &nxt);
    *out = nxt;
    return nxt - *out;
}

void rect_decode(const uint8_t **in, Rect *rect) {
    READ_INT_LE(in, &rect->x);
    READ_INT_LE(in, &rect->y);
    READ_INT_LE(in, &rect->width);
    READ_INT_LE(in, &rect->height);
}


Color color_new(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    Color ret;
    ret.r = r;
    ret.g = g;
    ret.b = b;
    ret.a = a;
    return ret;
}

size_t color_encode(const Color *color, uint8_t **out) {
    uint8_t *nxt = *out;
    WRITE_INT_LE(color->r, &nxt);
    WRITE_INT_LE(color->g, &nxt);
    WRITE_INT_LE(color->b, &nxt);
    WRITE_INT_LE(color->a, &nxt);
    *out = nxt;
    return nxt - *out;
}

void color_decode(const uint8_t **in, Color *color) {
    READ_INT_LE(in, &color->r);
    READ_INT_LE(in, &color->g);
    READ_INT_LE(in, &color->b);
    READ_INT_LE(in, &color->a);
}
