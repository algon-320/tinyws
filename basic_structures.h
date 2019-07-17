#ifndef INCLUDE_GUARD_BASIC_STRUCTURES_H
#define INCLUDE_GUARD_BASIC_STRUCTURES_H

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
    int r;
    int g;
    int b;
} Color;

Color color_new(int r, int g, int b);

#endif