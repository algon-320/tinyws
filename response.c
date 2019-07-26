#include <stdio.h>
#include <assert.h>

#include "response.h"

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
    }
}


#define WRITE_INT_LE(value, dst)\
    do {\
        for (size_t i = 0; i < sizeof(value); i++) {\
            dst[i] = (value >> (i * 8)) & ((1 << 8) - 1);\
        }\
        dst += sizeof(value);\
    } while (0)

size_t response_encode(const struct Response *resp, uint8_t *out, size_t size) {
    assert(size > 0);

    uint8_t *nxt = out;
    WRITE_INT_LE(resp->success, nxt);
    WRITE_INT_LE((int32_t)resp->type, nxt);
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



#define READ_INT_LE(src, dst)\
    do {\
        *dst = 0;\
        for (size_t i = 0; i < sizeof(*dst); i++) {\
            *dst |= src[i] << (i * 8);\
        }\
        src += sizeof(*dst);\
    } while (0)

struct Response response_decode(const uint8_t *buf, size_t size) {
    struct Response ret;
    
    READ_INT_LE(buf, &ret.success);

    {
        int32_t tmp_query_type;
        READ_INT_LE(buf, &tmp_query_type);
        ret.type = tmp_query_type;
    }

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
