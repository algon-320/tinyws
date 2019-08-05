#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "common.h"
#include "basic_structures.h"
#include "lib/queue.h"
#include "lib/stack.h"
#include "tcp.h"
#include "display.h"
#include "draw.h"
#include "request.h"
#include "response.h"
#include "window.h"
#include "client.h"

struct Display disp;
window_id_t root_win_id;
window_id_t overlay_win_id;

Queue request_queue;  // <struct Request>
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void request_queue_push(struct Request *req) {
    lock_mutex(&mutex);
    {
        queue_push(&request_queue, req);
        pthread_cond_signal(&cond);
    }
    unlock_mutex(&mutex);
    return;
}

bool request_queue_pop(struct Request *req) {
    lock_mutex(&mutex);
    {
        if (queue_empty(&request_queue)) {
            pthread_cond_wait(&cond, &mutex);
        }
        *req = *(struct Request *)queue_front(&request_queue);
        queue_pop(&request_queue);
    }
    unlock_mutex(&mutex);
    return true;
}

int receive_request(uint8_t *line, size_t size, FILE *in) {
    if (fread(line, sizeof(uint8_t), size, in) == 0) {
        return -1;
    }
    return 0;
}

void refresh_screen() {
    struct Request request;
    request.type = TINYWS_REQUEST_REFRESH;
    request.source = -1;
    request.target_window_id = -1;
    request_queue_push(&request);
}

struct interaction_thread_arg {
    int com;
};
int interaction_thread(struct interaction_thread_arg *arg) {
    // print peer address
    {
        struct sockaddr_storage addr;
        socklen_t addr_len; /* MacOSX: __uint32_t */
        addr_len = sizeof(addr);
        if (getpeername(arg->com, (struct sockaddr *)&addr, &addr_len) < 0) {
            perror("tcp_peeraddr_print");
        } else {
            printf("[%d] connection (fd == %d) from ", getpid(), arg->com);
            // sockaddr_print((struct sockaddr *)&addr, addr_len);
            printf("\n");
        }
    }

    // communicating
    const int BUFFSIZE = 1024;
    uint8_t buf[BUFFSIZE * 2], response_buf[BUFFSIZE * 2];
    FILE *in, *out;

    if (fdopen_sock(arg->com, &in, &out) < 0) {
        perror("fdopen");
        exit(1);
    }

    client_id_t cleint_id = client_new();

    while (receive_request(buf, BUFFSIZE, in) >= 0) {
        // printf("receive %d bytes: %s\n", rcount, buf);
        // debugprint("buf: [");
        // for (int i = 0; i < 20; i++) {
        //     debugprint("%d ", (uint8_t)buf[i]);
        // }
        // debugprint("]\n");
        fflush(stdout);

        struct Request request = request_decode(buf, BUFFSIZE);
        request.source = cleint_id;
        // request_print(&request);


        struct Response resp;
        resp.dest = cleint_id;

        switch (request.type) {
            // if it is a window management request, process here
            case TINYWS_REQUEST_CREATE_WINDOW:
            {
                window_id_t parent_id = request.target_window_id;
                if (!window_is_valid(parent_id)) {
                    resp.success = 0;
                    resp.type = TINYWS_RESPONSE_NOCONTENT;
                    break;
                }

                window_id_t win_id = window_new(parent_id, &disp, 
                        cleint_id,
                        -1,
                        request.param.create_window.rect,
                        "test window",
                        request.param.create_window.bg_color
                        );

                resp.success = 1;
                resp.type = TINYWS_RESPONSE_WINDOW_ID;
                resp.content.window_id.id = win_id;

                // top level window only
                if (parent_id == 0) {
                    client_openning_window_push(cleint_id, win_id);
                }
                break;
            }
            case TINYWS_REQUEST_CLOSE_WINDOW:
            {
                window_id_t win_id = request.target_window_id;
                if (!window_is_valid(win_id)) {
                    resp.success = 0;
                    resp.type = TINYWS_RESPONSE_NOCONTENT;
                    break;
                }

                window_close(win_id);
                
                resp.success = 1;
                resp.type = TINYWS_RESPONSE_NOCONTENT;
                break;
            }
            case TINYWS_REQUEST_SET_WINDOW_POS:
            {
                window_id_t win_id = request.target_window_id;
                if (!window_is_valid(win_id)) {
                    resp.success = 0;
                    resp.type = TINYWS_RESPONSE_NOCONTENT;
                    break;
                }
                
                struct Window *win = window_get_own(win_id);
                {
                    window_set_pos(win, request.param.set_window_pos.pos);
                }
                window_return_own(win);

                resp.success = 1;
                resp.type = TINYWS_RESPONSE_NOCONTENT;
                break;
            }
            case TINYWS_REQUEST_SET_WINDOW_VISIBILITY:
            {
                window_id_t win_id = request.target_window_id;
                if (!window_is_valid(win_id)) {
                    resp.success = 0;
                    resp.type = TINYWS_RESPONSE_NOCONTENT;
                    break;
                }
                
                struct Window *win = window_get_own(win_id);
                {
                    window_set_visibility(win, request.param.set_window_visibility.visible ? true : false);
                }
                window_return_own(win);

                resp.success = 1;
                resp.type = TINYWS_RESPONSE_NOCONTENT;
                break;
            }
            case TINYWS_REQUEST_WINDOW_REPARENT:
            {
                window_id_t win_id = request.target_window_id;
                window_id_t par_id = request.param.reparent.parent_window_id;
                if (!window_is_valid(win_id) || !window_is_valid(par_id)) {
                    resp.success = 0;
                    resp.type = TINYWS_RESPONSE_NOCONTENT;
                    break;
                }

                window_reparent(win_id, par_id);

                resp.success = 1;
                resp.type = TINYWS_RESPONSE_NOCONTENT;
                break;
            }
            case TINYWS_REQUEST_SET_FOCUS:
            {
                window_id_t win_id = request.target_window_id;
                if (!window_is_valid(win_id)) {
                    resp.success = 0;
                    resp.type = TINYWS_RESPONSE_NOCONTENT;
                    break;
                }

                window_set_focus(win_id);

                resp.success = 1;
                resp.type = TINYWS_RESPONSE_NOCONTENT;
                break;
            }
            case TINYWS_REQUEST_MOVE_WINDOW_TOP:
            {
                window_id_t win_id = request.target_window_id;
                if (!window_is_valid(win_id)) {
                    resp.success = 0;
                    resp.type = TINYWS_RESPONSE_NOCONTENT;
                    break;
                }

                window_move_top(win_id);

                resp.success = 1;
                resp.type = TINYWS_RESPONSE_NOCONTENT;
                break;
            }

            case TINYWS_REQUEST_APPLY_FOR_WM:
            {
                window_id_t win_id = request.target_window_id;
                if (!window_is_valid(win_id)) {
                    resp.success = 0;
                    resp.type = TINYWS_RESPONSE_NOCONTENT;
                    break;
                }

                struct Window *win = window_get_own(win_id);
                {
                    if (win->window_manager != -1) {
                        resp.success = 0;
                        resp.type = TINYWS_RESPONSE_NOCONTENT;
                        window_return_own(win);
                        break;
                    }
                    window_set_wm(win, cleint_id);
                }
                window_return_own(win);

                resp.success = 1;
                resp.type = TINYWS_RESPONSE_NOCONTENT;
                break;
            }

            case TINYWS_REQUEST_GET_EVENT:
            {
                struct Event event;
                if (!client_event_pop(cleint_id, &event)) {
                    resp.success = 0;
                    resp.type = TINYWS_RESPONSE_NOCONTENT;
                    break;
                }

                resp.success = 1;
                resp.type = TINYWS_RESPONSE_EVENT_NOTIFY;
                resp.content.event_notify.event = event;
                break;
            }

            // otherwise, send to drawing thread
            case TINYWS_REQUEST_DRAW_RECT:
            case TINYWS_REQUEST_DRAW_CIRCLE:
            case TINYWS_REQUEST_DRAW_LINE:
            case TINYWS_REQUEST_DRAW_PIXEL:
            case TINYWS_REQUEST_CLEAR_WINDOW:
            {
                window_id_t win_id = request.target_window_id;
                if (!window_is_valid(win_id)) {
                    resp.success = 0;
                    resp.type = TINYWS_RESPONSE_NOCONTENT;
                    break;
                }
                
                request_queue_push(&request);

                resp.success = 1;
                resp.type = TINYWS_RESPONSE_NOCONTENT;
                break;
            }
            default:
            {
                resp.success = 0;
                resp.type = TINYWS_RESPONSE_NOCONTENT;
                break;
            }
        }

        // 再描画
        refresh_screen();

        size_t bytes = response_encode(&resp, response_buf, BUFFSIZE);
        fwrite(response_buf, sizeof(uint8_t), BUFFSIZE, out);
    }

    client_close(cleint_id);
    refresh_screen();

    printf("connection closed.\n");
    fclose(in);
    fclose(out);
    
    free(arg);
    return 0;
}

struct wait_for_connection_thread_arg {
    int portno;
};
int wait_for_connection_thread(struct wait_for_connection_thread_arg *arg) {
    int com;
    int acc = tcp_acc_port(arg->portno, 4);
    if (acc < 0) {
        return -1;
    }

    while (1) {
        printf("[%d] acception incoming connections (acc == %d) ...\n", getpid(), acc);
        if ((com = accept(acc, 0, 0)) < 0) {
            perror("accept");
            continue;
        }
        struct interaction_thread_arg *arg = malloc(sizeof(struct interaction_thread_arg));
        arg->com = com;
        pthread_t com_thread_id;
        if (pthread_create(&com_thread_id, NULL, (void *)interaction_thread, (void *)arg) != 0) {
            perror("pthread_create(): communication");
            continue;
        }
        pthread_detach(com_thread_id);
    }
    return 0;
}

int event_thread() {
    SDL_Event event;
    while (1) {
        if (SDL_WaitEvent(&event) != 1) {
            fprintf(stderr, "event_thread: SDL_WaitEvent: %s\n", SDL_GetError());
            exit(1);
        }

        window_id_t focused_win_id = window_get_focused();
        struct Window *focused_win = window_get_own(focused_win_id);
        if (focused_win == NULL) {
            continue;
        }

        struct Event tinyws_event;
        tinyws_event.window_id = focused_win->id;

        switch (event.type) {
            case SDL_QUIT:
            {
                exit(0);
                break;
            }
            case SDL_KEYDOWN:
            {
                tinyws_event.type = TINYWS_EVENT_KEY_DOWN;
                switch (event.key.keysym.sym) {
                    case SDLK_UP:
                        tinyws_event.param.keyboard.keycode = TINYWS_KEYCODE_ARROW_UP;
                        break;
                    case SDLK_DOWN:
                        tinyws_event.param.keyboard.keycode = TINYWS_KEYCODE_ARROW_DOWN;
                        break;
                    case SDLK_LEFT:
                        tinyws_event.param.keyboard.keycode = TINYWS_KEYCODE_ARROW_LEFT;
                        break;
                    case SDLK_RIGHT:
                        tinyws_event.param.keyboard.keycode = TINYWS_KEYCODE_ARROW_RIGHT;
                        break;
                    case SDLK_SPACE:
                        tinyws_event.param.keyboard.keycode = TINYWS_KEYCODE_SPACE;
                        break;
                    case SDLK_RETURN:
                        tinyws_event.param.keyboard.keycode = TINYWS_KEYCODE_ENTER;
                        break;
                }
                break;
            }

            case SDL_MOUSEBUTTONDOWN:
            {
                int mouse_x = event.button.x;
                int mouse_y = event.button.y;
                if (window_check_inner_point(focused_win, point_new(mouse_x, mouse_y))) {
                    // inner of focused window
                    tinyws_event.type = TINYWS_EVENT_MOUSE_DOWN;
                    switch (event.button.button) {
                        case SDL_BUTTON_LEFT:
                            tinyws_event.param.mouse.button = TINYWS_MOUSE_LEFT_BUTTON;
                            break;
                        case SDL_BUTTON_RIGHT:
                            tinyws_event.param.mouse.button = TINYWS_MOUSE_RIGHT_BUTTON;
                            break;
                    }
                    tinyws_event.param.mouse.pos_x = event.button.x - focused_win->pos.x;
                    tinyws_event.param.mouse.pos_y = event.button.y - focused_win->pos.y;
                }
                break;
            }
            case SDL_MOUSEBUTTONUP:
            {
                tinyws_event.type = TINYWS_EVENT_MOUSE_UP;
                switch (event.button.button) {
                    case SDL_BUTTON_LEFT:
                        tinyws_event.param.mouse.button = TINYWS_MOUSE_LEFT_BUTTON;
                        break;
                    case SDL_BUTTON_RIGHT:
                        tinyws_event.param.mouse.button = TINYWS_MOUSE_RIGHT_BUTTON;
                        break;
                }
                tinyws_event.param.mouse.pos_x = event.button.x - focused_win->pos.x;
                tinyws_event.param.mouse.pos_y = event.button.y - focused_win->pos.y;
                break;
            }
            case SDL_MOUSEMOTION:
            {
                tinyws_event.type = TINYWS_EVENT_MOUSE_MOVE;
                tinyws_event.param.mouse.pos_x = event.button.x - focused_win->pos.x;
                tinyws_event.param.mouse.pos_y = event.button.y - focused_win->pos.y;
                break;
            }
        }


        if (focused_win->client_id == -1) {
            // server window

        } else {
            // TODO: send notification to (focused_win->parent->window_manager)
            if (client_is_valid(focused_win->client_id)) {
                client_event_push(focused_win->client_id, &tinyws_event);
            }
        }

        window_return_own(focused_win);

        debugprint("event push: id=%d ", focused_win->id);
        event_print(&tinyws_event);
        
        // 再描画
        refresh_screen();
    }
    return 0;
}

static uint32_t mouse_cursor_bitmap[16][16] = {
	{1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{1,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{1,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0},
	{1,2,2,2,1,0,0,0,0,0,0,0,0,0,0,0},
	{1,2,2,2,2,1,0,0,0,0,0,0,0,0,0,0},
	{1,2,2,2,2,2,1,0,0,0,0,0,0,0,0,0},
	{1,2,2,2,2,2,2,1,0,0,0,0,0,0,0,0},
	{1,2,2,2,2,2,2,2,1,0,0,0,0,0,0,0},
	{1,2,2,2,2,2,2,2,2,1,0,0,0,0,0,0},
	{1,2,2,2,2,2,1,1,1,1,1,0,0,0,0,0},
	{1,2,2,1,2,2,1,0,0,0,0,0,0,0,0,0},
	{1,2,1,0,1,2,2,1,0,0,0,0,0,0,0,0},
	{1,1,0,0,1,2,2,1,0,0,0,0,0,0,0,0},
	{1,0,0,0,0,1,2,2,1,0,0,0,0,0,0,0},
	{0,0,0,0,0,1,2,2,1,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0},
};

// window management and drawing
int main_thread() {
    while (1) {
        struct Request request;
        request_queue_pop(&request);

        {
            struct Window *root_win = window_get_own(root_win_id);
            clear_screen(root_win);
            window_return_own(root_win);
        }

        {
            struct Window *overlay_win = window_get_own(overlay_win_id);
            clear_screen(overlay_win);
            window_return_own(overlay_win);
        }

        // mouse cursor
        {
            struct Window *overlay_win = window_get_own(overlay_win_id);
            int mouse_x, mouse_y;
            int pushed = SDL_GetMouseState(&mouse_x, &mouse_y);
            Color c1 = color_new(0, 0, 0, 255);
            Color c2 = color_new(255, 255, 255, 255);
            Color cc = color_new(0, 255, 0, 255);
            if (pushed & SDL_BUTTON(SDL_BUTTON_LEFT)) {
                for (int iy = 0; iy < 16; iy++) {
                    for (int ix = 0; ix < 16; ix++) {
                        if (mouse_cursor_bitmap[iy][ix] == 1) {
                            draw_pixel(overlay_win, mouse_x + ix, mouse_y + iy, c1);
                        } else if (mouse_cursor_bitmap[iy][ix] == 2) {
                            draw_pixel(overlay_win, mouse_x + ix, mouse_y + iy, cc);
                        }
                    }
                }
            } else {
                for (int iy = 0; iy < 16; iy++) {
                    for (int ix = 0; ix < 16; ix++) {
                        if (mouse_cursor_bitmap[iy][ix] == 1) {
                            draw_pixel(overlay_win, mouse_x + ix, mouse_y + iy, c1);
                        } else if (mouse_cursor_bitmap[iy][ix] == 2) {
                            draw_pixel(overlay_win, mouse_x + ix, mouse_y + iy, c2);
                        }
                    }
                }
            }
            disp.curosr_pos = point_new(mouse_x, mouse_y);
            window_return_own(overlay_win);
        }

        // debugprint("drawing --> "); request_print(&request);

        struct Window *target_win = window_get_own(request.target_window_id);
        switch (request.type) {
            case TINYWS_REQUEST_DRAW_RECT:
            {
                assert(target_win);
                int x = request.param.draw_rect.rect.x;
                int y = request.param.draw_rect.rect.y;
                int w = request.param.draw_rect.rect.width;
                int h = request.param.draw_rect.rect.height;
                Color c = request.param.draw_rect.color;
                if (draw_rect(target_win, x, y, w, h, c) < 0) {
                    return -1;
                }
                break;
            }
            case TINYWS_REQUEST_DRAW_CIRCLE:
            {
                assert(target_win);
                int x_center = request.param.draw_circle.center.x;
                int y_center = request.param.draw_circle.center.y;
                int radius = request.param.draw_circle.radius;
                char filled = request.param.draw_circle.filled;
                Color c = request.param.draw_circle.color;
                if (draw_circle(target_win, x_center, y_center, radius, filled, c) < 0) {
                    return -1;
                }
                break;
            }
            case TINYWS_REQUEST_DRAW_LINE:
            {
                assert(target_win);
                int x1 = request.param.draw_line.p1.x;
                int y1 = request.param.draw_line.p1.y;
                int x2 = request.param.draw_line.p2.x;
                int y2 = request.param.draw_line.p2.y;
                Color c = request.param.draw_line.color;
                if (draw_line(target_win, x1, y1, x2, y2, c) < 0) {
                    return -1;
                }
                break;
            }
            case TINYWS_REQUEST_DRAW_PIXEL:
            {
                int x = request.param.draw_pixel.p.x;
                int y = request.param.draw_pixel.p.y;
                Color c = request.param.draw_pixel.color;
                if (draw_pixel(target_win, x, y, c) < 0) {
                    return -1;
                }
                break;
            }
            case TINYWS_REQUEST_CLEAR_WINDOW:
            {
                assert(target_win);
                clear_screen(target_win);
                break;
            }
            case TINYWS_REQUEST_REFRESH:
            {
                break;
            }
            
            default:
            {
                fprintf(stderr, "invalid request\n");
                break;
            }
        }
        window_return_own(target_win);


        SDL_SetRenderDrawColor(disp.ren, 0, 0, 0, 0);
        SDL_SetRenderTarget(disp.ren, NULL);
        SDL_RenderClear(disp.ren);
        
        // draw windows
        window_draw(root_win_id);

        // debugprint("draw ok\n");

        // update screen
        display_flush(&disp);
    }
}

int initialize() {
    SDL_CALL_NONNEG(SDL_Init, SDL_INIT_VIDEO);
    SDL_ShowCursor(SDL_DISABLE);

    const int DISPLAY_WIDTH  = 1280;
    const int DISPLAY_HEIGHT = 960;
    
    if (display_new(&disp, size_new(DISPLAY_WIDTH, DISPLAY_HEIGHT), "tinyws virtual display") < 0) {
        return -1;
    }

    window_subsystem_init();
    client_subsystem_init();

    request_queue = queue_new(sizeof(struct Request));
    
    // root window
    root_win_id = window_new(WINDOW_ID_INVALID, &disp, -1, -1, rect_new(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT), "root", color_new(0x8D, 0x1D, 0x2D, 255));
    debugprint("root_win ok\n");
    // overlay window
    overlay_win_id = window_new(WINDOW_ID_INVALID, &disp, -1, -1, rect_new(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT), "cursor", color_new(0, 0, 0, 0));
    debugprint("overlay_win ok\n");

    struct Window *root_win = window_get_own(root_win_id);
    {
        struct Window *overlay_win = window_ref(overlay_win_id);
        linked_list_insert_next(&root_win->next, &overlay_win->next);
    }
    window_return_own(root_win);

    window_set_focus(root_win_id);

    return 0;
}

int finalize() {
    display_release(&disp);
    queue_free(&request_queue);
    SDL_Quit();
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s portno\n", argv[0]);
        return 1;
    }

    int exit_status = 0;
    if (initialize() < 0) {
        return -1;
    }

    debugprint("initialize finished\n");

    pthread_t event_thread_id;
    if (pthread_create(&event_thread_id, NULL, (void *)event_thread, NULL) != 0) {
        perror("pthread_create(): drawing");
        exit_status = -1;
        goto EXIT_QUIT;
    }

    pthread_t wait_for_connection_thread_id;
    struct wait_for_connection_thread_arg arg;
    arg.portno = strtol(argv[1], NULL, 10);
    if (pthread_create(&wait_for_connection_thread_id, NULL, (void *)wait_for_connection_thread, &arg) != 0) {
        perror("pthread_create(): drawing");
        exit_status = -1;
        goto EXIT_QUIT;
    }

    debugprint("start main thread\n");
    main_thread();

    pthread_join(event_thread_id, NULL);
    pthread_join(wait_for_connection_thread_id, NULL);
    exit_status = 0;

EXIT_QUIT:
    if (finalize() < 0) {
        return -1;
    }
    return exit_status;
}