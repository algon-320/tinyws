#ifndef INCLUDE_GUARD_EVENT_H
#define INCLUDE_GUARD_EVENT_H

#include <stdint.h>
#include <stddef.h>

#include "window.h"

enum EventType {
    TINYWS_EVENT_MOUSE_DOWN,
    TINYWS_EVENT_MOUSE_UP,
    TINYWS_EVENT_MOUSE_MOVE,
    TINYWS_EVENT_KEY_DOWN,
    TINYWS_EVENT_KEY_UP,
    TINYWS_EVENT_CLOSE_CHILD_WINDOW,
    TINYWS_WM_EVENT_NOTIFY_CREATE_WINDOW,
};

enum MouseButton {
    TINYWS_MOUSE_LEFT_BUTTON,
    TINYWS_MOUSE_RIGHT_BUTTON,
};

enum KeyCode {
    TINYWS_KEYCODE_ARROW_UP,
    TINYWS_KEYCODE_ARROW_DOWN,
    TINYWS_KEYCODE_ARROW_LEFT,
    TINYWS_KEYCODE_ARROW_RIGHT,
    TINYWS_KEYCODE_SPACE,
    TINYWS_KEYCODE_ENTER,
};

struct Event {
    enum EventType type;
    window_id_t window_id;
    union {
        struct Mouse {
            enum MouseButton button;
            int32_t pos_x;
            int32_t pos_y;
            int32_t display_pos_x;
            int32_t display_pos_y;
            window_id_t top_window_id;
        } mouse;
        struct KeyBoard {
            enum KeyCode keycode;
        } keyboard;
        struct WMEventCreateWindow {
            window_id_t client_window_id;
            Rect rect;
        } wm_event_create_window;
        struct CloseChildWindow {
            window_id_t child_window_id;
        } close_child_window;
    } param;
};

void event_print(const struct Event *event);

size_t event_encode(const struct Event *event, uint8_t **out);
struct Event event_decode(const uint8_t **in);

#endif