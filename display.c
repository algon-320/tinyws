#include <stdio.h>
#include "common.h"
#include "display.h"

int display_new(struct Display *disp, Size window_size, const char *title) {
    struct Display ret;
    ret.size = window_size;
    ret.win = SDL_CreateWindow(
                    title,
                    SDL_WINDOWPOS_UNDEFINED,
                    SDL_WINDOWPOS_UNDEFINED,
                    window_size.width,
                    window_size.height,
                    SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
                );

    if (ret.win == NULL) {
        fprintf(stderr, "display.c: SDL_CreateWindow Error: %s\n", SDL_GetError());
        return -1;
    }

    ret.ren = SDL_CreateRenderer(ret.win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (ret.ren == NULL) {
        SDL_DestroyWindow(ret.win);
        fprintf(stderr, "Display: SDL_CreateRenderer Error: %s\n", SDL_GetError());
        return -1;
    }

    SDL_CALL_NONNEG(SDL_SetRenderTarget, ret.ren, NULL);

    ret.curosr_pos.x = 0;
    ret.curosr_pos.y = 0;

    *disp = ret;
    return 0;
}

void display_release(struct Display *disp) {
    SDL_DestroyRenderer(disp->ren);
    SDL_DestroyWindow(disp->win);
}

int display_flush(struct Display *disp) {
    SDL_CALL_NONNEG(SDL_SetRenderTarget, disp->ren, NULL);
    SDL_RenderPresent(disp->ren);
    return 0;
}