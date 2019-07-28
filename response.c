#include <stdio.h>
#include <assert.h>

#include "response.h"
#include "common.h"

void response_print(const struct Response *resp) {
    if (resp->success) {
        printf("response [success], ");
    } else {
        printf("response [error], ");
    }
    switch (resp->type) {
        case TINYWS_RESPONSE_NOCONTENT:
        {
            printf("TINYWS_RESPONSE_NOCONTENT\n");
            break;
        }
        case TINYWS_RESPONSE_WINDOW_ID:
        {
            printf("TINYWS_RESPONSE_WINDOW_ID(id=%d)\n",
                    resp->content.window_id.id
                    );
            break;
        }
        case TINYWS_RESPONSE_EVENT_NOTIFY:
        {
            printf("TINYES_RESPONSE_EVENT_NOTIFY\n");
            break;
        }
    }
}

size_t response_encode(const struct Response *resp, uint8_t *out, size_t size) {
    assert(size > 0);

    uint8_t *nxt = out;
    WRITE_INT_LE(resp->success, nxt);
    WRITE_ENUM_LE(resp->type, nxt);
    switch (resp->type) {
        case TINYWS_RESPONSE_NOCONTENT:
        {
            break;
        }
        case TINYWS_RESPONSE_WINDOW_ID:
        {
            WRITE_INT_LE(resp->content.window_id.id, nxt);
            break;
        }
    }
    return (nxt - out);
}


struct Response response_decode(const uint8_t *buf, size_t size) {
    struct Response ret;
    
    READ_INT_LE(buf, &ret.success);
    READ_ENUM_LE(buf, &ret.type);

    switch (ret.type) {
        case TINYWS_RESPONSE_NOCONTENT:
        {
            break;
        }
        case TINYWS_RESPONSE_WINDOW_ID:
        {
            READ_INT_LE(buf, &ret.content.window_id.id);
            break;
        }
    }
    return ret;
}
