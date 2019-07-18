#ifndef INCLUDE_GUARD_BASIC_STRUCTURES_H
#define INCLUDE_GUARD_BASIC_STRUCTURES_H

#include <stdint.h>

typedef struct {
    int width;
    int height;
} Size;

Size size_new(int width, int height);

typedef struct {
    int x;
    int y;
} Point;

Point point_new(int width, int height);


typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} Color;

Color color_new(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

#endif