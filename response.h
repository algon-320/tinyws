#ifndef INCLUDE_GUARD_RESPONSE_H
#define INCLUDE_GUARD_RESPONSE_H

#include <stddef.h>
#include <stdint.h>

#include "common.h"
#include "event.h"

typedef enum {
    TINYWS_RESPONSE_NOCONTENT,
    TINYWS_RESPONSE_WINDOW_INFO,
    TINYWS_RESPONSE_EVENT_NOTIFY,
} ResponseType;

struct Response {
    ResponseType type;
    uint8_t success;
    union {
        struct {
            window_id_t id;
            Rect rect;
        } window_info;
        struct {
            struct Event event;
        } event_notify;
    } content;
};

void response_print(const struct Response *resp);

size_t response_encode(const struct Response *resp, uint8_t *out, size_t size);
struct Response response_decode(const uint8_t *buf, size_t size);

struct Response response_new_nocontent(uint8_t success);
struct Response response_new_window_info(uint8_t success, window_id_t id, Rect rect);
struct Response response_new_event_notify(uint8_t success, struct Event event);

#endif