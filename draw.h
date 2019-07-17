#ifndef INCLUDE_GUARD_DRWA_H
#define INCLUDE_GUARD_DRWA_H

#include <SDL2/SDL.h>

#include "display.h"

int draw_rect(struct Display *disp, int x, int y, int w, int h, SDL_Color color);

int draw_line(struct Display *disp, int x1, int y1, int x2, int y2, SDL_Color color);

int draw_pixel(struct Display *disp, int x, int y, SDL_Color color);

int draw_circle(struct Display *disp, int x_center, int y_center, int radius, char filled, SDL_Color color);

int clear_screen(struct Display *disp, SDL_Color color);

#endif