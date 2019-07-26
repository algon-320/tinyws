#ifndef INCLUDE_GUARD_WINDOW_H
#define INCLUDE_GUARD_WINDOW_H

#include <SDL2/SDL.h>

#include "lib/deque.h"
#include "basic_structures.h"
#include "display.h"

struct Window {
    struct Window *parent;
    uint32_t id;
    Point pos;
    Size size;
    uint8_t visible;
    char title[256];
    Color background_color;
    Deque children;  // <struct Window *>
    SDL_Texture *buffer;
    struct Display *disp;
    struct Window *z_ord_prev;
    struct Window *z_ord_next;
};

void window_subsystem_init();

struct Window *window_new(struct Window *parent, struct Display *disp, Point pos, Size size, const char *title, Color bg_color);

int window_release(struct Window *win);

int window_draw(struct Window *win, struct Display *disp);

struct Window *window_get_by_id(uint32_t win_id);

#endif