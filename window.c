#include <string.h>

#include "window.h"
#include "draw.h"

static int next_window_id = 0;

int window_new(struct Window *win, struct Display *disp, Point pos, Size size, const char *title, Color bg_color) {
    struct Window ret;
    ret.id = next_window_id++;
    ret.ren = disp->ren;
    ret.buffer = SDL_CreateTexture(ret.ren, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, size.width, size.height);
    if (ret.buffer == NULL) {
        SDL_DestroyRenderer(ret.ren);
        fprintf(stderr, "Window: SDL_CreateTexture Error: %s\n", SDL_GetError());
        return -1;
    }
    if (SDL_SetTextureBlendMode(ret.buffer, SDL_BLENDMODE_BLEND) < 0) {
        SDL_DestroyRenderer(ret.ren);
        SDL_DestroyTexture(ret.buffer);
        fprintf(stderr, "Window: SDL_SetTextureBlendMode Error: %s\n", SDL_GetError());
        return -1;
    }

    ret.pos = pos;
    ret.size = size;
    ret.background_color = bg_color;
    ret.visible = 1;
    strncpy(ret.tilte, title, 256);
    ret.children = NULL;
    ret.children_size = 0;
    *win = ret;
    return 0;
}

int window_release(struct Window *win) {
    for (size_t i = 0; i < win->children_size; i++) {
        window_release(win->children[i]);
        win->children[i] = NULL;
    }
    SDL_DestroyTexture(win->buffer);
    return 0;
}

int window_draw(struct Window *win, struct Display *disp) {
    if (!win->visible) {
        return 0;
    }

    SDL_Rect rect;
    rect.x = win->pos.x;
    rect.y = win->pos.y;
    rect.w = win->size.width;
    rect.h = win->size.height;

    if (SDL_SetRenderTarget(disp->ren, NULL) < 0) {
        fprintf(stderr, "Window: SDL_SetRenderTarget Error: %s\n", SDL_GetError());
        return -1;
    }

    if (SDL_RenderCopy(disp->ren, win->buffer, NULL, &rect) < 0) {
        fprintf(stderr, "Window: SDL_RenderCopy Error: %s\n", SDL_GetError());
        return -1;
    }
    if (SDL_SetRenderDrawColor(disp->ren, 255, 255, 255, 255) < 0) {
        fprintf(stderr, "Window: SDL_SetRenderDrawColor Error: %s\n", SDL_GetError());
        return -1;
    }
    rect.x -= 1;
    rect.y -= 1;
    rect.w += 2;
    rect.h += 2;
    if (SDL_RenderDrawRect(disp->ren, &rect) < 0) {
        fprintf(stderr, "Window: SDL_RenderDrawRect Error: %s\n", SDL_GetError());
        return -1;
    }
    return 0;
}