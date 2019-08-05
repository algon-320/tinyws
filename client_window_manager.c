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

struct Response interact(const struct Request *request, FILE *in, FILE *out) {
    uint8_t message[BUFFERSIZE];
    size_t bytes = request_encode(request, message, BUFFERSIZE);
    if (bytes == 0) {
        fprintf(stderr, "buffer too small\n");
        exit(1);
    }
    printf("message: [");
    for (size_t i = 0; i < bytes; i++) {
        printf("%d ", (uint8_t)message[i]);
    }
    printf("]\n");

    // send request
    if (fwrite(message, sizeof(uint8_t), BUFFERSIZE, out) == 0) {
        fprintf(stderr, "fwrite()\n");
        exit(1);
    }
    
    // receive responce
    uint8_t response_buf[BUFFERSIZE];
    if (fread(response_buf, sizeof(uint8_t), BUFFERSIZE, in) == 0 && ferror(in)) {
        fprintf(stderr, "fprintf()\n");
        exit(1);
    }

    return response_decode(response_buf, BUFFERSIZE);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s portno\n", argv[0]);
        return 1;
    }

    char server[] = "localhost";
    int portno = strtol(argv[1], NULL, 10);

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

    static const int root_id = 0;

    // apply for window manger
    struct Request request;
    request.type = TINYWS_REQUEST_APPLY_FOR_WM;
    request.target_window_id = root_id;
    struct Response resp = interact(&request, in, out);
    response_print(&resp);
    if (resp.success == 0) {
        fprintf(stderr, "failed to become window manager. other window manager has already existed.\n");
        exit(1);
    }

    window_id_t frame_window_id_map[1024];

    while (1) {
        request.type = TINYWS_REQUEST_GET_EVENT;

        struct Response resp = interact(&request, in, out);
        response_print(&resp);
        switch (resp.type) {
            case TINYWS_RESPONSE_EVENT_NOTIFY:
            {
                struct Event event = resp.content.event_notify.event;
                switch (event.type) {
                    case TINYWS_WM_EVENT_NOTIFY_CREATE_WINDOW:
                    {
                        window_id_t client_window_id = event.param.wm_event_create_window.client_window_id;
                        Rect rect = event.param.wm_event_create_window.rect;

                        struct Request req_tmp;
                        struct Response resp_tmp;

                        // create frame window
                        window_id_t frame_window_id;
                        {
                            rect.x -= 1;
                            rect.y -= 20;
                            rect.width += 2 + 5;
                            rect.height += 20 + 1 + 5;

                            req_tmp.type = TINYWS_REQUEST_CREATE_WINDOW;
                            req_tmp.target_window_id = root_id;
                            req_tmp.param.create_window.rect = rect;
                            req_tmp.param.create_window.bg_color = color_new(0x4D, 0x4D, 0x4D, 0);
                            resp_tmp = interact(&req_tmp, in, out);
                            assert(resp_tmp.type == TINYWS_RESPONSE_WINDOW_ID);
                            assert(resp_tmp.success);
                            frame_window_id = resp_tmp.content.window_id.id;
                        }

                        frame_window_id_map[client_window_id] = frame_window_id;

                        {
                            // shadow
                            rect.width -= 5;
                            rect.height -= 5;
                            req_tmp.type =TINYWS_REQUEST_DRAW_RECT;
                            req_tmp.target_window_id = frame_window_id;
                            req_tmp.param.draw_rect.rect = rect_new(5, 5, rect.width, rect.height);
                            req_tmp.param.draw_rect.color = color_new(0x4D, 0x4D, 0x4D, 30);
                            resp_tmp = interact(&req_tmp, in, out);
                            assert(resp_tmp.type == TINYWS_RESPONSE_NOCONTENT);
                            assert(resp_tmp.success);

                            // frame
                            req_tmp.type =TINYWS_REQUEST_DRAW_RECT;
                            req_tmp.target_window_id = frame_window_id;
                            req_tmp.param.draw_rect.rect = rect_new(0, 0, rect.width, rect.height);
                            req_tmp.param.draw_rect.color = color_new(0x4D, 0x4D, 0x4D, 255);
                            resp_tmp = interact(&req_tmp, in, out);
                            assert(resp_tmp.type == TINYWS_RESPONSE_NOCONTENT);
                            assert(resp_tmp.success);
                        }
                        // left top
                        {
                            req_tmp.type =TINYWS_REQUEST_DRAW_PIXEL;
                            req_tmp.target_window_id = frame_window_id;
                            req_tmp.param.draw_pixel.p = point_new(0, 0);
                            req_tmp.param.draw_pixel.color = color_new(0, 0, 0, 0);
                            resp_tmp = interact(&req_tmp, in, out);
                            assert(resp_tmp.type == TINYWS_RESPONSE_NOCONTENT);
                            assert(resp_tmp.success);
                        }
                        // right top
                        {
                            req_tmp.type =TINYWS_REQUEST_DRAW_PIXEL;
                            req_tmp.target_window_id = frame_window_id;
                            req_tmp.param.draw_pixel.p = point_new(rect.width - 1, 0);
                            req_tmp.param.draw_pixel.color = color_new(0, 0, 0, 0);
                            resp_tmp = interact(&req_tmp, in, out);
                            assert(resp_tmp.type == TINYWS_RESPONSE_NOCONTENT);
                            assert(resp_tmp.success);
                        }
                        // left down
                        {
                            req_tmp.type =TINYWS_REQUEST_DRAW_PIXEL;
                            req_tmp.target_window_id = frame_window_id;
                            req_tmp.param.draw_pixel.p = point_new(0, rect.height - 1);
                            req_tmp.param.draw_pixel.color = color_new(0, 0, 0, 0);
                            resp_tmp = interact(&req_tmp, in, out);
                            assert(resp_tmp.type == TINYWS_RESPONSE_NOCONTENT);
                            assert(resp_tmp.success);
                        }
                        // left down
                        {
                            req_tmp.type =TINYWS_REQUEST_DRAW_PIXEL;
                            req_tmp.target_window_id = frame_window_id;
                            req_tmp.param.draw_pixel.p = point_new(rect.width - 1, rect.height - 1);
                            req_tmp.param.draw_pixel.color = color_new(0, 0, 0, 0);
                            resp_tmp = interact(&req_tmp, in, out);
                            assert(resp_tmp.type == TINYWS_RESPONSE_NOCONTENT);
                            assert(resp_tmp.success);
                        }

                        printf("frame_window_id=%d\n", frame_window_id);


                        // reparent
                        {
                            req_tmp.type = TINYWS_REQUEST_WINDOW_REPARENT;
                            req_tmp.target_window_id = client_window_id;
                            req_tmp.param.reparent.parent_window_id = frame_window_id;
                            resp_tmp = interact(&req_tmp, in, out);
                            assert(resp_tmp.type == TINYWS_RESPONSE_NOCONTENT);
                            assert(resp_tmp.success);
                        }

                        // move
                        {
                            req_tmp.type = TINYWS_REQUEST_SET_WINDOW_POS;
                            req_tmp.target_window_id = client_window_id;
                            req_tmp.param.set_window_pos.pos = point_new(1, 20);
                            resp_tmp = interact(&req_tmp, in, out);
                            assert(resp_tmp.type == TINYWS_RESPONSE_NOCONTENT);
                            assert(resp_tmp.success);
                        }
                        break;
                    }
                    case TINYWS_EVENT_CLOSE_CHILD_WINDOW:
                    {
                        // close frame window
                        window_id_t child_win_id = event.param.close_child_window.child_window_id;
                        window_id_t frame_window_id = frame_window_id_map[child_win_id];

                        struct Request req_tmp;
                        struct Response resp_tmp;

                        {
                            req_tmp.type = TINYWS_REQUEST_CLOSE_WINDOW;
                            req_tmp.target_window_id = frame_window_id;
                            resp_tmp = interact(&req_tmp, in, out);
                            assert(resp_tmp.type == TINYWS_RESPONSE_NOCONTENT);
                            assert(resp_tmp.success);
                        }
                        break;
                    }
                }
                break;
            }
        }

        usleep(50000);
    }

CLOSE_SOCK:
    close(sock);
    return ret_code;
}