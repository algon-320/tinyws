#ifndef INCLUDE_GUARD_WINDOW_H
#define INCLUDE_GUARD_WINDOW_H

#include <SDL2/SDL.h>

#include "basic_structures.h"
#include "display.h"

struct Window {
    int id;
    Point pos;
    Size size;
    char tilte[256];
    size_t children_size;
    Color background_color;
    struct Window **children;
    SDL_Texture *buffer;
    SDL_Renderer *ren;
};

int window_new(struct Window *win, struct Display *disp, Point pos, Size size, const char *title, Color bg_color);

int window_release(struct Window *win);

int window_draw(struct Window *win, struct Display *disp);

#endif