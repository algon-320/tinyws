#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>

#include "tcp.h"
#include "display.h"
#include "draw.h"
#include "query.h"


// 失敗したら負を返す関数用のエラー処理ラッパー
#define SDL_CALL_NONNGE(func_name, ...)\
do {\
    if (func_name(__VA_ARGS__) < 0) {\
        fprintf(stderr, #func_name " Error: %s\n", SDL_GetError());\
        return -1;\
    }\
} while (0)


SDL_Color make_color(int r, int g, int b) {
    SDL_Color ret;
    ret.r = r;
    ret.g = g;
    ret.b = b;
    return ret;
}

#include "lib/queue.h"
Queue drawing_query_queue;

int receive_request(unsigned char *line, size_t size, FILE *in) {
    if (fread(line, sizeof(unsigned char), size, in)) {
        return 0;
    } else {
        if (ferror(in)) {
            return -1;
        }
        return -1;
    }
}

struct communication_arg {
    int com;
};
int communication_thread(struct communication_arg *arg) {
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
    const int LINE_SIZE = 1024;
    unsigned char line[LINE_SIZE];
    FILE *in, *out;

    if (fdopen_sock(arg->com, &in, &out) < 0) {
        perror("fdopen");
        exit(1);
    }

    while (receive_request(line, LINE_SIZE, in) >= 0) {
        // printf("receive %d bytes: %s\n", rcount, line);
        printf("line: [");
        for (int i = 0; i < 20; i++) {
            printf("%d ", (unsigned char)line[i]);
        }
        printf("]\n");
        fflush(stdout);

        struct DrawingQuery query = decode_query(line, LINE_SIZE);
        print_drawing_query(query);
        queue_push(&drawing_query_queue, &query);
        fprintf(out, "accepted\n");
    }

    printf("connection closed.\n");
    fclose(in);
    fclose(out);
    return 0;
}

int drawing_thread() {
    struct Display disp;
    if (display_init(&disp, 1280, 960, "SDL2 test") < 0) {
        return -1;
    }

    drawing_query_queue = queue_new(sizeof(struct DrawingQuery));

    SDL_Event event;
    while (1) {
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT) {
            return 0;
        }

        if(!queue_empty(&drawing_query_queue)) {
            struct DrawingQuery query = *(struct DrawingQuery *)queue_front(&drawing_query_queue);
            queue_pop(&drawing_query_queue);

            switch (query.op) {
                case DrawRect:
                {
                    int x = query.param.draw_rect_param.x;
                    int y = query.param.draw_rect_param.y;
                    int w = query.param.draw_rect_param.w;
                    int h = query.param.draw_rect_param.h;
                    SDL_Color c = make_color(255, 0, 0);
                    if (draw_rect(&disp, x, y, w, h, c) < 0) {
                        return -1;
                    }
                    break;
                }
                case DrawCircle:
                {
                    int x_center = query.param.draw_circle_param.x;
                    int y_center = query.param.draw_circle_param.y;
                    int radius = query.param.draw_circle_param.radius;
                    char filled = query.param.draw_circle_param.filled;

                    SDL_Color c = make_color(0, 255, 128);
                    if (draw_circle(&disp, x_center, y_center, radius, filled, c) < 0) {
                        return -1;
                    }
                    break;
                }
                case DrawLine:
                {
                    int x1 = query.param.draw_line_param.x1;
                    int y1 = query.param.draw_line_param.y1;
                    int x2 = query.param.draw_line_param.x2;
                    int y2 = query.param.draw_line_param.y2;
                    SDL_Color c = make_color(255, 128, 0);
                    if (draw_line(&disp, x1, y1, x2, y2, c) < 0) {
                        return -1;
                    }
                    break;
                }
                case DrawPixel:
                {
                    int x = query.param.draw_pixel_param.x;
                    int y = query.param.draw_pixel_param.y;
                    SDL_Color c = make_color(255, 0, 128);
                    if (draw_pixel(&disp, x, y, c) < 0) {
                        return -1;
                    }
                    break;
                }
                case ClearScreen:
                {
                    printf("screen cleared\n");
                    clear_screen(&disp, make_color(0, 0, 0));
                    break;
                }
                default:
                {
                    printf("invalid query\n");
                    break;
                }
            }

            present(&disp);

            printf("draw ok\n");
        }
    }

    display_release(&disp);
    queue_free(&drawing_query_queue);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s portno\n", argv[0]);
        return 1;
    }

    int exit_status = 0;
    SDL_CALL_NONNGE(SDL_Init, SDL_INIT_VIDEO);

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

    while (1) {
        printf("[%d] acception incoming connections (acc == %d) ...\n", getpid(), acc);
        if ((com = accept(acc, 0, 0)) < 0) {
            perror("accept");
            exit_status = -1;
            goto EXIT_QUIT;
        }
        struct communication_arg arg;
        arg.com = com;
        pthread_t com_thread_id;
        if (pthread_create(&com_thread_id, NULL, (void *)communication_thread, (void *)&arg) != 0) {
            perror("pthread_create(): communication");
            exit_status = -1;
            goto EXIT_QUIT;
        }
        pthread_detach(com_thread_id);
    }

    pthread_join(drawing_thread_id, NULL);
    
    exit_status = 0;

EXIT_QUIT:
    SDL_Quit();
    return exit_status;
}