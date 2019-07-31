#ifndef INCLUDE_GUARD_WINDOW_H
#define INCLUDE_GUARD_WINDOW_H

#include <SDL.h>

#include "common.h"
#include "lib/queue.h"
#include "lib/embedded_linked_list.h"
#include "basic_structures.h"
#include "display.h"

struct Window {
    struct Window *parent;
    window_id_t id;
    client_id_t client_id;
    client_id_t window_manager;
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
void window_set_focus(window_id_t window_id);

struct Window *window_new(struct Window *parent, struct Display *disp, int32_t settion_id, int32_t window_manager, Rect rect, const char *title, Color bg_color);

int window_close(struct Window *win);

int window_draw(struct Window *win, struct Display *disp);

struct Window *window_get_by_id(window_id_t window_id);

void window_move_top(struct Window *win);

void window_reparent(struct Window *win, struct Window *new_parent);

#endif