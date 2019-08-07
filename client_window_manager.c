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
    bool is_frame_window[1024];
    for (int i = 0; i < 1024; i++) {
        frame_window_id_map[i] = -1;
        is_frame_window[i] = false;
    }

    int prev_pos_x = -1;
    int prev_pos_y = -1;
    window_id_t clicked_win_id = -1;
    Rect clicked_win_rect;

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
                            rect.width += 2;
                            rect.height += 20 + 1;

                            req_tmp.type = TINYWS_REQUEST_CREATE_WINDOW;
                            req_tmp.target_window_id = root_id;
                            req_tmp.param.create_window.rect = rect;
                            req_tmp.param.create_window.bg_color = color_new(0x4D, 0x4D, 0x4D, 0);
                            resp_tmp = interact(&req_tmp, in, out);
                            assert(resp_tmp.type == TINYWS_RESPONSE_WINDOW_INFO);
                            assert(resp_tmp.success);
                            frame_window_id = resp_tmp.content.window_info.id;
                        }

                        frame_window_id_map[client_window_id] = frame_window_id;
                        is_frame_window[frame_window_id] = true;

                        {
                            // frame
                            req_tmp.type =TINYWS_REQUEST_DRAW_RECT;
                            req_tmp.target_window_id = frame_window_id;
                            req_tmp.param.draw_rect.rect = rect_new(0, 0, rect.width, rect.height);
                            req_tmp.param.draw_rect.color = color_new(0x4D, 0x4D, 0x4D, 255);
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
                    case TINYWS_EVENT_MOUSE_DOWN:
                    {
                        window_id_t front_window_id = resp.content.event_notify.event.param.mouse.front_window_id;
                        if (front_window_id == root_id) {
                            break;
                        }

                        prev_pos_x = resp.content.event_notify.event.param.mouse.display_pos_x;
                        prev_pos_y = resp.content.event_notify.event.param.mouse.display_pos_y;
                        
                        struct Request req_tmp;
                        struct Response resp_tmp;
                        {
                            // get frame window
                            req_tmp.type = TINYWS_REQUEST_GET_TOPLEVEL_WINDOW;
                            req_tmp.target_window_id = front_window_id;
                            req_tmp.param.get_toplevel_window.root_win_id = root_id;
                            resp_tmp = interact(&req_tmp, in, out);
                            assert(resp_tmp.type == TINYWS_RESPONSE_WINDOW_INFO);
                            assert(resp_tmp.success);
                            clicked_win_id = resp_tmp.content.window_info.id;
                            clicked_win_rect = resp_tmp.content.window_info.rect;
                        }

                        {
                            req_tmp.type = TINYWS_REQUEST_MOVE_WINDOW_TOP;
                            req_tmp.target_window_id = clicked_win_id;
                            resp_tmp = interact(&req_tmp, in, out);
                            assert(resp_tmp.type == TINYWS_RESPONSE_NOCONTENT);
                            assert(resp_tmp.success);
                        }
                        {
                            req_tmp.type = TINYWS_REQUEST_SET_FOCUS;
                            req_tmp.target_window_id = front_window_id;
                            resp_tmp = interact(&req_tmp, in, out);
                            assert(resp_tmp.type == TINYWS_RESPONSE_NOCONTENT);
                            assert(resp_tmp.success);
                        }

                        // not draggable
                        if (!is_frame_window[front_window_id]) {
                            clicked_win_id = -1;
                            prev_pos_x = -1;
                            prev_pos_y = -1;
                        }
                        break;
                    }
                    case TINYWS_EVENT_MOUSE_UP:
                    {
                        if (prev_pos_x != -1) {
                            int cur_pos_x = resp.content.event_notify.event.param.mouse.display_pos_x;
                            int cur_pos_y = resp.content.event_notify.event.param.mouse.display_pos_y;
                            int dx = cur_pos_x - prev_pos_x;
                            int dy = cur_pos_y - prev_pos_y;
                            
                            // move window
                            struct Request req_tmp;
                            struct Response resp_tmp;
                            {
                                req_tmp.type = TINYWS_REQUEST_SET_WINDOW_POS;
                                req_tmp.target_window_id = clicked_win_id;
                                req_tmp.param.set_window_pos.pos = point_new(clicked_win_rect.x + dx, clicked_win_rect.y + dy);
                                resp_tmp = interact(&req_tmp, in, out);
                                assert(resp_tmp.type == TINYWS_RESPONSE_NOCONTENT);
                                assert(resp_tmp.success);
                            }

                            prev_pos_x = cur_pos_x;
                            prev_pos_y = cur_pos_y;
                        }
                        prev_pos_x = -1;
                        prev_pos_y = -1;
                        clicked_win_id = -1;
                        break;
                    }
                    case TINYWS_EVENT_MOUSE_MOVE:
                    {
                        break;
                    }
                }
                break;
            }
        }

        // usleep(5000);
    }

CLOSE_SOCK:
    close(sock);
    return ret_code;
}