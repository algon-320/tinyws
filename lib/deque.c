#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "deque.h"
#include "utils.h"

Deque deque_new(size_t num, size_t elem_size_byte) {
    return deque_new_with_capacity(num, elem_size_byte, num * 2);
}

Deque deque_new_filled_with(size_t num, size_t elem_size_byte, void *elem) {
    Deque ret = deque_new(num, elem_size_byte);
    for (int i = 0; i < num; ++i) {
        memcpy(deque_at(&ret, i), elem, elem_size_byte);
    }
    return ret;
}

Deque deque_new_with_capacity(size_t num, size_t elem_size_byte, size_t capacity) {
    assert(capacity > 0);
    Deque deq;
    deq.data = calloc(capacity, elem_size_byte);
    deq.elem_size_byte = elem_size_byte;
    deq.capacity = capacity;
    deq.front_idx = 0;
    deq.end_idx = num;
    return deq;
}

void deque_free(Deque *deq) {
    free(deq->data);
    deq->data = NULL;
    deq->capacity = 0;
    deq->front_idx = 0;
    deq->end_idx = 0;
}

void *deque_at(Deque *deq, size_t idx) {
    if (deq->front_idx + idx >= deq->capacity) {
        return deq->data + ((deq->front_idx + idx) - deq->capacity) * (deq->elem_size_byte);
    } else {
        return deq->data + (deq->front_idx + idx) * (deq->elem_size_byte);
    }
}

void *deque_front(Deque *deq) {
    if (deque_size(deq) == 0) {
        return NULL;
    }
    return deque_at(deq, 0);
}
void *deque_back(Deque *deq) {
    if (deque_size(deq) == 0) {
        return NULL;
    }
    return deque_at(deq, deque_size(deq) - 1);
}

size_t deque_size(Deque *deq) {
    if (deq->front_idx <= deq->end_idx) {
        return deq->end_idx - deq->front_idx;
    } else {
        return (deq->capacity - deq->front_idx) + deq->end_idx;
    }
}

void move_to_other(Deque *deq, void *dest) {
    if (deq->front_idx > deq->end_idx) {
        memcpy(
            dest,
            deque_at(deq, 0),
            deq->elem_size_byte * (deq->capacity - deq->front_idx)
        );
        memcpy(
            dest + deq->elem_size_byte * (deq->capacity - deq->front_idx),
            deq->data,
            deq->elem_size_byte * deq->end_idx
        );
    } else {
        memcpy(
            dest,
            deque_at(deq, 0),
            deq->elem_size_byte * (deq->end_idx - deq->front_idx)
        );
    }
    free(deq->data);
    deq->data = dest;
    deq->front_idx = 0;
    deq->end_idx = deque_size(deq);
}

void deque_push_back(Deque *deq, void *elem) {
    if (deq->capacity - 1 == deque_size(deq)) {
        assert((deq->end_idx + 1) % deq->capacity == deq->front_idx);

        void *new_ptr = calloc(deq->capacity * 2, deq->elem_size_byte);
        move_to_other(deq, new_ptr);
        deq->capacity = deq->capacity * 2;
    }

    memcpy(deque_at(deq, deque_size(deq)), elem, deq->elem_size_byte);
    if (deq->end_idx == deq->capacity - 1) {
        deq->end_idx = 0;
    } else {
        deq->end_idx += 1;
    }
}

void deque_pop_back(Deque *deq) {
    assert(deque_size(deq) > 0);
    if (deq->end_idx == 0) {
        deq->end_idx = deq->capacity - 1;
    } else {
        deq->end_idx -= 1;
    }
    memset(deque_at(deq, deque_size(deq)), 0, deq->elem_size_byte);
}

void deque_push_front(Deque *deq, void *elem) {
    if (deq->capacity - 1 == deque_size(deq)) {
        assert((deq->end_idx + 1) % deq->capacity == deq->front_idx);

        void *new_ptr = calloc(deq->capacity * 2, deq->elem_size_byte);
        move_to_other(deq, new_ptr);
        deq->capacity = deq->capacity * 2;
    }

    if (deq->front_idx == 0) {
        deq->front_idx = deq->capacity - 1;
    } else {
        deq->front_idx -= 1;
    }
    memcpy(deque_at(deq, 0), elem, deq->elem_size_byte);
}

void deque_pop_front(Deque *deq) {
    assert(deque_size(deq) > 0);
    memset(deque_at(deq, 0), 0, deq->elem_size_byte);
    if (deq->front_idx == deq->capacity - 1) {
        deq->front_idx = 0;
    } else {
        deq->front_idx += 1;
    }
}