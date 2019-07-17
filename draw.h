#ifndef INCLUDE_GUARD_DRWA_H
#define INCLUDE_GUARD_DRWA_H

#include <SDL2/SDL.h>

#include "basic_structures.h"
#include "display.h"

int draw_rect(struct Display *disp, int x, int y, int w, int h, Color color);

int draw_line(struct Display *disp, int x1, int y1, int x2, int y2, Color color);

int draw_pixel(struct Display *disp, int x, int y, Color color);

int draw_circle(struct Display *disp, int x_center, int y_center, int radius, char filled, Color color);

int clear_screen(struct Display *disp, Color color);

#endif