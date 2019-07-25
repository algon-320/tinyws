#ifndef INCLUDE_GUARD_DRAW_H
#define INCLUDE_GUARD_DRAW_H

#include <SDL2/SDL.h>

#include "basic_structures.h"
#include "window.h"

int draw_rect(struct Window *win, int x, int y, int w, int h, Color color);

int draw_line(struct Window *win, int x1, int y1, int x2, int y2, Color color);

int draw_pixel(struct Window *win, int x, int y, Color color);

int draw_circle(struct Window *win, int x_center, int y_center, int radius, uint8_t filled, Color color);

int clear_screen(struct Window *win);

#endif