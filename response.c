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
        case TINYWS_RESPONSE_WINDOW_INFO:
        {
            printf("TINYWS_RESPONSE_WINDOW_INFO(id=%d, rect=(%d,%d,%d,%d))\n",
                    resp->content.window_info.id,
                    resp->content.window_info.rect.x,
                    resp->content.window_info.rect.y,
                    resp->content.window_info.rect.width,
                    resp->content.window_info.rect.height
                    );
            break;
        }
        case TINYWS_RESPONSE_EVENT_NOTIFY:
        {
            printf("TINYES_RESPONSE_EVENT_NOTIFY()\n");
            event_print(&resp->content.event_notify.event);
            break;
        }
    }
}

size_t response_encode(const struct Response *resp, uint8_t *out, size_t size) {
    assert(size > 0);

    uint8_t *nxt = out;
    WRITE_INT_LE(resp->success, &nxt);
    WRITE_ENUM_LE(resp->type, &nxt);
    switch (resp->type) {
        case TINYWS_RESPONSE_NOCONTENT:
        {
            break;
        }
        case TINYWS_RESPONSE_WINDOW_INFO:
        {
            WRITE_INT_LE(resp->content.window_info.id, &nxt);
            rect_encode(&resp->content.window_info.rect, &nxt);
            break;
        }
        case TINYWS_RESPONSE_EVENT_NOTIFY:
        {
            event_encode(&resp->content.event_notify.event, &nxt);
            break;
        }
    }
    return (nxt - out);
}


struct Response response_decode(const uint8_t *buf, size_t size) {
    struct Response ret;
    
    READ_INT_LE(&buf, &ret.success);
    READ_ENUM_LE(&buf, &ret.type);

    switch (ret.type) {
        case TINYWS_RESPONSE_NOCONTENT:
        {
            break;
        }
        case TINYWS_RESPONSE_WINDOW_INFO:
        {
            READ_INT_LE(&buf, &ret.content.window_info.id);
            rect_decode(&buf, &ret.content.window_info.rect);
            break;
        }
        case TINYWS_RESPONSE_EVENT_NOTIFY:
        {
            ret.content.event_notify.event = event_decode(&buf);
            break;
        }
    }
    return ret;
}

struct Response response_new_nocontent(uint8_t success) {
    struct Response resp;
    resp.type = TINYWS_RESPONSE_NOCONTENT;
    resp.success = success;
    return resp;
}
struct Response response_new_window_info(uint8_t success, window_id_t id, Rect rect) {
    struct Response resp;
    resp.type = TINYWS_RESPONSE_WINDOW_INFO;
    resp.success = success;
    resp.content.window_info.id = id;
    resp.content.window_info.rect = rect;
    return resp;
}
struct Response response_new_event_notify(uint8_t success, struct Event event) {
    struct Response resp;
    resp.type = TINYWS_RESPONSE_EVENT_NOTIFY;
    resp.success = success;
    resp.content.event_notify.event = event;
    return resp;
}