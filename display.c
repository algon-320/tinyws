#include <stdio.h>
#include "display.h"

#define SDL_CALL_NONNEG(func_name, ...)\
do {\
    if (func_name(__VA_ARGS__) < 0) {\
        fprintf(stderr, #func_name " Error: %s\n", SDL_GetError());\
        return -1;\
    }\
} while (0)

int display_init(struct Display *disp, int display_width, int display_height, const char *title) {
    disp->win = SDL_CreateWindow(
                    title,
                    SDL_WINDOWPOS_UNDEFINED,
                    SDL_WINDOWPOS_UNDEFINED,
                    display_width,
                    display_height,
                    SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
                );

    if (disp->win == NULL) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        return -1;
    }


    disp->ren = SDL_CreateRenderer(disp->win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (disp->ren == NULL) {
        SDL_DestroyWindow(disp->win);
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        return -1;
    }

    disp->buffer = SDL_CreateTexture(disp->ren, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, display_width, display_height);
    if (disp->buffer == NULL) {
        SDL_DestroyRenderer(disp->ren);
        SDL_DestroyWindow(disp->win);
        fprintf(stderr, "SDL_CreateTexture Error: %s\n", SDL_GetError());
        return -1;
    }

    if (SDL_SetRenderTarget(disp->ren, disp->buffer) < 0) {
        display_release(disp);
        fprintf(stderr, "SDL_SetRenderTarget Error: %s\n", SDL_GetError());
        return -1;
    }

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