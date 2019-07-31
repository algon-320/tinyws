#ifndef INCLUDE_GUARD_WINDOW_H
#define INCLUDE_GUARD_WINDOW_H

#include <SDL2/SDL.h>

#include "lib/queue.h"
#include "lib/embedded_linked_list.h"
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
    Queue events;  // <struct Event>
    LinkedList child;  // <struct Window *>
    LinkedList next;   // <struct Window *>
    SDL_Texture *buffer;
    struct Display *disp;
};

void window_subsystem_init();

struct Window *window_get_focused();
void window_set_focus(uint32_t window_id);

struct Window *window_new(struct Window *parent, struct Display *disp, Rect rect, const char *title, Color bg_color);

int window_release(struct Window *win);

int window_draw(struct Window *win, struct Display *disp);

struct Window *window_get_by_id(uint32_t win_id);

void window_move_top(struct Window *win);

void window_reparent(struct Window *win, struct Window *new_parent);

#endif