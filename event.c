#include "event.h"
#include "common.h"

size_t event_encode(const struct Event *event, uint8_t *out) {
    uint8_t *nxt = out;
    WRITE_ENUM_LE(event->type, nxt);
    switch (event->type) {
        // mouse event
        case TINYWS_EVENT_MOUSE_DOWN:
        case TINYWS_EVENT_MOUSE_UP:
        case TINYWS_EVENT_MOUSE_MOVE:
        {
            WRITE_ENUM_LE(event->param.mouse.button, nxt);
            WRITE_INT_LE(event->param.mouse.pos_x, nxt);
            WRITE_INT_LE(event->param.mouse.pos_y, nxt);
            break;
        }
    }
    return (nxt - out);
}

struct Event event_decode(const uint8_t *in) {
    struct Event event;
    READ_ENUM_LE(in, &event.type);

    switch (event.type) {
        // mouse event
        case TINYWS_EVENT_MOUSE_DOWN:
        case TINYWS_EVENT_MOUSE_UP:
        case TINYWS_EVENT_MOUSE_MOVE:
        {
            READ_ENUM_LE(in, &event.param.mouse.button);
            READ_INT_LE(in, &event.param.mouse.pos_x);
            READ_INT_LE(in, &event.param.mouse.pos_y);
            break;
        }
    }
    return event;
}