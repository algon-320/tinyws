#ifndef INCLUDE_GUARD_EVENT_H
#define INCLUDE_GUARD_EVENT_H

#include <stdint.h>

enum EventType {
    TINYWS_EVENT_MOUSE_DOWN,
    TINYWS_EVENT_MOUSE_UP,
    TINYWS_EVENT_MOUSE_MOVE,
};

enum MouseButton {
    TINYWS_MOUSE_LEFT_BUTTON,
    TINYWS_MOUSE_RIGHT_BUTTON,
};

struct Event {
    enum EventType type;
    union {
        struct Mouse {
            enum MouseButton button;
            int32_t pos_x;
            int32_t pos_y;
        } mouse;
    } param;
};

void event_encode(const struct Event *event, uint8_t *out);
struct Event event_decode(uint8_t *in);

#endif