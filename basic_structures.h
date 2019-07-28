#ifndef INCLUDE_GUARD_BASIC_STRUCTURES_H
#define INCLUDE_GUARD_BASIC_STRUCTURES_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    int32_t width;
    int32_t height;
} Size;

typedef struct {
    int32_t x;
    int32_t y;
} Point;

typedef struct {
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
} Rect;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} Color;


Size size_new(int width, int height);
size_t size_encode(const Size *size, uint8_t **out);
void size_decode(const uint8_t **in, Size *size);

Point point_new(int x, int y);
size_t point_encode(const Point *point, uint8_t **out);
void point_decode(const uint8_t **in, Point *point);

Rect rect_new(int x, int y, int width, int height);
size_t rect_encode(const Rect *rect, uint8_t **out);
void rect_decode(const uint8_t **in, Rect *rect);

Color color_new(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
size_t color_encode(const Color *color, uint8_t **out);
void color_decode(const uint8_t **in, Color *color);

#endif