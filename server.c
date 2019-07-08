#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

SDL_Color make_color(int r, int g, int b) {
    SDL_Color ret;
    ret.r = r;
    ret.g = g;
    ret.b = b;
    return ret;
}

// 失敗したら負を返す関数用のエラー処理ラッパー
#define SDL_CALL_NONNGE(func_name, ...)\
do {\
    if (func_name(__VA_ARGS__) < 0) {\
        fprintf(stderr, #func_name " Error: %s\n", SDL_GetError());\
        return -1;\
    }\
} while (0)


struct Display {
    SDL_Window *win;
    SDL_Renderer *ren;
};
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

int draw_rect(struct Display *disp, int x, int y, int w, int h, SDL_Color color) {
    SDL_CALL_NONNGE(SDL_SetRenderDrawColor, disp->ren, color.r, color.g, color.b, 255);

    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    
    SDL_CALL_NONNGE(SDL_RenderFillRect, disp->ren, &rect);
    SDL_RenderPresent(disp->ren);
}

int main(void) {
    int exit_status = 0;
    SDL_CALL_NONNGE(SDL_Init, SDL_INIT_VIDEO);

    struct Display disp;
    if (display_init(&disp, 640, 480, "SDL2 test") < 0) {
        exit_status = 1;
        goto EXIT_QUIT;
    }

    // rendering
    {
        if (draw_rect(&disp, 100, 100, 200, 200, make_color(0, 255, 128)) < 0) {
            exit_status = 1;
            goto EXIT_RELEASE;
        }
        SDL_Delay(3000);
    }
    
    exit_status = 0;
EXIT_RELEASE:
    display_release(&disp);
EXIT_QUIT:
    SDL_Quit();
    return exit_status;
}