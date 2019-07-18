#include "basic_structures.h"

Size size_new(int width, int height) {
    Size ret;
    ret.width = width;
    ret.height = height;
    return ret;
}

Point point_new(int x, int y) {
    Point ret;
    ret.x = x;
    ret.y = y;
    return ret;
}

Color color_new(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    Color ret;
    ret.r = r;
    ret.g = g;
    ret.b = b;
    ret.a = a;
    return ret;
}