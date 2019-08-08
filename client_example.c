#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <assert.h>

#include "basic_structures.h"
#include "tcp.h"
#include "request.h"
#include "response.h"

#define BUFFERSIZE 1024

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s portno\n", argv[0]);
        return 1;
    }

    char server[] = "localhost";
    int portno = strtol(argv[1], NULL, 10);
    uint8_t request_buf[BUFFERSIZE * 2], response_buf[BUFFERSIZE * 2];
    memset(request_buf, 0, sizeof(request_buf));

    int ret_code = 0;
    FILE *in, *out;

    int sock = tcp_connect(server, portno);
    if (sock < 0) {
        return 1;
    }
    if (fdopen_sock(sock, &in, &out) < 0) {
        fprintf(stderr, "fdopen()\n");
        ret_code = 1;
        goto CLOSE_SOCK;
    }

    while (1) {
        char command[1024];
        printf("> "); fflush(stdout);
        scanf("%s", command);

        struct Request request;
        
        if (strcmp(command, "?") == 0 || strcmp(command, "help") == 0) {
            printf("command list\n");

            printf("\tcreate: create new window\n");
            printf("\tmove: move specific window\n");
            printf("\tsetvis: show / hide specific window\n");
            printf("\tclose: close specific window\n");
            printf("\treparent: change parent of specific window\n");
            printf("\tfocus: focus specific window\n");
            printf("\tmovetop: move specific window top\n");
            printf("\tdraw_rect: drawing a rectangle\n");
            printf("\tdraw_circle: drawing a cirlce\n");
            printf("\tdraw_line: drawing a line\n");
            printf("\tdraw_pixel: drawing a pixel\n");
            printf("\tclear: clear specific window\n");
            printf("\texit: exit this app\n");
            continue;
        } else if (strcmp(command, "create") == 0) {
            int w, h, x, y, parent, r, g, b;
            printf("? parent = "); fflush(stdout);
            scanf("%d", &parent);
            printf("? w, h, x, y = "); fflush(stdout);
            scanf("%d %d %d %d", &w, &h, &x, &y);
            printf("? r, g, b = "); fflush(stdout);
            scanf("%d %d %d", &r, &g, &b);
            request = requeset_new_create_window(parent, rect_new(x, y, w, h), color_new(r, g, b, 255));
        } else if (strcmp(command, "close") == 0) {
            int win;
            printf("? win_id = "); fflush(stdout);
            scanf("%d", &win);
            request = requeset_new_close_window(win);
        } else if (strcmp(command, "move") == 0) {
            int win, x, y;
            printf("? win_id, x, y = "); fflush(stdout);
            scanf("%d %d %d", &win, &x, &y);
            request = requeset_new_set_window_pos(win, point_new(x, y));
        } else if (strcmp(command, "setvis") == 0) {
            int win;
            printf("? win_id = "); fflush(stdout);
            scanf("%d", &win);
            int vis;
            printf("? vis (show: 1, hide: 0) = "); fflush(stdout);
            scanf("%d", &vis);
            request = requeset_new_set_window_visibility(win, vis);
        } else if (strcmp(command, "reparent") == 0) {
            int win;
            printf("? win_id = "); fflush(stdout);
            scanf("%d", &win);
            int parent;
            printf("? parent = "); fflush(stdout);
            scanf("%d", &parent);
            request = requeset_new_window_reparent(win, parent);
        } else if (strcmp(command, "focus") == 0) {
            int win;
            printf("? win_id = "); fflush(stdout);
            scanf("%d", &win);
            request = requeset_new_set_focus(win);
        } else if (strcmp(command, "movetop") == 0) {
            int win;
            printf("? win_id = "); fflush(stdout);
            scanf("%d", &win);
            request = requeset_new_move_window_top(win);
        } else if (strcmp(command, "draw_rect") == 0) {
            int win, x, y, w, h, filled, r, g, b;
            printf("? win_id = "); fflush(stdout);
            scanf("%d", &win);
            printf("? w h x y filled = "); fflush(stdout);
            scanf("%d %d %d %d %d", &w, &h, &x, &y, &filled);
            printf("? r g b = "); fflush(stdout);
            scanf("%d %d %d", &r, &g, &b);
            request = requeset_new_draw_rect(win, rect_new(x, y, w, h), filled, color_new(r, g, b, 255));
        } else if (strcmp(command, "draw_circle") == 0) {
            int win, x, y, radius, filled, r, g, b;
            printf("? win_id = "); fflush(stdout);
            scanf("%d", &win);
            printf("? x y radius filled = "); fflush(stdout);
            scanf("%d %d %d %d", &x, &y, &radius, &filled);
            printf("? r g b = "); fflush(stdout);
            scanf("%d %d %d", &r, &g, &b);
            request = requeset_new_draw_circle(win, point_new(x, y), radius, filled, color_new(r, g, b, 255));
        } else if (strcmp(command, "draw_line") == 0) {
            int win, x1, y1, x2, y2, r, g, b;
            printf("? win_id = "); fflush(stdout);
            scanf("%d", &win);
            printf("? x1 y1 x2 y2 = "); fflush(stdout);
            scanf("%d %d %d %d", &x1, &y1, &x2, &y2);
            printf("? r g b = "); fflush(stdout);
            scanf("%d %d %d", &r, &g, &b);
            request = requeset_new_draw_line(win, point_new(x1, y1), point_new(x2, y2), color_new(r, g, b, 255));
        } else if (strcmp(command, "draw_pixel") == 0) {
            int win, x, y, r, g, b;
            printf("? win_id = "); fflush(stdout);
            scanf("%d", &win);
            printf("? x y = "); fflush(stdout);
            scanf("%d %d", &x, &y);
            printf("? r g b = "); fflush(stdout);
            scanf("%d %d %d", &r, &g, &b);
            request = requeset_new_draw_pixel(win, point_new(x, y), color_new(r, g, b, 255));
        } else if (strcmp(command, "clear") == 0) {
            int win;
            printf("? win_id = "); fflush(stdout);
            scanf("%d", &win);
            request = requeset_new_clear_window(win);
        } else if (strcmp(command, "exit") == 0) {
            break;
        } else {
            printf("invalid command\n");
            continue;
        }

        request_print(&request);
        size_t bytes = request_encode(&request, request_buf, BUFFERSIZE);
        if (bytes == 0) {
            fprintf(stderr, "buffer too small\n");
            return 1;
        }
        printf("request: [");
        for (size_t i = 0; i < bytes; i++) {
            printf("%d ", (uint8_t)request_buf[i]);
        }
        printf("]\n");

        // send request
        if (fwrite(request_buf, sizeof(uint8_t), BUFFERSIZE, out) == 0) {
            fprintf(stderr, "fwrite()\n");
            ret_code = 1;
            goto CLOSE_FP;
        }
        
        // receive responce
        if (fread(response_buf, sizeof(uint8_t), BUFFERSIZE, in) == 0 && ferror(in)) {
            fprintf(stderr, "fprintf()\n");
            ret_code = 1;
            goto CLOSE_FP;
        }
        struct Response resp = response_decode(response_buf, BUFFERSIZE);
        response_print(&resp);

        printf("response: [");
        for (size_t i = 0; i < bytes; i++) {
            printf("%d ", (uint8_t)response_buf[i]);
        }
        printf("]\n");
    }

CLOSE_FP:
    fclose(in);
    fclose(out);
CLOSE_SOCK:
    close(sock);
    return ret_code;
}