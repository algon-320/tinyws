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

#define PORTNO_BUFSIZE (30)
int tcp_acc_port(int portno, int ip_version) {
    struct addrinfo hints, *ai;
    char portno_str[PORTNO_BUFSIZE];
    int err, s, on, pf;

    switch (ip_version) {
    case 4:
        pf = PF_INET;
        break;
    case 6:
        pf = PF_INET6;
        break;
    case 46:
    case 64:
        pf = 0;
        break;
    default:
        fprintf(stderr, "bad IP version: %d. 4 or 6 is allowed.\n", ip_version);
        goto error0;
    }

    snprintf(portno_str, sizeof(portno_str), "%d", portno);
    memset(&hints, 0, sizeof(hints));
    ai = NULL;
    hints.ai_family = pf;
    hints.ai_flags  = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;

    if ((err = getaddrinfo(NULL, portno_str, &hints, &ai))) {
        fprintf(stderr, "bad portno %d? (%s)\n", portno, gai_strerror(err));
        goto error0;
    }
    if ((s = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) < 0) {
        perror("socket");
		goto error1;
    }
#ifdef IPV6_ONLY
    if (ai->ai_family == PF_INET6 && ip_version == 6) {
        on = 1;
        if (setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on)) < 0) {
            perror("setsockopt(,,IPV6_V6ONLY)");
            goto error1;
        }
    }
#endif
    if (bind(s, ai->ai_addr, ai->ai_addrlen) < 0) {
        perror("bind");
        fprintf(stderr, "Port nummber %d\n", portno);
        goto error2;
    }
    on = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        perror("setsockopt(,,SO_REUSEADDR)");
        goto error2;
    }
    if (listen(s, 5) < 0) {
        perror("listen");
        goto error2;
    }
    freeaddrinfo(ai);
    return s;

error2:
    close(s);
error1:
    freeaddrinfo(ai);
error0:
    return -1;
}

int fdopen_sock(int sock, FILE **inp, FILE **outp) {
    int sock2;
    if ((sock2 = dup(sock)) < 0) {
        return -1;
    }
    if ((*inp = fdopen(sock2, "r")) == NULL) {
        close(sock2);
        return -1;
    }
    if ((*outp = fdopen(sock, "w")) == NULL) {
        fclose(*inp);
        *inp = NULL;
        return -1;
    }
    setvbuf(*outp, (char *)NULL, _IONBF, 0);
    return 0;
}

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

int draw_line(struct Display *disp, int x1, int y1, int x2, int y2, SDL_Color color) {
    SDL_CALL_NONNGE(SDL_SetRenderDrawColor, disp->ren, color.r, color.g, color.b, 255);
    SDL_CALL_NONNGE(SDL_RenderDrawLine, disp->ren, x1, y1, x2, y2);
    SDL_RenderPresent(disp->ren);
}

int draw_pixel(struct Display *disp, int x, int y, SDL_Color color) {
    SDL_CALL_NONNGE(SDL_SetRenderDrawColor, disp->ren, color.r, color.g, color.b, 255);
    SDL_CALL_NONNGE(SDL_RenderDrawPoint, disp->ren, x, y);
    SDL_RenderPresent(disp->ren);
}


int receive_request(unsigned char *line, size_t size, FILE *in) {
    if (fread(line, sizeof(unsigned char), size, in)) {
    // if (fgets(line, size, in)) {
        return 0;
    } else {
        if (ferror(in)) {
            return -1;
        }
        return -1;
    }
}


typedef enum {
    DrawRect,
    DrawCircle,
    DrawLine,
    DrawPixel,
    ClearScreen,
    Invalid,
} DrawingOperation;

struct DrawingQuery {
    DrawingOperation op;
    union {
        struct {
            int x, y, w, h;
        } draw_rect_param;
        struct {
            int x, y, radius;
        } draw_circle_param;
        struct {
            int x1, y1, x2, y2;
        } draw_line_param;
        struct {
            int x, y;
        } draw_pixel_param;
    } param;
};

void print_drawing_query(struct DrawingQuery query) {
    switch (query.op) {
        case DrawRect:
            printf("DrawRect(x=%d, y=%d, w=%d, h=%d)\n",
                query.param.draw_rect_param.x,
                query.param.draw_rect_param.y,
                query.param.draw_rect_param.w,
                query.param.draw_rect_param.h);
            break;
        case DrawCircle:
            printf("DrawCircle(x=%d, y=%d, radius=%d)\n",
                query.param.draw_circle_param.x,
                query.param.draw_circle_param.y,
                query.param.draw_circle_param.radius);
            break;
        case DrawLine:
            printf("DrawLine(x1=%d, y1=%d, x2=%d, y2=%d)\n",
                query.param.draw_line_param.x1,
                query.param.draw_line_param.y1,
                query.param.draw_line_param.x2,
                query.param.draw_line_param.y2);
            break;  
        case DrawPixel:
            printf("DrawRect(x=%d, y=%d)\n",
                query.param.draw_rect_param.x,
                query.param.draw_rect_param.y);
            break;
        case ClearScreen:
            printf("ClearScreen\n");
            break;
        default:
            printf("invlid operator\n");
            break;
    }
}


int decode_int32_little(const unsigned char *buf) {
    int ret = 0;
    for (int i = 0; i < 4; i++) {
        int x = buf[i];
        ret |= (x << (i * 8));
    }
    return ret;
}

struct DrawingQuery decode_query(const unsigned char *buf, size_t size) {
    struct DrawingQuery ret;
    switch (buf[0]) {
        case DrawRect:
        {
            if (size < 17) {
                ret.op = Invalid;
                break;
            }

            int x, y, w, h;
            x = decode_int32_little(buf + 1);
            y = decode_int32_little(buf + 5);
            w = decode_int32_little(buf + 9);
            h = decode_int32_little(buf + 13);

            ret.op = DrawRect;
            ret.param.draw_rect_param.x = x;
            ret.param.draw_rect_param.y = y;
            ret.param.draw_rect_param.w = w;
            ret.param.draw_rect_param.h = h;
            break;
        }
        case DrawCircle:
        {
            if (size < 13) {
                ret.op = Invalid;
                break;
            }

            int x, y, radius;
            x = decode_int32_little(buf + 1);
            y = decode_int32_little(buf + 5);
            radius = decode_int32_little(buf + 9);

            ret.op = DrawCircle;
            ret.param.draw_circle_param.x = x;
            ret.param.draw_circle_param.y = y;
            ret.param.draw_circle_param.radius = radius;
            break;
        }
        case DrawLine:
        {
            if (size < 17) {
                ret.op = Invalid;
                break;
            }

            int x1, y1, x2, y2;
            x1 = decode_int32_little(buf + 1);
            y1 = decode_int32_little(buf + 5);
            x2 = decode_int32_little(buf + 9);
            y2 = decode_int32_little(buf + 13);

            ret.op = DrawLine;
            ret.param.draw_line_param.x1 = x1;
            ret.param.draw_line_param.y1 = y1;
            ret.param.draw_line_param.x2 = x2;
            ret.param.draw_line_param.y2 = y2;
            break;
        }
        case DrawPixel:
        {
            if (size < 9) {
                ret.op = Invalid;
                break;
            }

            int x, y;
            x = decode_int32_little(buf + 1);
            y = decode_int32_little(buf + 5);

            ret.op = DrawPixel;
            ret.param.draw_pixel_param.x = x;
            ret.param.draw_pixel_param.y = y;
            break;
        }
        case ClearScreen:
        {
            ret.op = ClearScreen;
            break;
        }
    }
    return ret;
}

struct Display disp;

#include "lib/queue.h"
Queue drawing_query_queue;

struct communication_arg {
    int com;
};
int communication(struct communication_arg *arg) {
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
    int rcount;
    int wcount;
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
    if (rcount < 0) {
        perror("fgets");
    }

    printf("connection closed.\n");
    fclose(in);
    fclose(out);
    return 0;
}

int drawing() {
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
                    // TODO
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
                    SDL_CALL_NONNGE(SDL_SetRenderDrawColor, disp.ren, 0, 0, 0, 255);
                    SDL_RenderClear(disp.ren);
                    SDL_RenderPresent(disp.ren);
                    break;
                }
                default:
                {
                    printf("invalid query\n");
                    break;
                }
            }
            printf("draw ok\n");
        }
    }

    queue_free(&drawing_query_queue);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s portno\n", argv[0]);
        return 1;
    }

    int exit_status = 0;
    SDL_CALL_NONNGE(SDL_Init, SDL_INIT_VIDEO);

    // struct Display disp;
    if (display_init(&disp, 1280, 960, "SDL2 test") < 0) {
        exit_status = 1;
        goto EXIT_QUIT;
    }

    int com;
    int portno = strtol(argv[1], NULL, 10);
    int acc = tcp_acc_port(portno, 4);
    if (acc < 0) {
        return 1;
    }

    pthread_t drawing_thread;
    if (pthread_create(&drawing_thread, NULL, (void *)drawing, NULL) != 0) {
        perror("pthread_create(): drawing");
        exit(1);
    }

    while (1) {
        printf("[%d] acception incoming connections (acc == %d) ...\n", getpid(), acc);
        if ((com = accept(acc, 0, 0)) < 0) {
            perror("accept");
            return -1;
        }
        struct communication_arg arg;
        arg.com = com;
        pthread_t worker;
        if (pthread_create(&worker, NULL, (void *)communication, (void *)&arg) != 0) {
            perror("pthread_create(): communication");
            exit(1);
        }
        pthread_detach(worker);
    }

    pthread_join(drawing_thread, NULL);
    
    exit_status = 0;
EXIT_RELEASE:
    display_release(&disp);
EXIT_QUIT:
    SDL_Quit();
    return exit_status;
}