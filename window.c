#include <string.h>

#include "common.h"
#include "window.h"
#include "draw.h"
#include "lib/deque.h"
#include "lib/stack.h"

Deque windows;        // <struct Window>
Stack free_win;       // <struct Window *>

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

struct Window *window_new(struct Window *parent, struct Display *disp, Point pos, Size size, const char *title, Color bg_color) {
    if (stack_empty(&free_win)) {
        size_t expand_len = deque_size(&windows);
        expand_windows(expand_len);
    }

    struct Window *win = DEQUE_TAKE(stack_top(&free_win), struct Window *);
    stack_pop(&free_win);

    win->parent = parent;

    if (win->parent) {
        linked_list_init(&win->children, NULL, NULL);
        struct Window *ptr = win->parent;
        while (ptr->children.next) {
            ptr = CONTAINNER_OF(ptr->children.next, struct Window, children);
        }
        linked_list_insert_next(&ptr->children, &win->children);
    }

    win->buffer = SDL_CreateTexture(disp->ren, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, size.width, size.height);
    if (win->buffer == NULL) {
        fprintf(stderr, "Window: SDL_CreateTexture Error: %s\n", SDL_GetError());
        return NULL;
    }
    if (SDL_SetTextureBlendMode(win->buffer, SDL_BLENDMODE_BLEND) < 0) {
        SDL_DestroyTexture(win->buffer);
        fprintf(stderr, "Window: SDL_SetTextureBlendMode Error: %s\n", SDL_GetError());
        return NULL;
    }

    win->pos = pos;
    win->size = size;
    win->background_color = bg_color;
    win->visible = 1;
    strncpy(win->title, title, sizeof(win->title));

    win->disp = disp;

    clear_screen(win);
    return win;
}

int window_release(struct Window *win) {
    // release recursively
    while (win->children.next) {
        struct Window *ptr = CONTAINNER_OF(win->children.next, struct Window, children);
        window_release(ptr);
    }
    linked_list_erase(&win->children);

    SDL_DestroyTexture(win->buffer);
    win->parent = NULL;
    win->buffer = NULL;
    win->disp = NULL;

    stack_push(&free_win, &win);
    return 0;
}

int window_draw(struct Window *win, struct Display *disp) {
    if (!win->visible) {
        return 0;
    }

    SDL_Rect rect;

    if (SDL_SetRenderTarget(disp->ren, NULL) < 0) {
        fprintf(stderr, "Window: SDL_SetRenderTarget Error: %s\n", SDL_GetError());
        return -1;
    }

    if (SDL_SetRenderDrawColor(disp->ren, 0, 255, 255, 255) < 0) {
        fprintf(stderr, "Window: SDL_SetRenderDrawColor Error: %s\n", SDL_GetError());
        return -1;
    }

    struct Window *p = win->parent;
    rect.x = win->pos.x;
    rect.y = win->pos.y;
    if (p) {
        rect.x += p->pos.x;
        rect.y += p->pos.y;
    }
    rect.w = win->size.width;
    rect.h = win->size.height;

    if (SDL_RenderCopy(disp->ren, win->buffer, NULL, &rect) < 0) {
        fprintf(stderr, "Window: SDL_RenderCopy Error: %s\n", SDL_GetError());
        return -1;
    }
    
    // draw recursively
    struct Window *ptr = win;
    while (ptr->children.next) {
        ptr = CONTAINNER_OF(ptr->children.next, struct Window, children);
        window_draw(ptr, disp);
    }
    return 0;
}

struct Window *window_get_by_id(uint32_t win_id) {
    if (win_id < deque_size(&windows)) {
        struct Window *win = deque_at(&windows, win_id);
        if (win->disp) {
            return win;
        }
    }
    return NULL;
}