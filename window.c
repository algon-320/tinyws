#include <string.h>
#include <assert.h>

#include "common.h"
#include "window.h"
#include "draw.h"
#include "lib/deque.h"
#include "lib/stack.h"
#include "event.h"
#include "client.h"

Deque windows;        // <struct Window>
Stack free_win;       // <struct Window *>
struct Window *focused;

void expand_windows(size_t num) {
    int cur_len = deque_size(&windows);
    for (int idx = cur_len + num - 1; idx >= cur_len; idx--) {
        deque_push_back(&windows, NULL);
    }
    for (int idx = cur_len + num - 1; idx >= cur_len; idx--) {
        struct Window *win = deque_at(&windows, idx);
        win->id = idx;
        stack_push(&free_win, &win);
    }
}


void window_subsystem_init() {
    static const int INITIAL_WINDOW_ALLOC_NUM = 16;
    windows = deque_new_with_capacity(0, sizeof(struct Window), INITIAL_WINDOW_ALLOC_NUM);
    free_win = stack_new(sizeof(struct Window *));

    expand_windows(INITIAL_WINDOW_ALLOC_NUM);
}

struct Window *window_get_focused() {
    return focused;
}

void window_set_focus(window_id_t window_id) {
    struct Window *win = window_get_by_id(window_id);
    assert(win);
    focused = win;
}

struct Window *window_new(struct Window *parent, struct Display *disp, client_id_t client_id, client_id_t window_manager, Rect rect, const char *title, Color bg_color) {
    if (stack_empty(&free_win)) {
        size_t expand_len = deque_size(&windows);
        expand_windows(expand_len);
    }

    struct Window *win = DEQUE_TAKE(stack_top(&free_win), struct Window *);
    stack_pop(&free_win);

    focused = win;
    win->parent = parent;
    win->client_id = client_id;
    win->window_manager = window_manager;

    linked_list_init(&win->next, NULL, NULL);
    linked_list_init(&win->child, NULL, NULL);

    if (win->parent) {
        if (!win->parent->child.next) {
            linked_list_insert_next(&win->parent->child, &win->next);
        } else {
            LinkedList *ptr = win->parent->child.next;
            while (ptr->next) {
                ptr = ptr->next;
            }
            linked_list_insert_next(ptr, &win->next);
        }
    }

    win->buffer = SDL_CreateTexture(disp->ren, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, rect.width, rect.height);
    if (win->buffer == NULL) {
        fprintf(stderr, "Window: SDL_CreateTexture Error: %s\n", SDL_GetError());
        return NULL;
    }
    if (SDL_SetTextureBlendMode(win->buffer, SDL_BLENDMODE_BLEND) < 0) {
        SDL_DestroyTexture(win->buffer);
        fprintf(stderr, "Window: SDL_SetTextureBlendMode Error: %s\n", SDL_GetError());
        return NULL;
    }

    win->pos = point_new(rect.x, rect.y);
    win->size = size_new(rect.width, rect.height);
    win->background_color = bg_color;
    win->visible = 1;
    strncpy(win->title, title, sizeof(win->title));

    win->disp = disp;

    clear_screen(win);


    // send event to the window manager
    if (win->parent && win->parent->window_manager != -1 && win->parent->window_manager != win->client_id) {
        struct Client *window_manager = client_get_by_id(win->parent->window_manager);
        assert(window_manager != NULL);
        struct Event wm_notify;
        wm_notify.type = TINYWS_WM_EVENT_NOTIFY_CREATE_WINDOW;
        wm_notify.window_id = win->parent->id;
        wm_notify.param.wm_event_create_window.client_window_id = win->id;
        wm_notify.param.wm_event_create_window.rect = rect_new(win->pos.x, win->pos.y, win->size.width, win->size.height);
        client_send_event(window_manager, &wm_notify);
        debugprint("send wm notify\n");
    }

    return win;
}

int window_close(struct Window *win) {
    debugprint("window_close win=%d\n", win->id);

     // send event to the parent
    if (win->parent->client_id != -1) {
        struct Client *client = client_get_by_id(win->parent->client_id);
        assert(client != NULL);
        struct Event event;
        event.type = TINYWS_EVENT_CLOSE_CHILD_WINDOW;
        event.window_id = win->parent->id;
        event.param.close_child_window.child_window_id = win->id;
        client_send_event(client, &event);
    }

    // release recursively
    while (win->child.next) {
        struct Window *ptr = CONTAINNER_OF(win->child.next, struct Window, next);
        window_close(ptr);
    }
    
    linked_list_erase(&win->child);
    linked_list_erase(&win->next);

    focused = win->parent;
    win->window_manager = -1;
    win->client_id = -1;

    SDL_DestroyTexture(win->buffer);
    win->parent = NULL;
    win->buffer = NULL;
    win->disp = NULL;

    stack_push(&free_win, &win);
    return 0;
}

int window_draw(struct Window *win, struct Display *disp) {
    // debugprint("window_draw: win=%d\n", win->id);
    if (win->visible) {
        SDL_Rect rect;
        rect.x = win->pos.x;
        rect.y = win->pos.y;
        rect.w = win->size.width;
        rect.h = win->size.height;

        // draw a frame for the focused window
        // if (focused == win) {
        //     SDL_Rect tmp = rect;
        //     tmp.x -= 2;
        //     tmp.y -= 2;
        //     tmp.w += 4;
        //     tmp.h += 4;
        //     SDL_SetRenderDrawColor(win->disp->ren, 255, 0, 0, 255);
        //     SDL_RenderDrawRect(win->disp->ren, &tmp);
        // }

        SDL_Texture *parent_buffer = SDL_GetRenderTarget(disp->ren);

        if (SDL_SetRenderTarget(disp->ren, win->buffer) < 0) {
            fprintf(stderr, "Window: SDL_SetRenderTarget Error: %s\n", SDL_GetError());
            return -1;
        }

        // draw children
        if (win->child.next) {
            struct Window *ptr = CONTAINNER_OF(win->child.next, struct Window, next);
            window_draw(ptr, disp);
        }

        // copy buffer to parent
        if (SDL_SetRenderTarget(disp->ren, parent_buffer) < 0) {
            fprintf(stderr, "Window: SDL_SetRenderTarget Error: %s\n", SDL_GetError());
            return -1;
        }
        if (SDL_RenderCopy(disp->ren, win->buffer, NULL, &rect) < 0) {
            fprintf(stderr, "Window: SDL_RenderCopy Error: %s\n", SDL_GetError());
            return -1;
        }
    }

    struct Window *ptr = win;
    while (ptr->next.next) {
        ptr = CONTAINNER_OF(ptr->next.next, struct Window, next);
        window_draw(ptr, disp);
    }
    return 0;
}

struct Window *window_get_by_id(window_id_t window_id) {
    if (window_id < deque_size(&windows)) {
        struct Window *win = deque_at(&windows, window_id);
        if (win->disp) {
            return win;
        }
    }
    return NULL;
}

// for debug
void window_print_all() {
    for (size_t i = 0; i < deque_size(&windows); i++) {
        struct Window *win = deque_at(&windows, i);
        debugprint("id=%d\n", win->id);
        if (win->child.next) {
            struct Window *ch = CONTAINNER_OF(win->child.next, struct Window, next);
            debugprint("child=%d\n", ch->id);
        } else {
            debugprint("child=NULL\n");
        }
        if (win->next.next) {
            struct Window *ch = CONTAINNER_OF(win->next.next, struct Window, next);
            debugprint("next=%d\n", ch->id);
        } else {
            debugprint("next=NULL\n");
        }
    }
}

void window_move_top(struct Window *win) {
    // window_print_all();
    if (!win->next.next) {
        // already top
        debugprint("window_move_top: %d already top\n", win->id);
        return;
    } else {
        LinkedList *ptr = win->next.next;
        linked_list_erase(&win->next);
        while (ptr->next) {
            ptr = ptr->next;
        }
        linked_list_insert_next(ptr, &win->next);
    }
}

void window_reparent(struct Window *win, struct Window *new_parent) {
    linked_list_erase(&win->next);
    win->parent = new_parent;
    
    if (!new_parent->child.next) {
        linked_list_insert_next(&new_parent->child, &win->next);
    } else {
        LinkedList *ptr = new_parent->child.next;
        while (ptr->next) {
            ptr = ptr->next;
        }
        linked_list_insert_next(ptr, &win->next);
    }

    window_print_all();

    win->pos.x -= new_parent->pos.x;
    win->pos.y -= new_parent->pos.y;
}