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
        printf("TINYWS_EVENT_MOUSE_DOWN(win=%d, button=%s, x=%d, y=%d)\n",
                event->window_id,
                mouse_button_name(event->param.mouse.button),
                event->param.mouse.pos_x,
                event->param.mouse.pos_y
                );
        break;
    case TINYWS_EVENT_MOUSE_UP:
        printf("TINYWS_EVENT_MOUSE_UP(win=%d, button=%s, x=%d, y=%d)\n",
                event->window_id,
                mouse_button_name(event->param.mouse.button),
                event->param.mouse.pos_x,
                event->param.mouse.pos_y
                );
        break;
    case TINYWS_EVENT_MOUSE_MOVE:
        printf("TINYWS_EVENT_MOUSE_UP(win=%d, x=%d, y=%d)\n",
                event->window_id,
                event->param.mouse.pos_x,
                event->param.mouse.pos_y
                );
        break;
    case TINYWS_EVENT_KEY_DOWN:
        // TODO
        break;
    case TINYWS_EVENT_KEY_UP:
        // TODO
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
            break;
        }
        case TINYWS_EVENT_KEY_DOWN:
        case TINYWS_EVENT_KEY_UP:
        {
            WRITE_ENUM_LE(event->param.keyboard.keycode, &nxt);
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
            break;
        }
        case TINYWS_EVENT_KEY_DOWN:
        case TINYWS_EVENT_KEY_UP:
        {
            READ_ENUM_LE(in, &event.param.keyboard.keycode);
            break;
        }
    }
    return event;
}