#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <assert.h>

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
    if (argv[2][0] == '0') {
        query.type = TINYWS_QUERY_DRAW_RECT;
        query.param.draw_rect.x = 200;
        query.param.draw_rect.y = 200;
        query.param.draw_rect.w = 50;
        query.param.draw_rect.h = 50;
    }
    if (argv[2][0] == '1') {
        query.type = TINYWS_QUERY_DRAW_CIRCLE;
        query.param.draw_circle.x = 200;
        query.param.draw_circle.y = 200;
        query.param.draw_circle.radius = 100;
        query.param.draw_circle.filled = 0;
    }
    if (argv[2][0] == '2') {
        query.type = TINYWS_QUERY_DRAW_LINE;
        query.param.draw_line.x1 = 400;
        query.param.draw_line.y1 = 200;
        query.param.draw_line.x2 = 250;
        query.param.draw_line.y2 = 400;
    }
    if (argv[2][0] == '3') {
        query.type = TINYWS_QUERY_DRAW_PIXEL;
        query.param.draw_pixel.x = 10;
        query.param.draw_pixel.y = 10;
    }
    if (argv[2][0] == '4') {
        query.type = TINYWS_QUERY_CLEAR_SCREEN;
    }
    if (argv[2][0] == '5') {
        query.type = TINYWS_QUERY_CREATE_WINDOW;
        query.param.create_window.parent_window_id = 0;
        query.param.create_window.width = 640;
        query.param.create_window.height = 480;
        query.param.create_window.pos_x = 100;
        query.param.create_window.pos_y = 100;
    }
    if (argv[2][0] == '6') {
        query.type = TINYWS_QUERY_SET_WINDOW_POS;
        query.param.set_window_pos.window_id = 0;
        query.param.set_window_pos.pos_x = 123;
        query.param.set_window_pos.pos_y = 456;
    }
    if (argv[2][0] == '7') {
        query.type = TINYWS_QUERY_SET_WINDOW_VISIBILITY;
        query.param.set_window_visibility.window_id = 0;
        query.param.set_window_visibility.visible = 1;
    }

    size_t bytes = encode_query(&query, message, BUFFERSIZE);
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
    struct Response resp = decode_response(response_buf, BUFFERSIZE);
    print_response(&resp);

    // memset(message, 0, sizeof(message));
    // query.type = TINYWS_QUERY_DRAW_RECT;
    // query.param.draw_rect.x = 700;
    // query.param.draw_rect.y = 700;
    // query.param.draw_rect.w = 100;
    // query.param.draw_rect.h = 100;
    // if (encode_query(query, message, 1024) < 0) {
    //     fprintf(stderr, "buffer too small\n");
    //     TINYWS_QUERY_DRAW_CIRCLE 1;
    // }
    // printf("message: [");
    // for (int i = 0; i < 20; i++) {
    //     printf("%d ", (uint8_t)message[i]);
    // }
    // printf("]\n");

    // // send request
    // res = fwrite(message, sizeof(uint8_t), 1024, out);
    // // res = fprintf(out, "%s", message);
    // if (res < 0) {
    //     fprintf(stderr, "fwrite()\n");
    //     ret_code = 1;
    //     goto CLOSE_FP;
    // }
    
    // // receive responce
    // if (fgets(rbuf, BUFFERSIZE, in) == NULL) {
    //     fprintf(stderr, "fprintf()\n");
    //     ret_code = 1;
    //     goto CLOSE_FP;
    // }
    // printf("%s", rbuf);

CLOSE_FP:
    fclose(in);
    fclose(out);
CLOSE_SOCK:
    close(sock);
    return ret_code;
}