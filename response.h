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
    client_id_t dest;
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

#endif