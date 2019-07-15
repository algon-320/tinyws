#include <stdio.h>
#include "display.h"

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

    return 0;
}
void display_release(struct Display *disp) {
    SDL_DestroyRenderer(disp->ren);
    SDL_DestroyWindow(disp->win);
}
