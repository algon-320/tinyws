#ifndef INCLUDE_GUARD_EVENT_H
#define INCLUDE_GUARD_EVENT_H

#include <stdint.h>
#include <stddef.h>

enum EventType {
    TINYWS_EVENT_MOUSE_DOWN,
    TINYWS_EVENT_MOUSE_UP,
    TINYWS_EVENT_MOUSE_MOVE,
    TINYWS_EVENT_KEY_DOWN,
    TINYWS_EVENT_KEY_UP,
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
    union {
        struct Mouse {
            enum MouseButton button;
            int32_t pos_x;
            int32_t pos_y;
        } mouse;
        struct KeyBoard {
            enum KeyCode keycode;
        } keyboard;
    } param;
};

void event_print(const struct Event *event);

size_t event_encode(const struct Event *event, uint8_t **out);
struct Event event_decode(const uint8_t **in);

#endif