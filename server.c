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



void echo_send_replay(const char *line, FILE *out) {
    fprintf(out, "%s", line);
}

int echo_receive_request(char *line, size_t size, FILE *in) {
    if (fgets(line, size, in)) {
        return strlen(line);
    } else {
        if (ferror(in)) {
            return -1;
        } else {
            return 0;
        }
    }
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


int receive_request(char *line, size_t size, FILE *in) {
    if (fgets(line, size, in)) {
        return strlen(line);
    } else {
        if (ferror(in)) {
            return -1;
        } else {
            return 0;
        }
    }
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s portno\n", argv[0]);
        return 1;
    }

    int exit_status = 0;
    SDL_CALL_NONNGE(SDL_Init, SDL_INIT_VIDEO);

    struct Display disp;
    if (display_init(&disp, 640, 480, "SDL2 test") < 0) {
        exit_status = 1;
        goto EXIT_QUIT;
    }

    int com;
    int portno = strtol(argv[1], NULL, 10);
    int acc = tcp_acc_port(portno, 4);
    if (acc < 0) {
        return 1;
    }

    while (1) {
        printf("[%d] acception incoming connections (acc == %d) ...\n", getpid(), acc);
        if ((com = accept(acc, 0, 0)) < 0) {
            perror("accept");
            return -1;
        }
        
        // print peer address
        {
            struct sockaddr_storage addr ;
            socklen_t addr_len ; /* MacOSX: __uint32_t */
            addr_len = sizeof( addr );
            if (getpeername(com, (struct sockaddr *)&addr, &addr_len) <0) {
                perror("tcp_peeraddr_print");
            } else {
                printf("[%d] connection (fd == %d) from ", getpid(), com);
                // sockaddr_print((struct sockaddr *)&addr, addr_len);
                printf("\n");
            }
        }

        // communicating
        {
            char line[1024];
            int rcount;
            int wcount;
            FILE *in, *out;

            if (fdopen_sock(com, &in, &out) < 0) {
                perror("fdopen");
                exit(1);
            }

            while ((rcount = receive_request(line, 1024, in))) {
                printf("receive %d bytes: %s\n", rcount, line);
                fflush(stdout);

                if (strncmp(line, "green", 5) == 0) {
                    if (draw_rect(&disp, 100, 100, 200, 200, make_color(0, 255, 128)) < 0) {
                        exit_status = 1;
                        goto EXIT_RELEASE;
                    }
                }
                if (strncmp(line, "red", 3) == 0) {
                    if (draw_rect(&disp, 100, 100, 200, 200, make_color(255, 0, 0)) < 0) {
                        exit_status = 1;
                        goto EXIT_RELEASE;
                    }
                }

                // send_reply
                fprintf(out, "sent from server: %s", line);
            }
            if (rcount < 0) {
                perror("fgets");
            }

            printf("connection closed.\n");
            fclose(in);
            fclose(out);
        }
    }

    exit_status = 0;
EXIT_RELEASE:
    display_release(&disp);
EXIT_QUIT:
    SDL_Quit();
    return exit_status;
}