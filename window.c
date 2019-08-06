#include <string.h>
#include <assert.h>

#include "common.h"
#include "window.h"
#include "draw.h"
#include "lib/deque.h"
#include "lib/stack.h"
#include "event.h"
#include "client.h"

const window_id_t WINDOW_ID_INVALID = -1;

// TODO: use dictionary
// from window_id to window ptr
Deque windows_ptr;  // <struct Window *>
window_id_t next_window_id;
pthread_mutex_t windows_mutex;

window_id_t focused;


void window_subsystem_init() {
    next_window_id = 0;
    windows_ptr = deque_new(0, sizeof(struct Window *));
    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&windows_mutex, &mutex_attr);
}

window_id_t window_get_focused() {
    window_id_t ret;
    lock_mutex(&windows_mutex);
    ret = focused;
    unlock_mutex(&windows_mutex);
    return ret;
}

void window_set_focus(window_id_t win_id) {
    lock_mutex(&windows_mutex);
    if (window_is_valid(win_id)) {
        focused = win_id;
    }
    unlock_mutex(&windows_mutex);
}

window_id_t window_new(window_id_t parent_id, struct Display *disp, client_id_t client_id, client_id_t window_manager, Rect rect, const char *title, Color bg_color) {
    struct Window *win = calloc(1, sizeof(struct Window));
    win->id = next_window_id++;
    win->parent = NULL;
    win->disp = NULL;
    linked_list_init(&win->next, NULL, NULL);
    linked_list_init(&win->child, NULL, NULL);

    window_id_t window_id = win->id;

    lock_mutex(&windows_mutex);
    {
        deque_push_back(&windows_ptr, &win);
    }
    unlock_mutex(&windows_mutex);

    client_id_t wm_id = -1;
    struct Event wm_notify;

    {
        win = window_get_own(win->id);
        assert(win);

        struct Window *parent_ptr = window_ref(parent_id);
        focused = win->id;
        win->parent = parent_ptr;
        win->client_id = client_id;
        win->window_manager = window_manager;

        linked_list_init(&win->next, NULL, NULL);
        linked_list_init(&win->child, NULL, NULL);

        if (win->parent != NULL) {
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
            return WINDOW_ID_INVALID;
        }
        if (SDL_SetTextureBlendMode(win->buffer, SDL_BLENDMODE_BLEND) < 0) {
            SDL_DestroyTexture(win->buffer);
            fprintf(stderr, "Window: SDL_SetTextureBlendMode Error: %s\n", SDL_GetError());
            return WINDOW_ID_INVALID;
        }
        debugprint("win=%d, buffer=%p\n", win->id, win->buffer);

        win->pos = point_new(rect.x, rect.y);
        win->size = size_new(rect.width, rect.height);
        win->background_color = bg_color;
        win->visible = 1;
        strncpy(win->title, title, sizeof(win->title));

        win->disp = disp;
        
        clear_screen(win);

        // if win is not a window created by window manager
        // send event to the window manager
        if (parent_ptr && parent_ptr->window_manager != -1 && parent_ptr->window_manager != win->client_id) {
            wm_id = parent_ptr->window_manager;
            wm_notify.type = TINYWS_WM_EVENT_NOTIFY_CREATE_WINDOW;
            wm_notify.window_id = parent_id;
            wm_notify.param.wm_event_create_window.client_window_id = win->id;
            wm_notify.param.wm_event_create_window.rect = rect_new(win->pos.x, win->pos.y, win->size.width, win->size.height);
        }
        
        window_return_own(win);
    }

    if (wm_id != CLIENT_ID_INVALID) {
        client_event_push(wm_id, &wm_notify);
        debugprint("send wm notify\n");
    }
    return window_id;
}

void window_print_all();

int window_close_ptr(struct Window *win) {
    if (win == NULL) {
        return -1;
    }

    debugprint("window_close win=%d\n", win->id);

    // send event to the parent
    client_id_t window_manager_id = win->parent->client_id;
    if (window_manager_id != -1 && client_is_valid(window_manager_id)) {
        struct Event event;
        event.type = TINYWS_EVENT_CLOSE_CHILD_WINDOW;
        event.window_id = win->parent->id;
        event.param.close_child_window.child_window_id = win->id;
        client_event_push(window_manager_id, &event);
    }

    assert(win->disp);

    // release recursively
    while (win->child.next) {
        struct Window *ptr = CONTAINNER_OF(win->child.next, struct Window, next);
        window_close_ptr(ptr);
    }
    
    linked_list_erase(&win->child);
    linked_list_erase(&win->next);

    focused = win->parent->id;
    win->window_manager = -1;
    win->client_id = -1;

    assert(win->buffer);
    SDL_DestroyTexture(win->buffer);

    win->parent = NULL;
    win->buffer = NULL;
    win->disp = NULL;

    DEQUE_TAKE(deque_at(&windows_ptr, win->id), struct Window *) = NULL;
    free(win);
    return 0;
}
int window_close(window_id_t win_id) {
    if (!window_is_valid(win_id)) {
        return -1;
    }

    lock_mutex(&windows_mutex);
    {
        window_print_all();

        struct Window *win = window_ref(win_id);
        window_close_ptr(win);
    }
    unlock_mutex(&windows_mutex);

    debugprint("window_close ok.\n");
    return 0;
}

int window_draw_ptr(struct Window *win) {
    if (!win->disp) {
        debugprint("win=%d is closed.\n", win->id);
        return 0;
    }

    // debugprint("window_draw: win=%d\n", win->id);
    if (win->visible) {
        SDL_Rect rect;
        rect.x = win->pos.x;
        rect.y = win->pos.y;
        rect.w = win->size.width;
        rect.h = win->size.height;

        SDL_Texture *parent_buffer = SDL_GetRenderTarget(win->disp->ren);

        if (SDL_SetRenderTarget(win->disp->ren, win->buffer) < 0) {
            fprintf(stderr, "Window: SDL_SetRenderTarget Error: %s\n", SDL_GetError());
            return -1;
        }

        // draw children
        if (win->child.next) {
            struct Window *ptr = CONTAINNER_OF(win->child.next, struct Window, next);
            window_draw_ptr(ptr);
        }

        // copy buffer to parent
        if (SDL_SetRenderTarget(win->disp->ren, parent_buffer) < 0) {
            fprintf(stderr, "Window: SDL_SetRenderTarget Error: %s\n", SDL_GetError());
            return -1;
        }
        if (SDL_RenderCopy(win->disp->ren, win->buffer, NULL, &rect) < 0) {
            fprintf(stderr, "Window: SDL_RenderCopy Error: %s\n", SDL_GetError());
            return -1;
        }
    }

    struct Window *ptr = win;
    while (ptr->next.next) {
        ptr = CONTAINNER_OF(ptr->next.next, struct Window, next);
        window_draw_ptr(ptr);
    }
    return 0;
}
int window_draw(window_id_t win_id) {
    if (!window_is_valid(win_id)) {
        return -1;
    }
    struct Window *win = window_get_own(win_id);
    // debugprint("window_draw ==============\n");
    window_draw_ptr(win);
    window_return_own(win);
    return 0;
}

bool window_is_valid(window_id_t win_id) {
    lock_mutex(&windows_mutex);
    bool ret = true;
    if (!(0 <= win_id && (size_t)win_id < deque_size(&windows_ptr))) {
        debugprint("win_id out of range\n");
        ret = false;
    } else {
        struct Window *tmp = DEQUE_TAKE(deque_at(&windows_ptr, win_id), struct Window *);
        if (!tmp) {
            debugprint("win_id is NULL\n");
            ret = false;
        }
    }
    unlock_mutex(&windows_mutex);
    return ret;
}

// get window without ownnership
struct Window *window_ref(window_id_t win_id) {
    if (!(0 <= win_id && (size_t)win_id < deque_size(&windows_ptr))) {
        return NULL;
    }
    return DEQUE_TAKE(deque_at(&windows_ptr, win_id), struct Window *);
}

// get window with ownnership
struct Window *window_get_own(window_id_t win_id) {
    lock_mutex(&windows_mutex);
    struct Window *win = window_ref(win_id);
    if (!win) {
        unlock_mutex(&windows_mutex);
        return NULL;
    }
    return win;
}

// return ownnership
void window_return_own(struct Window *win) {
    if (!win) {
        return;
    }
    unlock_mutex(&windows_mutex);
}

// for debug
void window_print_all() {
    for (size_t i = 0; i < deque_size(&windows_ptr); i++) {
        struct Window *win = DEQUE_TAKE(deque_at(&windows_ptr, i), struct Window *);
        debugprint("id=%lu\n", i);
        if (!win) {
            debugprint("\tNULL\n");
            continue;
        }

        if (win->child.next) {
            struct Window *ch = CONTAINNER_OF(win->child.next, struct Window, next);
            debugprint("\tchild=%d\n", ch->id);
        } else {
            debugprint("\tchild=NULL\n");
        }
        if (win->next.next) {
            struct Window *ch = CONTAINNER_OF(win->next.next, struct Window, next);
            debugprint("\tnext=%d\n", ch->id);
        } else {
            debugprint("\tnext=NULL\n");
        }
    }
}

void window_move_top(window_id_t win_id) {
    struct Window *win = window_get_own(win_id);
    assert(win);

    // window_print_all();
    if (!win->next.next) {
        // already top
        debugprint("window_move_top: %d already top\n", win_id);
    } else {
        LinkedList *ptr = win->next.next;
        linked_list_erase(&win->next);
        while (ptr->next) {
            ptr = ptr->next;
        }
        linked_list_insert_next(ptr, &win->next);
    }
    window_return_own(win);
}

void window_reparent(window_id_t win_id, window_id_t new_parent_id) {
    struct Window *win = window_get_own(win_id);

    linked_list_erase(&win->next);

    struct Window *new_parent = window_ref(new_parent_id);
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

    window_return_own(win);
}

void window_set_pos(struct Window *win, Point pos) {
    win->pos = pos;
}

void window_set_visibility(struct Window *win, bool visible) {
    win->visible = (visible ? 1 : 0);
}

void window_set_wm(struct Window *win, client_id_t window_manager) {
    win->window_manager = window_manager;
}

bool window_check_inner_point(struct Window *win, Point p) {
    return (win->pos.x <= p.x
            && p.x < win->pos.x + win->size.width
            && win->pos.y <= p.y
            && p.y < win->pos.y + win->size.height);
}

window_id_t window_get_top_ptr(struct Window *win, Point p) {
    if (!win) return WINDOW_ID_INVALID;
    window_id_t ret = WINDOW_ID_INVALID;
    if (window_check_inner_point(win, p)) {
        ret = win->id;
    }
    LinkedList *list = &win->child;
    while (list->next) {
        list = list->next;
        struct Window *tmp = CONTAINNER_OF(list, struct Window, next);
        Point rel_p = point_new(p.x - win->pos.x, p.y - win->pos.y);
        if (window_check_inner_point(tmp, rel_p)) {
            window_id_t res = window_get_top_ptr(tmp, rel_p);
            if (res != WINDOW_ID_INVALID) {
                ret = res;
            }
        }
    }
    return ret;
}
window_id_t window_get_top(window_id_t win_id, Point p) {
    struct Window *win = window_get_own(win_id);
    window_id_t ret = window_get_top_ptr(win, p);
    window_return_own(win);
    return ret;
}