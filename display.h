#ifndef INCLUDE_GUARD_DISPLAY_H
#define INCLUDE_GUARD_DISPLAY_H

#include <SDL2/SDL.h>

#include "basic_structures.h"

struct Display {
    SDL_Window *win;
    SDL_Renderer *ren;
    Point curosr_pos;
    Size size;
};

int display_new(struct Display *disp, Size window_size, const char *title);
void display_release(struct Display *disp);
int display_flush(struct Display *disp);

#endif