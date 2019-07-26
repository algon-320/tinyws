#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "basic_structures.h"
#include "lib/queue.h"
#include "lib/stack.h"
#include "lib/linked_list.h"
#include "tcp.h"
#include "display.h"
#include "draw.h"
#include "query.h"
#include "response.h"
#include "window.h"

// 失敗したら負を返す関数用のエラー処理ラッパー
#define SDL_CALL_NONNEG(func_name, ...)\
do {\
    if (func_name(__VA_ARGS__) < 0) {\
        fprintf(stderr, __FILE__ ": " #func_name " Error: %s\n", SDL_GetError());\
        return -1;\
    }\
} while (0)

struct Display disp;
struct Window *root_win;
struct Window *overlay_win;

Queue query_queue;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int receive_request(uint8_t *line, size_t size, FILE *in) {
    if (fread(line, sizeof(uint8_t), size, in) == 0) {
        return -1;
    }
    return 0;
}

struct interaction_thread_arg {
    int com;
};
int interaction_thread(struct interaction_thread_arg *arg) {
    // print peer address
    {
        struct sockaddr_storage addr ;
        socklen_t addr_len ; /* MacOSX: __uint32_t */
        addr_len = sizeof( addr );
        if (getpeername(arg->com, (struct sockaddr *)&addr, &addr_len) < 0) {
            perror("tcp_peeraddr_print");
        } else {
            printf("[%d] connection (fd == %d) from ", getpid(), arg->com);
            // sockaddr_print((struct sockaddr *)&addr, addr_len);
            printf("\n");
        }
    }

    // communicating
    const int BUFFSIZE = 1024;
    uint8_t buf[BUFFSIZE * 2], response_buf[BUFFSIZE * 2];
    FILE *in, *out;

    if (fdopen_sock(arg->com, &in, &out) < 0) {
        perror("fdopen");
        exit(1);
    }

    while (receive_request(buf, BUFFSIZE, in) >= 0) {
        // printf("receive %d bytes: %s\n", rcount, buf);
        printf("buf: [");
        for (int i = 0; i < 20; i++) {
            printf("%d ", (uint8_t)buf[i]);
        }
        printf("]\n");
        fflush(stdout);

        struct Query query = query_decode(buf, BUFFSIZE);
        query_print(&query);

        struct Response resp;
        resp.success = 1;
        resp.type = TINYWS_RESPONSE_NOCONTENT;

        switch (query.type) {
            // if it is a window management query, process her
            case TINYWS_QUERY_CREATE_WINDOW:
            {
                struct Window *parent_win = window_get_by_id(query.param.create_window.parent_window_id);
                if (parent_win == NULL) {
                    resp.success = 0;
                    break;
                }

                struct Window *win = window_new(parent_win, &disp, 
                        point_new(query.param.create_window.pos_x, query.param.create_window.pos_x),
                        size_new(query.param.create_window.width, query.param.create_window.height),
                        "test window", color_new(0x1D, 0x1D, 0x1D, 0xFF)
                        );
                resp.type = TINYWS_RESPONSE_WINDOW_ID;
                resp.content.window_id.id = win->id;
                break;
            }
            case TINYWS_QUERY_SET_WINDOW_POS:
            {

                break;
            }
            case TINYWS_QUERY_SET_WINDOW_VISIBILITY:
            {
                break;
            }

            // otherwise, send to drawing thread
            case TINYWS_QUERY_DRAW_RECT:
            case TINYWS_QUERY_DRAW_CIRCLE:
            case TINYWS_QUERY_DRAW_LINE:
            case TINYWS_QUERY_DRAW_PIXEL:
            {
                int r;
                r = pthread_mutex_lock(&mutex);
                if (r != 0) {
                    fprintf(stderr, "can not lock\n");
                    break;
                }
                {
                    queue_push(&query_queue, &query);
                    pthread_cond_signal(&cond);
                }
                r = pthread_mutex_unlock(&mutex);
                if (r != 0) {
                    fprintf(stderr, "can not lock\n");
                    break;
                }
                break;
            }
            default:
            {
                resp.success = 0;
                break;
            }
        }

        size_t bytes = response_encode(&resp, response_buf, BUFFSIZE);
        fwrite(response_buf, sizeof(uint8_t), BUFFSIZE, out);
    }

    printf("connection closed.\n");
    fclose(in);
    fclose(out);
    return 0;
}

int event_thread() {
    SDL_Event event;
    while (1) {
        if (SDL_WaitEvent(&event) != 1) {
            fprintf(stderr, "event_thread: SDL_WaitEvent: %s\n", SDL_GetError());
            exit(1);
        }

        switch (event.type) {
            case SDL_QUIT:
            {
                exit(0);
                break;
            }
            case SDL_KEYDOWN:
            {
                if (deque_size(&root_win->children)) {
                    struct Window *win = DEQUE_TAKE(deque_at(&root_win->children, 0), struct Window *);
                    switch (event.key.keysym.sym) {
                        case SDLK_UP:
                            win->pos.y -= 50;
                            break;
                        case SDLK_DOWN:
                            win->pos.y += 50;
                            break;
                        case SDLK_LEFT:
                            win->pos.x -= 50;
                            break;
                        case SDLK_RIGHT:
                            win->pos.x += 50;
                            break;
                    }
                }
                break;
            }
        }
        
        // 再描画
        {
            int r;
            r = pthread_mutex_lock(&mutex);
            if (r != 0) {
                fprintf(stderr, "can not lock\n");
                break;
            }
            {
                struct Query query;
                query.type = TINYWS_QUERY_REFRESH;
                queue_push(&query_queue, &query);
                pthread_cond_signal(&cond);
            }
            r = pthread_mutex_unlock(&mutex);
            if (r != 0) {
                fprintf(stderr, "can not lock\n");
                break;
            }
        }
    }
    return 0;
}

int drawing_thread() {
    while (1) {
        struct Query query;
        {
            int r;
            r = pthread_mutex_lock(&mutex);
            if (r != 0) {
                fprintf(stderr, "can not lock\n");
                break;
            }
            {
                if (queue_empty(&query_queue)) {
                    pthread_cond_wait(&cond, &mutex);
                }
                query = *(struct Query *)queue_front(&query_queue);
                queue_pop(&query_queue);
            }
            r = pthread_mutex_unlock(&mutex);
            if (r != 0) {
                fprintf(stderr, "can not lock\n");
                break;
            }
        }

        clear_screen(root_win);
        clear_screen(overlay_win);

        // mouse cursor
        {
            int mouse_x, mouse_y;
            int pushed = SDL_GetMouseState(&mouse_x, &mouse_y);
            if (pushed & SDL_BUTTON(SDL_BUTTON_LEFT)) {
                draw_rect(overlay_win, mouse_x, mouse_y, 10, 10, color_new(0, 255, 0, 255));
            } else {
                draw_rect(overlay_win, mouse_x, mouse_y, 10, 10, color_new(255, 255, 0, 255));
            }
            disp.curosr_pos = point_new(mouse_x, mouse_y);
        }

        printf("drawing --> "); query_print(&query);

        if (deque_size(&root_win->children)) {
            struct Window *win = DEQUE_TAKE(deque_at(&root_win->children, 0), struct Window *);

            switch (query.type) {
                case TINYWS_QUERY_DRAW_RECT:
                {
                    int x = query.param.draw_rect.x;
                    int y = query.param.draw_rect.y;
                    int w = query.param.draw_rect.w;
                    int h = query.param.draw_rect.h;
                    Color c = color_new(255, 0, 0, 255);
                    if (draw_rect(win, x, y, w, h, c) < 0) {
                        return -1;
                    }
                    break;
                }
                case TINYWS_QUERY_DRAW_CIRCLE:
                {
                    int x_center = query.param.draw_circle.x;
                    int y_center = query.param.draw_circle.y;
                    int radius = query.param.draw_circle.radius;
                    char filled = query.param.draw_circle.filled;

                    Color c = color_new(0, 255, 128, 255);
                    if (draw_circle(win, x_center, y_center, radius, filled, c) < 0) {
                        return -1;
                    }
                    break;
                }
                case TINYWS_QUERY_DRAW_LINE:
                {
                    int x1 = query.param.draw_line.x1;
                    int y1 = query.param.draw_line.y1;
                    int x2 = query.param.draw_line.x2;
                    int y2 = query.param.draw_line.y2;
                    Color c = color_new(255, 128, 0, 255);
                    if (draw_line(win, x1, y1, x2, y2, c) < 0) {
                        return -1;
                    }
                    break;
                }
                case TINYWS_QUERY_DRAW_PIXEL:
                {
                    int x = query.param.draw_pixel.x;
                    int y = query.param.draw_pixel.y;
                    Color c = color_new(255, 0, 128, 255);
                    if (draw_pixel(win, x, y, c) < 0) {
                        return -1;
                    }
                    break;
                }
                case TINYWS_QUERY_CLEAR_SCREEN:
                {
                    clear_screen(win);
                    break;
                }
                
                default:
                {
                    printf("invalid query\n");
                    break;
                }
            }
        }

        printf("draw ok\n");

        SDL_SetRenderDrawColor(disp.ren, 0, 0, 0, 0);
        SDL_SetRenderTarget(disp.ren, NULL);
        SDL_RenderClear(disp.ren);
        
        // draw windows
        for (int i = 0, sz = deque_size(&window_z_ord); i < sz; i++) {
            struct Window *ptr = DEQUE_TAKE(deque_at(&window_z_ord, i), struct Window *);
            window_draw(ptr, &disp);
        }

        // update screen
        display_flush(&disp);
    }

    display_release(&disp);
    queue_free(&query_queue);
    return 0;
}


int init() {
    SDL_CALL_NONNEG(SDL_Init, SDL_INIT_VIDEO);
    SDL_ShowCursor(SDL_DISABLE);

    const int DISPLAY_WIDTH  = 1280;
    const int DISPLAY_HEIGHT = 960;
    
    if (display_new(&disp, size_new(DISPLAY_WIDTH, DISPLAY_HEIGHT), "tinyws virtual display") < 0) {
        return -1;
    }

    window_subsystem_init();

    query_queue = queue_new(sizeof(struct Query));
    
    // root window
    root_win = window_new(NULL, &disp, point_new(0, 0), size_new(DISPLAY_WIDTH, DISPLAY_HEIGHT), "root", color_new(0x8D, 0x1D, 0x2D, 255));

    // overlay window
    overlay_win = window_new(NULL, &disp, point_new(0, 0), size_new(DISPLAY_WIDTH, DISPLAY_HEIGHT), "cursor", color_new(0, 0, 0, 0));
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s portno\n", argv[0]);
        return 1;
    }

    int exit_status = 0;
    if (init() < 0) {
        return -1;
    }

    int com;
    int portno = strtol(argv[1], NULL, 10);
    int acc = tcp_acc_port(portno, 4);
    if (acc < 0) {
        exit_status = -1;
        goto EXIT_QUIT;
    }

    pthread_t drawing_thread_id;
    if (pthread_create(&drawing_thread_id, NULL, (void *)drawing_thread, NULL) != 0) {
        perror("pthread_create(): drawing");
        exit_status = -1;
        goto EXIT_QUIT;
    }

    pthread_t event_thread_id;
    if (pthread_create(&event_thread_id, NULL, (void *)event_thread, NULL) != 0) {
        perror("pthread_create(): drawing");
        exit_status = -1;
        goto EXIT_QUIT;
    }

    while (1) {
        printf("[%d] acception incoming connections (acc == %d) ...\n", getpid(), acc);
        if ((com = accept(acc, 0, 0)) < 0) {
            perror("accept");
            continue;
        }
        struct interaction_thread_arg arg;
        arg.com = com;
        pthread_t com_thread_id;
        if (pthread_create(&com_thread_id, NULL, (void *)interaction_thread, (void *)&arg) != 0) {
            perror("pthread_create(): communication");
            continue;
        }
        pthread_detach(com_thread_id);
    }

    pthread_join(drawing_thread_id, NULL);
    pthread_join(event_thread_id, NULL);

    exit_status = 0;

EXIT_QUIT:
    SDL_Quit();
    return exit_status;
}