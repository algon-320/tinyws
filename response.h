#ifndef INCLUDE_GUARD_RESPONSE_H
#define INCLUDE_GUARD_RESPONSE_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
    TINYWS_RESPONSE_NOCONTENT,
    TINYWS_RESPONSE_WINDOW_ID,
} ResponseType;

struct Response {
    uint8_t success;
    ResponseType type;
    union {
        struct {
            int32_t id;
        } window_id;
    } content;
};

void print_response(const struct Response *resp);

size_t encode_response(const struct Response *resp, uint8_t *out, size_t size);
struct Response decode_response(const uint8_t *buf, size_t size);

#endif