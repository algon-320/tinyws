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
    if (argc < 3) {
        fprintf(stderr, "Usage: %s portno op\n", argv[0]);
        return 1;
    }

    char server[] = "localhost";
    int portno = strtol(argv[1], NULL, 10);
    uint8_t message[BUFFERSIZE * 2], response_buf[BUFFERSIZE * 2];
    memset(message, 0, sizeof(message));

    struct Query query;
    query.target_window_id = 2;
    if (argv[2][0] == '0') {
        query.type = TINYWS_QUERY_DRAW_RECT;
        query.param.draw_rect.x = 200;
        query.param.draw_rect.y = 200;
        query.param.draw_rect.w = 50;
        query.param.draw_rect.h = 50;
        query.param.draw_rect.color = color_new(255, 0, 0, 255);
    }
    if (argv[2][0] == '1') {
        query.type = TINYWS_QUERY_DRAW_CIRCLE;
        query.param.draw_circle.x = 200;
        query.param.draw_circle.y = 200;
        query.param.draw_circle.radius = 100;
        query.param.draw_circle.filled = 0;
        query.param.draw_circle.color = color_new(255, 255, 0, 255);
    }
    if (argv[2][0] == '2') {
        query.type = TINYWS_QUERY_DRAW_LINE;
        query.param.draw_line.x1 = 400;
        query.param.draw_line.y1 = 200;
        query.param.draw_line.x2 = 250;
        query.param.draw_line.y2 = 400;
        query.param.draw_line.color = color_new(255, 0, 255, 255);
    }
    if (argv[2][0] == '3') {
        query.type = TINYWS_QUERY_DRAW_PIXEL;
        query.param.draw_pixel.x = 10;
        query.param.draw_pixel.y = 10;
        query.param.draw_pixel.color = color_new(0, 255, 255, 255);
    }
    if (argv[2][0] == '4') {
        query.type = TINYWS_QUERY_CLEAR_WINDOW;
    }
    if (argv[2][0] == '5') {
        query.type = TINYWS_QUERY_CREATE_WINDOW;
        query.param.create_window.parent_window_id = 0;
        query.param.create_window.width = 640;
        query.param.create_window.height = 480;
        query.param.create_window.pos_x = 100;
        query.param.create_window.pos_y = 100;
        query.param.create_window.bg_color = color_new(0x1D, 0x1D, 0x1D, 255);
    }
    if (argv[2][0] == '6') {
        query.type = TINYWS_QUERY_CREATE_WINDOW;
        query.param.create_window.parent_window_id = 2;
        query.param.create_window.width = 50;
        query.param.create_window.height = 50;
        query.param.create_window.pos_x = 10;
        query.param.create_window.pos_y = 10;
        query.param.create_window.bg_color = color_new(0xFD, 0xFD, 0xFD, 255);
    }
    if (argv[2][0] == '7') {
        query.type = TINYWS_QUERY_SET_WINDOW_VISIBILITY;
        query.param.set_window_visibility.window_id = 0;
        query.param.set_window_visibility.visible = 1;
    }

    size_t bytes = query_encode(&query, message, BUFFERSIZE);
    if (bytes == 0) {
        fprintf(stderr, "buffer too small\n");
        return 1;
    }
    printf("message: [");
    for (int i = 0; i < bytes; i++) {
        printf("%d ", (uint8_t)message[i]);
    }
    printf("]\n");

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

CLOSE_FP:
    fclose(in);
    fclose(out);
CLOSE_SOCK:
    close(sock);
    return ret_code;
}