#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <assert.h>

#define BUFFERSIZE 1024
#define PORTNO_BUFSIZE 30

int tcp_connect(char *server, int portno) {
    struct addrinfo hints, *ai;
    char portno_str[PORTNO_BUFSIZE];
    int s, err;
    snprintf(portno_str, sizeof(portno_str), "%d", portno);
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    if ((err = getaddrinfo(server, portno_str, &hints, &ai))) {
        fprintf(stderr, "unknown server %s (%s)\n", server, gai_strerror(err));
        goto error0;
    }
    if ((s = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) < 0) {
        perror("socket");
        goto error1;
    }
    if (connect(s, ai->ai_addr, ai->ai_addrlen) < 0) {
        perror(server);
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
            char filled;
        } draw_circle_param;
        struct {
            int x1, y1, x2, y2;
        } draw_line_param;
        struct {
            int x, y;
        } draw_pixel_param;
    } param;
};

#define ENCODE_INT_LITTLE_FUNC(func_name, type)\
    void func_name(type value, unsigned char *out) {\
        for (int i = 0; i < sizeof(type); i++) {\
            out[i] = (unsigned char)(value & (1 << 8) - 1);\
            value >>= 8;\
        }\
    }
ENCODE_INT_LITTLE_FUNC(encode_int32_little, int)
ENCODE_INT_LITTLE_FUNC(encode_int8_little, char)

int encode_query(struct DrawingQuery query, unsigned char *out, size_t size) {
    assert(size > 0);
    out[0] = (unsigned char)query.op;
    switch (query.op) {
        case DrawRect:
        {
            if (size < 17) {
                return -1;
            }

            int param[4] = {
                query.param.draw_rect_param.x,
                query.param.draw_rect_param.y,
                query.param.draw_rect_param.w,
                query.param.draw_rect_param.h
            };
            for (int i = 0; i < 4; i++) {
                encode_int32_little(param[i], out + 1 + i * 4);
            }
            break;
        }
        case DrawCircle:
        {
            if (size < 14) {
                return -1;
            }

            int param[3] = {
                query.param.draw_circle_param.x,
                query.param.draw_circle_param.y,
                query.param.draw_circle_param.radius
            };
            for (int i = 0; i < 3; i++) {
                encode_int32_little(param[i], out + 1 + i * 4);
            }
            fprintf(stderr, "filled = %d\n", query.param.draw_circle_param.filled);
            encode_int8_little(query.param.draw_circle_param.filled, out + 13);
            break;
        }
        case DrawLine:
        {
            if (size < 17) {
                return -1;
            }

            int param[4] = {
                query.param.draw_line_param.x1,
                query.param.draw_line_param.y1,
                query.param.draw_line_param.x2,
                query.param.draw_line_param.y2
            };
            for (int i = 0; i < 4; i++) {
                encode_int32_little(param[i], out + 1 + i * 4);
            }
            break;
        }
        case DrawPixel:
        {
            if (size < 9) {
                return -1;
            }

            int param[2] = {
                query.param.draw_pixel_param.x,
                query.param.draw_pixel_param.y,
            };
            for (int i = 0; i < 2; i++) {
                encode_int32_little(param[i], out + 1 + i * 4);
            }
            break;
        }
        case ClearScreen:
        {
            // no body
            break;
        }
        default:
        {
            assert(0);
            break;
        }
    }
    return 0;
}


int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s portno op\n", argv[0]);
        return 1;
    }

    char server[] = "localhost";
    int portno = strtol(argv[1], NULL, 10);
    char message[1024];
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
    return 0;
}