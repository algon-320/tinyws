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
#include "query.h"
#include "response.h"

#define BUFFERSIZE 1024

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s portno\n", argv[0]);
        return 1;
    }

    char server[] = "localhost";
    int portno = strtol(argv[1], NULL, 10);
    uint8_t message[BUFFERSIZE * 2], response_buf[BUFFERSIZE * 2];
    memset(message, 0, sizeof(message));

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

        struct Query query;
        
        if (strcmp(command, "?") == 0 || strcmp(command, "help") == 0) {
            printf("command list\n");

            printf("\tcreate: create new window\n");
            printf("\tmove: move specific window\n");
            printf("\tsetvis: show / hide specific window\n");
            printf("\tclose: close specific window\n");
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

            query.type = TINYWS_QUERY_CREATE_WINDOW;
            query.param.create_window.width = w;
            query.param.create_window.height = h;
            query.param.create_window.pos_x = x;
            query.param.create_window.pos_y = y;
            query.param.create_window.pos_y = y;
            query.param.create_window.parent_window_id = parent;
            query.param.create_window.bg_color = color_new(r, g, b, 255);
        } else if (strcmp(command, "close") == 0) {
            printf("sorry, unimplemented !\n");
        } else if (strcmp(command, "move") == 0) {
            int win, x, y;
            printf("? win_id, x, y = "); fflush(stdout);
            scanf("%d %d %d", &win, &x, &y);

            query.type = TINYWS_QUERY_SET_WINDOW_POS;
            query.target_window_id = win;
            query.param.set_window_pos.pos_x = x;
            query.param.set_window_pos.pos_y = y;
        } else if (strcmp(command, "setvis") == 0) {
            int win;
            printf("? win_id = "); fflush(stdout);
            scanf("%d", &win);
            int vis;
            printf("? vis (show: 1, hide: 0) = "); fflush(stdout);
            scanf("%d", &vis);

            query.type = TINYWS_QUERY_SET_WINDOW_VISIBILITY;
            query.target_window_id = win;
            query.param.set_window_visibility.visible = vis;
        } else if (strcmp(command, "draw_rect") == 0) {
            int win, x, y, w, h, r, g, b;
            printf("? win_id = "); fflush(stdout);
            scanf("%d", &win);
            printf("? w h x y = "); fflush(stdout);
            scanf("%d %d %d %d", &w, &h, &x, &y);
            printf("? r g b = "); fflush(stdout);
            scanf("%d %d %d", &r, &g, &b);

            query.type = TINYWS_QUERY_DRAW_RECT;
            query.target_window_id = win;
            query.param.draw_rect.x = x;
            query.param.draw_rect.y = y;
            query.param.draw_rect.w = w;
            query.param.draw_rect.h = h;
            query.param.draw_rect.color = color_new(r, g, b, 255);
        } else if (strcmp(command, "draw_circle") == 0) {
            int win, x, y, radius, r, g, b;
            printf("? win_id = "); fflush(stdout);
            scanf("%d", &win);
            printf("? x y radius = "); fflush(stdout);
            scanf("%d %d %d", &x, &y, &radius);
            printf("? r g b = "); fflush(stdout);
            scanf("%d %d %d", &r, &g, &b);

            query.type = TINYWS_QUERY_DRAW_CIRCLE;
            query.target_window_id = win;
            query.param.draw_circle.x = x;
            query.param.draw_circle.y = y;
            query.param.draw_circle.radius = radius;
            query.param.draw_circle.color = color_new(r, g, b, 255);
        } else if (strcmp(command, "draw_line") == 0) {
            int win, x1, y1, x2, y2, r, g, b;
            printf("? win_id = "); fflush(stdout);
            scanf("%d", &win);
            printf("? x1 y1 x2 y2 = "); fflush(stdout);
            scanf("%d %d %d %d", &x1, &y1, &x2, &y2);
            printf("? r g b = "); fflush(stdout);
            scanf("%d %d %d", &r, &g, &b);

            query.type = TINYWS_QUERY_DRAW_LINE;
            query.target_window_id = win;
            query.param.draw_line.x1 = x1;
            query.param.draw_line.y1 = y1;
            query.param.draw_line.x2 = x2;
            query.param.draw_line.y2 = y2;
            query.param.draw_line.color = color_new(r, g, b, 255);
        } else if (strcmp(command, "draw_pixel") == 0) {
            int win, x, y, r, g, b;
            printf("? win_id = "); fflush(stdout);
            scanf("%d", &win);
            printf("? x y = "); fflush(stdout);
            scanf("%d %d", &x, &y);
            printf("? r g b = "); fflush(stdout);
            scanf("%d %d %d", &r, &g, &b);

            query.type = TINYWS_QUERY_DRAW_PIXEL;
            query.target_window_id = win;
            query.param.draw_pixel.x = x;
            query.param.draw_pixel.y = y;
            query.param.draw_pixel.color = color_new(r, g, b, 255);
        } else if (strcmp(command, "clear") == 0) {
            int win;
            printf("? win_id = "); fflush(stdout);
            scanf("%d", &win);

            query.type = TINYWS_QUERY_CLEAR_WINDOW;
            query.target_window_id = win;
        } else if (strcmp(command, "exit") == 0) {
            break;
        } else {
            printf("invalid command\n");
            continue;
        }

        size_t bytes = query_encode(&query, message, BUFFERSIZE);
        if (bytes == 0) {
            fprintf(stderr, "buffer too small\n");
            return 1;
        }
        printf("message: [");
        for (size_t i = 0; i < bytes; i++) {
            printf("%d ", (uint8_t)message[i]);
        }
        printf("]\n");

        // send request
        if (fwrite(message, sizeof(uint8_t), BUFFERSIZE, out) == 0) {
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
    }

CLOSE_FP:
    fclose(in);
    fclose(out);
CLOSE_SOCK:
    close(sock);
    return ret_code;
}