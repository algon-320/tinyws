#include "event.h"
#include "common.h"

const char *mouse_button_name(enum MouseButton button) {
    switch (button) {
        case TINYWS_MOUSE_LEFT_BUTTON:
            return "Left";
        case TINYWS_MOUSE_RIGHT_BUTTON:
            return "Right";
        default:
            return "invalid";
    }
}

void event_print(const struct Event *event) {
    switch (event->type) {
    case TINYWS_EVENT_MOUSE_DOWN:
        printf("TINYWS_EVENT_MOUSE_DOWN(win=%d, button=%s, x=%d, y=%d, disp_x=%d, disp_y=%d, front_win=%d)\n",
                event->window_id,
                mouse_button_name(event->param.mouse.button),
                event->param.mouse.pos_x,
                event->param.mouse.pos_y,
                event->param.mouse.display_pos_x,
                event->param.mouse.display_pos_y,
                event->param.mouse.front_window_id
                );
        break;
    case TINYWS_EVENT_MOUSE_UP:
        printf("TINYWS_EVENT_MOUSE_UP(win=%d, button=%s, x=%d, y=%d, disp_x=%d, disp_y=%d, front_win=%d)\n",
                event->window_id,
                mouse_button_name(event->param.mouse.button),
                event->param.mouse.pos_x,
                event->param.mouse.pos_y,
                event->param.mouse.display_pos_x,
                event->param.mouse.display_pos_y,
                event->param.mouse.front_window_id
                );
        break;
    case TINYWS_EVENT_MOUSE_MOVE:
        printf("TINYWS_EVENT_MOUSE_MOVE(win=%d, x=%d, y=%d, disp_x=%d, disp_y=%d, front_win=%d)\n",
                event->window_id,
                event->param.mouse.pos_x,
                event->param.mouse.pos_y,
                event->param.mouse.display_pos_x,
                event->param.mouse.display_pos_y,
                event->param.mouse.front_window_id
                );
        break;
    case TINYWS_EVENT_KEY_DOWN:
        // TODO
        break;
    case TINYWS_EVENT_KEY_UP:
        // TODO
        break;
    case TINYWS_WM_EVENT_NOTIFY_CREATE_WINDOW:
        printf("TINYWS_WM_EVENT_NOTIFY_CREATE_WINDOW(win=%d, client_win_id=%d, rect=(x=%d, y=%d, w=%d, h=%d))\n",
                event->window_id,
                event->param.wm_event_create_window.client_window_id,
                event->param.wm_event_create_window.rect.x,
                event->param.wm_event_create_window.rect.y,
                event->param.wm_event_create_window.rect.width,
                event->param.wm_event_create_window.rect.height
                );
        break;
    case TINYWS_EVENT_CLOSE_CHILD_WINDOW:
        printf("TINYWS_EVENT_CLOSE_CHILD_WINDOW(win=%d, child_win_id=%d)\n",
                event->window_id,
                event->param.close_child_window.child_window_id
                );
        break;
    default:
        printf("invalid event type\n");
        break;
    }
}

size_t event_encode(const struct Event *event, uint8_t **out) {
    uint8_t *nxt = *out;
    WRITE_ENUM_LE(event->type, &nxt);
    WRITE_ENUM_LE(event->window_id, &nxt);
    switch (event->type) {
        // mouse event
        case TINYWS_EVENT_MOUSE_DOWN:
        case TINYWS_EVENT_MOUSE_UP:
        case TINYWS_EVENT_MOUSE_MOVE:
        {
            WRITE_ENUM_LE(event->param.mouse.button, &nxt);
            WRITE_INT_LE(event->param.mouse.pos_x, &nxt);
            WRITE_INT_LE(event->param.mouse.pos_y, &nxt);
            WRITE_INT_LE(event->param.mouse.display_pos_x, &nxt);
            WRITE_INT_LE(event->param.mouse.display_pos_y, &nxt);
            WRITE_INT_LE(event->param.mouse.front_window_id, &nxt);
            break;
        }
        case TINYWS_EVENT_KEY_DOWN:
        case TINYWS_EVENT_KEY_UP:
        {
            WRITE_ENUM_LE(event->param.keyboard.keycode, &nxt);
            break;
        }

        case TINYWS_WM_EVENT_NOTIFY_CREATE_WINDOW:
        {
            WRITE_INT_LE(event->param.wm_event_create_window.client_window_id, &nxt);
            rect_encode(&event->param.wm_event_create_window.rect, &nxt);
            break;
        }
        case TINYWS_EVENT_CLOSE_CHILD_WINDOW:
        {
            WRITE_INT_LE(event->param.close_child_window.child_window_id, &nxt);
            break;
        }
    }
    *out = nxt;
    return (nxt - *out);
}

struct Event event_decode(const uint8_t **in) {
    struct Event event;
    READ_ENUM_LE(in, &event.type);
    READ_ENUM_LE(in, &event.window_id);
    switch (event.type) {
        // mouse event
        case TINYWS_EVENT_MOUSE_DOWN:
        case TINYWS_EVENT_MOUSE_UP:
        case TINYWS_EVENT_MOUSE_MOVE:
        {
            READ_ENUM_LE(in, &event.param.mouse.button);
            READ_INT_LE(in, &event.param.mouse.pos_x);
            READ_INT_LE(in, &event.param.mouse.pos_y);
            READ_INT_LE(in, &event.param.mouse.display_pos_x);
            READ_INT_LE(in, &event.param.mouse.display_pos_y);
            READ_INT_LE(in, &event.param.mouse.front_window_id);
            break;
        }
        case TINYWS_EVENT_KEY_DOWN:
        case TINYWS_EVENT_KEY_UP:
        {
            READ_ENUM_LE(in, &event.param.keyboard.keycode);
            break;
        }
        
        case TINYWS_WM_EVENT_NOTIFY_CREATE_WINDOW:
        {
            READ_INT_LE(in, &event.param.wm_event_create_window.client_window_id);
            rect_decode(in, &event.param.wm_event_create_window.rect);
            break;
        }
        case TINYWS_EVENT_CLOSE_CHILD_WINDOW:
        {
            READ_INT_LE(in, &event.param.close_child_window.child_window_id);
            break;
        }
    }
    return event;
}

struct Event event_new_mouse_down(window_id_t window_id, enum MouseButton button, int32_t pos_x, int32_t pos_y, int32_t display_pos_x, int32_t display_pos_y, window_id_t front_window_id) {
    struct Event event;
    event.type = TINYWS_EVENT_MOUSE_DOWN;
    event.window_id = window_id;
    event.param.mouse.button = button;
    event.param.mouse.pos_x = pos_x;
    event.param.mouse.pos_y = pos_y;
    event.param.mouse.display_pos_x = display_pos_x;
    event.param.mouse.display_pos_y = display_pos_y;
    event.param.mouse.front_window_id = front_window_id;
    return event;
}
struct Event event_new_mouse_up(window_id_t window_id, enum MouseButton button, int32_t pos_x, int32_t pos_y, int32_t display_pos_x, int32_t display_pos_y, window_id_t front_window_id) {
    struct Event event;
    event.type = TINYWS_EVENT_MOUSE_UP;
    event.window_id = window_id;
    event.param.mouse.button = button;
    event.param.mouse.pos_x = pos_x;
    event.param.mouse.pos_y = pos_y;
    event.param.mouse.display_pos_x = display_pos_x;
    event.param.mouse.display_pos_y = display_pos_y;
    event.param.mouse.front_window_id = front_window_id;
    return event;
}
struct Event event_new_mouse_move(window_id_t window_id, int32_t pos_x, int32_t pos_y, int32_t display_pos_x, int32_t display_pos_y, window_id_t front_window_id) {
    struct Event event;
    event.type = TINYWS_EVENT_MOUSE_MOVE;
    event.window_id = window_id;
    event.param.mouse.pos_x = pos_x;
    event.param.mouse.pos_y = pos_y;
    event.param.mouse.display_pos_x = display_pos_x;
    event.param.mouse.display_pos_y = display_pos_y;
    event.param.mouse.front_window_id = front_window_id;
    return event;
}
struct Event event_new_key_down(window_id_t window_id, enum KeyCode keycode) {
    struct Event event;
    event.type = TINYWS_EVENT_KEY_DOWN;
    event.window_id = window_id;
    event.param.keyboard.keycode = keycode;
    return event;
}
struct Event event_new_key_up(window_id_t window_id, enum KeyCode keycode) {
    struct Event event;
    event.type = TINYWS_EVENT_KEY_UP;
    event.window_id = window_id;
    event.param.keyboard.keycode = keycode;
    return event;
}
struct Event event_new_close_child_window(window_id_t window_id, window_id_t child_window_id) {
    struct Event event;
    event.type = TINYWS_EVENT_CLOSE_CHILD_WINDOW;
    event.window_id = window_id;
    event.param.close_child_window.child_window_id = child_window_id;
    return event;
}
struct Event event_new_wm_event_notify_create_window(window_id_t window_id, window_id_t client_window_id, Rect rect) {
    assert(false);
    struct Event event;
    event.type = TINYWS_WM_EVENT_NOTIFY_CREATE_WINDOW;
    event.window_id = window_id;
    event.param.wm_event_create_window.client_window_id = client_window_id;
    event.param.wm_event_create_window.rect = rect;
    return event;
}