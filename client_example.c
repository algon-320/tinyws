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

#define BUFFERSIZE 1024

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s portno op\n", argv[0]);
        return 1;
    }

    char server[] = "localhost";
    int portno = strtol(argv[1], NULL, 10);
    unsigned char message[1024];
    memset(message, 0, sizeof(message));

    struct DrawingQuery query;
    if (argv[2][0] == '0') {
        query.op = DrawRect;
        query.param.draw_rect_param.x = 400;
        query.param.draw_rect_param.y = 400;
        query.param.draw_rect_param.w = 100;
        query.param.draw_rect_param.h = 100;
    }
    if (argv[2][0] == '1') {
        query.op = DrawCircle;
        query.param.draw_circle_param.x = 400;
        query.param.draw_circle_param.y = 400;
        query.param.draw_circle_param.radius = 201;
        query.param.draw_circle_param.filled = 0;
    }
    if (argv[2][0] == '2') {
        query.op = DrawLine;
        query.param.draw_line_param.x1 = 800;
        query.param.draw_line_param.y1 = 400;
        query.param.draw_line_param.x2 = 800;
        query.param.draw_line_param.y2 = 800;
    }
    if (argv[2][0] == '3') {
        query.op = DrawPixel;
        query.param.draw_pixel_param.x = 10;
        query.param.draw_pixel_param.y = 10;
    }
    if (argv[2][0] == '4') {
        query.op = ClearScreen;
    }

    if (encode_query(query, message, 1024) < 0) {
        fprintf(stderr, "buffer too small\n");
        return 1;
    }
    printf("message: [");
    for (int i = 0; i < 20; i++) {
        printf("%d ", (unsigned char)message[i]);
    }
    printf("]\n");

    int ret_code = 0;
    FILE *in, *out;
    char rbuf[BUFFERSIZE];
    int res;

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
    res = fwrite(message, sizeof(unsigned char), 1024, out);
    // res = fprintf(out, "%s", message);
    if (res < 0) {
        fprintf(stderr, "fwrite()\n");
        ret_code = 1;
        goto CLOSE_FP;
    }
    
    // receive responce
    if (fgets(rbuf, BUFFERSIZE, in) == NULL) {
        fprintf(stderr, "fprintf()\n");
        ret_code = 1;
        goto CLOSE_FP;
    }
    printf("%s", rbuf);

CLOSE_FP:
    fclose(in);
    fclose(out);
CLOSE_SOCK:
    close(sock);
    return ret_code;
}