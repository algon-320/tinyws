#include <stdio.h>
#include "display.h"

#define SDL_CALL_NONNEG(func_name, ...)\
do {\
    if (func_name(__VA_ARGS__) < 0) {\
        fprintf(stderr, #func_name " Error: %s\n", SDL_GetError());\
        return -1;\
    }\
} while (0)

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
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        return -1;
    }

    ret.ren = SDL_CreateRenderer(ret.win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (ret.ren == NULL) {
        SDL_DestroyWindow(ret.win);
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        return -1;
    }

    ret.buffer = SDL_CreateTexture(ret.ren, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, window_size.width, window_size.height);
    if (ret.buffer == NULL) {
        SDL_DestroyRenderer(ret.ren);
        SDL_DestroyWindow(ret.win);
        fprintf(stderr, "SDL_CreateTexture Error: %s\n", SDL_GetError());
        return -1;
    }

    if (SDL_SetRenderTarget(ret.ren, ret.buffer) < 0) {
        display_release(&ret);
        fprintf(stderr, "SDL_SetRenderTarget Error: %s\n", SDL_GetError());
        return -1;
    }

    ret.curosr_pos.x = 0;
    ret.curosr_pos.y = 0;

    *disp = ret;
    return 0;
}

void display_release(struct Display *disp) {
    SDL_DestroyTexture(disp->buffer);
    SDL_DestroyRenderer(disp->ren);
    SDL_DestroyWindow(disp->win);
}

int display_flush(struct Display *disp) {
    SDL_CALL_NONNEG(SDL_SetRenderTarget, disp->ren, NULL);
    SDL_CALL_NONNEG(SDL_SetRenderDrawColor, disp->ren, 0, 0, 0, 255);
    SDL_CALL_NONNEG(SDL_RenderClear, disp->ren);
    SDL_CALL_NONNEG(SDL_RenderCopy, disp->ren, disp->buffer, NULL, NULL);
    SDL_RenderPresent(disp->ren);
    SDL_CALL_NONNEG(SDL_SetRenderTarget, disp->ren, disp->buffer);
    return 0;
}