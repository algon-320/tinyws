#include <stdio.h>
#include <SDL2/SDL.h>

#include "draw.h"

// 失敗したら負を返す関数用のエラー処理ラッパー
#define SDL_CALL_NONNEG(func_name, ...)\
do {\
    if (func_name(__VA_ARGS__) < 0) {\
        fprintf(stderr, __FILE__ ": " #func_name " Error: %s\n", SDL_GetError());\
        return -1;\
    }\
} while (0)


int draw_rect(struct Window *win, int x, int y, int w, int h, Color color) {   
    SDL_CALL_NONNEG(SDL_SetRenderTarget, win->ren, win->buffer);
    SDL_CALL_NONNEG(SDL_SetRenderDrawColor, win->ren, color.r, color.g, color.b, color.a);

    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    
    SDL_CALL_NONNEG(SDL_RenderFillRect, win->ren, &rect);
    return 0;
}

int draw_line(struct Window *win, int x1, int y1, int x2, int y2, Color color) {
    SDL_CALL_NONNEG(SDL_SetRenderTarget, win->ren, win->buffer);
    SDL_CALL_NONNEG(SDL_SetRenderDrawColor, win->ren, color.r, color.g, color.b, color.a);
    SDL_CALL_NONNEG(SDL_RenderDrawLine, win->ren, x1, y1, x2, y2);
    return 0;
}

int draw_pixel(struct Window *win, int x, int y, Color color) {
    SDL_CALL_NONNEG(SDL_SetRenderTarget, win->ren, win->buffer);
    SDL_CALL_NONNEG(SDL_SetRenderDrawColor, win->ren, color.r, color.g, color.b, color.a);
    SDL_CALL_NONNEG(SDL_RenderDrawPoint, win->ren, x, y);
    return 0;
}

int draw_circle(struct Window *win, int x_center, int y_center, int radius, char filled, Color color) {
    SDL_CALL_NONNEG(SDL_SetRenderTarget, win->ren, win->buffer);
    if (filled) {
        // filled
        for (int y = -radius - 1; y < radius + 1; ++y) {
            for (int x = -radius - 1; x < radius + 1; ++x) {
                if (x * x + y * y <= radius * radius) {
                    if (draw_pixel(win, x_center + x, y_center + y, color) < 0) return -1;
                }
            }
        }
    } else {
        // Mid-Point Circle Drawing Algorithm
        //     quoted from: https://www.geeksforgeeks.org/mid-point-circle-drawing-algorithm/
        int y = 0, x = radius;

        if (draw_pixel(win, x + x_center, y + y_center, color) < 0) return -1;
        if (radius > 0) {
            if (draw_pixel(win,  x + x_center, -y + y_center, color) < 0) return -1;
            if (draw_pixel(win,  y + x_center,  x + y_center, color) < 0) return -1;
            if (draw_pixel(win, -y + x_center,  x + y_center, color) < 0) return -1;
        }

        int P = 1 - radius;
        while (x > y) {
            y++;
            if (P <= 0) {
                P = P + 2 * y + 1;
            } else {
                x--;
                P = P + 2 * y - 2 * x + 1;
            }

            if (x < y) break;

            if (draw_pixel(win,  x + x_center,  y + y_center, color) < 0) return -1;
            if (draw_pixel(win, -x + x_center,  y + y_center, color) < 0) return -1;
            if (draw_pixel(win,  x + x_center, -y + y_center, color) < 0) return -1;
            if (draw_pixel(win, -x + x_center, -y + y_center, color) < 0) return -1;

            if (x != y) {
                if (draw_pixel(win,  y + x_center,  x + y_center, color) < 0) return -1;
                if (draw_pixel(win, -y + x_center,  x + y_center, color) < 0) return -1;
                if (draw_pixel(win,  y + x_center, -x + y_center, color) < 0) return -1;
                if (draw_pixel(win, -y + x_center, -x + y_center, color) < 0) return -1;
            }
        }
    }
    return 0;
}

int clear_screen(struct Window *win) {
    SDL_CALL_NONNEG(SDL_SetRenderTarget, win->ren, win->buffer);
    SDL_CALL_NONNEG(SDL_SetRenderDrawColor, win->ren,
            win->background_color.r,
            win->background_color.g,
            win->background_color.b,
            win->background_color.a
    );
    SDL_RenderClear(win->ren);
    return 0;
}
