#ifndef INCLUDE_GUARD_WINDOW_H
#define INCLUDE_GUARD_WINDOW_H

#include <SDL.h>

#include "common.h"
#include "lib/queue.h"
#include "lib/embedded_linked_list.h"
#include "basic_structures.h"
#include "display.h"

#include <pthread.h>

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
    LinkedList child;  // <struct Window *>
    LinkedList next;   // <struct Window *>
    SDL_Texture *buffer;
    struct Display *disp;

    pthread_mutex_t mutex;
};

void window_subsystem_init();

window_id_t window_get_focused();
void window_set_focus(window_id_t window_id);

window_id_t window_new(window_id_t parent_win, struct Display *disp, int32_t settion_id, int32_t window_manager, Rect rect, const char *title, Color bg_color);

int window_close(window_id_t win_id);

int window_draw(window_id_t win_id);


bool window_is_valid(window_id_t win_id);

// get window without ownnership
struct Window *window_ref(window_id_t window_id);

// get window with ownnership
struct Window *window_get_own(window_id_t window_id);

// return ownnership
void window_return_own(struct Window *win);

void window_move_top(window_id_t win_id);

void window_reparent(window_id_t win_id, window_id_t new_parent_id);


// set window attributes

void window_set_pos(struct Window *win, Point pos);

void window_set_visibility(struct Window *win, bool visible);

void window_set_wm(struct Window *win, client_id_t window_manager);

bool window_check_inner_point(struct Window *win, Point p);

#endif